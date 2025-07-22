#ifndef QPIN_DIALECT
#define QPIN_DIALECT

include "mlir/IR/DialectBase.td"
include "mlir/IR/OpBase.td"

def QPin_Dialect : Dialect {
    let name = "qpin";
    let summary = "pin quantum dialect";
    let description = [{
        Static qubit assignments and topology conforming quantum kernels.
    }];
    let cppNamespace = "::aqomplice::qpin";
    let useDefaultTypePrinterParser = 1;
}

#endif // QPIN_DIALECT