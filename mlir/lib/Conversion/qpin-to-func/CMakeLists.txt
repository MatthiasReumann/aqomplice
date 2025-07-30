add_mlir_conversion_library(AqompliceQPinToFunc
  qpin-to-func.cpp

  DEPENDS
  AqompliceConversionPassIncGen
)