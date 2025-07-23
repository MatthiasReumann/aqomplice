set(LLVM_TARGET_DEFINITIONS ToQPin.td)
mlir_tablegen(QZapToQPin.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIRQZapToQPinIncGen)