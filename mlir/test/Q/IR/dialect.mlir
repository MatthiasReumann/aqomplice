module {
    "q.kernel" () <{sym_name = "test"}> ({
        ^bb0:
            %nqubits = arith.constant 8 : i64
            %r = "q.allocreg"(%nqubits) : (i64) -> !q.QubitRegister
            "q.freereg"(%r) : (!q.QubitRegister) -> ()
    }) : () -> ()
}