#ifndef AQOMPLICE_QZAPDIALECT_H
#define AQOMPLICE_QZAPDIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "QZap/IR/QZapOpsDialect.h.inc"

#endif // AQOMPLICE_QZAPDIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QZAPTYPES_H
#define AQOMPLICE_QZAPTYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "QZap/IR/QZapOpsTypes.h.inc"

#endif // AQOMPLICE_QZAPTYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QZAPOPS_H
#define AQOMPLICE_QZAPOPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "common/traits.h" // IWYU pragma: keep

#define GET_OP_CLASSES
#include "QZap/IR/QZapOps.h.inc"

#endif // AQOMPLICE_QZAPOPS_H