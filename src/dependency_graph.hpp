#ifndef NETLIST_SRC_DEPENDENCY_GRAPH_HPP
#define NETLIST_SRC_DEPENDENCY_GRAPH_HPP

#include "program.hpp"

// ========================================================
// class DependencyGraph
// ========================================================

/// \brief Represents a dependency graph for the a Netlist program.
///
/// Notably, it is in this class that the scheduling is done.
///
/// For example:
/// ```
/// std::shared_ptr<Program> program = ...;
/// DependencyGraph graph = DependencyGraph::build(program);
/// graph.schedule();
/// // program is now correctly scheduled
/// ```
class DependencyGraph {
public:
  /// Builds the dependency graph for the given program.
  static DependencyGraph build(const std::shared_ptr<Program> &program);

  /// Returns true if \a from depends on \a to.
  [[nodiscard]] bool depends(reg_t from, reg_t to) const;

  /// Reorder the instructions of the graph's program so all all dependencies
  /// are respected. This function corresponds to the first questions of the Tutorial.
  void schedule();

  /// Same as dump_dot(std::ostream&) with the std::cout argument.
  void dump_dot();
  /// Dumps to the given output stream a Graphviz DOT representation of the
  /// dependency graph for debugging purposes.
  void dump_dot(std::ostream &out);

private:
  explicit DependencyGraph(const std::shared_ptr<Program> &program);

  /// Adds a dependency between two registers.
  void add_dependency(reg_t from, reg_t to);

  /// Computes a topological sort of the dependency graph. The topological sort
  /// is computed using a DFS.
  [[nodiscard]] std::vector<reg_t> topological_sort() const;

private:
  struct Builder;
  std::shared_ptr<Program> m_program;
  std::vector<std::vector<reg_t>> m_adjacency_list;
};

#endif // NETLIST_SRC_DEPENDENCY_GRAPH_HPP
