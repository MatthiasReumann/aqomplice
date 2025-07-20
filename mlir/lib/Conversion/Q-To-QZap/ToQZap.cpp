#include "Conversion/Q-To-QZap/ToQZap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
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
/// @brief TODO
struct LoweringState {
  /// @brief Maps q::qreq to qzap::qreq.
  std::unordered_map<mlir::Value, mlir::Value> qregs;

  /// @brief Maps q::qubit to qzap::qubit and its respective index in a qreg.
  std::unordered_map<mlir::Value, std::pair<mlir::Value, mlir::Value>> qubits;
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

struct AllocOpLowering : mlir::OpConversionPattern<q::AllocOp> {
  using OpConversionPattern<q::AllocOp>::OpConversionPattern;

  AllocOpLowering(mlir::TypeConverter &typeConverter,
                  mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<q::AllocOp>(typeConverter, context), state(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::AllocOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    if (op->getResultTypes().size() > 1) {
      return rewriter.notifyMatchFailure(op, [](mlir::Diagnostic &diag) {
        diag << "expected 'allocOp' to have exactly one result type";
      });
    }

    mlir::Type qreg = typeConverter->convertType(op.getQreg().getType());
    qzap::AllocOp alloc = rewriter.replaceOpWithNewOp<qzap::AllocOp>(
        op, qreg, adaptor.getNqubits());

    state->qregs[op.getQreg()] = alloc.getQreg();

    return mlir::success();
  }

private:
  LoweringState *state;
};

struct FreeOpLowering : mlir::OpConversionPattern<q::FreeOp> {
  using OpConversionPattern<q::FreeOp>::OpConversionPattern;

  FreeOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
                 LoweringState *state)
      : OpConversionPattern<q::FreeOp>(typeConverter, context), state(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::FreeOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Value qreg_in = state->qregs[op.getQreg()];
    mlir::Type qreg_out = typeConverter->convertType(op.getQreg().getType());

    // Store all qubits back to the register with `qzap::StoreOp`.
    for (auto &it : state->qubits) {
      auto &[qubit, index] = it.second;

      qzap::StoreOp store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qreg_out, qreg_in, qubit, index);
      qreg_in = store.getQregOut();
    }

    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, qreg_in);
    return mlir::success();
  }

private:
  LoweringState *state;
};

struct RetrieveOpLowering : mlir::OpConversionPattern<q::RetrieveOp> {
  using OpConversionPattern<q::RetrieveOp>::OpConversionPattern;

  RetrieveOpLowering(mlir::TypeConverter &typeConverter,
                     mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<q::RetrieveOp>(typeConverter, context),
        state(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::RetrieveOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type qreg_out =
        typeConverter->convertType(adaptor.getQreg().getType());
    mlir::Type qubit = typeConverter->convertType(op.getQubit().getType());
    mlir::Value qreg_in = state->qregs[op.getQreg()];

    qzap::RetrieveOp retrieve = rewriter.replaceOpWithNewOp<qzap::RetrieveOp>(
        op, qreg_out, qubit, qreg_in, adaptor.getIndex());

    state->qregs[op.getQreg()] = retrieve.getQreqOut();
    state->qubits[op.getQubit()] = {retrieve.getQubit(), retrieve.getIndex()};

    return mlir::success();
  }

private:
  LoweringState *state;
};

struct QToQZap : impl::QToQZapBase<QToQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    LoweringState state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect, mlir::func::FuncDialect>();

    QToQZapTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<KernelOpLowering, ReturnOpLowering, CallOpLowering>(typeConverter,
                                                                 context)
        .add<AllocOpLowering, FreeOpLowering, RetrieveOpLowering>(
            typeConverter, context, &state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace q
}; // namespace aqomplice