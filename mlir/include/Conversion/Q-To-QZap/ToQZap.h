#ifndef AQOMPLICE_Q_CONVERSION_TO_QZAP_H
#define AQOMPLICE_Q_CONVERSION_TO_QZAP_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
namespace q {
#define GEN_PASS_DECL
#include "Conversion/Q-To-QZap/QToQZap.h.inc"

#define GEN_PASS_REGISTRATION
#include "Conversion/Q-To-QZap/QToQZap.h.inc"
}; // namespace q
}; // namespace aqomplice

#endif // AQOMPLICE_Q_CONVERSION_TO_QZAP_H