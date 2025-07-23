module {
    qzap.kernel @ghz() -> memref<3xi1> {
      %nqubits = arith.constant 3 : i32
      
      %c0_i32 = arith.constant 0 : i32
      %c1_i32 = arith.constant 1 : i32
      %c2_i32 = arith.constant 2 : i32
      
       // Allocate quantum register with `nqubits` qubits.
      %r0 = qzap.alloc %nqubits
      
      %r1, %q00 = qzap.retrieve %r0[%c0_i32]
      %r2, %q10 = qzap.retrieve %r1[%c1_i32]
      %r3, %q20 = qzap.retrieve %r2[%c2_i32]
      
      // Apply GHZ gate sequence.
      %q01 = qzap.h %q00 : (!qzap.Qubit) -> !qzap.Qubit
      %q11, %q02 = qzap.x %q10 ctrl %q01 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      %q21, %q03 = qzap.x %q20 ctrl %q02 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      
      // Measure each qubit into a classical register.
      // Using value semantics we must "store" the qubits after use.
      %b0, %q04 = qzap.measure %q03 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r4 = qzap.store %q04, %r3[%c0_i32]
      
      %b1, %q12 = qzap.measure %q11 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r5 = qzap.store %q12, %r4[%c1_i32]
      
      %b2, %q22 = qzap.measure %q21 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r6 = qzap.store %q22, %r5[%c2_i32]
      
      %m = memref.alloc() : memref<3xi1>
      
      %i0 = arith.constant 0 : index
      %i1 = arith.constant 1 : index
      %i2 = arith.constant 2 : index
      
      memref.store %b0, %m[%i0] : memref<3xi1>
      memref.store %b1, %m[%i1] : memref<3xi1>
      memref.store %b2, %m[%i2] : memref<3xi1>
      
      qzap.free %r6
      qzap.return %m : memref<3xi1>
    }

    %m = qzap.call @ghz() : () -> memref<3xi1>
}