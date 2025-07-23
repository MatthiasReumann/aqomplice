#include "mlir/Dialect/Arith/IR/Arith.h"
#include "mlir/Dialect/Func/IR/FuncOps.h"
#include "mlir/Dialect/Index/IR/IndexDialect.h"
#include "mlir/Dialect/MemRef/IR/MemRef.h"
#include "mlir/Dialect/SCF/IR/SCF.h"
#include "mlir/IR/MLIRContext.h"
#include "mlir/Support/FileUtilities.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "Conversion/Q-To-QZap/ToQZap.h"
#include "Conversion/QZap-To-QPin/ToQPin.h"
#include "Q/IR/QDialect.h"
#include "QPin/IR/QPinDialect.h"
#include "QZap/IR/QZapDialect.h"

int main(int argc, char **argv) {
  aqomplice::q::registerPasses();
  aqomplice::qzap::registerPasses();

  mlir::DialectRegistry registry;
  registry.insert<aqomplice::q::QDialect, aqomplice::qzap::QZapDialect,
                  aqomplice::qpin::QPinDialect, mlir::arith::ArithDialect,
                  mlir::memref::MemRefDialect, mlir::func::FuncDialect,
                  mlir::scf::SCFDialect, mlir::index::IndexDialect>();

  return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "Q optimizer driver\n", registry));
}
