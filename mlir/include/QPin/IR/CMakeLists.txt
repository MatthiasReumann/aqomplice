set(LLVM_TARGET_DEFINITIONS QPinOps.td)
mlir_tablegen(QPinOps.h.inc -gen-op-decls)
mlir_tablegen(QPinOps.cpp.inc -gen-op-defs)
mlir_tablegen(QPinOpsDialect.h.inc -gen-dialect-decls)
mlir_tablegen(QPinOpsDialect.cpp.inc -gen-dialect-defs)
mlir_tablegen(QPinOpsTypes.h.inc -gen-typedef-decls)
mlir_tablegen(QPinOpsTypes.cpp.inc -gen-typedef-defs)
mlir_tablegen(QPinOpsInterfaces.h.inc -gen-op-interface-decls)
mlir_tablegen(QPinOpsInterfaces.cpp.inc -gen-op-interface-defs)

add_public_tablegen_target(AqompliceQPinIncGen)

add_mlir_doc(QPinOps QPinOps Dialects/ -gen-op-doc)