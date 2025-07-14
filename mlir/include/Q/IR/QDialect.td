#ifndef Q_DIALECT
#define Q_DIALECT

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"

def Q_Dialect : Dialect {
    let name = "q";
    let summary = "A wanna-be quantum compiler.";
    let cppNamespace = "::qompiler";
    let useDefaultTypePrinterParser = 1;
}

class Q_Op<string mnemonic, list<Trait> traits = []> :
        Op<Q_Dialect, mnemonic, traits>;

#endif // Q_DIALECT