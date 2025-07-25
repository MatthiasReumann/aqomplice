module {
    q.kernel @ghz() -> (i1, i1, i1)  {
        %nqubits = arith.constant 3 : i32
        
        %c0_i32 = arith.constant 0 : i32
        %c1_i32 = arith.constant 1 : i32
        %c2_i32 = arith.constant 2 : i32
        
        %r = q.alloc %nqubits

        %q0 = q.retrieve %r[%c0_i32]
        %q1 = q.retrieve %r[%c1_i32]
        %q2 = q.retrieve %r[%c2_i32]
        
        q.h %q0
        q.x %q1 ctrl %q0
        q.x %q2 ctrl %q0

        %b0 = q.measure %q0
        %b1 = q.measure %q1
        %b2 = q.measure %q2

        q.free %r
        q.return %b0, %b1, %b2 : i1, i1, i1
    }

    func.func @main() -> (i32) {
        %m:3 = q.call @ghz() : () -> (i1, i1, i1)
    
        %c0_i32 = arith.constant 0 : i32
        return %c0_i32 : i32
    }
}

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

    func.func @main() -> (i32) {
        %m:3 = q.call @qft() : () -> (i1, i1, i1)
    
        %c0_i32 = arith.constant 0 : i32
        return %c0_i32 : i32
    }
}