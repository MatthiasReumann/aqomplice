module {
    q.kernel @ghz() -> () {
        %nqubits = arith.constant 3 : i64
        %r = "q.allocreg"(%nqubits) : (i64) -> !q.QubitArray
        
        %idx0 = arith.constant 0 : i64
        %idx1 = arith.constant 1 : i64
        %idx2 = arith.constant 2 : i64

        %q0 = "q.retrieve"(%r, %idx0) : (!q.QubitArray, i64) -> !q.Qubit
        %q1 = "q.retrieve"(%r, %idx1) : (!q.QubitArray, i64) -> !q.Qubit
        %q2 = "q.retrieve"(%r, %idx2) : (!q.QubitArray, i64) -> !q.Qubit
        
        "q.h"(%q0) : (!q.Qubit) -> ()
        "q.cx"(%q0, %q1) : (!q.Qubit, !q.Qubit) -> ()
        "q.cx"(%q0, %q2) : (!q.Qubit, !q.Qubit) -> ()

        %m = "q.measure"(%r) : (!q.QubitArray) -> memref<3xi1>        
        "q.freereg"(%r) : (!q.QubitArray) -> ()

        "q.return"(%m) : (memref<3xi1>) -> ()
    }
}