set(LLVM_TARGET_DEFINITIONS qzap-to-qpin.td)
mlir_tablegen(qzap-to-qpin.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIR_QZAP_TO_QPIN_INCGEN)