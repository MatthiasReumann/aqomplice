module {
    qpin.kernel @ghz() -> memref<3xi1> {
        // Assign static(device) qubit values.
        %q0 = qpin.qubit 0
        %q1 = qpin.qubit 1 
        %q2 = qpin.qubit 2
        
        %c0_i32 = arith.constant 0 : i32
        %c1_i32 = arith.constant 1 : i32
        
        // Apply GHZ gate sequence.
        qpin.h %q0
        qpin.x %q1 ctrl %q0
        qpin.x %q2 ctrl %q0

        // Measure each qubit into a classical register.
        %b0 = qpin.measure %q0 
        %b1 = qpin.measure %q1 
        %b2 = qpin.measure %q2

        %m = memref.alloc() : memref<3xi1>
      
        %i0 = arith.constant 0 : index
        %i1 = arith.constant 1 : index
        %i2 = arith.constant 2 : index
      
        memref.store %b0, %m[%i0] : memref<3xi1>
        memref.store %b1, %m[%i1] : memref<3xi1>
        memref.store %b2, %m[%i2] : memref<3xi1>
      
        qpin.return %m : memref<3xi1>
    }

    %m = qpin.call @ghz() : () -> (memref<3xi1>)
}