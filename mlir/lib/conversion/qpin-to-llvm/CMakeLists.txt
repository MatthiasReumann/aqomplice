add_mlir_conversion_library(AqompliceQPinToLLVM
  qpin-to-llvm.cpp

  DEPENDS
  AqompliceConversionPassIncGen
)