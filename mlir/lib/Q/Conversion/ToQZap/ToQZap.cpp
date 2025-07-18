#include "Q/Conversion/ToQZap/ToQZap.h"

#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"

namespace aqomplice {
namespace q {
#define GEN_PASS_DEF_QTOQZAP
#include "Q/Conversion/ToQZap/QToQZap.h.inc" // adds `impl::QToQZapBase`

class QToQZapTypeConverter : public mlir::TypeConverter {
public:
  QToQZapTypeConverter(mlir::MLIRContext *ctx) {
    addConversion([](mlir::Type type) { return type; });
  }
};

struct KernelOpLowering : public mlir::OpConversionPattern<q::KernelOp> {
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

struct ReturnOpLowering : public mlir::OpConversionPattern<q::ReturnOp> {
  using OpConversionPattern<q::ReturnOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::ReturnOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qzap::ReturnOp>(
        op, op->getResultTypes(), op->getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct CallOpLowering : public mlir::OpConversionPattern<q::CallOp> {
  using OpConversionPattern<q::CallOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::CallOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qzap::CallOp>(op, op->getResultTypes(),
                                              op.getOperands(), op->getAttrs());
    return mlir::success();
  }
};

struct QTQZap : impl::QToQZapBase<QTQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QDialect>();
    target.addLegalDialect<qzap::QZapDialect, mlir::func::FuncDialect>();

    QToQZapTypeConverter typeConverter(context);
    mlir::RewritePatternSet patterns(context);
    patterns.add<KernelOpLowering, ReturnOpLowering, CallOpLowering>(context);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace q
}; // namespace aqomplice