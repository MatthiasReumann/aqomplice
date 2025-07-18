module {
    q.kernel @identity(%x : i1) -> (i1) {
        "q.return"(%x) : (i1) -> ()
    }

    %x = arith.constant 1 : i1
    %m = q.call @identity(%x) : (i1) -> (i1)
}