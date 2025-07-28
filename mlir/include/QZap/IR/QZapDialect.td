#ifndef AQOMPLICE_QZAP_DIALECT_TD
#define AQOMPLICE_QZAP_DIALECT_TD

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"

def QZap_Dialect : Dialect {
    let name = "qzap";
    let summary = "zap quantum dialect";
    let description = [{
        Enforces SSA form for optimizations.
    }];
    let cppNamespace = "::aqomplice::qzap";
    let useDefaultTypePrinterParser = 1;
}

#endif // AQOMPLICE_QZAP_DIALECT_TD