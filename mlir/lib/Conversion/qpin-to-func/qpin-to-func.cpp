#include "Conversion/qpin-to-func/qpin-to-func.h"

#include "QPin/IR/QPinDialect.h"

#include "mlir/Conversion/LLVMCommon/TypeConverter.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Transforms/DialectConversion.h"
#include "llvm/Support/FormatVariadic.h"
#include <cassert>

namespace aqomplice {
#define GEN_PASS_DEF_QPINTOFUNC
#include "Conversion/Passes.h.inc" // adds `impl::QPinToFuncBase`

namespace qpin {
namespace {
std::string getQIRCallableName(const std::string &op,
                               const std::string &specialization) {
  return llvm::formatv("__quantum__qis__{0}__{1}", op, specialization);
}
} // namespace

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

struct KernelOpLowering {};

//===----------------------------------------------------------------------===//
// Conversion Entry
//===----------------------------------------------------------------------===//

namespace {
struct QPinTypeConverter : mlir::TypeConverter {
  QPinTypeConverter(mlir::MLIRContext *ctx) {
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

    // A kernel implements the FuncOpInterface. To avoid
    // reimplementing (or worse, copying) the conversion to
    // LLVM IR, kernel operations require a conversion to the
    // mlir::func::FuncDialect first (--qpin-to-func).
    target.addIllegalOp<qpin::KernelOp>();
    target.addIllegalOp<qpin::ReturnOp>();
    target.addIllegalOp<qpin::CallOp>();

    QPinTypeConverter typeConverter(context);
    mlir::LLVMTypeConverter llvmTypeConverter(context);

    mlir::RewritePatternSet patterns(context);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace qpin
}; // namespace aqomplice