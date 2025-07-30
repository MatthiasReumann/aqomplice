#ifndef AQOMPLICE_Q_DIALECT_TD
#define AQOMPLICE_Q_DIALECT_TD

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"

def Q_Dialect : Dialect {
    let name = "q";
    let summary = "base quantum dialect";
    let cppNamespace = "::aqomplice::q";
    let useDefaultTypePrinterParser = 1;
}

#endif // AQOMPLICE_Q_DIALECT_TD