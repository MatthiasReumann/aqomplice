#include "Conversion/qpin-to-func/qpin-to-func.h"

#include "QPin/IR/QPinDialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>

namespace aqomplice {
#define GEN_PASS_DEF_QPINTOFUNC
#include "Conversion/Passes.h.inc" // adds `impl::QPinToFuncBase`

namespace qpin {
namespace {

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

struct KernelOpLowering : mlir::OpConversionPattern<qpin::KernelOp> {
  using OpConversionPattern<qpin::KernelOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::KernelOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    auto func = rewriter.create<mlir::func::FuncOp>(op.getLoc(), op.getName(),
                                                    op.getFunctionType());
    func->setAttr("target", mlir::StringAttr::get(op->getContext(), "qpu"));
    func.setNoInline(true); // QPU kernels can't be inlined.

    rewriter.inlineRegionBefore(op.getRegion(), func.getBody(), func.end());
    rewriter.eraseOp(op);

    return mlir::success();
  }
};

struct ReturnOpLowering : mlir::OpConversionPattern<qpin::ReturnOp> {
  using OpConversionPattern<qpin::ReturnOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::ReturnOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<mlir::func::ReturnOp>(
        op, op->getResultTypes(), op->getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct CallOpLowering : mlir::OpConversionPattern<qpin::CallOp> {
  using OpConversionPattern<qpin::CallOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(qpin::CallOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<mlir::func::CallOp>(
        op, op.getCallee(), op->getResultTypes(), op->getOperands());
    return mlir::success();
  }
};

//===----------------------------------------------------------------------===//
// Type Converter
//===----------------------------------------------------------------------===//

struct ConversionTypeConverter : mlir::TypeConverter {
  ConversionTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
  }
};
} // namespace

struct QPinToFunc : impl::QPinToFuncBase<QPinToFunc> {
  using QPinToFuncBase::QPinToFuncBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::ConversionTarget target(*context);
    target.addLegalDialect<mlir::func::FuncDialect>();
    target.addIllegalDialect<QPinDialect>();

    ConversionTypeConverter typeConverter(context);

    mlir::RewritePatternSet patterns(context);
    patterns.add<KernelOpLowering, ReturnOpLowering, CallOpLowering>(
        typeConverter, context);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace qpin
}; // namespace aqomplice