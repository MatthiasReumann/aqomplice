#ifndef AQOMPLICE_QZAP_DIALECT_H
#define AQOMPLICE_QZAP_DIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "QZap/IR/QZapOpsDialect.h.inc"

#endif // AQOMPLICE_QZAP_DIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QZAP_TYPES_H
#define AQOMPLICE_QZAP_TYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "QZap/IR/QZapOpsTypes.h.inc"

#endif // AQOMPLICE_QZAP_TYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QZAP_OPS_H
#define AQOMPLICE_QZAP_OPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "common/interfaces.h" // IWYU pragma: keep
#include "common/traits.h"     // IWYU pragma: keep

#define GET_OP_CLASSES
#include "QZap/IR/QZapOps.h.inc"

#endif // AQOMPLICE_QZAP_OPS_H