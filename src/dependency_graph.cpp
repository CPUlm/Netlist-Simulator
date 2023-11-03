#include "dependency_graph.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <ranges>

// ========================================================
// struct DependencyGraph::Builder
// ========================================================

struct DependencyGraph::Builder : ConstInstructionVisitor {
  DependencyGraph &graph;

  explicit Builder(DependencyGraph &g) : graph(g) {}

  void visit_const(const ConstInstruction &inst) override {}
  void visit_load(const LoadInstruction &inst) override;
  void visit_not(const NotInstruction &inst) override;
  void visit_reg(const RegInstruction &inst) override;
  void visit_mux(const MuxInstruction &inst) override;
  void visit_concat(const ConcatInstruction &inst) override;
  void visit_and(const AndInstruction &inst) override;
  void visit_nand(const NandInstruction &inst) override;
  void visit_or(const OrInstruction &inst) override;
  void visit_nor(const NorInstruction &inst) override;
  void visit_xor(const XorInstruction &inst) override;
  void visit_xnor(const XnorInstruction &inst) override;
  void visit_select(const SelectInstruction &inst) override;
  void visit_slice(const SliceInstruction &inst) override;
  void visit_rom(const RomInstruction &inst) override;
  void visit_ram(const RamInstruction &inst) override;
};

void DependencyGraph::Builder::visit_load(const LoadInstruction &inst) {
  graph.add_dependency(inst.output, inst.input);
}

void DependencyGraph::Builder::visit_not(const NotInstruction &inst) {
  graph.add_dependency(inst.output, inst.input);
}

void DependencyGraph::Builder::visit_reg(const RegInstruction &inst) {
  // graph.add_dependency(inst.output, inst.input);
}

void DependencyGraph::Builder::visit_mux(const MuxInstruction &inst) {
  graph.add_dependency(inst.output, inst.choice);
  graph.add_dependency(inst.output, inst.first);
  graph.add_dependency(inst.output, inst.second);
}

void DependencyGraph::Builder::visit_concat(const ConcatInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_and(const AndInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_nand(const NandInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_or(const OrInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_nor(const NorInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_xor(const XorInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_xnor(const XnorInstruction &inst) {
  graph.add_dependency(inst.output, inst.lhs);
  graph.add_dependency(inst.output, inst.rhs);
}

void DependencyGraph::Builder::visit_select(const SelectInstruction &inst) {
  graph.add_dependency(inst.output, inst.input);
}

void DependencyGraph::Builder::visit_slice(const SliceInstruction &inst) {
  graph.add_dependency(inst.output, inst.input);
}

void DependencyGraph::Builder::visit_rom(const RomInstruction &inst) {
  graph.add_dependency(inst.output, inst.read_addr);
}

void DependencyGraph::Builder::visit_ram(const RamInstruction &inst) {
  graph.add_dependency(inst.output, inst.read_addr);
  // There is no dependencies to the other registers.
  // graph.add_dependency(inst.output, inst.write_enable);
  // graph.add_dependency(inst.output, inst.write_addr);
  // graph.add_dependency(inst.output, inst.write_data);
}

// ========================================================
// class DependencyGraph
// ========================================================

DependencyGraph::DependencyGraph(const std::shared_ptr<Program> &program) : m_program(program) {
  m_adjacency_list = std::vector(program->registers.size(), std::vector<reg_t>{});
}

DependencyGraph DependencyGraph::build(const std::shared_ptr<Program> &program) {
  assert(program != nullptr);

  DependencyGraph graph(program);
  DependencyGraph::Builder builder(graph);

  for (const auto &instruction : program->instructions)
    instruction->visit(builder);

  return builder.graph;
}

bool DependencyGraph::depends(reg_t from, reg_t to) const {
  auto &edges = m_adjacency_list[from.index];
  auto it = std::ranges::find(edges, to);
  return it != edges.end();
}

enum class VertexState {
  NOT_VISITED,
  IN_PROGRESS,
  VISITED,
};

// Helper class to implement the DFS needed for the topological sort of
// the dependency graph.
struct DFSVisitor {
  ReportManager &report_manager;
  const std::vector<std::vector<reg_t>> &adjacency_list;
  std::vector<VertexState> states;
  std::vector<reg_t> topological_sort;

  void visit(reg_t reg) {
    if (states[reg.index] == VertexState::VISITED)
      return;

    if (states[reg.index] == VertexState::IN_PROGRESS) {
      // We got a cycle.
      report_manager.report(ReportSeverity::ERROR)
          .with_message("cycle detected in the dependency graph")
          .finish()
          .exit();
    }

    states[reg.index] = VertexState::IN_PROGRESS;

    const auto &dependencies = adjacency_list[reg.index];
    for (reg_t dependency : dependencies)
      visit(dependency);

    states[reg.index] = VertexState::VISITED;
    topological_sort.push_back(reg);
  }
};

void DependencyGraph::schedule(ReportManager &report_manager) {
  // The output register corresponds to the "label" of the equation.
  // So we do a mapping from the output register and its corresponding instruction.
  // Because a register may be written to multiple times, there can be many
  // corresponding instructions thus the multimap.
  std::multimap<reg_t, Instruction *> reg_instruction_mapping;
  for (auto *instruction : m_program->instructions) {
    reg_instruction_mapping.insert({instruction->output, instruction});
  }

  std::vector<Instruction *> new_order;
  new_order.reserve(m_program->instructions.size());

  // Reorder instructions using the topological sort.
  for (auto reg : topological_sort(report_manager)) {
    auto range = reg_instruction_mapping.equal_range(reg);
    for (auto it = range.first; it != range.second; ++it)
      new_order.push_back(it->second);
  }

  m_program->instructions = new_order;
}

void DependencyGraph::dump_dot() {
  dump_dot(std::cout);
}

void DependencyGraph::dump_dot(std::ostream &out) {
  std::cout << "digraph DependencyGraph {\n";

  for (std::uint_least32_t from = 0; from < m_adjacency_list.size(); ++from) {
    const auto &reg_info = m_program->registers[from];

    std::cout << "  _" << from << "[label=\""
              << "%" << from;

    if (!reg_info.name.empty()) {
      std::cout << " (aka '" << reg_info.name << "')";
    }

    if (reg_info.flags & RIF_INPUT) {
      std::cout << "\\nINPUT";
    }

    if (reg_info.flags & RIF_OUTPUT) {
      std::cout << "\\nOUTPUT";
    }

    std::cout << "\", shape=box];\n";

    for (auto to : m_adjacency_list[from]) {
      std::cout << "  _" << from << " -> _" << to.index << ";\n";
    }
  }

  std::cout << "}\n";
}

void DependencyGraph::add_dependency(reg_t from, reg_t to) {
  auto &edges = m_adjacency_list[from.index];
  auto it = std::ranges::find(edges, to);
  if (it != edges.end())
    return;

  edges.push_back(to);
}

std::vector<reg_t> DependencyGraph::topological_sort(ReportManager &report_manager) const {
  DFSVisitor visitor = {
      report_manager, m_adjacency_list, std::vector(m_adjacency_list.size(), VertexState::NOT_VISITED), {}};

  for (std::uint_least32_t i = 0; i < m_program->registers.size(); ++i) {
    const auto &reg_info = m_program->registers[i];
    visitor.visit({i});
  }

  return visitor.topological_sort;
}
