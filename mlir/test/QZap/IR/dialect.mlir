module {
    qzap.kernel @ghz(%nqubits : i64) -> memref<?xi1> {
        %idx = arith.constant 0 : i64
        %lb = arith.constant 1 : i64 
        %step = arith.constant 1 : i64

        %r_0 = qzap.allocreg %nqubits

        %r_1, %q_0 = qzap.retrieve %r_0[%idx]
        %q_1 = qzap.h %q_0 -> !qzap.Qubit

        %r_N, %q_N = scf.for %i = %lb to %nqubits step %step 
            iter_args(%r_in = %r_1, %control_in = %q_1) -> (!qzap.QubitArray, !qzap.Qubit) : i64 {
            
            %r_mid, %target_in = qzap.retrieve %r_in[%i]
            %target_out, %control_out = qzap.x %q_0 ctrl %control_in -> !qzap.Qubit, !qzap.Qubit
            %r_out = qzap.store %r_mid[%i] %target_out

            scf.yield %r_out, %control_out : !qzap.QubitArray, !qzap.Qubit
        }
        
        %r_final = qzap.store %r_N[%idx] %q_N

        %m = qzap.measurereg %r_final -> memref<?xi1>     
        qzap.freereg %r_final
        qzap.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i64
    %m = qzap.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}