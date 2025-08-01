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
  : Gate_Op<mnemonic, traits#[Unitary]> {
  let arguments = (ins QubitType:$a, Optional<QubitType>:$b);
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
  let summary = "apply SWAP operation";
  let assemblyFormat = " attr-dict $a `,` $b";
}

#endif // AQOMPLICE_Q_OPS_TD