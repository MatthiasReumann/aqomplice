#include "mlir/Dialect/Func/IR/FuncOps.h" // IWYU pragma: keep
#include "mlir/IR/Builders.h"             // IWYU pragma: keep
#include "mlir/IR/OpImplementation.h"     // IWYU pragma: keep

#include "Q/IR/QDialect.h"

using namespace mlir;
using namespace qompiler;

#define GET_OP_CLASSES
#include "Q/IR/QOps.cpp.inc"