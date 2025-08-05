#ifndef AQOMPLICE_QZAP_OPS_TD
#define AQOMPLICE_QZAP_OPS_TD

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "common/traits.td"
include "common/interfaces.td"

include "QZap/IR/QZapDialect.td"
include "QZap/IR/QZapTypes.td"

class QZap_Op<string mnemonic, list<Trait> traits = []>
    : Op<QZap_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

class Memory_Op<string mnemonic, list<Trait> traits = []>
    : QZap_Op<mnemonic, traits>;

def AllocOp : Memory_Op<"alloc", [InsideQPUKernel]> {
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
  let arguments = (ins QubitArrayType:$qreg_in, I32:$index);
  let results = (outs QubitArrayType:$qreq_out, QubitType:$qubit);
  let assemblyFormat = " attr-dict $qreg_in `[` $index `]` ";
}

def StoreOp : Memory_Op<"store"> {
  let summary = "store a qubit in a quantum register at a given index";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg_in, I32:$index, QubitType:$qubit);
  let results = (outs QubitArrayType:$qreg_out);
  let assemblyFormat = " attr-dict $qubit `,` $qreg_in `[` $index `]`";
}

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

def MeasureOp : QZap_Op<"measure"> {
  let summary = "measure in the computational basis";
  let description = [{}];
  let arguments = (ins QubitType:$qubit_in);
  let results = (outs I1:$bit, QubitType:$qubit_out);
  let assemblyFormat = " attr-dict $qubit_in `:` functional-type(operands, results) ";
}

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

class Gate_Op<string mnemonic, list<Trait> traits = []>
    : QZap_Op<mnemonic, traits>;

class Unitary_Op<string mnemonic, list<Trait> traits = []> 
  : Gate_Op<mnemonic, traits#[Unitary, UnitaryOpInterface]> {
  let arguments = (ins
    QubitType:$primary_in, 
    Optional<QubitType>:$secondary_in, 
    UnitAttr:$ctrld
  );
  
  let results = (outs
    QubitType:$primary_out,
    Optional<QubitType>:$secondary_out
  );

  let extraClassDeclaration = [{
    mlir::Value getPrimary() { return getPrimaryIn(); }
    mlir::Value getSecondary() { return getSecondaryIn(); }
    bool hasSecondary() { return getSecondaryIn() != nullptr; }
    bool isControlled() { return getCtrld(); }
  }];

  let assemblyFormat = " attr-dict (`ctrld` $ctrld^)? $primary_in ( `,` $secondary_in^ )? `:` functional-type(operands, results) ";
}

def HOp : Unitary_Op<"h", [Hermitian]> {
  let summary = "H operation";
}

def XOp : Unitary_Op<"x", [Hermitian]> {
  let summary = "X operation";
}

def YOp : Unitary_Op<"y", [Hermitian]> {
  let summary = "Y operation";
}

def ZOp : Unitary_Op<"z", [Hermitian]> {
  let summary = "Z operation";
}

def SOp : Unitary_Op<"s"> {
  let summary = "S operation";
}

def TOp : Unitary_Op<"t"> {
  let summary = "T operation";
}

def SwapOp : Unitary_Op<"swap", [Hermitian]> {
  let summary = "SWAP operation";
}

#endif // AQOMPLICE_QZAP_OPS_TD