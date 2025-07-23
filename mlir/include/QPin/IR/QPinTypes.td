#ifndef QPIN_TYPES
#define QPIN_TYPES

include "QPin/IR/QPinDialect.td"
include "mlir/IR/CommonTypeConstraints.td"

class QPin_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<QPin_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def QubitType : QPin_Type<"Qubit", "Qubit"> {
    let summary = "a qubit";
}

#endif // QPIN_TYPES