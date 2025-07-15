#ifndef Q_DIALECT
#define Q_DIALECT

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"

def Q_Dialect : Dialect {
    let name = "q";
    let summary = "A wanna-be quantum compiler.";
    let cppNamespace = "::aqomplice";
    let useDefaultTypePrinterParser = 1;
}

#endif // Q_DIALECT