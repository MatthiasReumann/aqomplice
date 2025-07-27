#ifndef AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H
#define AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_DECL_QZAPTOQPIN
#include "Conversion/Passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H