#ifndef AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H
#define AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_DECL_QPINTOLLVM
#include "conversion/passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H