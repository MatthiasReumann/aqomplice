#ifndef Q_TYPES
#define Q_TYPES

include "Q/IR/QDialect.td"

class Q_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<Q_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def QubitType : Q_Type<"Qubit", "Qubit"> {
    let summary = "A qubit.";
}

def QuantumRegisterType : Q_Type<"QubitRegister", "QubitRegister"> {
    let summary = "An array of qubits.";
}

#endif // Q_TYPES