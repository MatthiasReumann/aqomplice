#ifndef AQOMPLICE_Q_DIALECT_H
#define AQOMPLICE_Q_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "Q/IR/QOpsDialect.h.inc"

#endif // AQOMPLICE_Q_DIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_Q_TYPES_H
#define AQOMPLICE_Q_TYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "Q/IR/QOpsTypes.h.inc"

#endif // AQOMPLICE_Q_TYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_Q_OPS_H
#define AQOMPLICE_Q_OPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "common/traits.h" // IWYU pragma: keep

#define GET_OP_CLASSES
#include "Q/IR/QOps.h.inc"

#endif // AQOMPLICE_Q_OPS_H