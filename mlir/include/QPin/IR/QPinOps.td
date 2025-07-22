#ifndef QPIN_OPS
#define QPIN_OPS

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"
include "mlir/Interfaces/ControlFlowInterfaces.td"
include "mlir/IR/SymbolInterfaces.td"

include "meta/traits.td"

include "QPin/IR/QPinDialect.td"
include "QPin/IR/QPinTypes.td"

class QPin_Op<string mnemonic, list<Trait> traits = []>
    : Op<QPin_Dialect, mnemonic, traits>;

//===----------------------------------------------------------------------===//
// Kernel Operations
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Quantum Register Operations
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Measurement Operations
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Gate Operations
//===----------------------------------------------------------------------===//

#endif // QPIN_OPS