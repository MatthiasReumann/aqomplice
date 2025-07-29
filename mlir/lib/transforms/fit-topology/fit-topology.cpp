#include "transforms/fit-topology/fit-toplogy.h"

#include "QPin/IR/QPinDialect.h"

#include <initializer_list>
#include <queue>
#include <unordered_set>

namespace aqomplice {
#define GEN_PASS_DEF_FITTOPOLOGY
#include "transforms/passes.h.inc" // adds `impl::FitTopologyBase`

namespace {
template <std::size_t vN, std::size_t eN> class Architecture {
private:
public:
  using Edge = std::tuple<std::size_t, std::size_t>;

  Architecture(const std::initializer_list<Edge> &edges) {
    std::copy(edges.begin(), edges.end(), edges_.begin());
  };

  constexpr std::size_t getNQubits() const { return vN; }

  bool hasEdgeBetween(std::size_t a, std::size_t b) const {
    return hasEdge({a, b});
  }

  std::array<Edge, eN> getEdges() const { return edges_; }

  std::vector<std::size_t> getShortestPathBetween(std::size_t a,
                                                  std::size_t b) const {
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

private:
  bool hasEdge(const Edge &e) const {
    return std::find(edges_.cbegin(), edges_.cend(), e) != edges_.cend();
  }

  std::array<Edge, eN> edges_;
};

template <std::size_t vN, std::size_t eN>
void localOptimalSwap(const Architecture<vN, eN> &architecture,
                      mlir::OpBuilder &builder, mlir::Operation *op) {

  std::array<mlir::Value, vN> indices{};
  llvm::DenseMap<mlir::Value, mlir::Value> permutation{};

  std::ignore = op->walk([&](mlir::Operation *op) {
    if (auto qubitOp = mlir::dyn_cast<qpin::QubitOp>(op)) {
      indices[qubitOp.getIndex()] = qubitOp.getQubit();
    } else if (auto x = mlir::dyn_cast<qpin::XOp>(op)) {
      auto ctrl = x.getControl();
      if (!ctrl) { // Single Qubit Gates don't require mapping.
        return mlir::WalkResult::advance();
      }

      auto tgt = x.getTarget();

      // Identity mapping.
      if (!permutation[tgt]) {
        permutation[tgt] = tgt;
      }
      if (!permutation[ctrl]) {
        permutation[ctrl] = ctrl;
      }

      const std::size_t tI =
          mlir::dyn_cast<qpin::QubitOp>(permutation[tgt].getDefiningOp())
              .getIndex();
      const std::size_t cI =
          mlir::dyn_cast<qpin::QubitOp>(permutation[ctrl].getDefiningOp())
              .getIndex();

      if (architecture.hasEdgeBetween(tI, cI)) { // No SWAP required.
        return mlir::WalkResult::advance();
      }

      std::vector<std::size_t> path =
          architecture.getShortestPathBetween(tI, cI);

      builder.setInsertionPoint(x);
      for (auto it = path.rbegin(); it != std::prev(path.rend()); ++it) {
        const std::size_t a = *it;
        const std::size_t b = *(it + 1);

        // Add static qubits if they don't exist.
        if (!indices[a]) {
          auto qubitType = qpin::StaticQubitType::get(x.getContext());
          auto qubitOp =
              builder.create<qpin::QubitOp>(x->getLoc(), qubitType, a);
          indices[a] = qubitOp.getQubit();
        }
        if (!indices[b]) {
          auto qubitType = qpin::StaticQubitType::get(x.getContext());
          auto qubitOp =
              builder.create<qpin::QubitOp>(x->getLoc(), qubitType, b);
          indices[b] = qubitOp.getQubit();
        }

        const mlir::Value qa = indices[a];
        const mlir::Value qb = indices[b];

        builder.create<qpin::SwapOp>(x->getLoc(), qa, qb);

        permutation[qa] = qb;
        permutation[qb] = qa;

        std::swap(indices[a], indices[b]);
      }
    }

    return mlir::WalkResult::advance();
  });
}
}; // namespace

struct FitTopology : impl::FitTopologyBase<FitTopology> {
  using FitTopologyBase::FitTopologyBase;

  void runOnOperation() override {
    mlir::MLIRContext *context = &getContext();
    mlir::Operation *op = getOperation();

    mlir::OpBuilder builder(context);
    if (this->arch == "iqm-spark") {

      // clang-format off
      // IQM Spark 5-Qubit Star-Like Architecture
      //       QB0
      //        |
      // QB1 - QB2 - QB3
      //        |
      //       QB4
      Architecture<5, 8> spark({
        {0, 2}, {2, 0}, 
        {1, 2}, {2, 1}, 
        {2, 3}, {3, 2}, 
        {2, 4}, {4, 2}});
      // clang-format on

      localOptimalSwap(spark, builder, op);
    }
  }
};
} // namespace aqomplice