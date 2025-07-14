#pragma once

#ifndef QOMPILER_QOPS_H
#define QOMPILER_QOPS_H

// TODO: Check necessary imports
#include "mlir/IR/BuiltinTypes.h"
#include "mlir/IR/Dialect.h"
#include "mlir/IR/OpDefinition.h"
#include "mlir/Interfaces/InferTypeOpInterface.h"
#include "mlir/Interfaces/SideEffectInterfaces.h"

#define GET_OP_CLASSES
#include "Q/IR/QOps.h.inc"

#endif // QOMPILER_QOPS_H
