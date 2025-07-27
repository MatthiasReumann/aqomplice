#include "Conversion/q-to-qzap/q-to-qzap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>
#include <utility>

namespace aqomplice {
#define GEN_PASS_DEF_QTOQZAP
#include "Conversion/Passes.h.inc" // adds `impl::QToQZapBase`

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

  LoweringContext &getState() const { return state_; }

private:
  LoweringContext &state_;
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

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

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

    mlir::Value qubitIn = info.qubit;
    qzap::MeasureOp m = rewriter.replaceOpWithNewOp<qzap::MeasureOp>(
        op, op.getBit().getType(), qubitIn.getType(), qubitIn);

    if ((--info.uses) == 0) {
      auto qregIn = getState().qregs[info.qreg];
      auto store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qregIn.getType(), qregIn, info.index, m.getQubitOut());

      getState().qregs[info.qreg] = store.getQregOut();
      getState().qubits.erase(op.getQubit());
    } else {
      info.qubit = m.getQubitOut();
    }

    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

namespace {
template <typename SourceOp, typename DestOp>
class OptionallyControlledUnitaryOpLowering
    : public StatefulOpConversionPattern<SourceOp> {
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
    LoweringContext::QubitInfo &target =
        this->getState().qubits[op.getTarget()];
    mlir::Value targetIn = target.qubit;

    if (op.getControl()) {
      LoweringContext::QubitInfo &control =
          this->getState().qubits[op.getControl()];
      mlir::Value controlIn = control.qubit;

      auto u =
          rewriter.create<DestOp>(op->getLoc(), targetIn.getType(),
                                  controlIn.getType(), targetIn, controlIn);
      target.qubit = u.getTargetOut();
      control.qubit = u.getControlOut();

      if ((--control.uses) == 0) {
        auto qregIn = this->getState().qregs[control.qreg];
        auto store = rewriter.create<qzap::StoreOp>(
            op->getLoc(), qregIn.getType(), qregIn, control.index,
            control.qubit);
        this->getState().qregs[control.qreg] = store.getQregOut();
        this->getState().qubits.erase(op.getControl());
      }
    } else {
      auto u =
          rewriter.create<DestOp>(op->getLoc(), targetIn.getType(), targetIn);
      target.qubit = u.getTargetOut();
    }

    if ((--target.uses) == 0) {
      auto qregIn = this->getState().qregs[target.qreg];
      auto store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qregIn.getType(), qregIn, target.index, target.qubit);

      this->getState().qregs[target.qreg] = store.getQregOut();
      this->getState().qubits.erase(op.getTarget());
    }

    rewriter.eraseOp(op);
    return mlir::success();
  }
};
} // namespace

struct HOpLowering : OptionallyControlledUnitaryOpLowering<q::HOp, qzap::HOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::HOp, qzap::HOp>::OptionallyControlledUnitaryOpLowering;
};

struct XOpLowering : OptionallyControlledUnitaryOpLowering<q::XOp, qzap::XOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::XOp, qzap::XOp>::OptionallyControlledUnitaryOpLowering;
};

struct YOpLowering : OptionallyControlledUnitaryOpLowering<q::YOp, qzap::YOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::YOp, qzap::YOp>::OptionallyControlledUnitaryOpLowering;
};

struct ZOpLowering : OptionallyControlledUnitaryOpLowering<q::ZOp, qzap::ZOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::ZOp, qzap::ZOp>::OptionallyControlledUnitaryOpLowering;
};

struct SOpLowering : OptionallyControlledUnitaryOpLowering<q::SOp, qzap::SOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::SOp, qzap::SOp>::OptionallyControlledUnitaryOpLowering;
};

struct TOpLowering : OptionallyControlledUnitaryOpLowering<q::TOp, qzap::TOp> {
  using OptionallyControlledUnitaryOpLowering<
      q::TOp, qzap::TOp>::OptionallyControlledUnitaryOpLowering;
};

struct SwapOpLowering : StatefulOpConversionPattern<q::SwapOp> {
  using StatefulOpConversionPattern<q::SwapOp>::StatefulOpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::SwapOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    LoweringContext::QubitInfo &a = getState().qubits[op.getA()];
    LoweringContext::QubitInfo &b = getState().qubits[op.getB()];

    auto swap = rewriter.create<qzap::SwapOp>(
        op->getLoc(), a.qubit.getType(), b.qubit.getType(), a.qubit, b.qubit);
    a.qubit = swap.getAOut();
    b.qubit = swap.getBOut();

    if ((--a.uses) == 0) {
      auto qregIn = getState().qregs[a.qreg];
      auto store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qregIn.getType(), qregIn, a.index, a.qubit);
      getState().qregs[a.qreg] = store.getQregOut();
      getState().qubits.erase(op.getA());
    }

    if ((--b.uses) == 0) {
      auto qregIn = getState().qregs[b.qreg];
      auto store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qregIn.getType(), qregIn, b.index, b.qubit);
      getState().qregs[b.qreg] = store.getQregOut();
      getState().qubits.erase(op.getB());
    }

    rewriter.eraseOp(op);
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Conversion Entry
//===----------------------------------------------------------------------===//

/// @brief Q to QZap Dialect Conversion Pass
struct QToQZap : impl::QToQZapBase<QToQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    LoweringContext state{};

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect>();

    QToQZapTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns
        .add<KernelOpLowering, ReturnOpLowering, CallOpLowering>(typeConverter,
                                                                 context)
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