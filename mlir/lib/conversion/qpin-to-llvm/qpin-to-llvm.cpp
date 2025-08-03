#include "conversion/qpin-to-llvm/qpin-to-llvm.h"

#include "QPin/IR/QPinDialect.h"
#include "common/qir.h"

#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>

namespace aqomplice {
#define GEN_PASS_DEF_QPINTOLLVM
#include "conversion/passes.h.inc" // adds `impl::QPinToLLVMBase`

namespace qpin {
namespace {

bool addFuncDecl(std::string name, mlir::LLVM::LLVMFunctionType signature,
                 mlir::Operation *op,
                 mlir::ConversionPatternRewriter &rewriter) {
  mlir::ModuleOp module = op->getParentOfType<mlir::ModuleOp>();
  assert(module && "Expecting module");

  if (!module.lookupSymbol<mlir::LLVM::LLVMFuncOp>(name)) {
    mlir::OpBuilder::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    rewriter.create<mlir::LLVM::LLVMFuncOp>(op->getLoc(), name, signature);
    return true;
  }

  return false;
}

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct QubitOpLowering : mlir::OpConversionPattern<qpin::QubitOp> {
  using OpConversionPattern<qpin::QubitOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::QubitOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    // Since no other quantum operation can be executed without an invocation
    // of qpin.qubit, we insert QIR's __quantum__rt__initialize function here,
    // if necessary.
    mlir::Type ptrT = mlir::LLVM::LLVMPointerType::get(op->getContext());
    mlir::Type resT = mlir::LLVM::LLVMVoidType::get(op.getContext());
    auto signature = mlir::LLVM::LLVMFunctionType::get(resT, ptrT);
    std::string initF = getQIRFuncString("rt__initialize");

    if (addFuncDecl(initF, signature, op, rewriter)) {
      mlir::OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(op->getBlock());
      auto zero = rewriter.create<mlir::LLVM::ZeroOp>(op->getLoc(), ptrT);
      rewriter.create<mlir::LLVM::CallOp>(op->getLoc(), signature, initF,
                                          zero.getRes());
    }

    // Replace qubit operation with static device pointer.
    mlir::Value index = rewriter.create<mlir::LLVM::ConstantOp>(
        op.getLoc(), rewriter.getI32Type(),
        rewriter.getI32IntegerAttr(op.getIndex()));
    rewriter.replaceOpWithNewOp<mlir::LLVM::IntToPtrOp>(op, ptrT, index);
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : mlir::OpConversionPattern<qpin::MeasureOp> {
  using OpConversionPattern<qpin::MeasureOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::MeasureOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type ptrT = mlir::LLVM::LLVMPointerType::get(op->getContext());
    mlir::Type resT = op.getBit().getType();
    auto signature = mlir::LLVM::LLVMFunctionType::get(resT, ptrT);
    std::string mzF = getQIRInsName("mz", "body");
    addFuncDecl(mzF, signature, op, rewriter);
    rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, signature, mzF,
                                                    adaptor.getQubit());
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

template <typename OpType>
struct UnitaryOpLowering : mlir::OpConversionPattern<OpType> {
  using mlir::OpConversionPattern<OpType>::OpConversionPattern;

protected:
  mlir::LogicalResult
  matchAndRewriteImpl(OpType op, typename OpType::Adaptor adaptor,
                      mlir::ConversionPatternRewriter &rewriter,
                      const std::string &gateName) const {
    mlir::Type voidT = mlir::LLVM::LLVMVoidType::get(op->getContext());
    mlir::Type ptrT = mlir::LLVM::LLVMPointerType::get(op->getContext());

    // Try to handle controlled unitary ops via interface
    if (auto cu =
            mlir::dyn_cast<ControlledUnitaryOpInterface>(op.getOperation())) {
      llvm::SmallVector<mlir::Type, 2> params;
      llvm::SmallVector<mlir::Value, 2> args;
      std::string gateF;
      if (cu.hasControl()) {
        params = {ptrT, ptrT};
        args = {adaptor.getA(), adaptor.getB()};
        gateF = getQIRInsName(gateName, "ctl");
      } else {
        params = {ptrT};
        args = {adaptor.getA()};
        gateF = getQIRInsName(gateName, "body");
      }
      auto signature = mlir::LLVM::LLVMFunctionType::get(voidT, params);
      addFuncDecl(gateF, signature, op, rewriter);
      rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, signature, gateF,
                                                      args);
      return mlir::success();
    }

    // Default: two-qubit gate
    llvm::SmallVector<mlir::Type, 2> params{ptrT, ptrT};
    llvm::SmallVector<mlir::Value, 2> args{adaptor.getA(), adaptor.getB()};
    std::string gateF = getQIRInsName(gateName, "body");
    auto signature = mlir::LLVM::LLVMFunctionType::get(voidT, params);
    addFuncDecl(gateF, signature, op, rewriter);
    rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, signature, gateF, args);

    return mlir::success();
  }
};

#define DEFINE_UNITARY_OP_LOWERING(OP, NAME)                                   \
  struct OP##Lowering : UnitaryOpLowering<qpin::OP> {                          \
    using UnitaryOpLowering<qpin::OP>::UnitaryOpLowering;                      \
    mlir::LogicalResult                                                        \
    matchAndRewrite(qpin::OP op, OpAdaptor adaptor,                            \
                    mlir::ConversionPatternRewriter &rewriter) const final {   \
      return matchAndRewriteImpl(op, adaptor, rewriter, NAME);                 \
    }                                                                          \
  };

DEFINE_UNITARY_OP_LOWERING(HOp, "h")
DEFINE_UNITARY_OP_LOWERING(XOp, "x")
DEFINE_UNITARY_OP_LOWERING(YOp, "y")
DEFINE_UNITARY_OP_LOWERING(ZOp, "z")
DEFINE_UNITARY_OP_LOWERING(SOp, "s")
DEFINE_UNITARY_OP_LOWERING(TOp, "t")
DEFINE_UNITARY_OP_LOWERING(SwapOp, "swap")

//===----------------------------------------------------------------------===//
// Type Converter
//===----------------------------------------------------------------------===//

struct ConversionTypeConverter : mlir::TypeConverter {
  ConversionTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
    addConversion([](StaticQubitType type) {
      return mlir::LLVM::LLVMPointerType::get(type.getContext());
    });
  }
};
} // namespace

struct QPinToLLVM : impl::QPinToLLVMBase<QPinToLLVM> {
  using QPinToLLVMBase::QPinToLLVMBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::ConversionTarget target(*context);
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();
    target.addIllegalDialect<QPinDialect>();

    ConversionTypeConverter typeConverter(context);
    mlir::LLVMTypeConverter llvmTypeConverter(context);

    mlir::RewritePatternSet patterns(context);
    patterns.add<QubitOpLowering, MeasureOpLowering, HOpLowering, XOpLowering,
                 YOpLowering, ZOpLowering, SOpLowering, TOpLowering,
                 SwapOpLowering>(typeConverter, context);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace qpin
}; // namespace aqomplice