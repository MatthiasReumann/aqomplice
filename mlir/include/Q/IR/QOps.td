#ifndef Q_OPS
#define Q_OPS

include "mlir/IR/SymbolInterfaces.td"

include "Q/IR/QDialect.td"
include "Q/IR/QTypes.td"

class Q_Op<string mnemonic, list<Trait> traits = []> :
        Op<Q_Dialect, mnemonic, traits>;

// TODO: Change NoTerminator to actual return.
// TODO: A kernel could take `nqubits` as input and automatically alloc and free a register.
// TODO: Implement custom region s.t. HasParent becomes obsolete?
def KernelOp : Q_Op<"kernel", [Symbol, NoTerminator]> {
    let summary = "A quantum kernel.";
    let arguments = (ins SymbolNameAttr: $sym_name);
    let results = (outs);
    let regions = (region AnyRegion: $body);
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

def RetrieveOp : Memory_Op<"retrieve", []> {
    let summary = "Retrieve a qubit from a quantum register at a given index.";
    let description = [{}];

    let arguments = (ins
        QuantumRegisterType:$qreg,
        I64:$index
    );

    let results = (outs
        QubitType:$qubit
    );
}

def Unitary : NativeOpTrait<"UnitaryTrait">;
def Hermitian : NativeOpTrait<"HermitianTrait">;
def Controlled : NativeOpTrait<"ControlledTrait">;

class Gate_Op<string mnemonic, list<Trait> traits = []> : Q_Op<mnemonic, traits # [Unitary]>;

def HOp : Gate_Op<"h", [Hermitian]> {
    let summary = "Apply H operation to target qubit.";
    let description = [{}];
    let arguments = (ins QubitType:$target);
    let results = (outs);
}

def XOp : Gate_Op<"x", [Hermitian]> {
    let summary = "Apply X operation to target qubit.";
    let description = [{}];
    let arguments = (ins QubitType:$target);
    let results = (outs);
}

def YOp : Gate_Op<"y", [Hermitian]> {
    let summary = "Apply Y operation to target qubit.";
    let description = [{}];
    let arguments = (ins QubitType:$target);
    let results = (outs);
}

def ZOp : Gate_Op<"z", [Hermitian]> {
    let summary = "Apply Z operation to target qubit.";
    let description = [{}];
    let arguments = (ins QubitType:$target);
    let results = (outs);
}

def CXOp : Gate_Op<"cx", [Hermitian, Controlled]> {
    let summary = "Apply controlled-X operation to target qubit";
    let description = [{}];
    let arguments = (ins QubitType:$target, QubitType:$control);
    let results = (outs);
}

#endif // Q_OPS