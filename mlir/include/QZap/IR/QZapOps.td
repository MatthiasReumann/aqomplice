#ifndef QZAP_OPS
#define QZAP_OPS

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "meta/traits.td"

include "QZap/IR/QZapDialect.td"
include "QZap/IR/QZapTypes.td"

class QZap_Op<string mnemonic, list<Trait> traits = []>
    : Op<QZap_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

def KernelOp : QZap_Op<"kernel", [FunctionOpInterface, IsolatedFromAbove]> {
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
}

def ReturnOp : QZap_Op<"return", [Pure, HasParent<"KernelOp">, Terminator, ReturnLike]> {
  let summary = "return from quantum kernel";
  let description = [{}];
  let arguments = (ins MeasurementResultType:$input);
  let assemblyFormat = " attr-dict $input `:` type($input) ";
}

def CallOp : QZap_Op<"call", [DeclareOpInterfaceMethods<CallOpInterface>]> {
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
    : QZap_Op<mnemonic, traits>;

def AllocOp : Memory_Op<"allocreg", [HasParent<"KernelOp">]> {
  let summary = "allocate n qubits into a quantum register";
  let description = [{
    Ensures implicitly that all other quantum 
    operations are placed inside a kernel.
  }];
  let arguments = (ins I64:$nqubits);
  let results = (outs QubitArrayType:$qreg);
  let assemblyFormat = " attr-dict $nqubits ";
}

def FreeOp : Memory_Op<"freereg"> {
  let summary = "deallocate a quantum register";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg);
  let assemblyFormat = " attr-dict $qreg ";
}

def RetrieveOp : Memory_Op<"retrieve"> {
  let summary = "retrieve a qubit from a quantum register at a given index";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg_in, I64:$index);
  let results = (outs QubitArrayType:$qreq_out, QubitType:$qubit);
  let assemblyFormat = " attr-dict $qreg_in `[` $index `]` ";
}

def StoreOp : Memory_Op<"store"> {
  let summary = "store a qubit in a quantum register at a given index";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg_in, I64:$index, QubitType:$qubit);
  let results = (outs QubitArrayType:$qreg_out);
  let assemblyFormat = " attr-dict $qreg_in `[` $index `]` $qubit";
}

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

def MeasureOp : QZap_Op<"measurereg"> {
  let summary = "measure in the computational basis";
  let description = [{}];
  let arguments = (ins QubitArrayType:$target);
  let results = (outs MeasurementResultType:$res);
  let assemblyFormat = " attr-dict $target `->` type($res)";
}

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

class Gate_Op<string mnemonic, list<Trait> traits = []>
    : QZap_Op<mnemonic, traits>;

class Unitary_Op<string mnemonic, list<Trait> traits = []> 
  : Gate_Op<mnemonic, traits#[Unitary]> {
  let arguments = (ins QubitType:$target_in, Optional<QubitType>:$control_in);
  let results = (outs QubitType:$target_out, Optional<QubitType>:$control_out);
  let assemblyFormat = " attr-dict $target_in ( `ctrl` $control_in^ )? `->` type($target_out) ( `,` type($control_out)^ )? ";
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

#endif // QZAP_OPS