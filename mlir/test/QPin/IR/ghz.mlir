module {
    qpin.kernel @ghz() -> (i1, i1, i1) {
        // Assign static(device) qubit values.
        %q0 = qpin.qubit 0
        %q1 = qpin.qubit 1 
        %q2 = qpin.qubit 2
        
        // Apply GHZ gate sequence.
        qpin.h %q0
        qpin.x %q1 ctrl %q0
        qpin.x %q2 ctrl %q0

        // Measure each qubit into a classical register.
        %b0 = qpin.measure %q0 
        %b1 = qpin.measure %q1 
        %b2 = qpin.measure %q2
      
        qpin.return %b0, %b1, %b2 : i1, i1, i1
    }

    func.func @main() -> (i32) {
        %m:3 = qpin.call @ghz() : () -> (i1, i1, i1)

        %c0_i32 = arith.constant 0 : i32
        return %c0_i32 : i32
    }
}