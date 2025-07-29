#ifndef AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_ARCHITECTURE_H
#define AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_ARCHITECTURE_H

#include "mlir/IR/Builders.h"
#include "mlir/IR/Value.h"

#include <cstddef>
#include <initializer_list>

namespace aqomplice {
class Architecture {
private:
public:
  using Edge = std::tuple<std::size_t, std::size_t>;

  Architecture(const std::size_t nqubits,
               const std::initializer_list<Edge> &edges)
      : nqubits_(nqubits), edges_(edges) {};

  constexpr std::size_t getNQubits() const { return nqubits_; }

  bool hasEdgeBetween(std::size_t a, std::size_t b) const {
    return hasEdge({a, b});
  }

  std::vector<Edge> getEdges() const { return edges_; }

  std::vector<std::size_t> getShortestPathBetween(std::size_t a,
                                                  std::size_t b) const;

  void localOptimalSwap(mlir::OpBuilder &builder, mlir::Operation *op);

private:
  bool hasEdge(const Edge &e) const {
    return std::find(edges_.cbegin(), edges_.cend(), e) != edges_.cend();
  }

  void
  localOptimalPerformSwap(mlir::OpBuilder &builder, mlir::Operation *op,
                          llvm::DenseMap<mlir::Value, mlir::Value> &permutation,
                          std::vector<mlir::Value> &indices, mlir::Value a,
                          mlir::Value b);

  std::size_t nqubits_;
  std::vector<Edge> edges_;
};
} // namespace aqomplice

#endif // AQOMPLICE_TRANSFORM_FIT_TOPOLOGY_ARCHITECTURE_H