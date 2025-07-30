#ifndef AQOMPLICE_TRANSFORM_PASSES_H
#define AQOMPLICE_TRANSFORM_PASSES_H

#include "transforms/fit-topology/fit-toplogy.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_REGISTRATION
#include "transforms/passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_TRANSFORM_PASSES_H
