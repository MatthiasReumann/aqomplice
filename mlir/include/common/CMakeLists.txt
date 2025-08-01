set(LLVM_TARGET_DEFINITIONS interfaces.td)
mlir_tablegen(CommonInterfaces.h.inc -gen-op-interface-decls)
mlir_tablegen(CommonInterfaces.cpp.inc -gen-op-interface-defs)

add_public_tablegen_target(AqompliceCommonIncGen)