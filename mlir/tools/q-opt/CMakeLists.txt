get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
set(LIBS ${dialect_libs} MLIROptLib MLIRArithDialect MLIRQ)

add_mlir_tool(q-opt q-opt.cpp DEPENDS ${LIBS} SUPPORT_PLUGINS)
target_compile_options(q-opt PRIVATE -fexceptions)
target_link_libraries(q-opt PUBLIC ${LIBS})
llvm_update_compile_flags(q-opt)
mlir_check_all_link_libraries(q-opt)
export_executable_symbols_for_plugins(q-opt)