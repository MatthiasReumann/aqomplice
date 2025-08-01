#ifndef AQOMPLICE_COMMON_TRAITS_TD
#define AQOMPLICE_COMMON_TRAITS_TD

include "mlir/IR/OpBase.td"

def Unitary : NativeOpTrait<"UnitaryTrait">;
def Hermitian : NativeOpTrait<"HermitianTrait">;

#endif // AQOMPLICE_COMMON_TRAITS_TD