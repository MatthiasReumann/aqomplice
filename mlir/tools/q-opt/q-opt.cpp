#include "conversion/q-to-qzap/q-to-qzap.h"
#include "conversion/qpin-to-func/qpin-to-func.h"
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

namespace {
struct FullLoweringOptions
    : mlir::PassPipelineOptions<aqomplice::FitTopologyOptions> {
  Option<std::string> arch{*this, "arch",
                           llvm::cl::desc("The name of the architecture.")};
};

void fullLoweringPipelineBuilder(mlir::OpPassManager &pm,
                                 const FullLoweringOptions &options) {
  pm.addPass(aqomplice::createQToQZap());
  pm.addPass(aqomplice::createQZapToQPin());
  pm.addPass(aqomplice::createFitTopology(
      aqomplice::FitTopologyOptions{options.arch}));

  pm.addPass(mlir::createArithToLLVMConversionPass());
  pm.addPass(mlir::createConvertIndexToLLVMPass());
  pm.addPass(aqomplice::createQPinToLLVM());
  pm.addPass(aqomplice::createQPinToFunc());
  pm.addPass(mlir::createFinalizeMemRefToLLVMConversionPass());
  pm.addPass(mlir::createConvertFuncToLLVMPass());

  pm.addPass(mlir::createCanonicalizerPass());
}
}; // namespace

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  aqomplice::registerConversionsPasses();
  aqomplice::registerTransformsPasses();

  mlir::DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, aqomplice::qzap::QZapDialect,
                  aqomplice::qpin::QPinDialect, mlir::arith::ArithDialect,
                  mlir::memref::MemRefDialect, mlir::func::FuncDialect,
                  mlir::index::IndexDialect, mlir::scf::SCFDialect,
                  mlir::LLVM::LLVMDialect>();

  mlir::PassPipelineRegistration<FullLoweringOptions>(
      "full-lowering", "Lower from Q to LLVM IR.", fullLoweringPipelineBuilder);

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
