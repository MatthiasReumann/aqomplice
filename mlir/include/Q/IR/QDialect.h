#ifndef AQOMPLICE_QDIALECT_H
#define AQOMPLICE_QDIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "Q/IR/QOpsDialect.h.inc"

#endif // AQOMPLICE_QDIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QTYPES_H
#define AQOMPLICE_QTYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "Q/IR/QOpsTypes.h.inc"

#endif // AQOMPLICE_QTYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QOPS_H
#define AQOMPLICE_QOPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "meta/traits.h" // IWYU pragma: keep

#define GET_OP_CLASSES
#include "Q/IR/QOps.h.inc"

#endif // AQOMPLICE_QOPS_H