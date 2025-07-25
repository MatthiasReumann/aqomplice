set(LLVM_TARGET_DEFINITIONS q-to-qzap.td)
mlir_tablegen(q-to-qzap.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIR_Q_TO_QZAP_INCGEN)