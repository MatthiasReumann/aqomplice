#ifndef QZAP_DIALECT
#define QZAP_DIALECT

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

#endif // QZAP_DIALECT