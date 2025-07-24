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
struct LoweringContext {
  /// @brief Maps qzap::qubit to index i32 value.
  llvm::DenseMap<mlir::Value, mlir::Value> qubits{};
};

template <typename OpType>
class StatefulOpConversionPattern : public mlir::OpConversionPattern<OpType> {
  using mlir::OpConversionPattern<OpType>::OpConversionPattern;

public:
  StatefulOpConversionPattern(mlir::TypeConverter &typeConverter,
                              mlir::MLIRContext *context,
                              LoweringContext &state)
      : mlir::OpConversionPattern<OpType>(typeConverter, context),
        state_(state) {}

  LoweringContext &getState() const { return state_; }

private:
  LoweringContext &state_;
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

struct RetrieveOpLowering : StatefulOpConversionPattern<qzap::RetrieveOp> {
  using StatefulOpConversionPattern<
      qzap::RetrieveOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qzap::RetrieveOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto index = op.getIndex();
    assert(index.getType().isInteger()); // index must be arith.constant : i32
    auto attr = llvm::dyn_cast<mlir::IntegerAttr>(
        index.getDefiningOp()->getAttr("value"));
    assert(attr != nullptr); // cast must succeed.

    auto q = rewriter.create<qpin::QubitOp>(
        index.getLoc(), qpin::StaticQubitType::get(getContext()), attr);

    getState().qubits[op.getQubit()] = q.getQubit();
    rewriter.eraseOp(op);
    return mlir::success();
  }
};

struct StoreOpLowering : StatefulOpConversionPattern<qzap::StoreOp> {
  using StatefulOpConversionPattern<qzap::StoreOp>::StatefulOpConversionPattern;

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

struct MeasureOpLowering : StatefulOpConversionPattern<qzap::MeasureOp> {
  using StatefulOpConversionPattern<
      qzap::MeasureOp>::StatefulOpConversionPattern;

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
class UnitaryOpLowering : public StatefulOpConversionPattern<SourceOp> {
public:
  using StatefulOpConversionPattern<SourceOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }

private:
  mlir::LogicalResult
  matchAndRewriteImpl(SourceOp op, typename SourceOp::Adaptor adaptor,
                      mlir::ConversionPatternRewriter &rewriter) const {
    mlir::Value targetIndex = this->getState().qubits[op.getTargetIn()];

    if (op.getControlIn()) {
      mlir::Value controlIndex = this->getState().qubits[op.getControlIn()];

      rewriter.create<DestOp>(op->getLoc(), targetIndex, controlIndex);

      this->getState().qubits.erase(op.getControlIn());
      this->getState().qubits[op.getControlOut()] = controlIndex;
    } else {
      rewriter.create<DestOp>(op->getLoc(), targetIndex, nullptr);
    }

    this->getState().qubits.erase(op.getTargetIn());
    this->getState().qubits[op.getTargetOut()] = targetIndex;

    rewriter.eraseOp(op);
    return mlir::success();
  }
};
} // namespace

struct HOpLowering : UnitaryOpLowering<qzap::HOp, qpin::HOp> {
  using UnitaryOpLowering<qzap::HOp, qpin::HOp>::UnitaryOpLowering;
};

struct XOpLowering : UnitaryOpLowering<qzap::XOp, qpin::XOp> {
  using UnitaryOpLowering<qzap::XOp, qpin::XOp>::UnitaryOpLowering;
};

struct YOpLowering : UnitaryOpLowering<qzap::YOp, qpin::YOp> {
  using UnitaryOpLowering<qzap::YOp, qpin::YOp>::UnitaryOpLowering;
};

struct ZOpLowering : UnitaryOpLowering<qzap::ZOp, qpin::ZOp> {
  using UnitaryOpLowering<qzap::ZOp, qpin::ZOp>::UnitaryOpLowering;
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

    LoweringContext state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QZapDialect>();
    target.addLegalDialect<qpin::QPinDialect>();

    QZapToQPinTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<KernelOpLowering, ReturnOpLowering, CallOpLowering,
             AllocOpLowering, FreeOpLowering>(typeConverter, context)
        .add<RetrieveOpLowering, MeasureOpLowering, StoreOpLowering,
             HOpLowering, XOpLowering>(typeConverter, context, state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }

    assert(state.qubits.empty());
  }
};
}; // namespace qzap
}; // namespace aqomplice