#ifndef AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H
#define AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
namespace qpin {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "Conversion/qpin-to-llvm/qpin-to-llvm.h.inc"
}; // namespace qpin
}; // namespace aqomplice

#endif // AQOMPLICE_QPIN_CONVERSION_TO_LLVM_H