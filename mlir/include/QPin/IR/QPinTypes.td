#ifndef QPIN_TYPES
#define QPIN_TYPES

include "QPin/IR/QPinDialect.td"
include "mlir/IR/CommonTypeConstraints.td"

class QPin_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<QPin_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def BitArrayType : MemRefOf<[I1]> {
    let summary = "a bit-array";
}

def MeasurementResultType : AnyTypeOf<[I1, BitArrayType]> {}

#endif // QPIN_TYPES