module {
    q.kernel @alloc(%nqubits : i64) -> (i1) {
        %dummy = arith.constant 1 : i1

        %r = "q.allocreg"(%nqubits) : (i64) -> !q.QubitArray
        "q.freereg"(%r) : (!q.QubitArray) -> ()

        "q.return"(%dummy) : (i1) -> ()
    }

    %nqubits = arith.constant 4 : i64
    %m = q.call @alloc(%nqubits) : (i64) -> (i1)
}