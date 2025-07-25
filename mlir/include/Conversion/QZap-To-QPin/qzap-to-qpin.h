#ifndef AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H
#define AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
namespace qzap {
#define GEN_PASS_DECL
#define GEN_PASS_REGISTRATION
#include "Conversion/qzap-to-qpin/qzap-to-qpin.h.inc"
}; // namespace qzap
}; // namespace aqomplice

#endif // AQOMPLICE_QZAP_CONVERSION_TO_QPIN_H