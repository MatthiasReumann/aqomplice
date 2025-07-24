# aqomplice

A wanna-be quantum compiler.

## Workflow 

```
    "Q"          "QZap"         "QPin"         "QIR"
 ┌───────┐     ┌────────┐     ┌────────┐     ┌───────┐
 │   Q   │  →  │   Q⚡️  │  →  │   Q📌  │  →  │  QIR  │
 └───────┘     └────────┘     └────────┘     └───────┘
  Memory        Value          Topology       LLVM IR
  Semantics     Semantics      Conforming     
  ─────────     ─────────      ─────────      ─────────
  Interface     Optimizations  Routing        Executable
  
  <─────dynamic qubits─────>   <─────static qubits─────>
```

### Example

The following code blocks illustrate the lowering mechanism for the quantum fourier transform (qft) with three qubits.

```mlir
// q "interface" dialect
module {
    q.kernel @qft() -> (i1, i1, i1) {
        %nqubits = arith.constant 3 : i32
        
        %c0_i32 = arith.constant 0 : i32
        %c1_i32 = arith.constant 1 : i32
        %c2_i32 = arith.constant 2 : i32
        
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
        q.return %b0, %b1, %b2 : i1, i1, i1
    }

    %m:3 = q.call @qft() : () -> (i1, i1, i1)
}
```

```mlir
// qzap "optimization" dialect: obtained by running:
//     q-opt --q-to-qzap
module {
  qzap.kernel @qft() -> (i1, i1, i1) {
    %c3_i32 = arith.constant 3 : i32
    
    %c0_i32 = arith.constant 0 : i32
    %c1_i32 = arith.constant 1 : i32
    %c2_i32 = arith.constant 2 : i32

    // Allocate quantum register with `nqubits` qubits.
    %1 = qzap.alloc %c3_i32

    %qreq_out, %qubit = qzap.retrieve %1[%c0_i32]
    %qreq_out_0, %qubit_1 = qzap.retrieve %qreq_out[%c1_i32]
    %qreq_out_2, %qubit_3 = qzap.retrieve %qreq_out_0[%c2_i32]
    
    // Apply three qubit QFT gate sequence. Note the value semantics.
    %target_out = qzap.h %qubit : (!qzap.Qubit) -> !qzap.Qubit
    %target_out_4, %control_out = qzap.s %target_out ctrl %qubit_1 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_5, %control_out_6 = qzap.t %target_out_4 ctrl %qubit_3 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_7 = qzap.h %control_out : (!qzap.Qubit) -> !qzap.Qubit
    %target_out_8, %control_out_9 = qzap.s %target_out_7 ctrl %control_out_6 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    %target_out_10 = qzap.h %control_out_9 : (!qzap.Qubit) -> !qzap.Qubit
    %a_out, %b_out = qzap.swap %target_out_5, %target_out_10 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
    
    // Measure and store each qubit.
    %bit, %qubit_out = qzap.measure %a_out : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %2 = qzap.store %qubit_out, %qreq_out_2[%c0_i32]
    
    %bit_11, %qubit_out_12 = qzap.measure %target_out_8 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %3 = qzap.store %qubit_out_12, %2[%c1_i32]
    
    %bit_13, %qubit_out_14 = qzap.measure %b_out : (!qzap.Qubit) -> (i1, !qzap.Qubit)
    %4 = qzap.store %qubit_out_14, %3[%c2_i32]
    
    qzap.free %4
    qzap.return %bit, %bit_11, %bit_13 : i1, i1, i1
  }

  %0:3 = qzap.call @qft() : () -> (i1, i1, i1)
}
```

```mlir
// qpin "routing" dialect using static qubits: obtained by running: 
//     q-opt --pass-pipeline="builtin.module(qzap-to-qpin,remove-dead-values)"
module {
  qpin.kernel @qft() -> (i1, i1, i1) {
    // Assign static (device) qubit values.

    %1 = qpin.qubit 0
    %2 = qpin.qubit 1
    %3 = qpin.qubit 2

    // Apply three qubit QFT gate sequence.
    qpin.h %1
    qpin.s %1 ctrl %2
    qpin.t %1 ctrl %3
    qpin.h %2
    qpin.s %2 ctrl %3
    qpin.h %3
    qpin.swap %1, %3

    // Measure each qubit.
    %4 = qpin.measure %1
    %5 = qpin.measure %2
    %6 = qpin.measure %3
    
    qpin.return %4, %5, %6 : i1, i1, i1
  }

  %0:3 = qpin.call @qft() : () -> (i1, i1, i1)
}
```

## References

