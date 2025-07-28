#include "transforms/fit-topology/fit-toplogy.h"

#include "QPin/IR/QPinDialect.h"

#include "mlir/Transforms/Passes.h"

namespace aqomplice {
#define GEN_PASS_DEF_FITTOPOLOGY
#include "transforms/passes.h.inc" // adds `impl::FitTopologyBase`

struct FitTopology : impl::FitTopologyBase<FitTopology> {
  using FitTopologyBase::FitTopologyBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::RewritePatternSet patterns(context);
  }
};

}