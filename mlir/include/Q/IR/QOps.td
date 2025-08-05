#ifndef AQOMPLICE_Q_OPS_TD
#define AQOMPLICE_Q_OPS_TD

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "common/traits.td"
include "common/interfaces.td"

include "Q/IR/QDialect.td"
include "Q/IR/QTypes.td"

class Q_Op<string mnemonic, list<Trait> traits = []>
    : Op<Q_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

class Memory_Op<string mnemonic, list<Trait> traits = []>
    : Q_Op<mnemonic, traits>;

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
  : Gate_Op<mnemonic, traits#[Unitary, UnitaryOpInterface]> {
  let arguments = (ins
    QubitType:$primary,
    Optional<QubitType>:$secondary,
    UnitAttr:$ctrld);
  let results = (outs);
  let extraClassDeclaration = [{
        bool hasSecondary() { return getSecondary() != nullptr; }
        bool isControlled() { return getCtrld(); }
  }];
  let assemblyFormat = " attr-dict (`ctrld` $ctrld^)? $primary (`,` $secondary^)? ";
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

#endif // AQOMPLICE_Q_OPS_TD