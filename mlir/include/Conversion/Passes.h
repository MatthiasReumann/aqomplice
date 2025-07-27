#ifndef AQOMPLICE_CONVERSION_PASSES_H
#define AQOMPLICE_CONVERSION_PASSES_H

#include "Conversion/q-to-qzap/q-to-qzap.h"       // IWYU pragma: keep
#include "Conversion/qpin-to-func/qpin-to-func.h" // IWYU pragma: keep
#include "Conversion/qpin-to-llvm/qpin-to-llvm.h" // IWYU pragma: keep
#include "Conversion/qzap-to-qpin/qzap-to-qpin.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_REGISTRATION
#include "Conversion/Passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_CONVERSION_PASSES_H
