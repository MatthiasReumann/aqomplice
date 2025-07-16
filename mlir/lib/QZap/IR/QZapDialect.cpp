#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/DialectImplementation.h"          // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Transforms/InliningUtils.h"          // IWYU pragma: keep
#include "llvm/ADT/TypeSwitch.h"                    // IWYU pragma: keep

#include "QZap/IR/QZapDialect.h"

using namespace mlir;
using namespace aqomplice::qzap;

#include "QZap/IR/QZapOpsDialect.cpp.inc"

#define GET_TYPEDEF_CLASSES
#include "QZap/IR/QZapOpsTypes.cpp.inc"

void QZapDialect::initialize() {
  addTypes<
#define GET_TYPEDEF_LIST
#include "QZap/IR/QZapOpsTypes.cpp.inc"
      >();

  addOperations<
#define GET_OP_LIST
#include "QZap/IR/QZapOps.cpp.inc"
      >();
}