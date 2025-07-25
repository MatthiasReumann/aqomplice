#include "Conversion/qpin-to-llvm/qpin-to-llvm.h"

#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Func/Transforms/FuncConversions.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Transforms/DialectConversion.h"
#include <cassert>

namespace aqomplice {
namespace qpin {
#define GEN_PASS_DEF_QPINTOLLVM
#include "Conversion/qpin-to-llvm/qpin-to-llvm.h.inc" // adds `impl::QPinToLLVMBase`

//===----------------------------------------------------------------------===//
// Conversion Entry
//===----------------------------------------------------------------------===//

/// @brief QPin to LLVM Dialect Conversion Pass
struct QPinToLLVM : impl::QPinToLLVMBase<QPinToLLVM> {
  using QPinToLLVMBase::QPinToLLVMBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();

    mlir::ConversionTarget target(*context);
    target.addIllegalDialect<QPinDialect>();
    target.addLegalDialect<mlir::LLVM::LLVMDialect>();

    mlir::RewritePatternSet patterns(context);

    if (failed(applyPartialConversion(getOperation(), target,
                                      std::move(patterns)))) {
      signalPassFailure();
    }
  }
};
}; // namespace qpin
}; // namespace aqomplice