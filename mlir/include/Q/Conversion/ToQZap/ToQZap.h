#ifndef AQOMPLICE_Q_CONVERSION_TO_QZAP_H
#define AQOMPLICE_Q_CONVERSION_TO_QZAP_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
namespace q {
#define GEN_PASS_DECL
#include "Q/Conversion/ToQZap/QToQZap.h.inc"

#define GEN_PASS_REGISTRATION
#include "Q/Conversion/ToQZap/QToQZap.h.inc"
}; // namespace q
}; // namespace aqomplice

#endif // AQOMPLICE_Q_CONVERSION_TO_QZAP_H