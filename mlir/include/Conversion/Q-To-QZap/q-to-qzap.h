#ifndef AQOMPLICE_Q_CONVERSION_TO_QZAP_H
#define AQOMPLICE_Q_CONVERSION_TO_QZAP_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_DECL_QTOQZAP
#include "Conversion/Passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_Q_CONVERSION_TO_QZAP_H