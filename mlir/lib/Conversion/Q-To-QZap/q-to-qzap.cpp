#include "conversion/q-to-qzap/q-to-qzap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>
#include <utility>

using namespace mlir;

namespace aqomplice {
#define GEN_PASS_DEF_QTOQZAP
#include "conversion/passes.h.inc" // adds `impl::QToQZapBase`

namespace q {
namespace {

/// @brief Internal state for lowering quantum ops.
struct LoweringContext {
  struct QubitInfo {
    /// @brief The qubit returned from the quantum register.
    Value qubit;
    /// @brief The quantum register the qubit belongs to.
    Value qreg;
    /// @brief The index in the quantum register.
    Value index;
    /// @brief The amount of uses of the qubit in the kernel.
    std::size_t uses;
  };
  /// @brief Maps q::qubit to qzap::qubit and some additional infos.
  llvm::DenseMap<Value, QubitInfo> qubits{};
  /// @brief Maps q::qreq to qzap::qreq.
  llvm::DenseMap<Value, Value> qregs{};
  /// @brief Clean-up verification
  bool isCleanedUp() const { return qregs.empty() && qubits.empty(); }
};

template <typename OpType>
class StatefulOpConversionPattern : public OpConversionPattern<OpType> {
  using OpConversionPattern<OpType>::OpConversionPattern;

public:
  StatefulOpConversionPattern(TypeConverter &typeConverter,
                              MLIRContext *context, LoweringContext &state)
      : OpConversionPattern<OpType>(typeConverter, context), state_(state) {}

protected:
  LoweringContext &getState() const { return state_; }

  [[nodiscard]] bool hasZeroUses(Value q, const Location loc,
                                 ConversionPatternRewriter &rewriter) const {
    auto &info = getState().qubits[q];
    if ((--info.uses) == 0) {
      auto qregIn = getState().qregs[info.qreg];
      auto store = rewriter.create<qzap::StoreOp>(loc, qregIn.getType(), qregIn,
                                                  info.index, info.qubit);
      getState().qregs[info.qreg] = store.getQregOut();
      getState().qubits.erase(q);

      return true;
    }

    return false;
  }

private:
  LoweringContext &state_;
};

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct AllocOpLowering : StatefulOpConversionPattern<q::AllocOp> {
  using StatefulOpConversionPattern<q::AllocOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(q::AllocOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Type qreg = typeConverter->convertType(op.getQreg().getType());
    qzap::AllocOp alloc = rewriter.replaceOpWithNewOp<qzap::AllocOp>(
        op, qreg, adaptor.getNqubits());
    getState().qregs[op.getQreg()] = alloc.getQreg();
    return success();
  }
};

struct FreeOpLowering : StatefulOpConversionPattern<q::FreeOp> {
  using StatefulOpConversionPattern<q::FreeOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(q::FreeOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Value qregIn = getState().qregs[op.getQreg()];
    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, qregIn);
    getState().qregs.erase(op.getQreg());
    return success();
  }
};

struct RetrieveOpLowering : StatefulOpConversionPattern<q::RetrieveOp> {
  using StatefulOpConversionPattern<q::RetrieveOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(q::RetrieveOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Type qubit = typeConverter->convertType(op.getQubit().getType());
    Value qregIn = getState().qregs[op.getQreg()];

    const auto range = op.getQubit().getUses();
    const std::size_t uses = std::distance(range.begin(), range.end());

    qzap::RetrieveOp retrieve = rewriter.replaceOpWithNewOp<qzap::RetrieveOp>(
        op, qregIn.getType(), qubit, qregIn, adaptor.getIndex());

    getState().qregs[op.getQreg()] = retrieve.getQreqOut();
    getState().qubits[op.getQubit()] = {retrieve.getQubit(), op.getQreg(),
                                        retrieve.getIndex(), uses};

    return success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : StatefulOpConversionPattern<q::MeasureOp> {
  using StatefulOpConversionPattern<q::MeasureOp>::StatefulOpConversionPattern;

  LogicalResult
  matchAndRewrite(q::MeasureOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    LoweringContext::QubitInfo &info = getState().qubits[op.getQubit()];

    qzap::MeasureOp m = rewriter.replaceOpWithNewOp<qzap::MeasureOp>(
        op, op.getBit().getType(), info.qubit.getType(), info.qubit);

    info.qubit = m.getQubitOut();
    std::ignore = hasZeroUses(op.getQubit(), op.getLoc(), rewriter);

    return success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

template <typename SourceOp, typename DestOp>
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
      LoweringContext::QubitInfo &a = this->getState().qubits[op.getPrimary()];
      auto u = rewriter.create<DestOp>(op->getLoc(),
                                       /* primary_out = */ a.qubit.getType(),
                                       /* primary_in = */ a.qubit,
                                       /* secondary_in = */ nullptr,
                                       /* ctrld = */ ui.isControlled());
      a.qubit = u.getPrimaryOut();
      std::ignore = this->hasZeroUses(op.getPrimary(), op->getLoc(), rewriter);
      rewriter.eraseOp(op);
      return success();
    }

    LoweringContext::QubitInfo &a = this->getState().qubits[op.getPrimary()];
    LoweringContext::QubitInfo &b = this->getState().qubits[op.getSecondary()];
    auto u = rewriter.create<DestOp>(op->getLoc(),
                                     /* primary_out = */ a.qubit.getType(),
                                     /* secondary_out = */ b.qubit.getType(),
                                     /* primary_in = */ a.qubit,
                                     /* secondary_in = */ b.qubit,
                                     /* ctrld = */ ui.isControlled());
    a.qubit = u.getPrimaryOut();
    b.qubit = u.getSecondaryOut();
    std::ignore = this->hasZeroUses(op.getPrimary(), op->getLoc(), rewriter);
    std::ignore = this->hasZeroUses(op.getSecondary(), op->getLoc(), rewriter);
    rewriter.eraseOp(op);
    return success();
  }
};

#define DEFINE_UNITARY_OP_LOWERING(OP)                                         \
  struct OP##Lowering : UnitaryOpLowering<q::OP, qzap::OP> {                   \
    using UnitaryOpLowering<q::OP, qzap::OP>::UnitaryOpLowering;               \
  };

DEFINE_UNITARY_OP_LOWERING(HOp)
DEFINE_UNITARY_OP_LOWERING(XOp)
DEFINE_UNITARY_OP_LOWERING(YOp)
DEFINE_UNITARY_OP_LOWERING(ZOp)
DEFINE_UNITARY_OP_LOWERING(SOp)
DEFINE_UNITARY_OP_LOWERING(TOp)
DEFINE_UNITARY_OP_LOWERING(SwapOp)

//===----------------------------------------------------------------------===//
// Type Converter
//===----------------------------------------------------------------------===//

struct ConversionTypeConverter : TypeConverter {
  ConversionTypeConverter(MLIRContext *ctx) {
    addConversion([](Type type) { return type; });
    addConversion(
        [ctx](q::QubitType type) { return qzap::QubitType::get(ctx); });
    addConversion([ctx](q::QubitArrayType type) {
      return qzap::QubitArrayType::get(ctx);
    });
  }
};
}; // namespace

struct QToQZap : impl::QToQZapBase<QToQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override {
    MLIRContext *context = &getContext();
    Operation *op = getOperation();

    LoweringContext state{};

    ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect>();

    ConversionTypeConverter typeConverter(context);
    RewritePatternSet patterns(context);
    patterns.add<AllocOpLowering, FreeOpLowering, RetrieveOpLowering,
                 MeasureOpLowering, HOpLowering, XOpLowering, YOpLowering,
                 ZOpLowering, SOpLowering, TOpLowering, SwapOpLowering>(
        typeConverter, context, state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }

    assert(state.isCleanedUp());
  }
};
}; // namespace q
}; // namespace aqomplice