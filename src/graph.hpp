#ifndef NETLIST_GRAPH_HPP
#define NETLIST_GRAPH_HPP

#include <stdexcept>
#include <vector> // vector = dynamic size array

class HasCycle : public std::exception {}; // hérite de exception, permet de faire throw HasCycle

enum Mark {
  NotVisited,
  InProgress,
  Visited,
};

template <class T> struct Node { // Permet d'avoir un type générique en argument de Node, par ex Node<std::string> ou Node<int>.
  T label;
  Mark mark;
  std::vector<Node<T>> link_to; 
  std::vector<Node<T>> linked_by;
};

template <class T> struct Graph {
  std::vector<Node<T>> nodes;
};

template <class T> Graph<T> mk_graph() { return Graph<T>(); }

template <class T> void add_node(Graph<T> &graph, const T &label) { // & permet de passer une référence, le même objet mais sous un autre nom, on est sûr que c'est pas NULL.
  graph.nodes.push_back({label, NotVisited, {}, {}}); // push_back = ajoute à la fin
}

template <class T> Node<T> &node_of_label(Graph<T> &graph, const T &label) {
  for (Node<T> &node : graph.nodes) { // variable de stockage : élément parcouru (un std::vector). & précise qu'on en prend une référence, évite de copier l'objet inutiliement
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
    n1.link_to.emplace_back(n2);  // ajoute la référence à la fin, et non une copie de l'objet (comme l'aurait fait push_back)
    n2.linked_by.emplace_back(n1); // ajoute la référence à la fin, et non une copie de l'objet (comme l'aurait fait push_back)
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
      result.emplace_back(node); // ajoute la référence à la fin, et non une copie de l'objet (comme l'aurait fait push_back)
    }
  }

  return result;
}

template <class T> bool dfs_has_cycle(Node<T> &vertex){ //aux function for has_cycle
  for(Node<T> &node : vertex.link_to){
    switch (node.mark)
    {
    case InProgress:
      return true; // we got a cycle
      break;
    case NotVisited:
      node.mark = InProgress;
      if (dfs_has_cycle(node)) return true;
    default: // alreaady visited
      break;
    }
  }
  vertex.mark = Visited;
  return false;
}
template <class T> bool has_cycle(Graph<T> &graph) { 
  for(Node<T> &vertex : graph.nodes)
  {
    switch (vertex.mark)
    {
    case NotVisited: // visit the CC of vertex
      vertex.mark = InProgress;
      if(dfs_has_cycle(vertex)){
        clear_marks(graph);
        return true;
      }
      break;
    default:
      break;
    }
  }
  clear_marks(graph);
  return false; 
 
}

template <class T> void dfs_topological(Node<T> &vertex, std::vector<T> &res){ //aux function for has_cycle
	for(Node<T> &node : vertex.link_to){
		switch (node.mark)
		{
		case InProgress:
			return true; // we got a cycle
			break;
		case NotVisited:
			node.mark = InProgress;
			dfs_topological(node,res);
		default: // already visited
			break;
		}
	}
	vertex.mark = Visited;
	res.emplace(vertex);
	return;
}

// Must throw 'HasCycle' exception when given graph is cyclic. (Needed to pass
// tests)
template <class T> std::vector<T> topological(Graph<T> &graph) {
    /*
    Topological sort = reverse DFS order 
    */
	std::vector<T> res = std::vector<T>();
	for(Node<T> &vertex : graph.nodes)
	{
		switch (vertex.mark)
		{
		case NotVisited: // visit the CC of vertex
			vertex.mark = InProgress;
			dfs_topological(vertex);
			break;
		default:
			break;
		}
	}
	clear_marks(graph);
	return res; // We added at the beginning => no need to reverse.

}

#endif // NETLIST_GRAPH_HPP
