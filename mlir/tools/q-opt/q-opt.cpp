#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/LLVMIR/LLVMDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/InitAllPasses.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Conversion/Passes.h"
#include "Q/IR/QDialect.h"
#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

int main(int argc, char **argv) {
  mlir::registerAllPasses();
  aqomplice::registerPasses();

  mlir::DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, aqomplice::qzap::QZapDialect,
                  aqomplice::qpin::QPinDialect, mlir::arith::ArithDialect,
                  mlir::memref::MemRefDialect, mlir::func::FuncDialect,
                  mlir::scf::SCFDialect, mlir::LLVM::LLVMDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
