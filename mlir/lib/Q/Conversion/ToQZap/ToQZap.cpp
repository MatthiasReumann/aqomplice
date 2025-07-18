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
    addConversion([ctx](q::QubitArrayType type) {
      return qzap::QubitArrayType::get(ctx);
    });
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

struct AllocOpLowering : public mlir::OpConversionPattern<q::AllocOp> {
  using OpConversionPattern<q::AllocOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::AllocOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    if (op->getResultTypes().size() > 1) {
      return rewriter.notifyMatchFailure(op, [](mlir::Diagnostic &diag) {
        diag << "expected 'allocOp' to have exactly one result type";
      });
    }
    
    auto qzapQubitArray =
        typeConverter->convertType(op->getResultTypes().front());
    rewriter.replaceOpWithNewOp<qzap::AllocOp>(op, qzapQubitArray,
                                               adaptor.getNqubits());
    return mlir::success();
  }
};

struct FreeOpLowering : public mlir::OpConversionPattern<q::FreeOp> {
  using OpConversionPattern<q::FreeOp>::OpConversionPattern;

  mlir::LogicalResult
  matchAndRewrite(q::FreeOp op, OpAdaptor adaptor,
                  mlir::ConversionPatternRewriter &rewriter) const final {
    rewriter.replaceOpWithNewOp<qzap::FreeOp>(op, adaptor.getQreg());
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
    patterns.add<KernelOpLowering, ReturnOpLowering, CallOpLowering,
                 AllocOpLowering, FreeOpLowering>(typeConverter, context);

    if (failed(applyPartialConversion(op, target, std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace q
}; // namespace aqomplice