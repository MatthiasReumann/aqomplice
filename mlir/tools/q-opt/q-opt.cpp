#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Q/IR/QDialect.h"

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, mlir::arith::ArithDialect,
                  mlir::memref::MemRefDialect, mlir::scf::SCFDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
