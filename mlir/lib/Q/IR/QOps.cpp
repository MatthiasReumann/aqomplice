#include "Q/IR/QDialect.h"

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
#include "Q/IR/QOps.cpp.inc" // adds ops logic

namespace aqomplice {
namespace q {
using namespace mlir;

/**
 * @brief Verify that the operand qubits of an unitary operation aren't used
 * after measurement.
 */
// template <typename UnitaryOp>
// bool usedAfterMeasurement(const llvm::DenseSet<Value> &measured,
//                           const Operation &op) {
//   if (auto u = mlir::dyn_cast<UnitaryOp>(op)) {
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
// bool usedAfterMeasurement<q::SwapOp>(const llvm::DenseSet<Value> &measured,
//                                      const Operation &op) {
//   if (auto u = mlir::dyn_cast<q::SwapOp>(op)) {
//     return measured.find(u.getA()) != measured.end() &&
//            measured.find(u.getB()) != measured.end();
//   }
//   return false;
// }

/**
 * @brief Verify that the kernel fulfills QIR's base profile.
 */
// llvm::LogicalResult verifyRegions() {
//   bool hasFree = false;
//   bool hasAlloc = false;
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

//     if (!isa<q::QDialect>(op.getDialect())) {
//       continue;
//     }

//     if (auto allocOp = mlir::dyn_cast<q::AllocOp>(op)) {
//       if (hasAlloc) {
//         return emitOpError()
//                << "A kernel must have exactly zero or one alloc calls.";
//       }
//       hasAlloc = true;

//     } else if (auto freeOp = mlir::dyn_cast<q::FreeOp>(op)) {
//       hasFree = true;

//     } else if (auto measureOp = mlir::dyn_cast<q::MeasureOp>(op)) {
//       measured.insert(measureOp.getQubit());

//     } else { // Gate operations
//       if (usedAfterMeasurement<q::HOp>(measured, op) ||
//           usedAfterMeasurement<q::XOp>(measured, op) ||
//           usedAfterMeasurement<q::YOp>(measured, op) ||
//           usedAfterMeasurement<q::ZOp>(measured, op) ||
//           usedAfterMeasurement<q::SOp>(measured, op) ||
//           usedAfterMeasurement<q::TOp>(measured, op) ||
//           usedAfterMeasurement<q::SwapOp>(measured, op)) {
//         return emitOpError() << "Once a qubit is measured, nothing further "
//                                 "will be done with it other than releasing it.";
//       }
//     }
//   }

//   if (hasAlloc && !hasFree) {
//     return emitOpError() << "Missing q.free.";
//   }

//   return mlir::success();
// }

}; // namespace q
}; // namespace aqomplice