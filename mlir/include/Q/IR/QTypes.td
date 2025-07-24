#ifndef Q_TYPES
#define Q_TYPES

include "Q/IR/QDialect.td"
include "mlir/IR/CommonTypeConstraints.td"

class Q_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<Q_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def QubitType : Q_Type<"Qubit", "Qubit"> {
    let summary = "a qubit";
}

def QubitArrayType : Q_Type<"QubitArray", "QubitArray"> {
    let summary = "a qubit-array";
}

#endif // Q_TYPES