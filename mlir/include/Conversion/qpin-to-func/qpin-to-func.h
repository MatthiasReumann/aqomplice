#ifndef AQOMPLICE_QPIN_CONVERSION_TO_FUNC_H
#define AQOMPLICE_QPIN_CONVERSION_TO_FUNC_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_DECL_QPINTOFUNC
#include "Conversion/Passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_QPIN_CONVERSION_TO_FUNC_H