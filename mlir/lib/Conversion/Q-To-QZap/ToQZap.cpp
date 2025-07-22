#include "Conversion/Q-To-QZap/ToQZap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>
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

  /// @brief Maps q::qreq to qzap::qreq.
  std::unordered_map<mlir::Value, mlir::Value> qregs;
  /// @brief Maps q::qubit to qzap::qubit and some additional infos.
  std::unordered_map<mlir::Value, QubitInfo> qubits;
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
    mlir::Value qregIn = getState().qregs[op.getQreg()];
    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, qregIn);
    getState().qregs.erase(op.getQreg());
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

struct MeasureOpLowering : mlir::OpConversionPattern<q::MeasureOp>,
                           LoweringWithState {
  using OpConversionPattern<q::MeasureOp>::OpConversionPattern;

  MeasureOpLowering(mlir::TypeConverter &typeConverter,
                    mlir::MLIRContext *context, LoweringState *state)
      : OpConversionPattern<q::MeasureOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(q::MeasureOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Value qregIn = getState().qregs[op.getQreg()];
    mlir::Type res = typeConverter->convertType(op.getRes().getType());
    qzap::MeasureOp m = rewriter.replaceOpWithNewOp<qzap::MeasureOp>(op, res, qregIn.getType(), qregIn);
    state->qregs[op.getQreg()] = m.getQregOut();
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
    LoweringState::QubitInfo &target = getState().qubits[op.getTarget()];
    mlir::Value targetIn = target.qubit;

    if (op.getControl()) {
      LoweringState::QubitInfo &control = getState().qubits[op.getControl()];
      mlir::Value controlIn = control.qubit;

      auto u =
          rewriter.create<DestOp>(op->getLoc(), targetIn.getType(),
                                  controlIn.getType(), targetIn, controlIn);
      target.qubit = u.getTargetOut();
      control.qubit = u.getControlOut();

      if ((--control.uses) == 0) {
        auto qregIn = state->qregs[control.qreg];
        auto store = rewriter.create<qzap::StoreOp>(
            op->getLoc(), qregIn.getType(), qregIn, control.index,
            control.qubit);
        state->qregs[control.qreg] = store.getQregOut();
        state->qubits.erase(op.getControl());
      }
    } else {
      auto u =
          rewriter.create<DestOp>(op->getLoc(), targetIn.getType(), targetIn);
      target.qubit = u.getTargetOut();
    }

    if ((--target.uses) == 0) {
      auto qregIn = state->qregs[target.qreg];
      auto store = rewriter.create<qzap::StoreOp>(
          op->getLoc(), qregIn.getType(), qregIn, target.index, target.qubit);

      state->qregs[target.qreg] = store.getQregOut();
      state->qubits.erase(op.getTarget());
    }

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

//===----------------------------------------------------------------------===//
// SCF With Quantum Operations
//===----------------------------------------------------------------------===//

struct ForOpLowering : mlir::OpConversionPattern<mlir::scf::ForOp>,
                       LoweringWithState {
  using OpConversionPattern<mlir::scf::ForOp>::OpConversionPattern;

  ForOpLowering(mlir::TypeConverter &typeConverter, mlir::MLIRContext *context,
                LoweringState *state)
      : OpConversionPattern<mlir::scf::ForOp>(typeConverter, context),
        LoweringWithState(state) {}

  mlir::LogicalResult
  matchAndRewrite(mlir::scf::ForOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto deps = getLoopDependencies(op);
    for (const auto &o : deps) {
      llvm::outs() << o << '\n';
    }

    auto loop = rewriter.create<mlir::scf::ForOp>(
        op->getLoc(), op.getLowerBound(), op.getUpperBound(), op.getStep(),
        deps);
    rewriter.inlineBlockBefore(op.getBody(), loop.getBody(), loop.end());

    rewriter.eraseOp(op);
    return mlir::success();
  }

private:
  llvm::SmallVector<mlir::Value, 4>
  getLoopDependencies(mlir::scf::ForOp forOp) const {
    llvm::SmallVector<mlir::Value, 4> values;
    forOp.getRegion().walk([&](mlir::Operation *op) {
      for (mlir::Value operand : op->getOperands()) {
        if (!forOp.getRegion().isAncestor(operand.getParentRegion())) {
          values.push_back(operand);
        }
      }
    });
    return values;
  }
};

/// @brief Q to QZap Dialect Conversion Pass
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
        .add<AllocOpLowering, FreeOpLowering, RetrieveOpLowering,
             MeasureOpLowering, HOpLowering, XOpLowering, YOpLowering,
             ZOpLowering>(typeConverter, context, &state);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }

    assert(state.qregs.empty());
    assert(state.qubits.empty());
  }
};
}; // namespace q
}; // namespace aqomplice