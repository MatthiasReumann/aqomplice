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

void Architecture::localOptimalPerformSwap(
    mlir::OpBuilder &builder, mlir::Operation *op,
    llvm::DenseMap<mlir::Value, mlir::Value> &permutation,
    std::vector<mlir::Value> &indices, mlir::Value a, mlir::Value b) {
  // Identity mapping.
  if (!permutation[a]) {
    permutation[a] = a;
  }
  if (!permutation[b]) {
    permutation[b] = b;
  }

  const std::size_t tI =
      mlir::dyn_cast<qpin::QubitOp>(permutation[a].getDefiningOp()).getIndex();
  const std::size_t cI =
      mlir::dyn_cast<qpin::QubitOp>(permutation[b].getDefiningOp()).getIndex();

  if (hasEdgeBetween(tI, cI)) { // No SWAP required.
    return;
  }

  std::vector<std::size_t> path = getShortestPathBetween(tI, cI);

  builder.setInsertionPoint(op);
  for (auto it = path.rbegin(); it != std::prev(path.rend()); ++it) {
    const std::size_t x = *it;
    const std::size_t y = *(it + 1);

    // Add static qubits if they don't exist.
    if (!indices[x]) {
      auto qubitType = qpin::StaticQubitType::get(op->getContext());
      auto qubitOp = builder.create<qpin::QubitOp>(op->getLoc(), qubitType, x);
      indices[x] = qubitOp.getQubit();
    }
    if (!indices[y]) {
      auto qubitType = qpin::StaticQubitType::get(op->getContext());
      auto qubitOp = builder.create<qpin::QubitOp>(op->getLoc(), qubitType, b);
      indices[y] = qubitOp.getQubit();
    }

    const mlir::Value qa = indices[x];
    const mlir::Value qb = indices[y];

    builder.create<qpin::SwapOp>(op->getLoc(), qa, qb);

    permutation[qa] = qb;
    permutation[qb] = qa;

    std::swap(indices[x], indices[y]);
  }
}

void Architecture::localOptimalSwap(mlir::OpBuilder &builder,
                                    mlir::Operation *op) {

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
}
} // namespace aqomplice