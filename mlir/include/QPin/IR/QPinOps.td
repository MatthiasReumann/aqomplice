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
// Quantum Register Operations
//===----------------------------------------------------------------------===//

def QubitOp : QPin_Op<"qubit", [InsideQPUKernel]> {
  let summary = "Static qubit address";
  let description = [{
     The `qpin.qubit` operation produces an SSA value from the given index
     to a static (device) qubit.
  }];
  let arguments = (ins I32Attr:$index);
  let results = (outs StaticQubitType:$qubit);
  let assemblyFormat = "attr-dict $index";
}

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

def MeasureOp : QPin_Op<"measure"> {
  let summary = "Measure in the computational basis";
   let description = [{
      The `qpin.measure` operation represents the measurement operation. It takes a
      qubit as input and returns the measured bit. The returned bit indicates whether 
      the respective qubit collapsed to |0> or |1>.

      Example:
      ```mlir
      %b = "qpin.measure"(%q) : (!qpin.StaticQubit) -> (i1)
      ```
    }];
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
  : Gate_Op<mnemonic, traits#[Unitary, UnitaryOpInterface]> {
  let arguments = (ins
    StaticQubitType:$primary,
    Optional<StaticQubitType>:$secondary,
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

#endif // AQOMPLICE_QPIN_OPS_TD