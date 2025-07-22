#include "Q/IR/QDialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"           // IWYU pragma: keep
#include "mlir/IR/Attributes.h"                     // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/BuiltinTypes.h"                   // IWYU pragma: keep
#include "mlir/IR/OpImplementation.h"               // IWYU pragma: keep
#include "mlir/IR/OperationSupport.h"               // IWYU pragma: keep
#include "mlir/Interfaces/CallInterfaces.h"         // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h"     // IWYU pragma: keep
#include "mlir/Support/LLVM.h"                      // IWYU pragma: keep
#include "llvm/ADT/ArrayRef.h"                      // IWYU pragma: keep
#include "llvm/ADT/StringRef.h"                     // IWYU pragma: keep

#define GET_OP_CLASSES
#include "Q/IR/QOps.cpp.inc" // adds ops logic

namespace aqomplice {
namespace qpin {
using namespace mlir;
}; // namespace q
}; // namespace aqomplice