#ifndef AQOMPLICE_QPINDIALECT_H
#define AQOMPLICE_QPINDIALECT_H

#include "mlir/Bytecode/BytecodeOpInterface.h"  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                    // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h" // IWYU pragma: keep

#include "QPin/IR/QPinOpsDialect.h.inc"

#endif // AQOMPLICE_QPINDIALECT_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QPINTYPES_H
#define AQOMPLICE_QPINTYPES_H

#include "mlir/IR/BuiltinTypes.h" // IWYU pragma: keep

#define GET_TYPEDEF_CLASSES
#include "QPin/IR/QPinOpsTypes.h.inc"

#endif // AQOMPLICE_QPINTYPES_H

//===----------------------------------------------------------------------===//

#ifndef AQOMPLICE_QPINOPS_H
#define AQOMPLICE_QPINOPS_H

#include "mlir/IR/BuiltinTypes.h"                  // IWYU pragma: keep
#include "mlir/IR/Dialect.h"                       // IWYU pragma: keep
#include "mlir/IR/OpDefinition.h"                  // IWYU pragma: keep
#include "mlir/Interfaces/ControlFlowInterfaces.h" // IWYU pragma: keep
#include "mlir/Interfaces/InferTypeOpInterface.h"  // IWYU pragma: keep
#include "mlir/Interfaces/SideEffectInterfaces.h"  // IWYU pragma: keep

#include "meta/traits.h" // IWYU pragma: keep

#define GET_OP_CLASSES
#include "QPin/IR/QPinOps.h.inc"

#endif // AQOMPLICE_QPINOPS_H