module {
    func.func @qft(%shared: memref<?x3xi1>, %iv: index) -> () attributes {qpu.kernel, no_inline} {
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
        q.s ctrld %q0, %q1
        q.t ctrld %q0, %q2
        q.h %q1 
        q.s ctrld %q1, %q2 
        q.h %q2
        q.swap %q0, %q2

        // Measure each qubit.
        %b0 = q.measure %q0
        %b1 = q.measure %q1
        %b2 = q.measure %q2

        q.free %r

        memref.store %b0, %shared[%iv, %idx0] : memref<?x3xi1>
        memref.store %b1, %shared[%iv, %idx1] : memref<?x3xi1>
        memref.store %b2, %shared[%iv, %idx2] : memref<?x3xi1>

        func.return
    }

    func.func @main() -> () {
        %zero = index.constant 0
        %step = index.constant 1
        %shots = index.constant 1024
        
        %shared = memref.alloc(%shots) : memref<?x3xi1>
        scf.for %iv = %zero to %shots step %step {
            func.call @qft(%shared, %iv) : (memref<?x3xi1>, index) -> ()
        }

        func.return
    }
}