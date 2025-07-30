#ifndef AQOMPLICE_COMMON_QIR_H
#define AQOMPLICE_COMMON_QIR_H

#include "llvm/Support/FormatVariadic.h"

namespace aqomplice {
inline std::string getQIRFuncString(const std::string &suffix) {
  return llvm::formatv("__quantum__{}", suffix);
}

inline std::string getQIRInsName(const std::string &op,
                                 const std::string &specialization) {
  return getQIRFuncString(llvm::formatv("qis__{0}__{1}", op, specialization));
}
} // namespace aqomplice

#endif // AQOMPLICE_COMMON_QIR_H