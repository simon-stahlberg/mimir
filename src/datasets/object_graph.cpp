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

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/state.hpp"

#include <random>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{

static ObjectMap<graphs::VertexIndex> add_objects_graph_structures(State state, const ProblemImpl& problem, graphs::StaticVertexColoredGraph& out_digraph)
{
    auto object_to_atom_color = ObjectMap<PredicateVariantList> {};
    auto object_to_literal_color = ObjectMap<std::vector<std::pair<PredicateVariant, bool>>> {};
    auto initialize_object_colors_func = [&](auto&& atom)
    {
        for (const auto& object : atom->get_objects())
        {
            object_to_atom_color[object] = PredicateVariantList();
            object_to_literal_color[object] = std::vector<std::pair<PredicateVariant, bool>>();
        }
    };
    for (const auto& atom : problem.get_static_initial_atoms())
    {
        initialize_object_colors_func(atom);
    }
    for (const auto& atom_index : state->get_atoms<FluentTag>())
    {
        initialize_object_colors_func(problem.get_repositories().get_ground_atom<FluentTag>(atom_index));
    }
    for (const auto& atom_index : state->get_atoms<DerivedTag>())
    {
        initialize_object_colors_func(problem.get_repositories().get_ground_atom<DerivedTag>(atom_index));
    }
    boost::hana::for_each(problem.get_hana_goal_condition(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              for (const auto& literal : second)
                              {
                                  initialize_object_colors_func(literal->get_atom());
                              }
                          });

    auto add_unary_atom_graph_structures_func = [&](auto&& atom)
    {
        if (atom->get_arity() == 1)
        {
            object_to_atom_color[atom->get_objects().front()].push_back(atom->get_predicate());
        }
    };
    for (const auto& atom : problem.get_static_initial_atoms())
    {
        add_unary_atom_graph_structures_func(atom);
    }
    for (const auto& atom_index : state->get_atoms<FluentTag>())
    {
        add_unary_atom_graph_structures_func(problem.get_repositories().get_ground_atom<FluentTag>(atom_index));
    }
    for (const auto& atom_index : state->get_atoms<DerivedTag>())
    {
        add_unary_atom_graph_structures_func(problem.get_repositories().get_ground_atom<DerivedTag>(atom_index));
    }
    auto add_unary_literal_graph_structures_func = [&](auto&& literal)
    {
        if (literal->get_atom()->get_arity() == 1)
        {
            object_to_literal_color[literal->get_atom()->get_objects().front()].emplace_back(literal->get_atom()->get_predicate(), literal->get_polarity());
        }
    };
    boost::hana::for_each(problem.get_hana_goal_condition(),
                          [&](auto&& pair)
                          {
                              const auto& second = boost::hana::second(pair);
                              for (const auto& literal : second)
                              {
                                  add_unary_literal_graph_structures_func(literal);
                              }
                          });

    auto index_to_object = std::vector<std::pair<Index, Object>> {};
    for (const auto& [object, color] : object_to_atom_color)
    {
        index_to_object.emplace_back(object->get_index(), object);
    }
    for (const auto& [object, color] : object_to_literal_color)
    {
        index_to_object.emplace_back(object->get_index(), object);
    }
    std::sort(index_to_object.begin(), index_to_object.end());

    auto object_to_vertex_index = ObjectMap<graphs::VertexIndex> {};
    for (const auto& [index, object] : index_to_object)
    {
        std::sort(object_to_atom_color[object].begin(), object_to_atom_color[object].end());
        std::sort(object_to_literal_color[object].begin(), object_to_literal_color[object].end());

        object_to_vertex_index.emplace(
            object,
            out_digraph.add_vertex(graphs::Color(graphs::VariadicColor(object_to_atom_color[object], object_to_literal_color[object]))));
    }

    return object_to_vertex_index;
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atom_graph_structures(const ProblemImpl& problem,
                                             const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                             GroundAtom<P> atom,
                                             graphs::StaticVertexColoredGraph& out_digraph)
{
    if (atom->get_arity() == 0)
    {
        out_digraph.add_vertex(graphs::Color(graphs::VariadicColor(atom->get_predicate())));
    }
    else if (atom->get_arity() > 1)
    {
        for (size_t pos = 0; pos < atom->get_arity(); ++pos)
        {
            const auto vertex_index = out_digraph.add_vertex(graphs::Color(graphs::VariadicColor(atom->get_predicate(), pos)));

            out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(atom->get_objects().at(pos)));

            if (pos > 0)
            {
                out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
            }
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_atoms_graph_structures(const ProblemImpl& problem,
                                              const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                              const GroundAtomList<P>& atoms,
                                              graphs::StaticVertexColoredGraph& out_digraph)
{
    auto canonical_structure_to_atom = std::vector<std::tuple<Index, IndexList, GroundAtom<P>>> {};

    for (const auto& atom : atoms)
    {
        auto object_indices = IndexList {};
        for (const auto& object : atom->get_objects())
        {
            object_indices.push_back(object->get_index());
        }
        canonical_structure_to_atom.emplace_back(atom->get_predicate()->get_index(), object_indices, atom);
    }

    std::sort(canonical_structure_to_atom.begin(), canonical_structure_to_atom.end());

    for (const auto& [predicate_index, object_indices, atom] : canonical_structure_to_atom)
    {
        add_ground_atom_graph_structures(problem, object_to_vertex_index, atom, out_digraph);
    }
}

static void add_ground_atoms_graph_structures(State state,
                                              const ProblemImpl& problem,
                                              const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                              graphs::StaticVertexColoredGraph& out_digraph)
{
    add_ground_atoms_graph_structures(problem, object_to_vertex_index, problem.get_static_initial_atoms(), out_digraph);

    add_ground_atoms_graph_structures(problem,
                                      object_to_vertex_index,
                                      problem.get_repositories().get_ground_atoms_from_indices<FluentTag>(state->get_atoms<FluentTag>()),
                                      out_digraph);

    add_ground_atoms_graph_structures(problem,
                                      object_to_vertex_index,
                                      problem.get_repositories().get_ground_atoms_from_indices<DerivedTag>(state->get_atoms<DerivedTag>()),
                                      out_digraph);
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literal_graph_structures(const ProblemImpl& problem,
                                                const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                                GroundLiteral<P> literal,
                                                graphs::StaticVertexColoredGraph& out_digraph)
{
    if (literal->get_atom()->get_arity() == 0)
    {
        out_digraph.add_vertex(graphs::Color(graphs::VariadicColor(literal->get_atom()->get_predicate(), literal->get_polarity())));
    }
    else if (literal->get_atom()->get_arity() > 1)
    {
        for (size_t pos = 0; pos < literal->get_atom()->get_arity(); ++pos)
        {
            const auto vertex_index =
                out_digraph.add_vertex(graphs::Color(graphs::VariadicColor(literal->get_atom()->get_predicate(), pos, literal->get_polarity())));

            out_digraph.add_undirected_edge(vertex_index, object_to_vertex_index.at(literal->get_atom()->get_objects().at(pos)));

            if (pos > 0)
            {
                out_digraph.add_undirected_edge(vertex_index - 1, vertex_index);
            }
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
static void add_ground_literals_graph_structures(const ProblemImpl& problem,
                                                 const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                                 GroundLiteralList<P> literals,
                                                 graphs::StaticVertexColoredGraph& out_digraph)
{
    auto canonical_structure_to_literal = std::vector<std::tuple<Index, IndexList, bool, GroundLiteral<P>>> {};

    for (const auto& literal : literals)
    {
        auto object_indices = IndexList {};
        for (const auto& object : literal->get_atom()->get_objects())
        {
            object_indices.push_back(object->get_index());
        }
        canonical_structure_to_literal.emplace_back(literal->get_atom()->get_predicate()->get_index(), object_indices, literal->get_polarity(), literal);
    }

    std::sort(canonical_structure_to_literal.begin(), canonical_structure_to_literal.end());

    for (const auto& [predicate_index, object_indices, polarity, literal] : canonical_structure_to_literal)
    {
        add_ground_literal_graph_structures(problem, object_to_vertex_index, literal, out_digraph);
    }
}

static void add_ground_goal_literals_graph_structures(const ProblemImpl& problem,
                                                      const ObjectMap<graphs::VertexIndex>& object_to_vertex_index,
                                                      graphs::StaticVertexColoredGraph& out_digraph)
{
    add_ground_literals_graph_structures(problem, object_to_vertex_index, problem.get_goal_condition<StaticTag>(), out_digraph);

    add_ground_literals_graph_structures(problem, object_to_vertex_index, problem.get_goal_condition<FluentTag>(), out_digraph);

    add_ground_literals_graph_structures(problem, object_to_vertex_index, problem.get_goal_condition<DerivedTag>(), out_digraph);
}

graphs::StaticVertexColoredGraph create_object_graph(State state, const ProblemImpl& problem)
{
    if (!problem.get_derived_predicates().empty())
    {
        throw std::runtime_error("create_object_graph(state, problem): Cannot create object graphs for problems with additional derived predicates. If you "
                                 "have quantified goals then you should define them as derived predicates in the domain file.");
    }

    auto vertex_colored_digraph = graphs::StaticVertexColoredGraph();

    const auto object_to_vertex_index = add_objects_graph_structures(state, problem, vertex_colored_digraph);

    add_ground_atoms_graph_structures(state, problem, object_to_vertex_index, vertex_colored_digraph);

    add_ground_goal_literals_graph_structures(problem, object_to_vertex_index, vertex_colored_digraph);

    return vertex_colored_digraph;
}

}
