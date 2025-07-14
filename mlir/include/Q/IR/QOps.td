#ifndef Q_OPS
#define Q_OPS

include "mlir/Interfaces/FunctionInterfaces.td"

include "Q/IR/QDialect.td"

def KernelOp : Q_Op<"kernel", [FunctionOpInterface]> {
    let summary = "A quantum kernel.";
    let arguments = (ins SymbolNameAttr: $sym_name, TypeAttrOf<FunctionType>: $function_type);
    let regions = (region AnyRegion: $body);
}

#endif // Q_OPS