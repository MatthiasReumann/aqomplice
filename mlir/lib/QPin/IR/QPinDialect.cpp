#include "QPin/IR/QPinDialect.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/DialectImplementation.h"          // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Transforms/InliningUtils.h"          // IWYU pragma: keep
#include "llvm/ADT/TypeSwitch.h"                    // IWYU pragma: keep

#include "QPin/IR/QPinOpsDialect.cpp.inc" // adds `QPinDialect::QPinDialect`

#define GET_TYPEDEF_CLASSES
#include "QPin/IR/QPinOpsTypes.cpp.inc" // adds type utilities

namespace aqomplice {
namespace qpin {

void QPinDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "QPin/IR/QPinOpsTypes.cpp.inc" // adds list of comma-seperated type names
      >();

  addOperations<
#define GET_OP_LIST
#include "QPin/IR/QPinOps.cpp.inc" // adds list of comma-seperated op names
      >();
}
}; // namespace q
}; // namespace aqomplice