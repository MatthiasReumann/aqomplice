#ifndef AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_H
#define AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_H

#include "mlir/Pass/Pass.h" // IWYU pragma: keep

namespace aqomplice {
#define GEN_PASS_DECL_FITTOPOLOGY
#include "transforms/passes.h.inc"
}; // namespace aqomplice

#endif // AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_H