/// Conversion of kernels.
module {
    q.kernel @identity(%x : i1) -> (i1) {
        q.return %x : i1
    }

    %x = arith.constant 1 : i1
    %m = q.call @identity(%x) : (i1) -> (i1)
}

/// Conversion with kernels with alloc and free.
module {
    q.kernel @alloc(%nqubits : i64) -> (i1) {
        %dummy = arith.constant 1 : i1
        
        %r = q.allocreg %nqubits
        q.freereg %r

        q.return %dummy : i1
    }

    %nqubits = arith.constant 4 : i64
    %m = q.call @alloc(%nqubits) : (i64) -> (i1)
}

/// Conversion with kernels with alloc, gates, and free.
module {
    q.kernel @alloc() -> (i1) {
        %dummy = arith.constant 1 : i1
        
        %q0_idx = arith.constant 0 : i64
        %q1_idx = arith.constant 1 : i64
        %q2_idx = arith.constant 2 : i64
        %nqubits = arith.constant 3 : i64
        
        %r = q.allocreg %nqubits

        %q0 = q.retrieve %r[%q0_idx]
        %q1 = q.retrieve %r[%q1_idx]
        %q2 = q.retrieve %r[%q2_idx]
        
        q.h %q0
        q.x %q1 ctrl %q0
        q.x %q2 ctrl %q0

        q.freereg %r

        q.return %dummy : i1
    }

    %m = q.call @alloc() : () -> (i1)
}