module {
  qpin.kernel @qft(%shared: memref<3xi1>) -> () {    
    // Assign static (device) qubit values.

    %q0 = qpin.qubit 0
    %q1 = qpin.qubit 1
    %q2 = qpin.qubit 2
    
    // Apply three qubit QFT gate sequence.
    qpin.h %q0
    qpin.s %q0 ctrl %q1
    qpin.t %q0 ctrl %q2
    qpin.h %q1 
    qpin.s %q1 ctrl %q2 
    qpin.h %q2
    qpin.swap %q0, %q2
    
    // Measure each qubit.
    %b0 = qpin.measure %q0
    %b1 = qpin.measure %q1
    %b2 = qpin.measure %q2

    %i0 = index.constant 0
    %i1 = index.constant 1
    %i2 = index.constant 2

    memref.store %b0, %shared[%i0] : memref<3xi1>
    memref.store %b1, %shared[%i1] : memref<3xi1> 
    memref.store %b2, %shared[%i2] : memref<3xi1> 
    
    qpin.return
  }

  func.func @main() -> (i32) {
    %shared = memref.alloc() : memref<3xi1>
    qpin.call @qft(%shared) : (memref<3xi1>) -> ()
    
    %c0_i32 = arith.constant 0 : i32
    return %c0_i32 : i32
  }
}