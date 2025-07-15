#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/DialectImplementation.h"          // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Transforms/InliningUtils.h"          // IWYU pragma: keep
#include "llvm/ADT/TypeSwitch.h"                    // IWYU pragma: keep

#include "Q/IR/QDialect.h"

using namespace mlir;
using namespace qompiler;

#include "Q/IR/QOpsDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "Q/IR/QOpsTypes.cpp.inc"

void QDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Q/IR/QOpsTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "Q/IR/QOps.cpp.inc"
      >();
}