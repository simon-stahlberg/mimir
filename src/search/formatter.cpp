/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/formatter.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/search/algorithms/astar_eager/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/astar_lazy/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/brfs/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/gbfs_eager/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/gbfs_lazy/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/iw/event_handlers/statistics.hpp"
#include "mimir/search/algorithms/siw/event_handlers/statistics.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/partially_ordered_plan.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <ostream>

namespace mimir
{
namespace search
{
std::ostream& operator<<(std::ostream& out, const State& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const PackedStateImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const Plan& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const PartiallyOrderedPlan& element) { return mimir::print(out, element); }

namespace match_tree
{
template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<E>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundActionImpl>& element);
template std::ostream& operator<<(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundAxiomImpl>& element);

template<formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const INode<E>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const INode<formalism::GroundActionImpl>& element);
template std::ostream& operator<<(std::ostream& out, const INode<formalism::GroundAxiomImpl>& element);
}  // end match_tree

namespace astar_eager
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end astar_eager

namespace astar_lazy
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end astar_lazy

namespace brfs
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end brfs

namespace gbfs_eager
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end gbfs_eager

namespace gbfs_lazy
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end gbfs_lazy

namespace iw
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end iw

namespace siw
{
std::ostream& operator<<(std::ostream& out, const Statistics& element) { return mimir::print(out, element); }
}  // end siw

}  // end search

std::ostream& print(std::ostream& out, const mimir::search::astar_eager::Statistics& element)
{
    fmt::print(out,
               "[AStar] Search time: {}ms\n"
               "[AStar] Number of generated states: {}\n"
               "[AStar] Number of expanded states: {}\n"
               "[AStar] Number of pruned states: {}\n"
               "[AStar] Number of generated states until last f-layer: {}\n"
               "[AStar] Number of expanded states until last f-layer: {}\n"
               "[AStar] Number of pruned states until last f-layer: {}\n"
               "[AStar] Number of reached fluent atoms: {}\n"
               "[AStar] Number of reached derived atoms: {}\n"
               "[AStar] Number of states: {}\n"
               "[AStar] Number of nodes: {}",
               element.get_search_time_ms().count(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_generated_until_f_value().empty() ? 0 : element.get_num_generated_until_f_value().rbegin()->second,
               element.get_num_expanded_until_f_value().empty() ? 0 : element.get_num_expanded_until_f_value().rbegin()->second,
               element.get_num_pruned_until_f_value().empty() ? 0 : element.get_num_pruned_until_f_value().rbegin()->second,
               element.get_num_reached_fluent_atoms(),
               element.get_num_reached_derived_atoms(),
               element.get_num_states(),
               element.get_num_nodes());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::astar_lazy::Statistics& element)
{
    fmt::print(out,
               "[AStar] Search time: {}ms\n"
               "[AStar] Number of generated states: {}\n"
               "[AStar] Number of expanded states: {}\n"
               "[AStar] Number of pruned states: {}\n"
               "[AStar] Number of generated states until last f-layer: {}\n"
               "[AStar] Number of expanded states until last f-layer: {}\n"
               "[AStar] Number of pruned states until last f-layer: {}\n"
               "[AStar] Number of reached fluent atoms: {}\n"
               "[AStar] Number of reached derived atoms: {}\n"
               "[AStar] Number of states: {}\n"
               "[AStar] Number of nodes: {}",
               element.get_search_time_ms().count(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_generated_until_f_value().empty() ? 0 : element.get_num_generated_until_f_value().rbegin()->second,
               element.get_num_expanded_until_f_value().empty() ? 0 : element.get_num_expanded_until_f_value().rbegin()->second,
               element.get_num_pruned_until_f_value().empty() ? 0 : element.get_num_pruned_until_f_value().rbegin()->second,
               element.get_num_reached_fluent_atoms(),
               element.get_num_reached_derived_atoms(),
               element.get_num_states(),
               element.get_num_nodes());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::brfs::Statistics& element)
{
    fmt::print(out,
               "[BrFS] Search time: {}ms\n"
               "[BrFS] Number of generated states: {}\n"
               "[BrFS] Number of expanded states: {}\n"
               "[BrFS] Number of pruned states: {}\n"
               "[BrFS] Number of generated states until last f-layer: {}\n"
               "[BrFS] Number of expanded states until last f-layer: {}\n"
               "[BrFS] Number of pruned states until last f-layer: {}\n"
               "[BrFS] Number of reached fluent atoms: {}\n"
               "[BrFS] Number of reached derived atoms: {}\n"
               "[BrFS] Number of states: {}\n"
               "[BrFS] Number of nodes: {}",
               element.get_search_time_ms().count(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_generated_until_g_value().empty() ? 0 : element.get_num_generated_until_g_value().back(),
               element.get_num_expanded_until_g_value().empty() ? 0 : element.get_num_expanded_until_g_value().back(),
               element.get_num_pruned_until_g_value().empty() ? 0 : element.get_num_pruned_until_g_value().back(),
               element.get_num_reached_fluent_atoms(),
               element.get_num_reached_derived_atoms(),
               element.get_num_states(),
               element.get_num_nodes());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::gbfs_eager::Statistics& element)
{
    fmt::print(out,
               "[GBFS] Search time: {}ms\n"
               "[GBFS] Number of generated states: {}\n"
               "[GBFS] Number of expanded states: {}\n"
               "[GBFS] Number of pruned states: {}\n"
               "[GBFS] Number of reached fluent atoms: {}\n"
               "[GBFS] Number of reached derived atoms: {}\n"
               "[GBFS] Number of states: {}\n"
               "[GBFS] Number of nodes: {}",
               element.get_search_time_ms().count(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_reached_fluent_atoms(),
               element.get_num_reached_derived_atoms(),
               element.get_num_states(),
               element.get_num_nodes());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::gbfs_lazy::Statistics& element)
{
    fmt::print(out,
               "[GBFS] Search time: {}ms\n"
               "[GBFS] Number of generated states: {}\n"
               "[GBFS] Number of expanded states: {}\n"
               "[GBFS] Number of pruned states: {}\n"
               "[GBFS] Number of reached fluent atoms: {}\n"
               "[GBFS] Number of reached derived atoms: {}\n"
               "[GBFS] Number of states: {}\n"
               "[GBFS] Number of nodes: {}",
               element.get_search_time_ms().count(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_reached_fluent_atoms(),
               element.get_num_reached_derived_atoms(),
               element.get_num_states(),
               element.get_num_nodes());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::iw::Statistics& element)
{
    fmt::print(out,
               "[IW] Search time: {}ms\n"
               "[IW] Effective width: {}\n"
               "[IW] Number of generated states: {}\n"
               "[IW] Number of expanded states: {}\n"
               "[IW] Number of pruned states: {}\n"
               "[IW] Number of generated states until last g-layer: {}\n"
               "[IW] Number of expanded states until last g-layer: {}\n"
               "[IW] Number of pruned states until last g-layer: {}",
               element.get_search_time_ms().count(),
               element.get_effective_width(),
               element.get_brfs_statistics_by_arity().back().get_num_generated(),
               element.get_brfs_statistics_by_arity().back().get_num_expanded(),
               element.get_brfs_statistics_by_arity().back().get_num_pruned(),
               element.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().empty() ?
                   0 :
                   element.get_brfs_statistics_by_arity().back().get_num_generated_until_g_value().back(),
               element.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().empty() ?
                   0 :
                   element.get_brfs_statistics_by_arity().back().get_num_expanded_until_g_value().back(),
               element.get_brfs_statistics_by_arity().back().get_num_pruned_until_g_value().empty() ?
                   0 :
                   element.get_brfs_statistics_by_arity().back().get_num_pruned_until_g_value().back());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::siw::Statistics& element)
{
    fmt::print(out,
               "[SIW] Search time: {}ms\n"
               "[SIW] Maximum effective width: {}\n"
               "[SIW] Average effective width: {}\n"
               "[SIW] Number of generated states: {}\n"
               "[SIW] Number of expanded states: {}\n"
               "[SIW] Number of pruned states: {}\n"
               "[SIW] Number of generated states until last g-layer: {}\n"
               "[SIW] Number of expanded states until last g-layer: {}\n"
               "[SIW] Number of pruned states until last g-layer: {}",
               element.get_search_time_ms().count(),
               element.get_maximum_effective_width(),
               element.get_average_effective_width(),
               element.get_num_generated(),
               element.get_num_expanded(),
               element.get_num_pruned(),
               element.get_num_generated_until_last_g_layer(),
               element.get_num_expanded_until_last_g_layer(),
               element.get_num_pruned_until_last_g_layer());

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::State& element)
{
    auto fluent_ground_atoms = mimir::formalism::GroundAtomList<mimir::formalism::FluentTag> {};
    auto static_ground_atoms = mimir::formalism::GroundAtomList<mimir::formalism::StaticTag> {};
    auto derived_ground_atoms = mimir::formalism::GroundAtomList<mimir::formalism::DerivedTag> {};
    auto fluent_function_values = std::vector<std::pair<mimir::formalism::GroundFunction<mimir::formalism::FluentTag>, ContinuousCost>> {};

    element.get_problem().get_repositories().get_ground_atoms_from_indices(element.get_atoms<mimir::formalism::FluentTag>(), fluent_ground_atoms);
    element.get_problem().get_repositories().get_ground_atoms_from_indices(element.get_problem().get_positive_static_initial_atoms_bitset(),
                                                                           static_ground_atoms);
    element.get_problem().get_repositories().get_ground_atoms_from_indices(element.get_atoms<mimir::formalism::DerivedTag>(), derived_ground_atoms);
    element.get_problem().get_repositories().get_ground_function_values(element.get_numeric_variables(), fluent_function_values);

    // Sort by name for easier comparison
    std::sort(fluent_ground_atoms.begin(), fluent_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(static_ground_atoms.begin(), static_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(derived_ground_atoms.begin(), derived_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });

    fmt::print(out,
               "State(index={}, fluent_atoms={}, static_atoms={}, derived_atoms={}, fluent_numerics={})",
               element.get_index(),
               mimir::to_string(fluent_ground_atoms),
               mimir::to_string(static_ground_atoms),
               mimir::to_string(derived_ground_atoms),
               mimir::to_string(fluent_function_values));

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::PackedStateImpl& element)
{
    fmt::print(out,
               "PackedState(fluent_atoms={}, derived_atoms={}, numeric_variables={})",
               mimir::to_string(element.get_atoms<mimir::formalism::FluentTag>()),
               mimir::to_string(element.get_atoms<mimir::formalism::DerivedTag>()),
               mimir::to_string(element.get_numeric_variables()));

    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::Plan& element)
{
    const auto& problem = *element.get_search_context()->get_problem();
    const auto formatter = mimir::formalism::PlanFormatterTag {};

    fmt::print(
        out,
        "{}\n; cost = {}\n",
        fmt::join(element.get_actions()
                      | std::views::transform([&](auto&& action) { return to_string(std::make_tuple(std::cref(*action), std::cref(problem), formatter)); }),
                  "\n"),
        element.get_cost());
    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::PartiallyOrderedPlan& element)
{
    const auto& problem = *element.get_t_o_plan().get_search_context()->get_problem();
    const auto formatter = mimir::formalism::PlanFormatterTag {};

    fmt::print(
        out,
        "digraph Tree {{\nrankdir=TB;\n\n{}\n\n{}}}",
        fmt::join(element.get_graph().get_vertices()
                      | std::views::transform(
                          [&](auto&& vertex)
                          {
                              return fmt::format(
                                  "n{} [label=\"index={}, action={}\"];",
                                  vertex.first,
                                  vertex.first,
                                  to_string(std::make_tuple(std::cref(*element.get_t_o_plan().get_actions().at(vertex.first)), std::cref(problem), formatter)));
                          }),
                  "\n"),
        fmt::join(element.get_graph().get_edges()
                      | std::views::transform(
                          [&](auto&& edge)
                          { return fmt::format("n{} -> n{} [label=\"index={}\"];", edge.second.get_source(), edge.second.get_target(), edge.first); }),
                  "\n"));
    return out;
}

template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& print(std::ostream& out, const mimir::search::match_tree::IInverseNode<E>& element)
{
    auto nodes = mimir::search::match_tree::InverseNodes<E> {};
    auto edges = mimir::search::match_tree::InverseEdges {};

    auto visitor1 = mimir::search::match_tree::InitializeInverseNodesVisitor(nodes);
    element.visit(visitor1);

    auto visitor2 = mimir::search::match_tree::InitializeInverseEdgesVisitor(nodes, edges);
    element.visit(visitor2);

    fmt::print(
        out,
        "digraph Tree {{\nrankdir=TB;\n\n{}\n\n{}}}",
        fmt::join(nodes | std::views::transform([&](auto&& arg) { return fmt::format("n{} [label=\"{}\"];", arg.second.first, arg.second.second); }), "\n"),
        fmt::join(
            edges | std::views::transform([&](auto&& arg) { return fmt::format("n{} -> n{} [label=\"{}\"];", arg.first.first, arg.first.second, arg.second); }),
            "\n"));

    return out;
}

template std::ostream& print(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundActionImpl>& element);
template std::ostream& print(std::ostream& out, const mimir::search::match_tree::IInverseNode<mimir::formalism::GroundAxiomImpl>& element);

template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& print(std::ostream& out, const mimir::search::match_tree::INode<E>& element)
{
    auto nodes = mimir::search::match_tree::Nodes<E> {};
    auto edges = mimir::search::match_tree::Edges {};

    auto visitor1 = mimir::search::match_tree::InitializeNodesVisitor(nodes);
    element.visit(visitor1);

    auto visitor2 = mimir::search::match_tree::InitializeEdgesVisitor(nodes, edges);
    element.visit(visitor2);

    fmt::print(
        out,
        "digraph Tree {{\nrankdir=TB;\n\n{}\n\n{}}}",
        fmt::join(nodes | std::views::transform([&](auto&& arg) { return fmt::format("n{} [label=\"{}\"];", arg.second.first, arg.second.second); }), "\n"),
        fmt::join(
            edges | std::views::transform([&](auto&& arg) { return fmt::format("n{} -> n{} [label=\"{}\"];", arg.first.first, arg.first.second, arg.second); }),
            "\n"));

    return out;
}

template std::ostream& print(std::ostream& out, const mimir::search::match_tree::INode<mimir::formalism::GroundActionImpl>& element);
template std::ostream& print(std::ostream& out, const mimir::search::match_tree::INode<mimir::formalism::GroundAxiomImpl>& element);
}
