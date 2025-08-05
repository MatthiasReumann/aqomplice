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

using namespace mlir;

namespace aqomplice {
#define GEN_PASS_DEF_QPINTOLLVM
#include "conversion/passes.h.inc" // adds `impl::QPinToLLVMBase`

namespace qpin {
namespace {
bool addFuncDecl(std::string name, LLVM::LLVMFunctionType signature,
                 Operation *op, ConversionPatternRewriter &rewriter) {
  ModuleOp module = op->getParentOfType<ModuleOp>();
  assert(module && "Expecting module");

  if (!module.lookupSymbol<LLVM::LLVMFuncOp>(name)) {
    OpBuilder::InsertionGuard guard(rewriter);
    rewriter.setInsertionPointToStart(module.getBody());
    rewriter.create<LLVM::LLVMFuncOp>(op->getLoc(), name, signature);
    return true;
  }
  
  return false;
}

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct QubitOpLowering : OpConversionPattern<qpin::QubitOp> {
  using OpConversionPattern<qpin::QubitOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(qpin::QubitOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    // Since no other quantum operation can be executed without an invocation
    // of qpin.qubit, we insert QIR's __quantum__rt__initialize function here,
    // if necessary.
    Type ptrT = LLVM::LLVMPointerType::get(op->getContext());
    Type resT = LLVM::LLVMVoidType::get(op.getContext());
    auto signature = LLVM::LLVMFunctionType::get(resT, ptrT);
    std::string initF = getQIRFuncString("rt__initialize");

    if (addFuncDecl(initF, signature, op, rewriter)) {
      OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(op->getBlock());
      auto zero = rewriter.create<LLVM::ZeroOp>(op->getLoc(), ptrT);
      rewriter.create<LLVM::CallOp>(op->getLoc(), signature, initF,
                                    zero.getRes());
    }

    // Replace qubit operation with static device pointer.
    Value index = rewriter.create<LLVM::ConstantOp>(
        op.getLoc(), rewriter.getI32Type(),
        rewriter.getI32IntegerAttr(op.getIndex()));
    rewriter.replaceOpWithNewOp<LLVM::IntToPtrOp>(op, ptrT, index);
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

struct MeasureOpLowering : OpConversionPattern<qpin::MeasureOp> {
  using OpConversionPattern<qpin::MeasureOp>::OpConversionPattern;

  LogicalResult
  matchAndRewrite(qpin::MeasureOp op, OpAdaptor adaptor,
                  ConversionPatternRewriter &rewriter) const final {
    Type ptrT = LLVM::LLVMPointerType::get(op->getContext());
    Type resT = op.getBit().getType();
    auto signature = LLVM::LLVMFunctionType::get(resT, ptrT);
    std::string mzF = getQIRInsName("mz", "body");
    addFuncDecl(mzF, signature, op, rewriter);
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(op, signature, mzF,
                                              adaptor.getQubit());
    return success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

template <typename OpType>
struct UnitaryOpLowering : OpConversionPattern<OpType> {
  using OpConversionPattern<OpType>::OpConversionPattern;

protected:
  LogicalResult matchAndRewriteImpl(OpType op, typename OpType::Adaptor adaptor,
                                    ConversionPatternRewriter &rewriter,
                                    const std::string &gateName) const {
    Type voidT = LLVM::LLVMVoidType::get(op->getContext());
    Type ptrT = LLVM::LLVMPointerType::get(op->getContext());

    llvm::SmallVector<Type, 2> params;
    llvm::SmallVector<Value, 2> args;
    std::string gateF;

    auto cu = dyn_cast<UnitaryOpInterface>(op.getOperation());
    if (!cu.hasSecondary()) { // 1-Qubit Gate.
      params = {ptrT};
      args = {adaptor.getPrimary()};
      gateF = getQIRInsName(gateName, "body");
    } else { // 2-Qubit Gates.
      params = {ptrT, ptrT};
      args = {adaptor.getPrimary(), adaptor.getSecondary()};
      if (cu.isControlled()) { // Controlled 1-Qubit Gate.
        gateF = getQIRInsName(gateName, "ctl");
      } else { // Native 2-Qubit Gate.
        gateF = getQIRInsName(gateName, "body");
      }
    }

    auto signature = LLVM::LLVMFunctionType::get(voidT, params);
    addFuncDecl(gateF, signature, op, rewriter);
    rewriter.replaceOpWithNewOp<LLVM::CallOp>(op, signature, gateF, args);
    return success();
  }
};

#define DEFINE_UNITARY_OP_LOWERING(OP, NAME)                                   \
  struct OP##Lowering : UnitaryOpLowering<qpin::OP> {                          \
    using UnitaryOpLowering<qpin::OP>::UnitaryOpLowering;                      \
    LogicalResult                                                              \
    matchAndRewrite(qpin::OP op, OpAdaptor adaptor,                            \
                    ConversionPatternRewriter &rewriter) const final {         \
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

struct ConversionTypeConverter : TypeConverter {
  ConversionTypeConverter(MLIRContext *ctx) {
    addConversion([](Type type) { return type; });
    addConversion([](StaticQubitType type) {
      return LLVM::LLVMPointerType::get(type.getContext());
    });
  }
};
} // namespace

struct QPinToLLVM : impl::QPinToLLVMBase<QPinToLLVM> {
  using QPinToLLVMBase::QPinToLLVMBase;

  void runOnOperation() override {
    MLIRContext *context = &getContext();

    ConversionTarget target(*context);
    target.addLegalDialect<LLVM::LLVMDialect>();
    target.addIllegalDialect<QPinDialect>();

    ConversionTypeConverter typeConverter(context);
    LLVMTypeConverter llvmTypeConverter(context);

    RewritePatternSet patterns(context);
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