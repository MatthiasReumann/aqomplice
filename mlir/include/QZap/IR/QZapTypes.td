#ifndef QZAP_TYPES
#define QZAP_TYPES

include "QZap/IR/QZapDialect.td"
include "mlir/IR/CommonTypeConstraints.td"

class QZap_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<QZap_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def QubitType : QZap_Type<"Qubit", "Qubit"> {
    let summary = "a qubit";
}

def QubitArrayType : QZap_Type<"QubitArray", "QubitArray"> {
    let summary = "a qubit-array";
}

def BitArrayType : MemRefOf<[I1]> {
    let summary = "a bit-array";
}

def MeasurementResultType : AnyTypeOf<[I1, BitArrayType]> {}

#endif // QZAP_TYPES