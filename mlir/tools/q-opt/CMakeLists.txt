get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

set(LIBS
    ${dialect_libs} 
    ${conversion_libs}
    
    MLIROptLib
    MLIRFunctionInterfaces
        
    AqompliceQDialect
    AqompliceQPinDialect
    AqompliceQZapDialect

    AqompliceQToQZap
    AqompliceQZapToQPin
    AqompliceQPinToLLVM
    AqompliceQPinToFunc

    AqompliceFitTopology
)

add_mlir_tool(q-opt q-opt.cpp DEPENDS ${LIBS} SUPPORT_PLUGINS)

target_compile_options(q-opt PRIVATE -fexceptions)
target_link_libraries(q-opt PUBLIC ${LIBS})

llvm_update_compile_flags(q-opt)
mlir_check_all_link_libraries(q-opt)
export_executable_symbols_for_plugins(q-opt)