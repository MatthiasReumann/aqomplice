#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Q/IR/QDialect.h" // IWYU pragma: keep

int main(int argc, char **argv) {
  mlir::DialectRegistry registry;
  registry.insert<aqomplice::QDialect, mlir::arith::ArithDialect,
                  mlir::memref::MemRefDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
