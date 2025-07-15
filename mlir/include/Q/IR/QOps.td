#ifndef Q_OPS
#define Q_OPS

include "mlir/IR/SymbolInterfaces.td"

include "Q/IR/QDialect.td"
include "Q/IR/QTypes.td"

class Q_Op<string mnemonic, list<Trait> traits = []> :
        Op<Q_Dialect, mnemonic, traits>;

def KernelOp : Q_Op<"kernel", [Symbol, NoTerminator]> { // TODO: Change NoTerminator to actual return.
    let summary = "A quantum kernel.";
    let arguments = (ins SymbolNameAttr: $sym_name);
    let results = (outs);
    let regions = (region AnyRegion: $body); // TODO: Custom AnyRegion?
}

class Memory_Op<string mnemonic, list<Trait> traits = []> : Q_Op<mnemonic, traits>;

def AllocOp : Memory_Op<"allocreg", [HasParent<"KernelOp">]> {
    let summary = "Allocate n qubits into a quantum register.";
    let description = [{}];

    let arguments = (ins
        I64:$nqubits
    );

    let results = (outs
       QuantumRegisterType:$qreg
    );
}

def FreeOp : Memory_Op<"freereg", [HasParent<"KernelOp">]> {
    let summary = "Deallocate a quantum register.";
    let description = [{}];

    let arguments = (ins
        QuantumRegisterType:$qreg
    );
}

#endif // Q_OPS