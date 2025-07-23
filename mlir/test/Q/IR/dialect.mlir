module {
    q.kernel @ghz(%nqubits : i32) -> memref<?xi1> {
        // Allocate quantum register with `nqubits` qubits.
        %r = q.alloc %nqubits

        // Apply Hadamard gate to first qubit.
        %c0_i32 = arith.constant 0 : i32
        %q0 = q.retrieve %r[%c0_i32]
        q.h %q0

        // Apply x gate controlled by first qubit to the other qubits.
        %lb = arith.constant 1 : i32
        %step = arith.constant 1 : i32
        scf.for %i = %lb to %nqubits step %step : i32 {
            %qi = q.retrieve %r[%i]
            q.x %qi ctrl %q0
        }

        // Measure each qubit into a classical register.
        // Due to the type requirement we need to cast to index before. 
        %nqubitsi = index.casts %nqubits : i32 to index
        %m = memref.alloc(%nqubitsi) : memref<?xi1>
        scf.for %i = %c0_i32 to %nqubits step %step : i32 {
            %ii = index.casts %i : i32 to index
            
            %qi = q.retrieve %r[%i]
            %bit = q.measure %qi
            
            memref.store %bit, %m[%ii] : memref<?xi1>
        }

        // Release the quantum register.
        q.free %r
        q.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i32
    %m = q.call @kernel(%nqubits) : (i32) -> (memref<?xi1>)
}