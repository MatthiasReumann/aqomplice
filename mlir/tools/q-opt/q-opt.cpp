#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Conversion/Q-To-QZap/ToQZap.h"
#include "Q/IR/QDialect.h"
#include "QZap/IR/QZapDialect.h"

int main(int argc, char **argv) {
  aqomplice::q::registerPasses();

  mlir::DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, aqomplice::qzap::QZapDialect,
                  mlir::arith::ArithDialect, mlir::memref::MemRefDialect,
                  mlir::func::FuncDialect, mlir::scf::SCFDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
