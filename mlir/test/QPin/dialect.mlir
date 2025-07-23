module {
    qpin.kernel @ghz(%nqubits : i32) -> memref<?xi1> {
        // Apply Hadamard gate to the qubit with static "address" 0
        %c0_i32 = arith.constant 0 : i32
        qpin.h %c0_i32

        %lb = arith.constant 1 : i32 
        %step = arith.constant 1 : i32
        scf.for %i = %lb to %nqubits step %step : i32 {
            qpin.x %i ctrl %c0_i32
        }

        // Measure each qubit into a classical register.
        // Due to the type requirement we need to cast to index before. 
        %nqubitsi = index.casts %nqubits : i32 to index
        %m = memref.alloc(%nqubitsi) : memref<?xi1>
        scf.for %i = %c0_i32 to %nqubits step %step : i32 {
            %ii = index.casts %i : i32 to index

            %val = qpin.measure %i
            memref.store %val, %m[%ii] : memref<?xi1>
        }

        qpin.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i64
    %m = qpin.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}