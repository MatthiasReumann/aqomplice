#ifndef AQOMPLICE_COMMON_TRAITS_H
#define AQOMPLICE_COMMON_TRAITS_H

#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/IR/OpDefinition.h"

namespace mlir {
namespace OpTrait {

template <typename ConcreteType>
class UnitaryTrait : public TraitBase<ConcreteType, UnitaryTrait> {};

template <typename ConcreteType>
class HermitianTrait : public TraitBase<ConcreteType, HermitianTrait> {};

template <typename ConcreteType>
class InsideQPUKernelTrait
    : public OpTrait::TraitBase<ConcreteType, InsideQPUKernelTrait> {
public:
  static LogicalResult verifyTrait(Operation *op) {
    const auto p = op->getParentOp();
    if (llvm::isa_and_nonnull<func::FuncOp>(p) && p->hasAttr("qpu.kernel") &&
        p->hasAttr("no_inline")) {
      return success();
    }

    return op->emitOpError() << "expects parent op to be func.func with "
                                "unit attributes 'qpu.kernel' and 'no_inline'";
  }
};

} // namespace OpTrait
} // namespace mlir

#endif // AQOMPLICE_COMMON_TRAITS_H