#include "graph.hpp"
#include <stdexcept>

template <class T> Graph<T> mk_graph() { return Graph<T>(); }

template <class T> void add_node(Graph<T> &graph, const T &label) {
  graph.nodes.emplace_back(label, NotVisited, {}, {});
}

template <class T> Node<T> &node_of_label(Graph<T> &graph, const T &label) {
  for (Node<T> &node : graph.nodes) {
    if (node.label == label) {
      return node;
    }
  }

  throw std::runtime_error("Element not found.");
}

template <class T> void add_edge(Graph<T> &graph, const T &id1, const T &id2) {
  try {
    Node<T> &n1 = node_of_label(graph, id1);
    Node<T> &n2 = node_of_label(graph, id2);
    n1.link_to.emplace_back(n2);
    n2.linked_by.emplace_back(n1);
  } catch (std::runtime_error &e) {
    throw std::runtime_error(
        "Tried to add an edge between non-existing nodes.");
  }
}

template <class T> void clear_marks(Graph<T> &graph) {
  for (Node<T> &node : graph.nodes) {
    node.mark = NotVisited;
  }
}

template <class T> std::vector<Node<T> &> find_roots(Graph<T> &graph) {
  std::vector<Node<T> &> result = {};
  for (Node<T> &node : graph.nodes) {
    if (node.linked_by.empty()) {
      result.emplace_back(node);
    }
  }

  return result;
}

template <class T> bool has_cycle(Graph<T> &graph);

template <class T> std::list<Node<T>> topological(Graph<T> &graph);
