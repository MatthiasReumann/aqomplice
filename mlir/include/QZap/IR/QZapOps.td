#ifndef QZAP_OPS
#define QZAP_OPS

include "QZap/IR/QZapDialect.td"
include "QZap/IR/QZapTypes.td"

class QZap_Op<string mnemonic, list<Trait> traits = []>
    : Op<QZap_Dialect, mnemonic, traits>;

#endif // QZAP_OPS