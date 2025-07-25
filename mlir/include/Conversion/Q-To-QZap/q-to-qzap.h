#ifndef AQOMPLICE_Q_CONVERSION_TO_QZAP_H
#define AQOMPLICE_Q_CONVERSION_TO_QZAP_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
namespace q {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "Conversion/q-to-qzap/q-to-qzap.h.inc"
}; // namespace q
}; // namespace aqomplice

#endif // AQOMPLICE_Q_CONVERSION_TO_QZAP_H