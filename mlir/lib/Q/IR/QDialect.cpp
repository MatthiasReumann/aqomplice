#include "Q/IR/QDialect.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/DialectImplementation.h"          // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Transforms/InliningUtils.h"          // IWYU pragma: keep
#include "llvm/ADT/TypeSwitch.h"                    // IWYU pragma: keep

#include "Q/IR/QOpsDialect.cpp.inc" // adds `QDialect::QDialect`

#define GET_TYPEDEF_CLASSES
#include "Q/IR/QOpsTypes.cpp.inc" // adds type utilities

namespace aqomplice {
namespace q {

void QDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "Q/IR/QOpsTypes.cpp.inc" // adds list of comma-seperated type names
      >();

  addOperations<
#define GET_OP_LIST
#include "Q/IR/QOps.cpp.inc" // adds list of comma-seperated op names
      >();
}
}; // namespace q
}; // namespace aqomplice