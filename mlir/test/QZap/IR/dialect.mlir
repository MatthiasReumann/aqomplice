module {
    qzap.kernel @ghz(%nqubits : i32) -> memref<?xi1> {
        // Allocate quantum register with `nqubits` qubits.
        %r0 = qzap.alloc %nqubits

        // Apply Hadamard gate to first qubit. Since it used as control 
        // qubit in the loop we don't store it immediately.
        %c0_i32 = arith.constant 0 : i32
        %r1, %q0 = qzap.retrieve %r0[%c0_i32]
        %q1 = qzap.h %q0  : (!qzap.Qubit) -> (!qzap.Qubit)

        // Apply x gate controlled by first qubit to the other qubits.
        // The use of loop-carried values is due to value semantics.
        %lb = arith.constant 1 : i32 
        %step = arith.constant 1 : i32
        %r2, %q2 = scf.for %i = %lb to %nqubits step %step 
            iter_args(%rl0 = %r1, %ctrl_in = %q1) -> (!qzap.QubitArray, !qzap.Qubit) : i32 {
            
            %rl1, %tgt_in = qzap.retrieve %rl0[%i]
            %tgt_out, %ctrl_out = qzap.x %tgt_in ctrl %ctrl_in  : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit, !qzap.Qubit)
            %rl2 = qzap.store %tgt_out, %rl1[%i]

            scf.yield %rl2, %ctrl_out : !qzap.QubitArray, !qzap.Qubit
        }
        
        // Finally, store first qubit.
        %r3 = qzap.store %q2, %r2[%c0_i32]

        // Measure each qubit into a classical register.
        // Due to the type requirement we need to cast to index before. 
        // Again: The use of loop-carried values is due to value semantics.
        %nqubitsi = index.casts %nqubits : i32 to index
        %m = memref.alloc(%nqubitsi) : memref<?xi1>
        %r4 = scf.for %i = %c0_i32 to %nqubits step %step
            iter_args(%rl0 = %r3) -> (!qzap.QubitArray): i32 {
            %ii = index.casts %i : i32 to index
            
            %rl1, %qi = qzap.retrieve %rl0[%i]
            %bit, %qi2 = qzap.measure %qi : (!qzap.Qubit) -> (i1, !qzap.Qubit)
            %rl2 = qzap.store %qi2, %rl1[%i]
            
            memref.store %bit, %m[%ii] : memref<?xi1>

            scf.yield %rl2 : !qzap.QubitArray
        }
        
        // Release the quantum register.
        qzap.free %r4
        qzap.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i32
    %m = qzap.call @kernel(%nqubits) : (i32) -> (memref<?xi1>)
}