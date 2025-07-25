set(LLVM_TARGET_DEFINITIONS qpin-to-llvm.td)
mlir_tablegen(qpin-to-llvm.h.inc --gen-pass-decls)
add_public_tablegen_target(MLIR_QPIN_TO_LLVM_INCGEN)