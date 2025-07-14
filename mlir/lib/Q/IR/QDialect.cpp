#include "mlir/Dialect/Bufferization/IR/BufferizableOpInterface.h"
#include "mlir/IR/Builders.h"
#include "mlir/IR/DialectImplementation.h" // needed for generated type parser
#include "mlir/Interfaces/FunctionImplementation.h"
#include "mlir/Transforms/InliningUtils.h"

#include "Q/IR/QDialect.h"
#include "Q/IR/QOps.h"

using namespace mlir;
using namespace qompiler;

#include "Q/IR/QOpsDialect.cpp.inc"

void QDialect::initialize() {
  addOperations<
#define GET_OP_LIST
#include "Q/IR/QOps.cpp.inc"
      >();
}