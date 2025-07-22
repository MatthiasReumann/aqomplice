#include "QPin/IR/QPinDialect.h"

#include "mlir/Dialect/Func/IR/FuncOps.h"           // IWYU pragma: keep
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
}; // namespace q
}; // namespace aqomplice