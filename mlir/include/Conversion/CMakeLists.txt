set(LLVM_TARGET_DEFINITIONS passes.td)
mlir_tablegen(passes.h.inc --gen-pass-decls)
add_public_tablegen_target(AqompliceConversionPassIncGen)