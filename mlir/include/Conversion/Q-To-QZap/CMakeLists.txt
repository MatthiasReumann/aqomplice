set(LLVM_TARGET_DEFINITIONS ToQZap.td)
mlir_tablegen(QToQZap.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIRQToQZapIncGen)