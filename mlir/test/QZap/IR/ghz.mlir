module {
    qzap.kernel @ghz(%shared: memref<3xi1>) -> () {
      %nqubits = arith.constant 3 : i32
      
      %c0_i32 = arith.constant 0 : i32
      %c1_i32 = arith.constant 1 : i32
      %c2_i32 = arith.constant 2 : i32

      %idx0 = index.castu %c0_i32 : i32 to index
      %idx1 = index.castu %c1_i32 : i32 to index
      %idx2 = index.castu %c2_i32 : i32 to index
      
      %r0 = qzap.alloc %nqubits
      
      %r1, %q00 = qzap.retrieve %r0[%c0_i32]
      %r2, %q10 = qzap.retrieve %r1[%c1_i32]
      %r3, %q20 = qzap.retrieve %r2[%c2_i32]
      
      %q01 = qzap.h %q00 : (!qzap.Qubit) -> !qzap.Qubit
      %q11, %q02 = qzap.x %q10 ctrl %q01 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      %q21, %q03 = qzap.x %q20 ctrl %q02 : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
      
      %b0, %q04 = qzap.measure %q03 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r4 = qzap.store %q04, %r3[%c0_i32]
      
      %b1, %q12 = qzap.measure %q11 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r5 = qzap.store %q12, %r4[%c1_i32]
      
      %b2, %q22 = qzap.measure %q21 : (!qzap.Qubit) -> (i1, !qzap.Qubit)
      %r6 = qzap.store %q22, %r5[%c2_i32]
      
      qzap.free %r6

      memref.store %b0, %shared[%idx0] : memref<3xi1>
      memref.store %b1, %shared[%idx1] : memref<3xi1>
      memref.store %b2, %shared[%idx2] : memref<3xi1>

      qzap.return
    }

    func.func @main() -> () {
      %shared = memref.alloc() : memref<3xi1>
      qzap.call @ghz(%shared) : (memref<3xi1>) -> ()
      func.return
    }
}