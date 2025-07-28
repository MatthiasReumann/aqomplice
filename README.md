# aqomplice

A wanna-be quantum-classical compiler.

## Workflow 

```
    "Q"          "QZap"         "QPin"         "QIR"
 ┌───────┐     ┌────────┐     ┌────────┐     ┌───────┐
 │   Q   │  →  │   Q⚡️  │  →  │   Q📌  │  →  │  QIR  │
 └───────┘     └────────┘     └────────┘     └───────┘
  Memory        Value          Topology       LLVM IR
  Semantics     Semantics      Conforming     
  ─────────     ─────────      ─────────      ─────────
  Interface     Optimizations  Routing        Exit
  
  <─────dynamic qubits─────>   <─────static qubits─────>
```

### Example

The following code blocks illustrate the lowering mechanism for the quantum fourier transform (qft) with three qubits.

```mlir
// q "interface" dialect.
module {
    q.kernel @qft(%shared: memref<3xi1>) -> () {
        %nqubits = arith.constant 3 : i32
        
        %c0_i32 = arith.constant 0 : i32
        %c1_i32 = arith.constant 1 : i32
        %c2_i32 = arith.constant 2 : i32

        %idx0 = index.castu %c0_i32 : i32 to index
        %idx1 = index.castu %c1_i32 : i32 to index
        %idx2 = index.castu %c2_i32 : i32 to index
        
        // Allocate quantum register with `nqubits` qubits.
        %r = q.alloc %nqubits

        %q0 = q.retrieve %r[%c0_i32]
        %q1 = q.retrieve %r[%c1_i32]
        %q2 = q.retrieve %r[%c2_i32]
        
        // Apply three qubit QFT gate sequence.
        q.h %q0
        q.s %q0 ctrl %q1
        q.t %q0 ctrl %q2
        q.h %q1 
        q.s %q1 ctrl %q2 
        q.h %q2
        q.swap %q0, %q2

        // Measure each qubit.
        %b0 = q.measure %q0
        %b1 = q.measure %q1
        %b2 = q.measure %q2

        q.free %r

        memref.store %b0, %shared[%idx0] : memref<3xi1>
        memref.store %b1, %shared[%idx1] : memref<3xi1>
        memref.store %b2, %shared[%idx2] : memref<3xi1>

        q.return
    }

    func.func @main() -> () {
        %shared = memref.alloc() : memref<3xi1>
        q.call @qft(%shared) : (memref<3xi1>) -> ()
        func.return
    }
}
```

```mlir
// qzap "optimization" dialect. Obtained by running:
//     q-opt --q-to-qzap
module {
  qzap.kernel @qft(%arg0: memref<3xi1>) {
    %c3_i32 = arith.constant 3 : i32
    
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %c2_i32 = arith.constant 2 : i32
    
    %0 = index.castu %c0_i32 : i32 to index
    %1 = index.castu %c1_i32 : i32 to index
    %2 = index.castu %c2_i32 : i32 to index
    
    // Allocate quantum register with `nqubits` qubits.
    %3 = qzap.alloc %c3_i32
    
    %qreq_out, %qubit = qzap.retrieve %3[%c0_i32]
    %qreq_out_0, %qubit_1 = qzap.retrieve %qreq_out[%c1_i32]
    %qreq_out_2, %qubit_3 = qzap.retrieve %qreq_out_0[%c2_i32]
    
    // Apply three qubit QFT gate sequence.
    %target_out = qzap.h %qubit : (!qzap.Qubit) -> !qzap.Qubit
    %target_out_4, %control_out = qzap.s %target_out ctrl %qubit_1 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_5, %control_out_6 = qzap.t %target_out_4 ctrl %qubit_3 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_7 = qzap.h %control_out : (!qzap.Qubit) -> !qzap.Qubit
    %target_out_8, %control_out_9 = qzap.s %target_out_7 ctrl %control_out_6 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_10 = qzap.h %control_out_9 : (!qzap.Qubit) -> !qzap.Qubit
    %a_out, %b_out = qzap.swap %target_out_5, %target_out_10 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)

    // Measure each qubit.
    %bit, %qubit_out = qzap.measure %a_out : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %4 = qzap.store %qubit_out, %qreq_out_2[%c0_i32]
    %bit_11, %qubit_out_12 = qzap.measure %target_out_8 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %5 = qzap.store %qubit_out_12, %4[%c1_i32]
    %bit_13, %qubit_out_14 = qzap.measure %b_out : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %6 = qzap.store %qubit_out_14, %5[%c2_i32]
    
    qzap.free %6
    
    memref.store %bit, %arg0[%0] : memref<3xi1>
    memref.store %bit_11, %arg0[%1] : memref<3xi1>
    memref.store %bit_13, %arg0[%2] : memref<3xi1>
    
    qzap.return
  }
  
  func.func @main() {
    %alloc = memref.alloc() : memref<3xi1>
    qzap.call @qft(%alloc) : (memref<3xi1>) -> ()
    return
  }
}
```

```mlir
// qpin "routing" dialect using static qubits. Obtained by running: 
//     q-opt --pass-pipeline="builtin.module(qzap-to-qpin,remove-dead-values)"
module {
  qpin.kernel @qft(%arg0: memref<3xi1>) {
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %c2_i32 = arith.constant 2 : i32

    %0 = index.castu %c0_i32 : i32 to index
    %1 = index.castu %c1_i32 : i32 to index
    %2 = index.castu %c2_i32 : i32 to index

    // Assign static (device) qubit values.
    %3 = qpin.qubit 0
    %4 = qpin.qubit 1
    %5 = qpin.qubit 2
    
    // Apply three qubit QFT gate sequence.
    qpin.h %3
    qpin.s %3 ctrl %4
    qpin.t %3 ctrl %5
    qpin.h %4
    qpin.s %4 ctrl %5
    qpin.h %5
    qpin.swap %3, %5
    
    // Measure each qubit.
    %6 = qpin.measure %3
    %7 = qpin.measure %4
    %8 = qpin.measure %5
    
    memref.store %6, %arg0[%0] : memref<3xi1>
    memref.store %7, %arg0[%1] : memref<3xi1>
    memref.store %8, %arg0[%2] : memref<3xi1>
    
    qpin.return
  }

  func.func @main() {
    %alloc = memref.alloc() : memref<3xi1>
    qpin.call @qft(%alloc) : (memref<3xi1>) -> ()
    return
  }
}
```

```mlir
// LLVM IR with QIR. Obtained my running:
//     q-opt --full-lowering
module {
  // LLVM Func declarations.
  llvm.func @malloc(i64) -> !llvm.ptr
  llvm.func @__quantum__qis__mz__body(!llvm.ptr) -> i1
  llvm.func @__quantum__qis__swap__body(!llvm.ptr, !llvm.ptr)
  llvm.func @__quantum__qis__t__ctl(!llvm.ptr, !llvm.ptr)
  llvm.func @__quantum__qis__s__ctl(!llvm.ptr, !llvm.ptr)
  llvm.func @__quantum__qis__h__body(!llvm.ptr)
  llvm.func @__quantum__rt__initialize(!llvm.ptr)
  
  llvm.func @qft(%arg0: !llvm.ptr, %arg1: !llvm.ptr, %arg2: i64, %arg3: i64, %arg4: i64) attributes {target = "qpu"} {
    %0 = llvm.mlir.constant(2 : i32) : i32
    %1 = llvm.mlir.constant(1 : i32) : i32
    %2 = llvm.mlir.constant(0 : i32) : i32
    %3 = llvm.mlir.zero : !llvm.ptr
    llvm.call @__quantum__rt__initialize(%3) : (!llvm.ptr) -> ()
    
    // Assign static (device) qubit values.
    %4 = llvm.inttoptr %2 : i32 to !llvm.ptr
    %5 = llvm.inttoptr %1 : i32 to !llvm.ptr
    %6 = llvm.inttoptr %0 : i32 to !llvm.ptr
    
    // Apply three qubit QFT gate sequence.
    llvm.call @__quantum__qis__h__body(%4) : (!llvm.ptr) -> ()
    llvm.call @__quantum__qis__s__ctl(%4, %5) : (!llvm.ptr, !llvm.ptr) -> ()
    llvm.call @__quantum__qis__t__ctl(%4, %6) : (!llvm.ptr, !llvm.ptr) -> ()
    llvm.call @__quantum__qis__h__body(%5) : (!llvm.ptr) -> ()
    llvm.call @__quantum__qis__s__ctl(%5, %6) : (!llvm.ptr, !llvm.ptr) -> ()
    llvm.call @__quantum__qis__h__body(%6) : (!llvm.ptr) -> ()
    llvm.call @__quantum__qis__swap__body(%4, %6) : (!llvm.ptr, !llvm.ptr) -> ()
    
    // Measure each qubit.
    %7 = llvm.call @__quantum__qis__mz__body(%4) : (!llvm.ptr) -> i1
    %8 = llvm.call @__quantum__qis__mz__body(%5) : (!llvm.ptr) -> i1
    %9 = llvm.call @__quantum__qis__mz__body(%6) : (!llvm.ptr) -> i1
    
    llvm.store %7, %arg1 : i1, !llvm.ptr
    %10 = llvm.getelementptr %arg1[1] : (!llvm.ptr) -> !llvm.ptr, i1
    llvm.store %8, %10 : i1, !llvm.ptr
    %11 = llvm.getelementptr %arg1[2] : (!llvm.ptr) -> !llvm.ptr, i1
    llvm.store %9, %11 : i1, !llvm.ptr
    
    llvm.return
  }

  llvm.func @main() {
    %0 = llvm.mlir.constant(0 : index) : i64
    %1 = llvm.mlir.constant(3 : index) : i64
    %2 = llvm.mlir.constant(1 : index) : i64
    %3 = llvm.mlir.zero : !llvm.ptr
    %4 = llvm.getelementptr %3[3] : (!llvm.ptr) -> !llvm.ptr, i1
    %5 = llvm.ptrtoint %4 : !llvm.ptr to i64
    
    %6 = llvm.call @malloc(%5) : (i64) -> !llvm.ptr
    llvm.call @qft(%6, %6, %0, %1, %2) : (!llvm.ptr, !llvm.ptr, i64, i64, i64) -> ()
    llvm.return
  }
}
```

## Related Work

The `Q` and `QZap` dialects are based on the `Quantum` and respectively `QuantumSSA` dialect introduced by Ittah et al. [^1] It enforces **Static Single Assignment** (SSA) for quantum programs by utilizing value semantics. Thanks to SSA an array of (classical) optimization techniques can be applied to quantum programs with relative ease. 

A **kernel** separates the **real-time** from **near-time** computation, as discussed in [^2]. Where the former refers to computations constrained by the physical traits of a quantum device (for example: the decoherance times of qubits. think: error correction). Essentially, a kernel is a special function that requires execution on a quantum accelerator. All quantum operations are located inside kernels. Consequently, a kernel convienently defines the quantum computation that later will be mapped (routed) to the quantum device's topology. The idea of a kernel is inspired by QCOR [^3] as well as GPU kernels (and the associated MLIR dialect [^4]). As in the QCOR memory model, we assume that the quantum and classical device own shared memory. 

[^1]: [QIRO: A Static Single Assignment-based Quantum Program Representation for Optimization](https://dl.acm.org/doi/10.1145/3491247)

[^2]: [OpenQASM 3: A Broader and Deeper Quantum Assembly Language](https://dl.acm.org/doi/10.1145/3505636)

[^3]: [QCOR: A Language Extension Specification for the Heterogeneous Quantum-Classical Model of Computation](http://arxiv.org/abs/1909.02457)

[^4]: ['gpu' Dialect - MLIR](https://mlir.llvm.org/docs/Dialects/GPU/#gpufunc-gpugpufuncop)



