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
    q.kernel @alloc(%nqubits : i32) -> (i1) {
        %dummy = arith.constant 1 : i1
        
        %r = q.alloc %nqubits
        q.free %r

        q.return %dummy : i1
    }

    %nqubits = arith.constant 4 : i32
    %m = q.call @alloc(%nqubits) : (i32) -> (i1)
}

/// Conversion with kernels with alloc, gates, and free.
module {
    q.kernel @ghz() -> memref<3xi1>   {
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

        %m = memref.alloc() : memref<3xi1>
        
        %i0 = arith.constant 0 : index
        %i1 = arith.constant 1 : index
        %i2 = arith.constant 2 : index

        memref.store %b0, %m[%i0] : memref<3xi1>
        memref.store %b1, %m[%i1] : memref<3xi1>
        memref.store %b2, %m[%i2] : memref<3xi1>

        q.free %r
        q.return %m : memref<3xi1>
    }

    %m = q.call @ghz() : () -> memref<3xi1>
}