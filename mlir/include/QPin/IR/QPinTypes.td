#ifndef AQOMPLICE_QPIN_TYPES_TD
#define AQOMPLICE_QPIN_TYPES_TD

include "QPin/IR/QPinDialect.td"
include "mlir/IR/CommonTypeConstraints.td"

class QPin_Type<string name, string typeMnemonic, list<Trait> traits = []>
        : TypeDef<QPin_Dialect, name, traits> {
    let mnemonic = typeMnemonic;
}

def StaticQubitType : QPin_Type<"StaticQubit", "StaticQubit"> {
    let summary = "a static (device) qubit";
}

#endif // AQOMPLICE_QPIN_TYPES_TD