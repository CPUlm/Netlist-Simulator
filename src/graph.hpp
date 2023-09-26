#ifndef NETLIST_GRAPH_HPP
#define NETLIST_GRAPH_HPP

#include <list>
#include <vector>

enum Mark {
  NotVisited,
  InProgress,
  Visited,
};

template <class T> struct Node {
  T label;
  Mark mark;
  std::vector<Node<T>> link_to;
  std::vector<Node<T>> linked_by;
};

template <class T> struct Graph {
  std::vector<Node<T>> nodes;
};

template <class T> Graph<T> mk_graph();

template <class T> void add_node(Graph<T> &graph, const T &label);

template <class T> Node<T> &node_of_label(Graph<T> &graph, const T &label);

template <class T> void add_edge(Graph<T> &graph, const T &id1, const T &id2);

template <class T> void clear_marks(Graph<T> &graph);

template <class T> std::vector<Node<T> &> find_roots(Graph<T> &graph);

template <class T> bool has_cycle(Graph<T> &graph);

template <class T> std::list<Node<T>> topological(Graph<T> &graph);

#endif // NETLIST_GRAPH_HPP