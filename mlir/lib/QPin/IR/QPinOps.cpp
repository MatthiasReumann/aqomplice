#include "QPin/IR/QPinDialect.h"

#include "mlir/Dialect/Arith/IR/Arith.h"            // IWYU pragma: keep
#include "mlir/Dialect/Func/IR/FuncOps.h"           // IWYU pragma: keep
#include "mlir/Dialect/SCF/IR/SCF.h"                // IWYU pragma: keep
#include "mlir/IR/Attributes.h"                     // IWYU pragma: keep
#include "mlir/IR/Builders.h"                       // IWYU pragma: keep
#include "mlir/IR/BuiltinTypes.h"                   // IWYU pragma: keep
#include "mlir/IR/OpImplementation.h"               // IWYU pragma: keep
#include "mlir/IR/OperationSupport.h"               // IWYU pragma: keep
#include "mlir/Interfaces/CallInterfaces.h"         // IWYU pragma: keep
#include "mlir/Interfaces/FunctionImplementation.h" // IWYU pragma: keep
#include "mlir/Interfaces/FunctionInterfaces.h"     // IWYU pragma: keep
#include "mlir/Support/LLVM.h"                      // IWYU pragma: keep
#include "llvm/ADT/ArrayRef.h"                      // IWYU pragma: keep
#include "llvm/ADT/StringRef.h"                     // IWYU pragma: keep

#define GET_OP_CLASSES
#include "QPin/IR/QPinOps.cpp.inc" // adds ops logic

namespace aqomplice {
namespace qpin {
using namespace mlir;

/**
 * @brief Verify that the operand qubits of an unitary operation aren't used
 * after measurement.
 */
// template <typename UnitaryOp>
// bool usedAfterMeasurement(const llvm::DenseSet<Value> &measured,
//                           const Operation &op) {
//   if (auto u = dyn_cast<UnitaryOp>(op)) {
//     bool used = measured.find(u.getTarget()) != measured.end();
//     if (auto ctrl = u.getControl()) {
//       used |= measured.find(ctrl) != measured.end();
//     }
//     return used;
//   }

//   return false;
// }

// /**
//  * @brief Verify that the operand qubits of an unitary operation aren't used
//  * after measurement.
//  */
// template <>
// bool usedAfterMeasurement<qpin::SwapOp>(const llvm::DenseSet<Value> &measured,
//                                         const Operation &op) {
//   if (auto u = dyn_cast<qpin::SwapOp>(op)) {
//     return measured.find(u.getA()) != measured.end() &&
//            measured.find(u.getB()) != measured.end();
//   }
//   return false;
// }

/**
 * @brief Verify that the kernel fulfills QIR's base profile.
 */
// llvm::LogicalResult KernelOp::verifyRegions() {
//   llvm::DenseSet<Value> measured{};
//   for (auto &op : getBody().getOps()) {
//     if (isa<scf::SCFDialect>(op.getDialect())) {
//       return emitOpError() << "No classical control flow elements are allowed "
//                               "in a kernel (as of now).";
//     }

//     if (isa<arith::ArithDialect>(op.getDialect())) {
//       if (!isa<arith::ConstantOp>(op)) {
//         return emitOpError() << "No arithmetic or other calculations may be "
//                                 "performed with classical values.";
//       }
//     }

//     if (!isa<qpin::QPinDialect>(op.getDialect())) {
//       continue;
//     }

//     if (auto measureOp = dyn_cast<qpin::MeasureOp>(op)) {
//       measured.insert(measureOp.getQubit());

//     } else { // Gate operations
//       if (usedAfterMeasurement<qpin::HOp>(measured, op) ||
//           usedAfterMeasurement<qpin::XOp>(measured, op) ||
//           usedAfterMeasurement<qpin::YOp>(measured, op) ||
//           usedAfterMeasurement<qpin::ZOp>(measured, op) ||
//           usedAfterMeasurement<qpin::SOp>(measured, op) ||
//           usedAfterMeasurement<qpin::TOp>(measured, op) ||
//           usedAfterMeasurement<qpin::SwapOp>(measured, op)) {
//         return emitOpError() << "Once a qubit is measured, nothing further "
//                                 "will be done with it other than releasing it.";
//       }
//     }
//   }

//   return success();
// }
}; // namespace qpin
}; // namespace aqomplice