#ifndef AQOMPLICE_META_TRAITS_H
#define AQOMPLICE_META_TRAITS_H

#include "mlir/IR/OpDefinition.h"

namespace mlir {
namespace OpTrait {

template <typename ConcreteType>
class UnitaryTrait : public TraitBase<ConcreteType, UnitaryTrait> {};

template <typename ConcreteType>
class HermitianTrait : public TraitBase<ConcreteType, HermitianTrait> {};

template <typename ConcreteType>
class ControlledTrait : public TraitBase<ConcreteType, ControlledTrait> {};

} // namespace OpTrait
} // namespace mlir

#endif // AQOMPLICE_META_TRAITS_H