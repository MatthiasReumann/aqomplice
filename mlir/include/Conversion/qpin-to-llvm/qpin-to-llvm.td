#ifndef QPIN_CONVERSION_TO_LLVM
#define QPIN_CONVERSION_TO_LLVM

include "mlir/Pass/PassBase.td"

def QPinToLLVM : Pass<"qpin-to-llvm", "KernelOp"> {
  let summary = "lower `qpin` to `llvm` dialect";
  let description = [{}];
  let dependentDialects = [
    "mlir::LLVM::LLVMDialect"
  ];
}

#endif // QPIN_CONVERSION_TO_LLVM