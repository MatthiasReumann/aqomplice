module {
    qzap.kernel @ghz(%nqubits : i64) -> memref<?xi1> {
        %idx = arith.constant 0 : i64
        %lb = arith.constant 1 : i64 
        %step = arith.constant 1 : i64

        %r0 = "qzap.allocreg"(%nqubits) : (i64) -> !qzap.QubitArray

        %r1, %q00 = "qzap.retrieve"(%r0, %idx) : (!qzap.QubitArray, i64) -> (!qzap.QubitArray, !qzap.Qubit)
        %q01 = "qzap.h"(%q00) : (!qzap.Qubit) -> (!qzap.Qubit)

        %rN = scf.for %i = %lb to %nqubits step %step 
            iter_args(%ri0 = %r1) -> !qzap.QubitArray : i64 {
            
            %ri1, %qi0 = "qzap.retrieve"(%ri0, %i) : (!qzap.QubitArray, i64) -> (!qzap.QubitArray, !qzap.Qubit)
            %qi1 = "qzap.cx"(%q01, %qi0) : (!qzap.Qubit, !qzap.Qubit) -> (!qzap.Qubit)
            %ri2 = "qzap.store"(%ri1, %qi1, %i) : (!qzap.QubitArray, !qzap.Qubit, i64) -> !qzap.QubitArray

            scf.yield %ri2 : !qzap.QubitArray
        }
        
        %r = "qzap.store"(%rN, %q01, %idx) : (!qzap.QubitArray, !qzap.Qubit, i64) -> !qzap.QubitArray

        %m = "qzap.measure"(%r) : (!qzap.QubitArray) -> memref<?xi1>        
        "qzap.freereg"(%r) : (!qzap.QubitArray) -> ()

        "qzap.return"(%m) : (memref<?xi1>) -> ()
    }

    %nqubits = arith.constant 4 : i64
    %m = qzap.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}