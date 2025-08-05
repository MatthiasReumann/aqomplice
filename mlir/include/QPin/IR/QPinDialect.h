#ifndef AQOMPLICE_QPIN_DIALECT_H
#define AQOMPLICE_QPIN_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "QPin/IR/QPinOpsDialect.h.inc"

#endif // AQOMPLICE_QPIN_DIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QPIN_TYPES_H
#define AQOMPLICE_QPIN_TYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "QPin/IR/QPinOpsTypes.h.inc"

#endif // AQOMPLICE_QPIN_TYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QPIN_OP_INTERFACES_H
#define AQOMPLICE_QPIN_OP_INTERFACES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep
#include "mlir/IR/Dialect.h"      // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h" // IWYU pragma: keep

#include "QPin/IR/QPinOpsInterfaces.h.inc"

#endif // AQOMPLICE_QPIN_OP_INTERFACES_H

#ifndef AQOMPLICE_QPIN_OPS_H
#define AQOMPLICE_QPIN_OPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "common/interfaces.h" // IWYU pragma: keep
#include "common/traits.h"     // IWYU pragma: keep

#define GET_OP_CLASSES
#include "QPin/IR/QPinOps.h.inc"

#endif // AQOMPLICE_QPIN_OPS_H