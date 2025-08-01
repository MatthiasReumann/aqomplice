#include "conversion/q-to-qzap/q-to-qzap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>
#include <utility>

namespace aqomplice {
#define GEN_PASS_DEF_QTOQZAP
#include "conversion/passes.h.inc" // adds `impl::QToQZapBase`

namespace q {
namespace {
/// @brief Internal state for lowering quantum ops.
struct LoweringContext {
  struct QubitInfo {
    /// @brief The qubit returned from the quantum register.
    mlir::Value qubit;
    /// @brief The quantum register the qubit belongs to.
    mlir::Value qreg;
    /// @brief The index in the quantum register.
    mlir::Value index;
    /// @brief The amount of uses of the qubit in the kernel.
    std::size_t uses;
  };
  /// @brief Maps q::qubit to qzap::qubit and some additional infos.
  llvm::DenseMap<mlir::Value, QubitInfo> qubits{};
  /// @brief Maps q::qreq to qzap::qreq.
  llvm::DenseMap<mlir::Value, mlir::Value> qregs{};
  /// @brief Clean-up verification
  bool isCleanedUp() const { return qregs.empty() && qubits.empty(); }
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

protected:
  LoweringContext &getState() const { return state_; }

  [[nodiscard]] bool
  hasZeroUses(mlir::Value q, const mlir::Location loc,
              mlir::ConversionPatternRewriter &rewriter) const {
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

struct FreeOpLowering : StatefulOpConversionPattern<q::FreeOp> {
  using StatefulOpConversionPattern<q::FreeOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::FreeOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Value qregIn = getState().qregs[op.getQreg()];
    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, qregIn);
    getState().qregs.erase(op.getQreg());
    return mlir::success();
  }
};

struct RetrieveOpLowering : StatefulOpConversionPattern<q::RetrieveOp> {
  using StatefulOpConversionPattern<q::RetrieveOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::RetrieveOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type qubit = typeConverter->convertType(op.getQubit().getType());
    mlir::Value qregIn = getState().qregs[op.getQreg()];

    const auto range = op.getQubit().getUses();
    const std::size_t uses = std::distance(range.begin(), range.end());

    qzap::RetrieveOp retrieve = rewriter.replaceOpWithNewOp<qzap::RetrieveOp>(
        op, qregIn.getType(), qubit, qregIn, adaptor.getIndex());

    getState().qregs[op.getQreg()] = retrieve.getQreqOut();
    getState().qubits[op.getQubit()] = {retrieve.getQubit(), op.getQreg(),
                                        retrieve.getIndex(), uses};

    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : StatefulOpConversionPattern<q::MeasureOp> {
  using StatefulOpConversionPattern<q::MeasureOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::MeasureOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    LoweringContext::QubitInfo &info = getState().qubits[op.getQubit()];

    qzap::MeasureOp m = rewriter.replaceOpWithNewOp<qzap::MeasureOp>(
        op, op.getBit().getType(), info.qubit.getType(), info.qubit);

    info.qubit = m.getQubitOut();
    std::ignore = hasZeroUses(op.getQubit(), op.getLoc(), rewriter);

    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

template <typename SourceOp, typename DestOp>
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

    auto ctrld =
        mlir::dyn_cast<ControlledUnitaryOpInterface>(op.getOperation());
    if (ctrld && !ctrld.hasControl()) {
      LoweringContext::QubitInfo &a = this->getState().qubits[op.getA()];
      auto u =
          rewriter.create<DestOp>(op->getLoc(), a.qubit.getType(), a.qubit);
      a.qubit = u.getAOut();
      std::ignore = this->hasZeroUses(op.getA(), op->getLoc(), rewriter);
      rewriter.eraseOp(op);
      return mlir::success();
    }

    LoweringContext::QubitInfo &a = this->getState().qubits[op.getA()];
    LoweringContext::QubitInfo &b = this->getState().qubits[op.getB()];
    auto u = rewriter.create<DestOp>(op->getLoc(), a.qubit.getType(),
                                     b.qubit.getType(), a.qubit, b.qubit);
    a.qubit = u.getAOut();
    b.qubit = u.getBOut();
    std::ignore = this->hasZeroUses(op.getA(), op->getLoc(), rewriter);
    std::ignore = this->hasZeroUses(op.getB(), op->getLoc(), rewriter);
    rewriter.eraseOp(op);
    return mlir::success();
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

struct ConversionTypeConverter : mlir::TypeConverter {
  ConversionTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
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
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    LoweringContext state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect>();

    ConversionTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<AllocOpLowering, FreeOpLowering, RetrieveOpLowering,
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