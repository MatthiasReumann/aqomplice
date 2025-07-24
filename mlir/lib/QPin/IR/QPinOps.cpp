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
template <typename UnitaryOp>
bool usedAfterMeasurement(const llvm::DenseSet<Value> &measured,
                          const Operation &op) {
  if (auto u = mlir::dyn_cast<UnitaryOp>(op)) {
    bool used = measured.find(u.getTarget()) != measured.end();
    if (auto ctrl = u.getControl()) {
      used |= measured.find(ctrl) != measured.end();
    }
    return used;
  }

  return false;
}

/**
 * @brief Verify that the operand qubits of an unitary operation aren't used
 * after measurement.
 */
template <>
bool usedAfterMeasurement<qpin::SwapOp>(const llvm::DenseSet<Value> &measured,
                                        const Operation &op) {
  if (auto u = mlir::dyn_cast<qpin::SwapOp>(op)) {
    return measured.find(u.getA()) != measured.end() &&
           measured.find(u.getB()) != measured.end();
  }
  return false;
}

/**
 * @brief Verify that the kernel fulfills QIR's base profile.
 */
llvm::LogicalResult KernelOp::verifyRegions() {
  llvm::DenseSet<Value> measured{};
  for (auto &op : getBody().getOps()) {
    if (isa<scf::SCFDialect>(op.getDialect())) {
      return emitOpError() << "No classical control flow elements are allowed "
                              "in a kernel (as of now).";
    }

    if (isa<arith::ArithDialect>(op.getDialect())) {
      if (!isa<arith::ConstantOp>(op)) {
        return emitOpError() << "No arithmetic or other calculations may be "
                                "performed with classical values.";
      }
    }

    if (!isa<qpin::QPinDialect>(op.getDialect())) {
      continue;
    }

    if (auto measureOp = mlir::dyn_cast<qpin::MeasureOp>(op)) {
      measured.insert(measureOp.getQubit());
    }

    if (usedAfterMeasurement<qpin::HOp>(measured, op) ||
        usedAfterMeasurement<qpin::XOp>(measured, op) ||
        usedAfterMeasurement<qpin::YOp>(measured, op) ||
        usedAfterMeasurement<qpin::ZOp>(measured, op) ||
        usedAfterMeasurement<qpin::SOp>(measured, op) ||
        usedAfterMeasurement<qpin::TOp>(measured, op) ||
        usedAfterMeasurement<qpin::SwapOp>(measured, op)) {
      return emitOpError() << "Once a qubit is measured, nothing further "
                              "will be done with it other than releasing it.";
    }
  }

  return mlir::success();
}

void KernelOp::build(OpBuilder &builder, OperationState &state,
                     llvm::StringRef name, FunctionType type,
                     llvm::ArrayRef<NamedAttribute> attrs) {
  // FunctionOpInterface provides a convenient `build` method that will populate
  // the state of our FuncOp, and create an entry block.
  buildWithEntryBlock(builder, state, name, type, attrs, type.getInputs());
}

ParseResult KernelOp::parse(OpAsmParser &parser, OperationState &result) {
  // Dispatch to the FunctionOpInterface provided utility method that parses the
  // function operation.
  auto buildFuncType =
      [](Builder &builder, llvm::ArrayRef<Type> argTypes,
         llvm::ArrayRef<Type> results, function_interface_impl::VariadicFlag,
         std::string &) { return builder.getFunctionType(argTypes, results); };

  return function_interface_impl::parseFunctionOp(
      parser, result, false, getFunctionTypeAttrName(result.name),
      buildFuncType, getArgAttrsAttrName(result.name),
      getResAttrsAttrName(result.name));
}

void KernelOp::print(OpAsmPrinter &p) {
  // Dispatch to the FunctionOpInterface provided utility method that prints the
  // function operation.
  function_interface_impl::printFunctionOp(
      p, *this, false, getFunctionTypeAttrName(), getArgAttrsAttrName(),
      getResAttrsAttrName());
}

/**
 * @brief Return the callee of the generic call operation.
 * @note This is required by the call interface.
 */
CallInterfaceCallable CallOp::getCallableForCallee() {
  return (*this)->getAttrOfType<SymbolRefAttr>("callee");
}

/**
 * @brief Set the callee for the generic call operation.
 * @note This is required by the call interface.
 */
void CallOp::setCalleeFromCallable(CallInterfaceCallable callee) {
  (*this)->setAttr("callee", cast<SymbolRefAttr>(callee));
}

/**
 * @brief Get the argument operands to the called function.
 * @note This is required by the call interface.
 */
Operation::operand_range CallOp::getArgOperands() { return getOperands(); }

/**
 * @brief Get the argument operands to the called function as a mutable range.
 * @note This is required by the call interface.
 */
MutableOperandRange CallOp::getArgOperandsMutable() {
  return getOperandsMutable();
}
}; // namespace qpin
}; // namespace aqomplice