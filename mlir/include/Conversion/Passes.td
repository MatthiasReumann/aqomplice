#ifndef AQOMPLICE_CONVERSION_PASSES_TD
#define AQOMPLICE_CONVERSION_PASSES_TD

include "mlir/Pass/PassBase.td"

def QToQZap : Pass<"q-to-qzap"> {
  let summary = "Lower `q` to `qzap`";
  let description = [{}];
  let dependentDialects = [
    "aqomplice::qzap::QZapDialect"
  ];
}

def QZapToQPin : Pass<"qzap-to-qpin"> {
  let summary = "Lower `qzap` to `qpin`";
  let description = [{}];
  let dependentDialects = [
    "aqomplice::qpin::QPinDialect"
  ];
}

def QPinToLLVM : Pass<"qpin-to-llvm"> {
  let summary = "lower `qpin` to `llvm` dialect";
  let description = [{}];
  let dependentDialects = [
    "mlir::LLVM::LLVMDialect"
  ];
}

def QPinToFunc : Pass<"qpin-to-func"> {
  let summary = "lower `qpin` kernel ops to `func` dialect ops";
  let description = [{}];
  let dependentDialects = [
    "mlir::func::FuncDialect"
  ];
}

#endif // AQOMPLICE_CONVERSION_PASSES_TD