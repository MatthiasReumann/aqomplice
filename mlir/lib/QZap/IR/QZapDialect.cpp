#include "QZap/IR/QZapDialect.h"

#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/DialectImplementation.h"          // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Transforms/InliningUtils.h"          // IWYU pragma: keep
#include "llvm/ADT/TypeSwitch.h"                    // IWYU pragma: keep

#include "QZap/IR/QZapOpsDialect.cpp.inc" // adds `QZapDialect::QZapDialect`

#define GET_TYPEDEF_CLASSES
#include "QZap/IR/QZapOpsTypes.cpp.inc" // adds type utilities

namespace aqomplice {
namespace qzap {
void QZapDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "QZap/IR/QZapOpsTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "QZap/IR/QZapOps.cpp.inc" // adds list of comma-seperated op names
      >();
}
}; // namespace qzap
}; // namespace aqomplice