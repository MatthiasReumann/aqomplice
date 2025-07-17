#include "Q/Conversion/ToQZap/ToQZap.h"

#include "Q/IR/QDialect.h"                 // IWYU pragma: keep
#include "QZap/IR/QZapDialect.h"           // IWYU pragma: keep
#include "mlir/Dialect/Arith/IR/Arith.h"   // IWYU pragma: keep
#include "mlir/Dialect/MemRef/IR/MemRef.h" // IWYU pragma: keep
#include "mlir/Dialect/SCF/IR/SCF.h"       // IWYU pragma: keep

namespace aqomplice {
namespace q {
#define GEN_PASS_DEF_QTOQZAP
#include "Q/Conversion/ToQZap/QToQZap.h.inc"

struct QTQZap : impl::QToQZapBase<QTQZap> {
  using QToQZapBase::QToQZapBase;

  void runOnOperation() override { mlir::MLIRContext *context = &getContext(); }
};
}; // namespace q
}; // namespace aqomplice