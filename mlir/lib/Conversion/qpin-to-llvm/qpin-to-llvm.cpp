#include "Conversion/qpin-to-llvm/qpin-to-llvm.h"

#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Conversion/FuncToLLVM/ConvertFuncToLLVM.h"
#include "mlir/Conversion/LLVMCommon/Pattern.h"
#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMTypes.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/FormatVariadic.h"
#include <cassert>

namespace aqomplice {
namespace qpin {
#define GEN_PASS_DEF_QPINTOLLVM
#include "Conversion/qpin-to-llvm/qpin-to-llvm.h.inc" // adds `impl::QPinToLLVMBase`

namespace {
std::string getQIRCallableName(const std::string &op,
                               const std::string &specialization) {
  return llvm::formatv("__quantum__qis__{0}__{1}", op, specialization);
}
} // namespace

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

struct QubitOpLowering : mlir::OpConversionPattern<qpin::QubitOp> {
  using OpConversionPattern<qpin::QubitOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::QubitOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    mlir::Type ptrType = mlir::LLVM::LLVMPointerType::get(op->getContext());
    mlir::Value index = rewriter.create<mlir::LLVM::ConstantOp>(
        op.getLoc(), rewriter.getI32Type(),
        rewriter.getI32IntegerAttr(op.getIndex()));
    rewriter.replaceOpWithNewOp<mlir::LLVM::IntToPtrOp>(op, ptrType, index);
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

    mlir::Type result = op.getBit().getType();
    mlir::Type param = mlir::LLVM::LLVMPointerType::get(op->getContext());

    auto moduleOp = op->getParentOfType<mlir::ModuleOp>();
    if (!moduleOp) {
      return mlir::failure();
    }

    const char *funcName = "__quantum__qis__mz__body";
    auto funcOp = moduleOp.lookupSymbol<mlir::LLVM::LLVMFuncOp>(funcName);
    auto funcType = mlir::LLVM::LLVMFunctionType::get(result, param);
    if (!funcOp) {
      mlir::OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(moduleOp.getBody());
      rewriter.create<mlir::LLVM::LLVMFuncOp>(op->getLoc(), funcName, funcType);
    }

    rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, funcType, funcName,
                                                    adaptor.getQubit());
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

template <typename OpType>
struct OptionallyControlledUnitaryOpLowering
    : mlir::OpConversionPattern<OpType> {
  using mlir::OpConversionPattern<OpType>::OpConversionPattern;

protected:
  mlir::LogicalResult
  matchAndRewriteImpl(OpType op, typename OpType::Adaptor adaptor,
                      mlir::ConversionPatternRewriter &rewriter,
                      const std::string &opName) const {
    auto moduleOp = op->template getParentOfType<mlir::ModuleOp>();
    if (!moduleOp) {
      return mlir::failure();
    }

    const auto voidType = mlir::LLVM::LLVMVoidType::get(op.getContext());
    const auto param = mlir::LLVM::LLVMPointerType::get(op->getContext());

    std::string funcName;
    llvm::SmallVector<mlir::Value, 2> args{adaptor.getTarget()};
    llvm::SmallVector<mlir::Type, 2> params{param};

    if (auto ctrl = op.getControl()) {
      funcName = getQIRCallableName(opName, "ctl");
      args.push_back(adaptor.getControl());
      params.push_back(param);
    } else {
      funcName = getQIRCallableName(opName, "body");
    }

    const auto funcType = mlir::LLVM::LLVMFunctionType::get(voidType, params);
    const auto funcOp =
        moduleOp.template lookupSymbol<mlir::LLVM::LLVMFuncOp>(funcName);
    if (!funcOp) {
      mlir::OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(moduleOp.getBody());
      rewriter.create<mlir::LLVM::LLVMFuncOp>(op->getLoc(), funcName, funcType);
    }

    rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, funcType, funcName,
                                                    args);

    return mlir::success();
  }
};

struct HOpLowering : OptionallyControlledUnitaryOpLowering<qpin::HOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::HOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::HOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "h");
  }
};

struct XOpLowering : OptionallyControlledUnitaryOpLowering<qpin::XOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::XOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::XOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "x");
  }
};

struct YOpLowering : OptionallyControlledUnitaryOpLowering<qpin::YOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::YOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::YOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "y");
  }
};

struct ZOpLowering : OptionallyControlledUnitaryOpLowering<qpin::ZOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::ZOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::ZOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "z");
  }
};

struct SOpLowering : OptionallyControlledUnitaryOpLowering<qpin::SOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::SOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::SOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "s");
  }
};

struct TOpLowering : OptionallyControlledUnitaryOpLowering<qpin::TOp> {
  using OptionallyControlledUnitaryOpLowering<
      qpin::TOp>::OptionallyControlledUnitaryOpLowering;

  mlir::LogicalResult
  matchAndRewrite(qpin::TOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    return matchAndRewriteImpl(op, adaptor, rewriter, "t");
  }
};

struct SwapOpLowering : mlir::OpConversionPattern<qpin::SwapOp> {
  using mlir::OpConversionPattern<qpin::SwapOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::SwapOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto moduleOp = op->getParentOfType<mlir::ModuleOp>();
    if (!moduleOp) {
      return mlir::failure();
    }

    const auto voidType = mlir::LLVM::LLVMVoidType::get(op.getContext());
    const auto param = mlir::LLVM::LLVMPointerType::get(op->getContext());

    const std::string funcName = getQIRCallableName("swap", "body");
    const llvm::SmallVector<mlir::Value, 2> args{adaptor.getA(),
                                                 adaptor.getB()};
    const llvm::SmallVector<mlir::Type, 2> params{param, param};

    const auto funcType = mlir::LLVM::LLVMFunctionType::get(voidType, params);
    
    const auto funcOp = moduleOp.lookupSymbol<mlir::LLVM::LLVMFuncOp>(funcName);
    if (!funcOp) {
      mlir::OpBuilder::InsertionGuard guard(rewriter);
      rewriter.setInsertionPointToStart(moduleOp.getBody());
      rewriter.create<mlir::LLVM::LLVMFuncOp>(op->getLoc(), funcName, funcType);
    }

    rewriter.replaceOpWithNewOp<mlir::LLVM::CallOp>(op, funcType, funcName,
                                                    args);

    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Conversion Entry
//===----------------------------------------------------------------------===//

namespace {
struct QPinTypeConverter : mlir::TypeConverter {
  QPinTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
    addConversion([](StaticQubitType type) {
      return mlir::LLVM::LLVMPointerType::get(type.getContext());
    });
  }
};
} // namespace

/// @brief QPin to LLVM Dialect Conversion Pass
struct QPinToLLVM : impl::QPinToLLVMBase<QPinToLLVM> {
  using QPinToLLVMBase::QPinToLLVMBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QPinDialect>();
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();

    QPinTypeConverter typeConverter(context);
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