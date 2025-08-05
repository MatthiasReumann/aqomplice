#ifndef AQOMPLICE_COMMON_INTERFACES_TD
#define AQOMPLICE_COMMON_INTERFACES_TD

include "mlir/IR/OpBase.td"

def UnitaryOpInterface : OpInterface<"UnitaryOpInterface"> {
  let description = [{}];
  let methods = [
    InterfaceMethod<
      "Return primary qubit.", 
      "mlir::Value", "getPrimary"
    >,
    InterfaceMethod<
      "Returns secondary qubit.", 
      "mlir::Value", "getSecondary"
    >,
    InterfaceMethod<
      "Returns true if the op uses two qubits.", 
      "bool", "hasSecondary"
    >,
    InterfaceMethod<
      "Returns true if the op has the ctrld flag.", 
      "bool", "isControlled"
    >
  ];
  let cppNamespace = "::aqomplice";
}

#endif // AQOMPLICE_COMMON_INTERFACES_TD