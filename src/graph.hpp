#ifndef NETLIST_GRAPH_HPP
#define NETLIST_GRAPH_HPP

#include <stdexcept>
#include <vector>
#include <iostream>
#include <functional>
#include <exception>

class HasCycle : public std::exception {};

enum Mark {
  NotVisited,
  InProgress,
  Visited,
};

template <class T> struct Node {
  T label;
  Mark mark;
  std::vector<std::reference_wrapper<Node<T>>> link_to;
  std::vector<std::reference_wrapper<Node<T>>> linked_by;
};

template <class T> struct Graph {
  std::vector<Node<T>> nodes;
};

template <class T> Graph<T> mk_graph() { return Graph<T>(); }

template <class T> void add_node(Graph<T> &graph, const T &label) {
  graph.nodes.push_back({label, NotVisited, {}, {}});
}

template <class T> Node<T> &node_of_label(Graph<T> &graph, const T &label) {
  for (Node<T> &node : graph.nodes) {
    if (node.label == label) {
      return node;
    }
  }

  throw std::runtime_error("Element not found.");
}

template <class T> void add_edge(Graph<T> &graph, const T &src, const T &dest) {
  try {
    Node<T> &n1 = node_of_label(graph, src);
    Node<T> &n2 = node_of_label(graph, dest);
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


template <class T> bool dfs(Node<T> &node)	{
	if (node.mark == Visited)	return false;
	if (node.mark == InProgress)	return true;
	
	// NotVisited :
	node.mark = InProgress;
	for (Node<T>& child : node.link_to)	{
		if (dfs(child)) return true;
	}
	node.mark = Visited;
	return false;
}
template <class T> bool has_cycle(Graph<T> &graph) {	
	clear_marks(graph);
	for (Node<T>& node : graph.nodes)	{
		if (dfs(node))	return true;
	}
	return false;
}

// Must throw 'HasCycle' exception when given graph is cyclic. (Needed to pass
// tests)
template <class T> void dfsPostfix(Node<T> &node,std::vector<T> &postfix)	{
	if (node.mark == Visited)	return;
	if (node.mark == InProgress)	throw HasCycle();
	
	// NotVisited :
	node.mark = InProgress;
	for (Node<T>& child : node.link_to)	{
		dfsPostfix(child, postfix);
	}
	postfix.push_back(node.label);
	node.mark = Visited;
}

template <class T> std::vector<T> topological(Graph<T> &graph) {
	std::vector<T> postfix;
	clear_marks(graph);
	for (Node<T>& node : graph.nodes)	{
		dfsPostfix(node, postfix);
	}
	std::reverse(postfix.begin(), postfix.end());
	return postfix;
}


#endif // NETLIST_GRAPH_HPP
