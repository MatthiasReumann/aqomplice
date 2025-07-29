#ifndef AQOMPLICE_TRANSFORM_PASSES_TD
#define AQOMPLICE_TRANSFORM_PASSES_TD

include "mlir/Pass/PassBase.td"

def FitTopology : Pass<"fit-topology", "qpin::KernelOp"> {
  let summary = "Fit quantum operations in qpin.kernel to hardware topology.";
  let description = [{}];
  let options = [
    Option<"arch", "arch", "std::string", /*default=*/"",
           "The name of the architecture.">,
  ];
  let dependentDialects = [
    "aqomplice::qpin::QPinDialect"
  ];
}

#endif // AQOMPLICE_TRANSFORM_PASSES_TD