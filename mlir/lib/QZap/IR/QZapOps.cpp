#include "QZap/IR/QZapDialect.h"

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
#include "QZap/IR/QZapOps.cpp.inc" // adds ops logic

namespace aqomplice {
namespace qzap {
using namespace mlir;

/**
 * @brief Verify that the operand qubits of an unitary operation aren't used
 * after measurement.
 */
// template <typename UnitaryOp>
// bool usedAfterMeasurement(const llvm::DenseSet<Value> &measured,
//                           const Operation &op) {
//   if (auto u = mlir::dyn_cast<UnitaryOp>(op)) {
//     bool used = measured.find(u.getAIn()) != measured.end();
//     if (auto ctrl = u.getBIn()) {
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
// bool usedAfterMeasurement<qzap::SwapOp>(const llvm::DenseSet<Value> &measured,
//                                         const Operation &op) {
//   if (auto u = mlir::dyn_cast<qzap::SwapOp>(op)) {
//     return measured.find(u.getAIn()) != measured.end() &&
//            measured.find(u.getBIn()) != measured.end();
//   }
//   return false;
// }

// template <typename UnitaryOp>
// void updateRetrievedButUnstored(llvm::DenseSet<Value> &retrievedButUnstored,
//                                 const Operation &op) {
//   if (auto u = mlir::dyn_cast<UnitaryOp>(op)) {
//     retrievedButUnstored.erase(u.getAIn());
//     retrievedButUnstored.erase(u.getAOut());

//     if (auto ctrl = u.getBIn()) {
//       retrievedButUnstored.erase(u.getBIn());
//       retrievedButUnstored.erase(u.getBOut());
//     }
//   }
// }

// template <>
// void updateRetrievedButUnstored<qzap::SwapOp>(
//     llvm::DenseSet<Value> &retrievedButUnstored, const Operation &op) {
//   if (auto u = mlir::dyn_cast<qzap::SwapOp>(op)) {
//     retrievedButUnstored.erase(u.getAIn());
//     retrievedButUnstored.erase(u.getBIn());
//     retrievedButUnstored.erase(u.getAOut());
//     retrievedButUnstored.erase(u.getBOut());
//   }
// }

/**
 * @brief Verify that the kernel fulfills QIR's base profile and value
 * semantics.
 */
// llvm::LogicalResult KernelOp::verifyRegions() {
//   bool hasFree = false;
//   bool hasAlloc = false;

//   llvm::DenseSet<Value> measured{};
//   llvm::DenseSet<Value> retrievedButUnstored{};

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

//     if (!isa<qzap::QZapDialect>(op.getDialect())) {
//       continue;
//     }

//     if (auto allocOp = mlir::dyn_cast<qzap::AllocOp>(op)) {
//       if (hasAlloc) {
//         return emitOpError()
//                << "A kernel must have exactly zero or one alloc calls.";
//       }
//       hasAlloc = true;

//     } else if (auto freeOp = mlir::dyn_cast<qzap::FreeOp>(op)) {
//       hasFree = true;

//     } else if (auto retrieveOp = mlir::dyn_cast<qzap::RetrieveOp>(op)) {
//       retrievedButUnstored.insert(retrieveOp.getQubit());

//     } else if (auto storeOp = mlir::dyn_cast<qzap::StoreOp>(op)) {
//       retrievedButUnstored.erase(storeOp.getQubit());

//     } else if (auto measureOp = mlir::dyn_cast<qzap::MeasureOp>(op)) {
//       measured.insert(measureOp.getQubitOut());
//       retrievedButUnstored.erase(measureOp.getQubitIn());
//       retrievedButUnstored.insert(measureOp.getQubitOut());

//     } else { // Gate operations
//       updateRetrievedButUnstored<qzap::HOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::XOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::YOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::ZOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::SOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::TOp>(retrievedButUnstored, op);
//       updateRetrievedButUnstored<qzap::SwapOp>(retrievedButUnstored, op);

//       if (usedAfterMeasurement<qzap::HOp>(measured, op) ||
//           usedAfterMeasurement<qzap::XOp>(measured, op) ||
//           usedAfterMeasurement<qzap::YOp>(measured, op) ||
//           usedAfterMeasurement<qzap::ZOp>(measured, op) ||
//           usedAfterMeasurement<qzap::SOp>(measured, op) ||
//           usedAfterMeasurement<qzap::TOp>(measured, op) ||
//           usedAfterMeasurement<qzap::SwapOp>(measured, op)) {
//         return emitOpError() << "Once a qubit is measured, nothing further "
//                                 "will be done with it other than releasing it.";
//       }
//     }
//   }

//   if (!retrievedButUnstored.empty()) {
//     return emitOpError() << "Missing qzap.store for: "
//                          << retrievedButUnstored.begin()->getLoc();
//   }

//   if (hasAlloc && !hasFree) {
//     return emitOpError() << "Missing qzap.free.";
//   }

//   return mlir::success();
// }
}; // namespace qzap
}; // namespace aqomplice