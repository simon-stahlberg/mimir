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

#include "mimir/datasets/object_graph.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/graphs/property.hpp"
#include "mimir/search/state.hpp"

#include <random>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{

static void add_objects_graph_structures(const State& state,
                                         const ProblemImpl& problem,
                                         graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    struct ObjectColorData
    {
        PredicateVariantList atoms;
        std::vector<std::pair<PredicateVariant, bool>> literals;
    };

    auto object_colors = std::vector<ObjectColorData>(problem.get_problem_and_domain_objects().size());

    // --- Step 1: fill unary colors ---

    auto add_unary_atom_graph_structures_func = [&](auto&& atom)
    {
        if (atom->get_arity() == 1)
        {
            object_colors[atom->get_objects().front()->get_index()].atoms.push_back(atom->get_predicate());
        }
    };

    auto add_unary_literal_graph_structures_func = [&](auto&& literal)
    {
        if (literal->get_atom()->get_arity() == 1)
        {
            object_colors[literal->get_atom()->get_objects().front()->get_index()].literals.emplace_back(literal->get_atom()->get_predicate(),
                                                                                                         literal->get_polarity());
        }
    };

    // state atoms

    for (const auto& atom : problem.get_static_initial_atoms())
    {
        add_unary_atom_graph_structures_func(atom);
    }
    for (const auto& atom_index : state.get_atoms<FluentTag>())
    {
        add_unary_atom_graph_structures_func(problem.get_repositories().get_ground_atom<FluentTag>(atom_index));
    }
    for (const auto& atom_index : state.get_atoms<DerivedTag>())
    {
        add_unary_atom_graph_structures_func(problem.get_repositories().get_ground_atom<DerivedTag>(atom_index));
    }

    // goal literal

    boost::hana::for_each(problem.get_goal_literals(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              for (const auto& literal : second)
                              {
                                  add_unary_literal_graph_structures_func(literal);
                              }
                          });

    // --- Step 2: create object vertices ---

    for (Index object_index = 0; object_index < problem.get_problem_and_domain_objects().size(); ++object_index)
    {
        auto& color_data = object_colors[object_index];

        std::sort(color_data.atoms.begin(), color_data.atoms.end());
        std::sort(color_data.literals.begin(), color_data.literals.end());

        (void) out_digraph.add_vertex(graphs::PropertyValue(color_data.atoms, color_data.literals));
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atom_graph_structures(const ProblemImpl& problem,
                                             GroundAtom<P> atom,
                                             graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    if (atom->get_arity() == 0)
    {
        out_digraph.add_vertex(graphs::PropertyValue(atom->get_predicate()));
    }
    else if (atom->get_arity() >= 2)
    {
        for (size_t pos = 0; pos < atom->get_arity(); ++pos)
        {
            const auto vertex_index = out_digraph.add_vertex(graphs::PropertyValue(atom->get_predicate(), pos));

            out_digraph.add_undirected_edge(vertex_index, atom->get_objects().at(pos)->get_index());

            if (pos > 0)
            {
                out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
            }
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atoms_graph_structures(const ProblemImpl& problem,
                                              const GroundAtomList<P>& atoms,
                                              graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    for (const auto& atom : atoms)
    {
        add_ground_atom_graph_structures(problem, atom, out_digraph);
    }
}

static void add_ground_atoms_graph_structures(const State& state,
                                              const ProblemImpl& problem,
                                              graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    add_ground_atoms_graph_structures(problem, problem.get_static_initial_atoms(), out_digraph);

    add_ground_atoms_graph_structures(problem, problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(state.get_atoms<FluentTag>()), out_digraph);

    add_ground_atoms_graph_structures(problem,
                                      problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(state.get_atoms<DerivedTag>()),
                                      out_digraph);
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literal_graph_structures(const ProblemImpl& problem,
                                                GroundLiteral<P> literal,
                                                graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    if (literal->get_atom()->get_arity() == 0)
    {
        out_digraph.add_vertex(graphs::PropertyValue(literal->get_atom()->get_predicate(), literal->get_polarity()));
    }
    else if (literal->get_atom()->get_arity() >= 2)
    {
        for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
        {
            const auto vertex_index = out_digraph.add_vertex(graphs::PropertyValue(literal->get_atom()->get_predicate(), pos, literal->get_polarity()));

            out_digraph.add_undirected_edge(vertex_index, literal->get_atom()->get_objects().at(pos)->get_index());

            if (pos > 0)
            {
                out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
            }
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literals_graph_structures(const ProblemImpl& problem,
                                                 GroundLiteralList<P> literals,
                                                 graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    for (const auto& literal : literals)
    {
        add_ground_literal_graph_structures(problem, literal, out_digraph);
    }
}

static void add_ground_goal_literals_graph_structures(const ProblemImpl& problem,
                                                      graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>& out_digraph)
{
    add_ground_literals_graph_structures(problem, problem.get_goal_literals<StaticTag>(), out_digraph);

    add_ground_literals_graph_structures(problem, problem.get_goal_literals<FluentTag>(), out_digraph);

    add_ground_literals_graph_structures(problem, problem.get_goal_literals<DerivedTag>(), out_digraph);
}

graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>> create_object_graph(const State& state, const ProblemImpl& problem)
{
    auto vertex_colored_digraph = graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>();

    add_objects_graph_structures(state, problem, vertex_colored_digraph);

    add_ground_atoms_graph_structures(state, problem, vertex_colored_digraph);

    add_ground_goal_literals_graph_structures(problem, vertex_colored_digraph);

    return vertex_colored_digraph;
}

}
