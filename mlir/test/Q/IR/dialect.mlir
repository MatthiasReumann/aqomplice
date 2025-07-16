module {
    q.kernel @ghz(%nqubits : i64) -> memref<?xi1> {
        %r = "q.allocreg"(%nqubits) : (i64) -> !q.QubitArray

        %idx = arith.constant 0 : i64
        %q0 = "q.retrieve"(%r, %idx) : (!q.QubitArray, i64) -> !q.Qubit
        "q.h"(%q0) : (!q.Qubit) -> ()

        %lb = arith.constant 1 : i64 
        %step = arith.constant 1 : i64
        scf.for %i = %lb to %nqubits step %step : i64 {
            %qi = "q.retrieve"(%r, %i) : (!q.QubitArray, i64) -> !q.Qubit
            "q.cx"(%q0, %qi) : (!q.Qubit, !q.Qubit) -> ()
        }

        %m = "q.measure"(%r) : (!q.QubitArray) -> memref<?xi1>        
        "q.freereg"(%r) : (!q.QubitArray) -> ()

        "q.return"(%m) : (memref<?xi1>) -> ()
    }

    %nqubits = arith.constant 4 : i64
    %m = q.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}