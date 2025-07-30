#include "transforms/fit-topology/architecture.h"

#include "mlir/IR/Builders.h"
#include "mlir/IR/Value.h"

#include "QPin/IR/QPinDialect.h"

#include <queue>
#include <unordered_set>

namespace aqomplice {
std::vector<std::size_t>
Architecture::getShortestPathBetween(std::size_t a, std::size_t b) const {
  struct Node {
    std::size_t i;
    Node *parent;
  };

  std::queue<Node> q{};
  std::unordered_set<std::size_t> visited{};

  q.push({a, nullptr});
  while (!q.empty()) {
    Node &curr = q.front();
    q.pop();

    if (curr.i == b) {
      Node *it = &curr;
      std::vector<std::size_t> path{};
      while (it->parent != nullptr) { // while we are not at the root:
        it = it->parent;              // update the iterator
        path.push_back(it->i);        // add the path
      }
      return path;
    }

    for (const auto &[from, to] : getEdges()) { // iterate possible successors
      // if we haven't visited the successor:
      if (from == curr.i && visited.find(to) == visited.end()) {
        q.push({to, &curr}); // add it to the queue.
      }
    }

    visited.insert(curr.i);
  }
  return {};
}

mlir::Value Architecture::getOrCreateQubit(mlir::Operation *op,
                                           mlir::OpBuilder &builder,
                                           std::vector<mlir::Value> &indices,
                                           const std::size_t i) const {
  if (!indices[i]) {
    auto qubitType = qpin::StaticQubitType::get(op->getContext());
    auto qubitOp = builder.create<qpin::QubitOp>(op->getLoc(), qubitType, i);
    indices[i] = qubitOp.getQubit();
  }
  return indices[i];
}

void Architecture::localOptimalPerformSwap(
    mlir::OpBuilder &builder, mlir::Operation *op,
    llvm::DenseMap<mlir::Value, mlir::Value> &permutation,
    std::vector<mlir::Value> &indices, mlir::Value a, mlir::Value b) const {
  if (!a || !b) { // Return early if one of the parameters is null.
    return;
  }

  // Identity mapping.
  permutation.try_emplace(a, a);
  permutation.try_emplace(b, b);

  auto aDefiningOp = permutation[a].getDefiningOp();
  auto bDefiningOp = permutation[b].getDefiningOp();
  auto aQubitOp = mlir::dyn_cast<qpin::QubitOp>(aDefiningOp);
  auto bQubitOp = mlir::dyn_cast<qpin::QubitOp>(bDefiningOp);

  if (!aQubitOp || !bQubitOp) { // Return early if one is invalid.
    return;
  }

  const std::size_t tI = aQubitOp.getIndex();
  const std::size_t cI = bQubitOp.getIndex();

  if (hasEdgeBetween(tI, cI)) { // No SWAP required.
    return;
  }

  const std::vector<std::size_t> path = getShortestPathBetween(tI, cI);
  if (path.size() < 2) { // Return early if the path is empty or invalid.
    return;
  }

  builder.setInsertionPoint(op);
  for (auto it = path.rbegin(); it != std::prev(path.rend()); ++it) {
    const std::size_t x = *it;
    const std::size_t y = *(it + 1);

    // Get qubits by their indices (static qubit values). Add qpin.qubit if it
    // doesn't exist.
    const mlir::Value qa = getOrCreateQubit(op, builder, indices, x);
    const mlir::Value qb = getOrCreateQubit(op, builder, indices, y);

    builder.create<qpin::SwapOp>(op->getLoc(), qa, qb);

    permutation.try_emplace(qa, qa);
    permutation.try_emplace(qb, qb);

    std::swap(permutation[qa], permutation[qb]);
    std::swap(indices[x], indices[y]);
  }
}

void Architecture::localOptimalSwap(mlir::OpBuilder &builder,
                                    mlir::Operation *op) const {

  std::vector<mlir::Value> indices(getNQubits());
  llvm::DenseMap<mlir::Value, mlir::Value> permutation{};

  std::ignore = op->walk([&](mlir::Operation *op) {
    if (auto qubitOp = mlir::dyn_cast<qpin::QubitOp>(op)) {
      indices[qubitOp.getIndex()] = qubitOp.getQubit();
    } else if (auto unitary =
                   mlir::dyn_cast<qpin::ControlledUnitaryOpInterface>(op)) {
      if (!unitary.hasControl()) { // Single Qubit Gates don't require mapping.
        return mlir::WalkResult::advance();
      }
      localOptimalPerformSwap(builder, unitary, permutation, indices,
                              unitary.getTarget(), unitary.getControl());
    } else if (auto swap = mlir::dyn_cast<qpin::SwapOp>(op)) {
      localOptimalPerformSwap(builder, swap, permutation, indices, swap.getA(),
                              swap.getB());
    }
    return mlir::WalkResult::advance();
  });

  // Permute measurement bits for correct output.
  op->walk([&](qpin::MeasureOp op) {
    builder.setInsertionPointAfter(op);
    auto newOp = builder.create<qpin::MeasureOp>(
        op->getLoc(), op.getBit().getType(), permutation[op.getQubit()]);
    op.getBit().replaceAllUsesWith(newOp.getBit());
    op->erase();
  });
}
} // namespace aqomplice