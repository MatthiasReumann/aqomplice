#ifndef AQOMPLICE_QPIN_OPS_TD
#define AQOMPLICE_QPIN_OPS_TD

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "common/traits.td"
include "common/interfaces.td"

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
  : Gate_Op<mnemonic, traits#[Unitary]> {
  let arguments = (ins StaticQubitType:$a, Optional<StaticQubitType>:$b);
  let results = (outs);
}

class CUnitary_Op<string mnemonic, list<Trait> traits = []> 
  : Unitary_Op<mnemonic, traits#[ControlledUnitaryOpInterface]> {
   let extraClassDeclaration = [{
        mlir::Value getTarget() { return getA(); }
        mlir::Value getControl() { return getB(); }
        bool hasControl() { return getB() != nullptr; }
  }];
  let assemblyFormat = " attr-dict $a ( `ctrl` $b^ )?";
}

def HOp : CUnitary_Op<"h", [Hermitian]> {
  let summary = "apply H operation";
}

def XOp : CUnitary_Op<"x", [Hermitian]> {
  let summary = "apply X operation";
}

def YOp : CUnitary_Op<"y", [Hermitian]> {
  let summary = "apply Y operation";
}

def ZOp : CUnitary_Op<"z", [Hermitian]> {
  let summary = "apply Z operation";
}

def SOp : CUnitary_Op<"s"> {
  let summary = "apply S operation";
}

def TOp : CUnitary_Op<"t"> {
  let summary = "apply T operation";
}

def SwapOp : Unitary_Op<"swap", [Hermitian]> {
  let summary = "apply swap operation";
  let assemblyFormat = " attr-dict $a `,` $b";
}

#endif // AQOMPLICE_QPIN_OPS_TD