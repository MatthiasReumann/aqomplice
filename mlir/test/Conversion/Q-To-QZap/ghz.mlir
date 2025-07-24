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

    %m:3 = q.call @ghz() : () -> (i1, i1, i1)
}