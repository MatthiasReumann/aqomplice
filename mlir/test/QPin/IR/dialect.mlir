module {
    qpin.kernel @ghz(%nqubits : i32) -> memref<?xi1> {        
        %N = index.casts %nqubits : i32 to index
        
        %c0_i32 = arith.constant 0 : i32
        %c1_i32 = arith.constant 1 : i32
        
        // Apply Hadamard gate to the qubit with static "address" 0
        qpin.h %c0_i32
        
        scf.for %i = %c1_i32 to %nqubits step %c1_i32 : i32 {
            qpin.x %i ctrl %c0_i32
        }

        // Measure each qubit into a classical register.
        // Due to the type requirement we need to cast to index before. 
        %m = memref.alloc(%N) : memref<?xi1>
        scf.for %i = %c0_i32 to %nqubits step %c1_i32 : i32 {
            %ii = index.casts %i : i32 to index
            %val = qpin.measure %i
            memref.store %val, %m[%ii] : memref<?xi1>
        }

        qpin.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i32
    %m = qpin.call @ghz() : () -> (memref<?xi1>)
}