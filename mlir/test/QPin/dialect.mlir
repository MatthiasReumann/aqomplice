module {
    qpin.kernel @ghz(%nqubits : i32) -> memref<?xi1> {

        %c0_i32 = arith.constant 0 : i32
        qzap.h %c0_i32

        scf.for %i = %lb to %nqubits step %step : i32 {
            qzap.x %i ctrl %c0_i32
        }

        %lb = arith.constant 1 : i32 
        %step = arith.constant 1 : i32
        %m = memref.alloc() : memref<?xi1>
        scf.for %i = %lb to %nqubits step %step : i32 {
            %val = qpin.measure %i
            memref.store %val, %m[%i] : memref<?xi1>
        }

        qpin.return %m : memref<?xi1>
    }

    %nqubits = arith.constant 4 : i64
    %m = qpin.call @kernel(%nqubits) : (i64) -> (memref<?xi1>)
}