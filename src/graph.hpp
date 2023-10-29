#ifndef NETLIST_GRAPH_HPP
#define NETLIST_GRAPH_HPP

#include <stdexcept>
#include <vector>
#include <unordered_map>
#include <algorithm>

class HasCycle : public std::exception {};

enum Mark {
  NotVisited,
  InProgress,
  Visited,
};

template<class T> class Node {
public:
  using NodeRefVector = std::vector<std::reference_wrapper<Node<T>>>;

  const T label;
  Mark mark;
  NodeRefVector link_to;
  NodeRefVector linked_by;

  explicit Node(const T &label) : label(label), mark(NotVisited), link_to(), linked_by() {}

  // Delete Node Copy Constructors
  Node(Node &) = delete;
  Node(const Node &) = delete;
};

template<typename T>
class Graph {
public:
  using iterator = typename std::unordered_map<T, Node<T>>::const_iterator;
  using LabelList = typename std::vector<std::reference_wrapper<const T>>;

  void add_node(const T &label) noexcept {
    nodes.emplace(std::piecewise_construct, std::forward_as_tuple(label), std::forward_as_tuple(label));
  }

  void add_edge(const T &src, const T &dst) {
    if (!nodes.contains(src)) {
      add_node(src);
    }
    if (!nodes.contains(dst)) {
      add_node(dst);
    }

    Node<T> &n1 = nodes.at(src);
    Node<T> &n2 = nodes.at(dst);
    n1.link_to.emplace_back(n2);
    n2.linked_by.emplace_back(n1);
  }

  [[nodiscard]] bool has_cycle() noexcept {
    clear_marks();

    for (auto &[_, n] : nodes) {
      if (n.mark == NotVisited && dfs_cycled(n)) {
        return true;
      }
    }

    return false;
  }

  LabelList topological() {
    clear_marks();
    LabelList l;

    for (auto &[_, n] : nodes) {
      if (n.mark == NotVisited) {
        topological_dfs(n, l);
      }
    }

    std::reverse(l.begin(), l.end());
    return l;
  }

  [[nodiscard]] size_t size() const noexcept {
    return nodes.size();
  }

  [[nodiscard]] iterator begin() const noexcept { return nodes.begin(); }

  [[nodiscard]] iterator end() const noexcept { return nodes.end(); }

private:
  void clear_marks() noexcept {
    for (auto &[_, node] : nodes) {
      node.mark = NotVisited;
    }
  }

  [[nodiscard]] bool dfs_cycled(Node<T> &n) { // NOLINT(*-no-recursion)
    n.mark = InProgress;

    for (Node<int> &child : n.link_to) {
      if (child.mark == InProgress) {
        return true;
      }

      if (child.mark == NotVisited && dfs_cycled(child)) {
        return true;
      }
    }

    n.mark = Visited;
    return false;
  }

  void topological_dfs(Node<T> &n, LabelList &l) { // NOLINT(*-no-recursion)
    n.mark = InProgress;

    for (Node<T> &child : n.link_to) {
      if (child.mark == InProgress) {
        throw HasCycle();
      }

      if (child.mark == NotVisited) {
        topological_dfs(child, l);
      }
    }

    n.mark = Visited;
    l.emplace_back(n.label);
  }

  std::unordered_map<T, Node<T>> nodes;
};

#endif // NETLIST_GRAPH_HPP
