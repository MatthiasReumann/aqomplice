module {
    q.kernel @ghz(%shared: memref<3xi1>) -> ()  {
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
        
        // Apply GHZ gates.
        q.h %q0
        q.x %q1 ctrl %q0
        q.x %q2 ctrl %q0

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
        q.call @ghz(%shared) : (memref<3xi1>) -> ()
        func.return
    }
}