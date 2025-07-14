#include "mlir/IR/Builders.h"
#include "mlir/IR/OpImplementation.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"

#include "Q/IR/QDialect.h"
#include "Q/IR/QOps.h"

using namespace mlir;
using namespace qompiler;

#define GET_OP_CLASSES
#include "Q/IR/QOps.cpp.inc"