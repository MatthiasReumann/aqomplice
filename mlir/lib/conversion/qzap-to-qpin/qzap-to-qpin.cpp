#include "conversion/qzap-to-qpin/qzap-to-qpin.h"

#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>

using namespace mlir;

namespace aqomplice {
#define GEN_PASS_DEF_QZAPTOQPIN
#include "conversion/passes.h.inc" // adds `impl::QZapToQPinBase`

namespace qzap {
namespace {
/// @brief Internal state for lowering quantum ops.
struct LoweringContext {
  /// @brief Maps qzap::qubit to index i32 value.
  llvm::DenseMap<Value, Value> qubits{};
  /// @brief Clean-up verification
  bool isCleanedUp() const { return qubits.empty(); }
};

template <typename OpType>
class StatefulOpConversionPattern : public OpConversionPattern<OpType> {
  using OpConversionPattern<OpType>::OpConversionPattern;

public:
  StatefulOpConversionPattern(TypeConverter &typeConverter,
                              MLIRContext *context, LoweringContext &state)
      : OpConversionPattern<OpType>(typeConverter, context), state_(state) {}

  LoweringContext &getState() const { return state_; }

private:
  LoweringContext &state_;
};

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct AllocOpLowering : OpConversionPattern<qzap::AllocOp> {
  using OpConversionPattern<qzap::AllocOp>::OpConversionPattern;

  AllocOpLowering(TypeConverter &typeConverter, MLIRContext *context)
      : OpConversionPattern<qzap::AllocOp>(typeConverter, context) {}

  LogicalResult
  matchAndRewrite(qzap::AllocOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    rewriter.eraseOp(op);
    return success();
  }
};

struct FreeOpLowering : OpConversionPattern<qzap::FreeOp> {
  using OpConversionPattern<qzap::FreeOp>::OpConversionPattern;

  FreeOpLowering(TypeConverter &typeConverter, MLIRContext *context)
      : OpConversionPattern<qzap::FreeOp>(typeConverter, context) {}

  LogicalResult
  matchAndRewrite(qzap::FreeOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    rewriter.eraseOp(op);
    return success();
  }
};

struct RetrieveOpLowering : StatefulOpConversionPattern<qzap::RetrieveOp> {
  using StatefulOpConversionPattern<
      qzap::RetrieveOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(qzap::RetrieveOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    auto index = op.getIndex();
    assert(index.getType().isInteger()); // index must be arith.constant : i32
    auto attr = index.getDefiningOp()->getAttrOfType<IntegerAttr>("value");
    assert(attr != nullptr); // cast must succeed.

    auto q = rewriter.create<qpin::QubitOp>(
        index.getLoc(), qpin::StaticQubitType::get(getContext()), attr);

    getState().qubits[op.getQubit()] = q.getQubit();
    rewriter.eraseOp(op);
    return success();
  }
};

struct StoreOpLowering : StatefulOpConversionPattern<qzap::StoreOp> {
  using StatefulOpConversionPattern<qzap::StoreOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(qzap::StoreOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    getState().qubits.erase(op.getQubit());
    rewriter.eraseOp(op);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : StatefulOpConversionPattern<qzap::MeasureOp> {
  using StatefulOpConversionPattern<
      qzap::MeasureOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(qzap::MeasureOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Value index = getState().qubits[op.getQubitIn()];

    rewriter.replaceOpWithNewOp<qpin::MeasureOp>(op, op.getBit().getType(),
                                                 index);

    getState().qubits.erase(op.getQubitIn());
    getState().qubits[op.getQubitOut()] = index;

    return success();
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

  LogicalResult
  matchAndRewrite(SourceOp op, typename SourceOp::Adaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter);
  }

private:
  LogicalResult matchAndRewriteImpl(SourceOp op,
                                    typename SourceOp::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter) const {
    UnitaryOpInterface ui = dyn_cast<UnitaryOpInterface>(op.getOperation());
    if (!ui.hasSecondary()) {
      Value aIndex = this->getState().qubits[op.getPrimaryIn()];

      rewriter.create<DestOp>(op->getLoc(), /* primary = */ aIndex,
                              /* secondary = */ nullptr,
                              /* ctrld = */ ui.isControlled());

      this->getState().qubits.erase(op.getPrimaryIn());
      this->getState().qubits[op.getPrimaryOut()] = aIndex;

      rewriter.eraseOp(op);
      return success();
    }

    Value aIndex = this->getState().qubits[op.getPrimaryIn()];
    Value bIndex = this->getState().qubits[op.getSecondaryIn()];

    rewriter.create<DestOp>(op->getLoc(), /* primary = */ aIndex,
                            /* secondary = */ bIndex,
                            /* ctrld = */ ui.isControlled());

    this->getState().qubits.erase(op.getPrimaryIn());
    this->getState().qubits[op.getPrimaryOut()] = aIndex;

    this->getState().qubits.erase(op.getSecondaryIn());
    this->getState().qubits[op.getSecondaryOut()] = bIndex;

    rewriter.eraseOp(op);
    return success();
  }
};
} // namespace

#define DEFINE_UNITARY_OP_LOWERING(OP)                                         \
  struct OP##Lowering : UnitaryOpLowering<qzap::OP, qpin::OP> {                \
    using UnitaryOpLowering<qzap::OP, qpin::OP>::UnitaryOpLowering;            \
  };

DEFINE_UNITARY_OP_LOWERING(HOp)
DEFINE_UNITARY_OP_LOWERING(XOp)
DEFINE_UNITARY_OP_LOWERING(YOp)
DEFINE_UNITARY_OP_LOWERING(ZOp)
DEFINE_UNITARY_OP_LOWERING(SOp)
DEFINE_UNITARY_OP_LOWERING(TOp)
DEFINE_UNITARY_OP_LOWERING(SwapOp)

struct ConversionTypeConverter : TypeConverter {
  ConversionTypeConverter(MLIRContext *ctx) {
    addConversion([](Type type) { return type; });
  }
};
}; // namespace

struct QZapToQPin : impl::QZapToQPinBase<QZapToQPin> {
  using QZapToQPinBase::QZapToQPinBase;

  void runOnOperation() override {
    MLIRContext *context = &getContext();

    LoweringContext state{};

    ConversionTarget target(*context);
    target.addIllegalDialect<QZapDialect>();
    target.addLegalDialect<qpin::QPinDialect>();

    ConversionTypeConverter typeConverter(context);
    RewritePatternSet patterns(context);
    patterns.add<AllocOpLowering, FreeOpLowering>(typeConverter, context)
        .add<RetrieveOpLowering, MeasureOpLowering, StoreOpLowering,
             HOpLowering, XOpLowering, SOpLowering, TOpLowering,
             SwapOpLowering>(typeConverter, context, state);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns)))) {
      signalPassFailure();
    }

    assert(state.isCleanedUp());
  }
};
}; // namespace qzap
}; // namespace aqomplice