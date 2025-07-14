add_mlir_dialect_library(MLIRQ
    QDialect.cpp
    QOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${PROJECT_SOURCE_DIR}/include/Q/IR

    DEPENDS
    MLIRQOpsIncGen

    LINK_LIBS PUBLIC
    MLIRIR
    MLIRInferTypeOpInterface
    MLIRFuncDialect
)