#ifndef QOMPILER_QDIALECT_H
#define QOMPILER_QDIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "Q/IR/QOpsDialect.h.inc"

#endif // QOMPILER_QDIALECT_H

//===----------------------------------------------------------------------===//

#ifndef QOMPILER_QTYPES_H
#define QOMPILER_QTYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "Q/IR/QOpsTypes.h.inc"

#endif // QOMPILER_QTYPES_H

//===----------------------------------------------------------------------===//

#ifndef QOMPILER_QOPS_H
#define QOMPILER_QOPS_H

#include "mlir/IR/BuiltinTypes.h"                 // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                      // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                 // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h" // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h" // IWYU pragma: keep

#define GET_OP_CLASSES
#include "Q/IR/QOps.h.inc"

#endif // QOMPILER_QOPS_H