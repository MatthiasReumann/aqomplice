#include "Conversion/Q-To-QZap/ToQZap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <unordered_map>
#include <utility>

namespace std {
template <> struct hash<mlir::Value> {
  size_t operator()(mlir::Value const &val) const noexcept {
    return hash_value(val);
  }
};
} // namespace std

namespace aqomplice {
namespace q {
#define GEN_PASS_DEF_QTOQZAP
#include "Conversion/Q-To-QZap/QToQZap.h.inc" // adds `impl::QToQZapBase`

namespace {
/// @brief Internal state for lowering quantum ops.
struct LoweringState {
  /// @brief Maps q::qreq to qzap::qreq.
  std::unordered_map<mlir::Value, mlir::Value> qregs;

  /// @brief Maps q::qubit to qzap::qubit and its respective index in a qreg.
  std::unordered_map<mlir::Value, std::pair<mlir::Value, mlir::Value>> qubits;

  void clear() {
    qregs.clear();
    qubits.clear();
  }
};

struct LoweringWithState {
  LoweringWithState(LoweringState *state) : state(state) {}

  /// @brief Return reference to internal state.
  LoweringState &getState() const {
    assert(state != nullptr);
    return *state;
  }

  LoweringState *state;
};

} // namespace

struct QToQZapTypeConverter : mlir::TypeConverter {
  QToQZapTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
    addConversion(
        [ctx](q::QubitType type) { return qzap::QubitType::get(ctx); });
    addConversion([ctx](q::QubitArrayType type) {
      return qzap::QubitArrayType::get(ctx);
    });
  }
};

struct KernelOpLowering : mlir::OpConversionPattern<q::KernelOp> {
  using OpConversionPattern<q::KernelOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::KernelOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto kernel = rewriter.create<qzap::KernelOp>(op.getLoc(), op.getSymName(),
                                                  op.getFunctionType());
    kernel.eraseBody();
    rewriter.inlineRegionBefore(op.getRegion(), kernel.getBody(), kernel.end());
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct ReturnOpLowering : mlir::OpConversionPattern<q::ReturnOp> {
  using OpConversionPattern<q::ReturnOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::ReturnOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qzap::ReturnOp>(
        op, op->getResultTypes(), op->getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct CallOpLowering : mlir::OpConversionPattern<q::CallOp> {
  using OpConversionPattern<q::CallOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::CallOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qzap::CallOp>(op, op->getResultTypes(),
                                              op.getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct AllocOpLowering : mlir::OpConversionPattern<q::AllocOp>,
                         LoweringWithState {
  using OpConversionPattern<q::AllocOp>::OpConversionPattern;

  AllocOpLowering(mlir::TypeConverter &typeConverter,
                  mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<q::AllocOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::AllocOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type qreg = typeConverter->convertType(op.getQreg().getType());
    qzap::AllocOp alloc = rewriter.replaceOpWithNewOp<qzap::AllocOp>(
        op, qreg, adaptor.getNqubits());

    getState().qregs[op.getQreg()] = alloc.getQreg();

    return mlir::success();
  }
};

struct FreeOpLowering : mlir::OpConversionPattern<q::FreeOp>,
                        LoweringWithState {
  using OpConversionPattern<q::FreeOp>::OpConversionPattern;

  FreeOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
                 LoweringState *state)
      : OpConversionPattern<q::FreeOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::FreeOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Value qreg_in = getState().qregs[op.getQreg()];
    mlir::Type qreg_out = typeConverter->convertType(op.getQreg().getType());

    // Store all qubits back to the register with `qzap::StoreOp`.
    for (auto &it : getState().qubits) {
      auto &[qubit, index] = it.second;

      qzap::StoreOp store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qreg_out, qreg_in, index, qubit);
      qreg_in = store.getQregOut();
    }

    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, qreg_in);

    getState().clear(); // Assumption: Manage one register at a time.

    return mlir::success();
  }
};

struct RetrieveOpLowering : mlir::OpConversionPattern<q::RetrieveOp>,
                            LoweringWithState {
  using OpConversionPattern<q::RetrieveOp>::OpConversionPattern;

  RetrieveOpLowering(mlir::TypeConverter &typeConverter,
                     mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<q::RetrieveOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::RetrieveOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type qubit = typeConverter->convertType(op.getQubit().getType());
    mlir::Value qreg_in = getState().qregs[op.getQreg()];
    mlir::Type qreg_out = qreg_in.getType();

    qzap::RetrieveOp retrieve = rewriter.replaceOpWithNewOp<qzap::RetrieveOp>(
        op, qreg_out, qubit, qreg_in, adaptor.getIndex());

    getState().qregs[op.getQreg()] = retrieve.getQreqOut();
    getState().qubits[op.getQubit()] = {retrieve.getQubit(),
                                        retrieve.getIndex()};

    return mlir::success();
  }
};

namespace {
template <typename SourceOp, class DestOp>
struct UnitaryOpLowering : LoweringWithState {
  UnitaryOpLowering(LoweringState *state) : LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewriteImpl(SourceOp op, typename SourceOp::Adaptor adaptor,
                      mlir::ConversionPatternRewriter &rewriter) const {
    mlir::Value tgt = op.getTarget();
    mlir::Value target_in = getState().qubits[tgt].first;
    mlir::Type target_out = target_in.getType();

    mlir::Value ctrl = op.getControl();
    if (!ctrl) {
      auto h = rewriter.create<DestOp>(op->getLoc(), target_out, target_in);
      getState().qubits[tgt].first = h.getTargetOut();
      rewriter.eraseOp(op);
      return mlir::success();
    }

    mlir::Value control_in = getState().qubits[ctrl].first;
    mlir::Type control_out = control_in.getType();

    auto u = rewriter.create<DestOp>(op->getLoc(), target_out, control_out,
                                     target_in, control_in);
    getState().qubits[tgt].first = u.getTargetOut();
    getState().qubits[ctrl].first = u.getControlOut();
    rewriter.eraseOp(op);

    return mlir::success();
  }
};
} // namespace

struct HOpLowering : mlir::OpConversionPattern<q::HOp>,
                     UnitaryOpLowering<q::HOp, qzap::HOp> {
  using OpConversionPattern<q::HOp>::OpConversionPattern;

  HOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<q::HOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::HOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct XOpLowering : mlir::OpConversionPattern<q::XOp>,
                     UnitaryOpLowering<q::XOp, qzap::XOp> {
  using OpConversionPattern<q::XOp>::OpConversionPattern;

  XOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<q::XOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::XOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct YOpLowering : mlir::OpConversionPattern<q::YOp>,
                     UnitaryOpLowering<q::YOp, qzap::YOp> {
  using OpConversionPattern<q::YOp>::OpConversionPattern;

  YOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<q::YOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::YOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct ZOpLowering : mlir::OpConversionPattern<q::ZOp>,
                     UnitaryOpLowering<q::ZOp, qzap::ZOp> {
  using OpConversionPattern<q::ZOp>::OpConversionPattern;

  ZOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<q::ZOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::ZOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct QToQZap : impl::QToQZapBase<QToQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    LoweringState state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect>();

    QToQZapTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<KernelOpLowering, ReturnOpLowering, CallOpLowering>(typeConverter,
                                                                 context)
        .add<AllocOpLowering, FreeOpLowering, RetrieveOpLowering, HOpLowering,
             XOpLowering, YOpLowering, ZOpLowering>(typeConverter, context,
                                                    &state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace q
}; // namespace aqomplice