module {
    qzap.kernel @qft() -> (i1, i1, i1) {
      %nqubits = arith.constant 3 : i32
      
      %c0_i32 = arith.constant 0 : i32
      %c1_i32 = arith.constant 1 : i32
      %c2_i32 = arith.constant 2 : i32
      
      // Allocate quantum register with `nqubits` qubits.
      %r0 = qzap.alloc %nqubits
      
      %r1, %q00 = qzap.retrieve %r0[%c0_i32]
      %r2, %q10 = qzap.retrieve %r1[%c1_i32]
      %r3, %q20 = qzap.retrieve %r2[%c2_i32]
        
      // Apply three qubit QFT gate sequence.
      %q01 = qzap.h %q00 : (!qzap.Qubit) -> !qzap.Qubit
      %q02, %q11 = qzap.s %q01 ctrl %q10 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      %q03, %q21 = qzap.t %q02 ctrl %q20 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      %q12 = qzap.h %q11 : (!qzap.Qubit) -> !qzap.Qubit
      %q13, %q22 = qzap.s %q12 ctrl %q21 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      %q23 = qzap.h %q22 : (!qzap.Qubit) -> !qzap.Qubit
      %q04, %q24 = qzap.swap %q03, %q23 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)

      // Measure each qubit.
      // Using value semantics we must "store" the qubits after use.
      %b0, %q05 = qzap.measure %q04 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r4 = qzap.store %q05, %r3[%c0_i32]
      
      %b1, %q14 = qzap.measure %q13 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r5 = qzap.store %q14, %r4[%c1_i32]
      
      %b2, %q25 = qzap.measure %q24 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r6 = qzap.store %q25, %r5[%c2_i32]
    
      qzap.free %r6
      qzap.return %b0, %b1, %b2 : i1, i1, i1
    }

    %m:3 = qzap.call @qft() : () -> (i1, i1, i1)
}