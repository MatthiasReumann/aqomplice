#ifndef AQOMPLICE_CONVERSION_PASSES_H
#define AQOMPLICE_CONVERSION_PASSES_H

#include "conversion/q-to-qzap/q-to-qzap.h"       // IWYU pragma: keep
#include "conversion/qpin-to-func/qpin-to-func.h" // IWYU pragma: keep
#include "conversion/qpin-to-llvm/qpin-to-llvm.h" // IWYU pragma: keep
#include "conversion/qzap-to-qpin/qzap-to-qpin.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_REGISTRATION
#include "conversion/passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_CONVERSION_PASSES_H
