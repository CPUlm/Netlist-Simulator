#include <gtest/gtest.h>
#include <algorithm>

#include "graph.hpp"

const std::vector<std::vector<int>> g1 = {{1}, {1}, {1}};
const std::vector<std::vector<int>> g2 = {{}, {2}, {3}, {4}, {0, 1}};
const std::vector<std::vector<int>> g3 = {{1}, {2}, {3}, {}};
const std::vector<std::vector<int>> g4 = {{3}, {0, 2, 4}, {3}, {}, {6}, {1, 4, 7}, {3, 8}, {6, 9}, {}, {8}};

Graph<int> make(const std::vector<std::vector<int>> &graph) {
  Graph<int> g;
  for (int i = 0; i < graph.size(); i++) {
    add_node(g, i);
  }

  for (int i = 0; i < graph.size(); i++) {
    for (int j = 0; j < graph[i].size(); ++j) {
      add_edge(g, i, graph[i][j]);
    }
  }

  return g;
}

bool check_topo_order(Graph<int> &g) {
  auto topo_order = topological(g);
  if (topo_order.size() != g.nodes.size()) {
    return false;
  }

  if (!std::all_of(topo_order.begin(),
                   topo_order.end(),
                   [&g](int i) -> bool { return 0 <= i && i < g.nodes.size(); })) {
    return false;
  }

  std::vector<std::optional<int>> inverse_topo_order(g.nodes.size(), std::nullopt);
  for (int i = 0; i < g.nodes.size(); ++i) {
    inverse_topo_order[topo_order[i]] = i;
  }

  return std::all_of(g.nodes.begin(), g.nodes.end(), [&inverse_topo_order](Node<int> &src) -> bool {
    return std::all_of(src.link_to.begin(), src.link_to.end(), [&src, &inverse_topo_order](Node<int> &dst) -> bool {
      auto src_inv = inverse_topo_order[src.label];
      auto dst_inv = inverse_topo_order[dst.label];
      return src_inv.has_value() && dst_inv.has_value()
          && src_inv.value() < dst_inv.value();
    });
  });
}

class GraphTest : public ::testing::Test {
public:
  Graph<int> graph1 = make(g1);
  Graph<int> graph2 = make(g2);
  Graph<int> graph3 = make(g3);
  Graph<int> graph4 = make(g4);

};

TEST_F(GraphTest, has_cycle) {
  EXPECT_TRUE(has_cycle(graph1));
  EXPECT_TRUE(has_cycle(graph2));
  EXPECT_FALSE(has_cycle(graph3));
  EXPECT_FALSE(has_cycle(graph4));
}

TEST_F(GraphTest, topological) {
  EXPECT_THROW(check_topo_order(graph1), HasCycle);
  EXPECT_THROW(check_topo_order(graph2), HasCycle);
  EXPECT_TRUE(check_topo_order(graph3));
  EXPECT_TRUE(check_topo_order(graph4));
}
