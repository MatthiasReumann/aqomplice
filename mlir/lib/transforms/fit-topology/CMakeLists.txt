add_mlir_library(AqompliceFitTopology
  architecture.cpp
  fit-topology.cpp

  DEPENDS
  AqompliceQPinDialect
  AqompliceTransformPassIncGen
)