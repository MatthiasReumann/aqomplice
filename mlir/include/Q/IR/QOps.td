#ifndef Q_OPS
#define Q_OPS

include "mlir/Interfaces/FunctionInterfaces.td"
include "mlir/Interfaces/CallInterfaces.td"
include "mlir/Interfaces/SideEffectInterfaces.td"

include "meta/traits.td"

include "Q/IR/QDialect.td"
include "Q/IR/QTypes.td"

class Q_Op<string mnemonic, list<Trait> traits = []>
    : Op<Q_Dialect, mnemonic, traits>;

def KernelOp : Q_Op<"kernel", [FunctionOpInterface, IsolatedFromAbove]> {
  let summary = "quantum kernel";
  let arguments = (ins SymbolNameAttr:$sym_name,
      TypeAttrOf<FunctionType>:$function_type,
      OptionalAttr<DictArrayAttr>:$arg_attrs,
      OptionalAttr<DictArrayAttr>:$res_attrs);
  let regions = (region AnyRegion:$body);

  let builders = [OpBuilder<(ins "mlir::StringRef":$name,
      "mlir::FunctionType":$type,
      CArg<"mlir::ArrayRef<mlir::NamedAttribute>", "{}">:$attrs)>];

  let extraClassDeclaration = [{
        //===------------------------------------------------------------------===//
        // FunctionOpInterface Methods
        //===------------------------------------------------------------------===//

        /// Returns the argument types of this function.
        mlir::ArrayRef<mlir::Type> getArgumentTypes() { return getFunctionType().getInputs(); }

        /// Returns the result types of this function.
        mlir::ArrayRef<mlir::Type> getResultTypes() { return getFunctionType().getResults(); }

        mlir::Region *getCallableRegion() { return &getBody(); }
    }];

  let hasCustomAssemblyFormat = 1;
  let skipDefaultBuilders = 1;
}

def ReturnOp : Q_Op<"return", [Pure, Terminator, HasParent<"KernelOp">]> {
  let summary = "return from quantum kernel";
  let description = [{}];
  let arguments = (ins MeasurementResultType:$input);
}

def CallOp : Q_Op<"call", [DeclareOpInterfaceMethods<CallOpInterface>]> {
  let summary = "call quantum kernel";
  let description = [{}];
  let arguments = (ins
    FlatSymbolRefAttr:$callee,
    Variadic<AnyType>:$operands,
    OptionalAttr<DictArrayAttr>:$arg_attrs,
    OptionalAttr<DictArrayAttr>:$res_attrs
  );
  let results = (outs MeasurementResultType);
  let assemblyFormat = [{ 
    $callee `(` $operands `)` attr-dict `:` functional-type($operands, results)
  }];
}

class Memory_Op<string mnemonic, list<Trait> traits = []>
    : Q_Op<mnemonic, traits>;

def AllocOp : Memory_Op<"allocreg", [HasParent<"KernelOp">]> {
  let summary = "allocate n qubits into a quantum register";
  let description = [{
    Ensures implicitly that all other quantum 
    operations are placed inside a kernel.
  }];
  let arguments = (ins I64:$nqubits);
  let results = (outs QubitArrayType:$qreg);
}

def FreeOp : Memory_Op<"freereg"> {
  let summary = "Deallocate a quantum register.";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg);
}

def RetrieveOp : Memory_Op<"retrieve"> {
  let summary = "retrieve a qubit from a quantum register at a given index";
  let description = [{}];
  let arguments = (ins QubitArrayType:$qreg, I64:$index);
  let results = (outs QubitType:$qubit);
}

def MeasureOp : Q_Op<"measure"> {
  let summary = "measure in the computational basis";
  let description = [{}];
  let arguments = (ins AnyTypeOf<[QubitType, QubitArrayType]>:$target);
  let results = (outs MeasurementResultType:$res);
}

class Gate_Op<string mnemonic, list<Trait> traits = []>
    : Q_Op<mnemonic, traits#[Unitary]>;

def HOp : Gate_Op<"h", [Hermitian]> {
  let summary = "apply H operation to target qubit";
  let description = [{}];
  let arguments = (ins QubitType:$target);
  let results = (outs);
}

def XOp : Gate_Op<"x", [Hermitian]> {
  let summary = "apply X operation to target qubit";
  let description = [{}];
  let arguments = (ins QubitType:$target);
  let results = (outs);
}

def YOp : Gate_Op<"y", [Hermitian]> {
  let summary = "apply Y operation to target qubit";
  let description = [{}];
  let arguments = (ins QubitType:$target);
  let results = (outs);
}

def ZOp : Gate_Op<"z", [Hermitian]> {
  let summary = "apply Z operation to target qubit";
  let description = [{}];
  let arguments = (ins QubitType:$target);
  let results = (outs);
}

def CXOp : Gate_Op<"cx", [Hermitian, Controlled]> {
  let summary = "apply controlled-X operation to target qubit";
  let description = [{}];
  let arguments = (ins QubitType:$target, QubitType:$control);
  let results = (outs);
}

#endif // Q_OPS