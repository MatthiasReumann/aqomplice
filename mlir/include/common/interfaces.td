#ifndef AQOMPLICE_COMMON_INTERFACES_TD
#define AQOMPLICE_COMMON_INTERFACES_TD

include "mlir/IR/OpBase.td"

def ControlledUnitaryOpInterface : OpInterface<"ControlledUnitaryOpInterface"> {
  let description = [{}];
  let methods = [
    InterfaceMethod<
      "Return the target qubit.", 
      "mlir::Value", "getTarget"
    >,
    InterfaceMethod<
      "Return the control qubit.", 
      "mlir::Value", "getControl"
    >,
    InterfaceMethod<
      "Returns true if the op has a control qubit.", 
      "bool", "hasControl"
    >
  ];
  let cppNamespace = "::aqomplice";
}

#endif // AQOMPLICE_COMMON_INTERFACES_TD