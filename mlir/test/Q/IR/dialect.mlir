module {
    q.kernel @ghz(%nqubits : i64) -> memref<?xi1> {
        %idx = arith.constant 0 : i64
        %lb = arith.constant 1 : i64 
        %step = arith.constant 1 : i64
        
        %r = q.allocreg %nqubits

        %q0 = q.retrieve %r[%idx]
        q.h %q0

        scf.for %i = %lb to %nqubits step %step : i64 {
            %qi = q.retrieve %r[%i]
            q.x %qi ctrl %q0
        }

        %m = q.measurereg %r : (!q.QubitArray) -> memref<?xi1> 
        q.freereg %r
        q.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i64
    %m = q.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}