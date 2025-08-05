#include "conversion/q-to-qzap/q-to-qzap.h"
#include "conversion/qpin-to-llvm/qpin-to-llvm.h"
#include "conversion/qzap-to-qpin/qzap-to-qpin.h"
#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Index/IR/IndexDialect.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Pass/PassOptions.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "conversion/passes.h"
#include "transforms/fit-topology/fit-toplogy.h"
#include "transforms/passes.h"

#include "Q/IR/QDialect.h"
#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"
#include <string>

#include "common/CommonInterfaces.cpp.inc" // adds interface methods

using namespace mlir;

namespace {
struct ToPinOptions : PassPipelineOptions<aqomplice::FitTopologyOptions> {
  Option<std::string> arch{*this, "arch",
                           llvm::cl::desc("The name of the architecture.")};
};
}; // namespace

int main(int argc, char **argv) {
  DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, aqomplice::qzap::QZapDialect,
                  aqomplice::qpin::QPinDialect, arith::ArithDialect,
                  memref::MemRefDialect, func::FuncDialect, index::IndexDialect,
                  scf::SCFDialect, LLVM::LLVMDialect>();

  PassPipelineRegistration<>(
      "to-zap", "Lower Q interface dialect to QZap optimization dialect.",
      [](OpPassManager &pm) {
        pm.addPass(aqomplice::createQToQZap());
        pm.addPass(createCanonicalizerPass());
      });

  PassPipelineRegistration<ToPinOptions>(
      "to-pin", "Lower QZap optimization dialect to QPin routing dialect.",
      [](OpPassManager &pm, const ToPinOptions &options) {
        pm.addPass(aqomplice::createQZapToQPin({options.arch}));
        pm.addPass(aqomplice::createFitTopology({options.arch}));
        pm.addPass(createRemoveDeadValuesPass());
      });

  PassPipelineRegistration<>(
      "to-llvm", "Lower all operations to LLVM IR.", [](OpPassManager &pm) {
        pm.addPass(aqomplice::createQPinToLLVM()); // Quantum Kernels.

        pm.addPass(createConvertSCFToCFPass()); // Control Flow Elements.
        pm.addPass(createConvertControlFlowToLLVMPass());

        pm.addPass(createFinalizeMemRefToLLVMConversionPass()); // MemRefs.

        pm.addPass(createArithToLLVMConversionPass()); // Indices.
        pm.addPass(createConvertIndexToLLVMPass());
        pm.addPass(createConvertFuncToLLVMPass()); // Funcs.

        pm.addPass(createCSEPass());
        pm.addPass(createCanonicalizerPass());
      });

  return asMainReturnCode(
      MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
