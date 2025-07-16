module {
    "q.kernel" () <{sym_name = "test"}> ({
        %nqubits = arith.constant 8 : i64
        %r = "q.allocreg"(%nqubits) : (i64) -> !q.QubitRegister
        
        %idx = arith.constant 0 : i64
        %q = "q.retrieve"(%r, %idx) : (!q.QubitRegister, i64) -> !q.Qubit
        
        "q.freereg"(%r) : (!q.QubitRegister) -> ()
    }) : () -> ()
}