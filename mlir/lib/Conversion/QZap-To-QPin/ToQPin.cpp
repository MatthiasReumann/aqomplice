#include "Conversion/QZap-To-QPin/ToQPin.h"

#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>

namespace std {
template <> struct hash<mlir::Value> {
  size_t operator()(mlir::Value const &val) const noexcept {
    return hash_value(val);
  }
};
} // namespace std

namespace aqomplice {
namespace qzap {
#define GEN_PASS_DEF_QZAPTOQPIN
#include "Conversion/QZap-To-QPin/QZapToQPin.h.inc" // adds `impl::QZapToQPinBase`

namespace {
/// @brief Internal state for lowering quantum ops.
struct LoweringState {
  /// @brief Maps qzap::qubit to index i32 value.
  std::unordered_map<mlir::Value, mlir::Value> qubits;
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

struct QZapToQPinTypeConverter : mlir::TypeConverter {
  QZapToQPinTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
  }
};

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

struct KernelOpLowering : mlir::OpConversionPattern<qzap::KernelOp> {
  using OpConversionPattern<qzap::KernelOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qzap::KernelOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto kernel = rewriter.create<qpin::KernelOp>(op.getLoc(), op.getSymName(),
                                                  op.getFunctionType());
    kernel.eraseBody();
    rewriter.inlineRegionBefore(op.getRegion(), kernel.getBody(), kernel.end());
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct ReturnOpLowering : mlir::OpConversionPattern<qzap::ReturnOp> {
  using OpConversionPattern<qzap::ReturnOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qzap::ReturnOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qpin::ReturnOp>(
        op, op->getResultTypes(), op->getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct CallOpLowering : mlir::OpConversionPattern<qzap::CallOp> {
  using OpConversionPattern<qzap::CallOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qzap::CallOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qpin::CallOp>(op, op->getResultTypes(),
                                              op.getOperands(), op->getAttrs());
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct AllocOpLowering : mlir::OpConversionPattern<qzap::AllocOp> {
  using OpConversionPattern<qzap::AllocOp>::OpConversionPattern;

  AllocOpLowering(mlir::TypeConverter &typeConverter,
                  mlir::MLIRContext *context)
      : OpConversionPattern<qzap::AllocOp>(typeConverter, context) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::AllocOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct FreeOpLowering : mlir::OpConversionPattern<qzap::FreeOp> {
  using OpConversionPattern<qzap::FreeOp>::OpConversionPattern;

  FreeOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context)
      : OpConversionPattern<qzap::FreeOp>(typeConverter, context) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::FreeOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct RetrieveOpLowering : mlir::OpConversionPattern<qzap::RetrieveOp>,
                            LoweringWithState {
  using OpConversionPattern<qzap::RetrieveOp>::OpConversionPattern;

  RetrieveOpLowering(mlir::TypeConverter &typeConverter,
                     mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<qzap::RetrieveOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::RetrieveOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    getState().qubits[op.getQubit()] = op.getIndex();
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct StoreOpLowering : mlir::OpConversionPattern<qzap::StoreOp>,
                         LoweringWithState {
  using OpConversionPattern<qzap::StoreOp>::OpConversionPattern;

  StoreOpLowering(mlir::TypeConverter &typeConverter,
                  mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<qzap::StoreOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::StoreOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    getState().qubits.erase(op.getQubit());
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : mlir::OpConversionPattern<qzap::MeasureOp>,
                           LoweringWithState {
  using OpConversionPattern<qzap::MeasureOp>::OpConversionPattern;

  MeasureOpLowering(mlir::TypeConverter &typeConverter,
                    mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<qzap::MeasureOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::MeasureOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Value index = getState().qubits[op.getQubitIn()];

    rewriter.replaceOpWithNewOp<qpin::MeasureOp>(op, op.getBit().getType(),
                                                 index);

    getState().qubits.erase(op.getQubitIn());
    getState().qubits[op.getQubitOut()] = index;

    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

namespace {
template <typename SourceOp, class DestOp>
struct UnitaryOpLowering : LoweringWithState {
  UnitaryOpLowering(LoweringState *state) : LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewriteImpl(SourceOp op, typename SourceOp::Adaptor adaptor,
                      mlir::ConversionPatternRewriter &rewriter) const {
    mlir::Value targetIndex = getState().qubits[op.getTargetIn()];

    if (op.getControlIn()) {
      mlir::Value controlIndex = getState().qubits[op.getControlIn()];

      rewriter.create<DestOp>(op->getLoc(), targetIndex, controlIndex);

      getState().qubits.erase(op.getControlIn());
      getState().qubits[op.getControlOut()] = controlIndex;
    } else {
      rewriter.create<DestOp>(op->getLoc(), targetIndex, nullptr);
    }

    getState().qubits.erase(op.getTargetIn());
    getState().qubits[op.getTargetOut()] = targetIndex;

    rewriter.eraseOp(op);
    return mlir::success();
  }
};
} // namespace

struct HOpLowering : mlir::OpConversionPattern<qzap::HOp>,
                     UnitaryOpLowering<qzap::HOp, qpin::HOp> {
  using OpConversionPattern<qzap::HOp>::OpConversionPattern;

  HOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<qzap::HOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::HOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct XOpLowering : mlir::OpConversionPattern<qzap::XOp>,
                     UnitaryOpLowering<qzap::XOp, qpin::XOp> {
  using OpConversionPattern<qzap::XOp>::OpConversionPattern;

  XOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<qzap::XOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::XOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct YOpLowering : mlir::OpConversionPattern<qzap::YOp>,
                     UnitaryOpLowering<qzap::YOp, qpin::YOp> {
  using OpConversionPattern<qzap::YOp>::OpConversionPattern;

  YOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<qzap::YOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::YOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

struct ZOpLowering : mlir::OpConversionPattern<qzap::ZOp>,
                     UnitaryOpLowering<qzap::ZOp, qpin::ZOp> {
  using OpConversionPattern<qzap::ZOp>::OpConversionPattern;

  ZOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
              LoweringState *state)
      : OpConversionPattern<qzap::ZOp>(typeConverter, context),
        UnitaryOpLowering(state) {}

  mlir::LogicalResult
  matchAndRewrite(qzap::ZOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }
};

//===----------------------------------------------------------------------===//
// Conversion Entry
//===----------------------------------------------------------------------===//

/// @brief QZap to QPin Dialect Conversion Pass
struct QZapToQPin : impl::QZapToQPinBase<QZapToQPin> {
  using QZapToQPinBase::QZapToQPinBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    LoweringState state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QZapDialect>();
    target.addLegalDialect<qpin::QPinDialect>();

    QZapToQPinTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<KernelOpLowering, ReturnOpLowering, CallOpLowering,
             AllocOpLowering, FreeOpLowering>(typeConverter, context)
        .add<RetrieveOpLowering, MeasureOpLowering, StoreOpLowering,
             HOpLowering, XOpLowering>(typeConverter, context, &state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }

    assert(state.qubits.empty());
  }
};
}; // namespace qzap
}; // namespace aqomplice