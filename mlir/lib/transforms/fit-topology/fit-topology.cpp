#include "transforms/fit-topology/architecture.h"
#include "transforms/fit-topology/fit-toplogy.h"

#include "QPin/IR/QPinDialect.h"

namespace aqomplice {
#define GEN_PASS_DEF_FITTOPOLOGY
#include "transforms/passes.h.inc" // adds `impl::FitTopologyBase`

struct FitTopology : impl::FitTopologyBase<FitTopology> {
  using FitTopologyBase::FitTopologyBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    mlir::OpBuilder builder(context);
    if (this->arch == "iqm-spark") {

      // clang-format off

      //       QB0
      //        |
      // QB1 - QB2 - QB3     IQM Spark 5-Qubit Star-Like Architecture
      //        |
      //       QB4

      Architecture spark(5, {
        {0, 2}, {2, 0}, 
        {1, 2}, {2, 1}, 
        {2, 3}, {3, 2}, 
        {2, 4}, {4, 2}});
      // clang-format on

      spark.localOptimalSwap(builder, op);
    }
  }
};
} // namespace aqomplice