#ifndef AQOMPLICE_COMMON_TRAITS_H
#define AQOMPLICE_COMMON_TRAITS_H

#include "mlir/IR/OpDefinition.h"

namespace mlir {
namespace OpTrait {

template <typename ConcreteType>
class UnitaryTrait : public TraitBase<ConcreteType, UnitaryTrait> {};

template <typename ConcreteType>
class HermitianTrait : public TraitBase<ConcreteType, HermitianTrait> {};

} // namespace OpTrait
} // namespace mlir

#endif // AQOMPLICE_COMMON_TRAITS_H