#ifndef Q_OPS
#define Q_OPS

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "meta/traits.td"

include "Q/IR/QDialect.td"
include "Q/IR/QTypes.td"

class Q_Op<string mnemonic, list<Trait> traits = []>
    : Op<Q_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

def KernelOp : Q_Op<"kernel", [FunctionOpInterface, IsolatedFromAbove]> {
  let summary = "quantum kernel";
  let arguments = (ins SymbolNameAttr:$sym_name,
      TypeAttrOf<FunctionType>:$function_type,
      OptionalAttr<DictArrayAttr>:$arg_attrs,
      OptionalAttr<DictArrayAttr>:$res_attrs);
  let regions = (region AnyRegion:$body);

  let builders = [
      OpBuilder<(ins "mlir::StringRef":$name,
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
}

def ReturnOp : Q_Op<"return", [Pure, HasParent<"KernelOp">, Terminator, ReturnLike]> {
  let summary = "return from quantum kernel";
  let description = [{}];
  let arguments = (ins MeasurementResultType:$input);
  let assemblyFormat = " attr-dict $input `:` type($input) ";
}

def CallOp : Q_Op<"call", [DeclareOpInterfaceMethods<CallOpInterface>]> {
  let summary = "call quantum kernel";
  let description = [{}];
  let arguments = (ins
    FlatSymbolRefAttr:$callee,
    Variadic<AnyType>:$operands,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );
  let results = (outs MeasurementResultType);
  let assemblyFormat = [{ 
    $callee `(` $operands `)` attr-dict `:` functional-type($operands, results)
  }];
}

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

class Memory_Op<string mnemonic, list<Trait> traits = []>
    : Q_Op<mnemonic, traits>;

def AllocOp : Memory_Op<"alloc", [HasParent<"KernelOp">]> {
  let summary = "allocate n qubits into a quantum register";
  let description = [{
    Ensures implicitly that all other quantum 
    operations are placed inside a kernel.
  }];
  let arguments = (ins I32:$nqubits);
  let results = (outs QubitArrayType:$qreg);
  let assemblyFormat = " attr-dict $nqubits ";
}

def FreeOp : Memory_Op<"free"> {
  let summary = "deallocate a quantum register";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg);
  let assemblyFormat = " attr-dict $qreg ";
}

def RetrieveOp : Memory_Op<"retrieve"> {
  let summary = "retrieve a qubit from a quantum register at a given index";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg, I32:$index);
  let results = (outs QubitType:$qubit);
  let assemblyFormat = " attr-dict $qreg `[` $index `]` ";
}

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

def MeasureOp : Q_Op<"measure"> {
  let summary = "measure in the computational basis";
  let description = [{}];
  let arguments = (ins QubitType:$qubit);
  let results = (outs I1:$bit);
  let assemblyFormat = " attr-dict $qubit";
}

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

class Gate_Op<string mnemonic, list<Trait> traits = []>
    : Q_Op<mnemonic, traits>;

class Unitary_Op<string mnemonic, list<Trait> traits = []> 
  : Gate_Op<mnemonic, traits#[Unitary]> {
  let arguments = (ins QubitType:$target, Optional<QubitType>:$control);
  let results = (outs);
  let assemblyFormat = " attr-dict $target ( `ctrl` $control^ )?";
}

def HOp : Unitary_Op<"h", [Hermitian]> {
  let summary = "apply H operation to target qubit";
  let description = [{}];
}

def XOp : Unitary_Op<"x", [Hermitian]> {
  let summary = "apply X operation to target qubit";
  let description = [{}];
}

def YOp : Unitary_Op<"y", [Hermitian]> {
  let summary = "apply Y operation to target qubit";
  let description = [{}];
}

def ZOp : Unitary_Op<"z", [Hermitian]> {
  let summary = "apply Z operation to target qubit";
  let description = [{}];
}

#endif // Q_OPS