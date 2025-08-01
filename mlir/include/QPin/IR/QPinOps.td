#ifndef AQOMPLICE_QPIN_OPS_TD
#define AQOMPLICE_QPIN_OPS_TD

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "common/traits.td"

include "QPin/IR/QPinDialect.td"
include "QPin/IR/QPinTypes.td"

class QPin_Op<string mnemonic, list<Trait> traits = []>
    : Op<QPin_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

def KernelOp : QPin_Op<"kernel", [FunctionOpInterface, IsolatedFromAbove]> {
  let summary = "quantum kernel";
  let arguments = (ins SymbolNameAttr:$sym_name,
      TypeAttrOf<FunctionType>:$function_type,
      OptionalAttr<DictArrayAttr>:$arg_attrs,
      OptionalAttr<DictArrayAttr>:$res_attrs);
  let regions = (region AnyRegion:$body);

  let builders = [OpBuilder<(ins "mlir::StringRef":$name,
      "mlir::FunctionType":$type,
      CArg<"mlir::ArrayRef<mlir::NamedAttribute>", "{}">:$attrs)>];

  // Add FunctionOpInterface methods
  let extraClassDeclaration = [{
        mlir::ArrayRef<mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }
        mlir::ArrayRef<mlir::Type> getResultTypes() { return getFunctionType().getResults(); }
        mlir::Region *getCallableRegion() { return &getBody(); }
    }];

  let hasCustomAssemblyFormat = 1;
  let skipDefaultBuilders = 1;
  let hasRegionVerifier = 1;
}

def ReturnOp : QPin_Op<"return", [Pure, HasParent<"KernelOp">, Terminator, ReturnLike]> {
  let summary = "return from quantum kernel";
  let description = [{}];
  let arguments = (ins Variadic<AnyType>:$operands);
  let assemblyFormat = "attr-dict ($operands^ `:` type($operands))?";
  let hasVerifier = 1;
}

def CallOp : QPin_Op<"call", [DeclareOpInterfaceMethods<CallOpInterface>]> {
  let summary = "call quantum kernel";
  let description = [{}];
  let arguments = (ins
    FlatSymbolRefAttr:$callee,
    Variadic<AnyType>:$operands,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );
  let results = (outs Variadic<AnyType>);
  let assemblyFormat = [{ 
    $callee `(` $operands `)` attr-dict `:` functional-type($operands, results)
  }];
}

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

def QubitOp : QPin_Op<"qubit"> {
  let summary = "static qubit address";
  let description = [{}];
  let arguments = (ins I32Attr:$index);
  let results = (outs StaticQubitType:$qubit);
  let assemblyFormat = "attr-dict $index";
}

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

def MeasureOp : QPin_Op<"measure"> {
  let summary = "measure in the computational basis";
  let description = [{}];
  let arguments = (ins StaticQubitType:$qubit);
  let results = (outs I1:$bit);
  let assemblyFormat = " attr-dict $qubit";
}

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

class Gate_Op<string mnemonic, list<Trait> traits = []>
    : QPin_Op<mnemonic, traits>;

class Unitary_Op<string mnemonic, list<Trait> traits = []> 
  : Gate_Op<mnemonic, traits#[Unitary]>;

def ControlledUnitaryOpInterface : OpInterface<"ControlledUnitaryOpInterface"> {
  let description = [{}];
  let methods = [
    InterfaceMethod<
      "Return the target qubit.", 
      "mlir::Value", "getTarget"
    >,
    InterfaceMethod<
      "Returns the control qubit.", 
      "mlir::Value", "getControl"
    >,
    InterfaceMethod<
      "Returns true if the op has a control qubit.", 
      "bool", "hasControl"
    >
  ];
  let cppNamespace = "::aqomplice::qpin";
}

class OptionallyControlledUnitary_Op<string mnemonic, list<Trait> traits = []> 
  : Unitary_Op<mnemonic, traits#[ControlledUnitaryOpInterface]> {
  let arguments = (ins StaticQubitType:$target, Optional<StaticQubitType>:$control);
  let results = (outs);
  let assemblyFormat = " attr-dict $target ( `ctrl` $control^ )?";
  
  // Add UnitaryOpInterface methods
  let extraClassDeclaration = [{
    bool hasControl() { 
      if (auto ctrl = getControl()) { return true; }
      return false; 
    }
  }];
}

def HOp : OptionallyControlledUnitary_Op<"h", [Hermitian]> {
  let summary = "apply H operation to target qubit";
  let description = [{}];
}

def XOp : OptionallyControlledUnitary_Op<"x", [Hermitian]> {
  let summary = "apply X operation to target qubit";
  let description = [{}];
}

def YOp : OptionallyControlledUnitary_Op<"y", [Hermitian]> {
  let summary = "apply Y operation to target qubit";
  let description = [{}];
}

def ZOp : OptionallyControlledUnitary_Op<"z", [Hermitian]> {
  let summary = "apply Z operation to target qubit";
  let description = [{}];
}

def SOp : OptionallyControlledUnitary_Op<"s"> {
  let summary = "apply S operation to target qubit";
  let description = [{}];
}

def TOp : OptionallyControlledUnitary_Op<"t"> {
  let summary = "apply T operation to target qubit";
  let description = [{}];
}

def SwapOp : Unitary_Op<"swap", [Hermitian]> {
  let summary = "apply swap operation to qubits a and b";
  let description = [{}];
  let arguments = (ins StaticQubitType:$a, StaticQubitType:$b);
  let results = (outs);
  let assemblyFormat = " attr-dict $a `,` $b";
}

#endif // AQOMPLICE_QPIN_OPS_TD