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

#ifndef MIMIR_SEARCH_HEURISTICS_RPG_BASE_HPP_
#define MIMIR_SEARCH_HEURISTICS_RPG_BASE_HPP_

#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics/interface.hpp"
#include "mimir/search/openlists/priority_queue.hpp"

namespace mimir::search::rpg
{

/**
 * UnaryGroundAction
 */

class UnaryGroundAction
{
public:
    UnaryGroundAction(Index index, size_t num_preconditions, Index fluent_effect, bool polarity) :
        m_index(index),
        m_num_preconditions(num_preconditions),
        m_fluent_effect(fluent_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_fluent_effect() const { return m_fluent_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    size_t m_num_preconditions;
    Index m_fluent_effect;
    bool m_polarity;
};
using UnaryGroundActionList = std::vector<UnaryGroundAction>;

/**
 * UnaryGroundAxiom
 */

class UnaryGroundAxiom
{
public:
    UnaryGroundAxiom(Index index, size_t num_preconditions, Index derived_effect, bool polarity) :
        m_index(index),
        m_num_preconditions(num_preconditions),
        m_derived_effect(derived_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_derived_effect() const { return m_derived_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    size_t m_num_preconditions;
    Index m_derived_effect;
    bool m_polarity;
};
using UnaryGroundAxiomList = std::vector<UnaryGroundAxiom>;

/**
 * Proposition
 */

class Proposition
{
public:
    Proposition(Index index, IndexList is_precondition_of_actions, IndexList is_precondition_of_axioms, bool is_goal) :
        m_index(index),
        m_is_precondition_of_actions(std::move(is_precondition_of_actions)),
        m_is_precondition_of_axioms(std::move(is_precondition_of_axioms)),
        m_is_goal(is_goal)
    {
    }

    Index get_index() const { return m_index; }
    const IndexList& is_precondition_of_actions() const { return m_is_precondition_of_actions; }
    const IndexList& is_precondition_of_axioms() const { return m_is_precondition_of_axioms; }
    bool is_goal() const { return m_is_goal; }

private:
    Index m_index;
    IndexList m_is_precondition_of_actions;
    IndexList m_is_precondition_of_axioms;
    bool m_is_goal;
};

using PropositionList = std::vector<Proposition>;

/**
 * Annotations
 */

template<typename... T>
using Annotations = std::tuple<T...>;
template<typename... T>
using AnnotationsList = std::vector<Annotations<T...>>;

/**
 * RelaxedPlanningGraph
 */

/// @brief `RelaxedPlanningGraph` implements a common base class for heuristics based on the relaxed planning graph.
///
/// Notes: Deriving not x is not trivial, see footnote 1: https://www.ijcai.org/Proceedings/15/Papers/226.pdf
/// "[...] if A_y = { y <- phi_i | i=1,...,k } is the set of axioms that define y,
/// A_{not y} = { not y <- And_{i=1,...,k} not phi_i }, which is admissible
/// because the derived value default value is taken to be neither true nor false."
/// This translation is worst-case exponential due to CNF to DNF translation.
/// The above is the way how fast downward does it: https://github.com/aibasel/downward/blob/main/src/search/tasks/default_value_axioms_task.h
/// Fast downward also provides an approximation where not y <- T, i.e., not y is set to true immediately.
/// We provide a stronger but cheaper approximation which is not y <- not phi_i for each i=1,...,k.
/// In other words, we derive not y if at least one y <- phi_i fails to derive y.
/// This is very cheap to represent and compute.
/// @tparam Derived is the derived class.
template<typename Derived>
class RelaxedPlanningGraph : public IHeuristic
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    friend Derived;

    static constexpr Index DUMMY_PROPOSITION_INDEX = 0;

public:
    ContinuousCost compute_heuristic(State state, bool is_goal_state) override
    {
        self().initialize_and_annotations();
        self().initialize_or_annotations();
        self().initialize_or_annotations_and_queue(state);
        dijksta();
        return (m_num_unsat_goals > 0) ? INFINITY_CONTINUOUS_COST : self().extract_impl();
    }

private:
    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) :
        m_positive_offsets(),
        m_negative_offsets(),
        m_atom_indices(),
        m_unary_actions(),
        m_unary_axioms(),
        m_num_unsat_goals(0)
    {
        auto is_positive_fluent_precondition_of_action = IndexMap<IndexList> {};
        auto is_positive_derived_precondition_of_action = IndexMap<IndexList> {};
        auto is_negative_fluent_precondition_of_action = IndexMap<IndexList> {};
        auto is_negative_derived_precondition_of_action = IndexMap<IndexList> {};
        auto trivial_unary_actions = IndexList {};

        for (const auto& action : delete_relaxation.create_ground_actions())
        {
            for (const auto& cond_effect : action->get_conditional_effects())
            {
                for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_propositional_effects<formalism::PositiveTag>())
                {
                    const auto unary_action_index = m_unary_actions.size();

                    /* Positive */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        is_positive_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        is_positive_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        is_positive_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        is_positive_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    /* Negative */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        is_negative_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        is_negative_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        is_negative_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        is_negative_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }

                    m_unary_actions.push_back(UnaryGroundAction(
                        unary_action_index,
                        action->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::FluentTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::DerivedTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::FluentTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::DerivedTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::FluentTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::DerivedTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::FluentTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::DerivedTag>().size(),
                        eff_atom_index,
                        true));

                    if (m_unary_actions.back().get_num_preconditions() == 0)
                    {
                        trivial_unary_actions.push_back(unary_action_index);
                    }
                }
                for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_propositional_effects<formalism::NegativeTag>())
                {
                    const auto unary_action_index = m_unary_actions.size();

                    /* Positive */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        is_positive_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        is_positive_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        is_positive_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        is_positive_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    /* Negative */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        is_negative_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        is_negative_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        is_negative_fluent_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        is_negative_derived_precondition_of_action[pre_atom_index].push_back(unary_action_index);
                    }

                    m_unary_actions.push_back(UnaryGroundAction(
                        unary_action_index,
                        action->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::FluentTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::DerivedTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::FluentTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::DerivedTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::FluentTag>().size()
                            + action->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::DerivedTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::FluentTag>().size()
                            + cond_effect->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::DerivedTag>().size(),
                        eff_atom_index,
                        false));

                    if (m_unary_actions.back().get_num_preconditions() == 0)
                    {
                        trivial_unary_actions.push_back(unary_action_index);
                    }
                }
            }
        }

        auto is_positive_fluent_precondition_of_axiom = IndexMap<IndexList> {};
        auto is_positive_derived_precondition_of_axiom = IndexMap<IndexList> {};
        auto is_negative_fluent_precondition_of_axiom = IndexMap<IndexList> {};
        auto is_negative_derived_precondition_of_axiom = IndexMap<IndexList> {};
        auto trivial_unary_axioms = IndexList {};

        auto positive_head_to_unary_axiom = std::unordered_map<Index, IndexList> {};

        for (const auto& axiom : delete_relaxation.create_ground_axioms())
        {
            const auto unary_axiom_index = m_unary_axioms.size();

            /* Positive */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
            {
                is_positive_fluent_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
            {
                is_positive_derived_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }
            /* Negative */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
            {
                is_negative_fluent_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
            {
                is_negative_derived_precondition_of_axiom[pre_atom_index].push_back(unary_axiom_index);
            }

            m_unary_axioms.push_back(
                UnaryGroundAxiom(unary_axiom_index,
                                 axiom->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::FluentTag>().size()
                                     + axiom->get_conjunctive_condition()->get_compressed_precondition<formalism::PositiveTag, formalism::DerivedTag>().size()
                                     + axiom->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::FluentTag>().size()
                                     + axiom->get_conjunctive_condition()->get_compressed_precondition<formalism::NegativeTag, formalism::DerivedTag>().size(),
                                 axiom->get_literal()->get_atom()->get_index(),
                                 true));

            positive_head_to_unary_axiom[axiom->get_literal()->get_atom()->get_index()].push_back(unary_axiom_index);

            if (m_unary_axioms.back().get_num_preconditions() == 0)
            {
                trivial_unary_axioms.push_back(unary_axiom_index);
            }

            /* Positive */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
            {
                // Instantiate an additional unary axiom that derives not-head if positive pre_atom_index is false
                const auto negated_unary_axiom_index = m_unary_axioms.size();

                is_negative_fluent_precondition_of_axiom[pre_atom_index].push_back(negated_unary_axiom_index);

                m_unary_axioms.push_back(UnaryGroundAxiom(negated_unary_axiom_index, 1, axiom->get_literal()->get_atom()->get_index(), false));
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
            {
                // Instantiate an additional unary axiom that derives not-head if positive pre_atom_index is false

                const auto negated_unary_axiom_index = m_unary_axioms.size();

                is_negative_derived_precondition_of_axiom[pre_atom_index].push_back(negated_unary_axiom_index);

                m_unary_axioms.push_back(UnaryGroundAxiom(negated_unary_axiom_index, 1, axiom->get_literal()->get_atom()->get_index(), false));
            }
            /* Negative */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
            {
                // Instantiate an additional unary axiom that derives not-head if positive pre_atom_index is true

                const auto negated_unary_axiom_index = m_unary_axioms.size();

                is_positive_fluent_precondition_of_axiom[pre_atom_index].push_back(negated_unary_axiom_index);

                m_unary_axioms.push_back(UnaryGroundAxiom(negated_unary_axiom_index, 1, axiom->get_literal()->get_atom()->get_index(), false));
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
            {
                // Instantiate an additional unary axiom that derives not-head if positive pre_atom_index is true

                const auto negated_unary_axiom_index = m_unary_axioms.size();

                is_positive_derived_precondition_of_axiom[pre_atom_index].push_back(negated_unary_axiom_index);

                m_unary_axioms.push_back(UnaryGroundAxiom(negated_unary_axiom_index, 1, axiom->get_literal()->get_atom()->get_index(), false));
            }
        }

        // std::cout << "Num actions: " << m_unary_actions.size() << std::endl;
        // std::cout << "Num axioms: " << m_unary_axioms.size() << std::endl;
        // std::cout << "Num trivial actions: " << trivial_unary_actions.size() << std::endl;
        // std::cout << "Num trivial axioms: " << trivial_unary_axioms.size() << std::endl;

        // We create a dummy proposition with index 0 that contains all trivial unary actions and axioms without preconditions.
        m_propositions.emplace_back(m_propositions.size(), std::move(trivial_unary_actions), std::move(trivial_unary_axioms), false);

        const auto& fluent_ground_atom_repository = boost::hana::at_key(delete_relaxation.get_problem()->get_repositories().get_hana_repositories(),
                                                                        boost::hana::type<formalism::GroundAtomImpl<formalism::FluentTag>> {});
        auto& fluent_atoms = get_atom_indices<formalism::FluentTag>();
        auto& positive_fluent_offsets = get_positive_offsets<formalism::FluentTag>();
        auto& negative_fluent_offsets = get_negative_offsets<formalism::FluentTag>();

        for (Index atom_index = 0; atom_index < fluent_ground_atom_repository.size(); ++atom_index)
        {
            // const auto& atom = fluent_ground_atom_repository.at(atom_index);

            fluent_atoms.push_back(atom_index);

            /* Positive */
            {
                const auto proposition_index = m_propositions.size();

                // std::cout << proposition_index << " " << *atom << std::endl;

                if (atom_index >= positive_fluent_offsets.size())
                {
                    positive_fluent_offsets.resize(atom_index + 1, MAX_INDEX);
                }
                positive_fluent_offsets[atom_index] = proposition_index;

                const auto is_goal = delete_relaxation.get_problem()->get_goal_atoms_bitset<formalism::PositiveTag, formalism::FluentTag>().get(atom_index);

                m_propositions.emplace_back(proposition_index,
                                            std::move(is_positive_fluent_precondition_of_action[atom_index]),
                                            std::move(is_positive_fluent_precondition_of_axiom[atom_index]),
                                            is_goal);

                if (is_goal)
                    m_goal_propositions.push_back(proposition_index);
            }
            /* Negative */
            {
                const auto proposition_index = m_propositions.size();

                // std::cout << proposition_index << " not " << *atom << std::endl;

                if (atom_index >= negative_fluent_offsets.size())
                {
                    negative_fluent_offsets.resize(atom_index + 1, MAX_INDEX);
                }
                negative_fluent_offsets[atom_index] = proposition_index;

                const auto is_goal = delete_relaxation.get_problem()->get_goal_atoms_bitset<formalism::NegativeTag, formalism::FluentTag>().get(atom_index);

                m_propositions.emplace_back(proposition_index,
                                            std::move(is_negative_fluent_precondition_of_action[atom_index]),
                                            std::move(is_negative_fluent_precondition_of_axiom[atom_index]),
                                            is_goal);

                if (is_goal)
                    m_goal_propositions.push_back(proposition_index);
            }
        }
        std::sort(fluent_atoms.begin(), fluent_atoms.end());

        const auto& derived_ground_atom_repository = boost::hana::at_key(delete_relaxation.get_problem()->get_repositories().get_hana_repositories(),
                                                                         boost::hana::type<formalism::GroundAtomImpl<formalism::DerivedTag>> {});
        auto& derived_atoms = get_atom_indices<formalism::DerivedTag>();
        auto& positive_derived_offsets = get_positive_offsets<formalism::DerivedTag>();
        auto& negative_derived_offsets = get_negative_offsets<formalism::DerivedTag>();

        for (Index atom_index = 0; atom_index < derived_ground_atom_repository.size(); ++atom_index)
        {
            // const auto& atom = derived_ground_atom_repository.at(atom_index);

            derived_atoms.push_back(atom_index);

            /* Positive */
            {
                const auto proposition_index = m_propositions.size();

                // std::cout << proposition_index << " " << *atom << " " << atom_index << std::endl;

                if (atom_index >= positive_derived_offsets.size())
                {
                    positive_derived_offsets.resize(atom_index + 1, MAX_INDEX);
                }
                positive_derived_offsets[atom_index] = proposition_index;

                const auto is_goal = delete_relaxation.get_problem()->get_goal_atoms_bitset<formalism::PositiveTag, formalism::DerivedTag>().get(atom_index);

                m_propositions.emplace_back(proposition_index,
                                            std::move(is_positive_derived_precondition_of_action[atom_index]),
                                            std::move(is_positive_derived_precondition_of_axiom[atom_index]),
                                            is_goal);

                if (is_goal)
                    m_goal_propositions.push_back(proposition_index);
            }
            /* Negative */
            {
                const auto proposition_index = m_propositions.size();

                // std::cout << proposition_index << " not " << *atom << " " << atom_index << std::endl;

                if (atom_index >= negative_derived_offsets.size())
                {
                    negative_derived_offsets.resize(atom_index + 1, MAX_INDEX);
                }
                negative_derived_offsets[atom_index] = proposition_index;

                const auto is_goal = delete_relaxation.get_problem()->get_goal_atoms_bitset<formalism::NegativeTag, formalism::DerivedTag>().get(atom_index);

                m_propositions.emplace_back(proposition_index,
                                            std::move(is_negative_derived_precondition_of_action[atom_index]),
                                            std::move(is_negative_derived_precondition_of_axiom[atom_index]),
                                            is_goal);

                if (is_goal)
                    m_goal_propositions.push_back(proposition_index);
            }
        }
        std::sort(derived_atoms.begin(), derived_atoms.end());

        m_action_annotations.resize(this->m_unary_actions.size());
        m_axiom_annotations.resize(this->m_unary_axioms.size());
        m_proposition_annotations.resize(this->m_propositions.size());
    }

    void initialize_and_annotations()
    {
        for (const auto& action : m_unary_actions)
        {
            self().initialize_and_annotations_impl(action);
        }
        for (const auto& axiom : m_unary_axioms)
        {
            self().initialize_and_annotations_impl(axiom);
        }
    }

    void initialize_or_annotations()
    {
        for (const auto& proposition : m_propositions)
        {
            self().initialize_or_annotations_impl(proposition);
        }
    }

    template<formalism::IsFluentOrDerivedTag P>
    void initialize_or_annotations_and_queue(State state)
    {
        auto& positive_offsets = get_positive_offsets<P>();
        auto& negative_offsets = get_negative_offsets<P>();
        const auto& all_atoms = get_atom_indices<P>();
        const auto& state_atoms = state->get_atoms<P>();
        auto it = state_atoms.begin();
        auto it2 = all_atoms.begin();
        const auto end = state_atoms.end();
        const auto end2 = all_atoms.end();

        while (it != end && it2 != end2)
        {
            if (*it == *it2)
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[positive_offsets[*it]]);
                ++it;
                ++it2;
            }
            else if (*it < *it2)
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[positive_offsets[*it]]);
                ++it;
            }
            else
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[negative_offsets[*it2]]);
                ++it2;
            }
        }
        while (it != end)
        {
            self().initialize_or_annotations_and_queue_impl(m_propositions[positive_offsets[*it]]);
            ++it;
        }
        while (it2 != end2)
        {
            self().initialize_or_annotations_and_queue_impl(m_propositions[negative_offsets[*it2]]);
            ++it2;
        }
    }

    void initialize_or_annotations_and_queue(State state)
    {
        this->m_queue.clear();

        initialize_or_annotations_and_queue<formalism::FluentTag>(state);
        initialize_or_annotations_and_queue<formalism::DerivedTag>(state);

        // Trivial dummy proposition to trigger actions and axioms without preconditions
        self().initialize_or_annotations_and_queue_impl(m_propositions[0]);
    }

    void on_process_action(const Proposition& proposition, const UnaryGroundAction& action)
    {
        auto& action_annotation = m_action_annotations[action.get_index()];

        self().update_and_annotation_impl(proposition, action);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(action_annotation);
        }

        if (get_num_unsatisfied_preconditions(action_annotation) == 0)
        {
            // std::cout << "Process satisfied action: " << action.get_index() << std::endl;

            const auto effect_proposition_index = action.get_polarity() ? get_positive_offsets<formalism::FluentTag>()[action.get_fluent_effect()] :
                                                                          get_negative_offsets<formalism::FluentTag>()[action.get_fluent_effect()];
            const auto& effect_proposition = this->m_propositions[effect_proposition_index];

            self().update_or_annotation_impl(action, effect_proposition);
        }
    }

    void on_process_axiom(const Proposition& proposition, const UnaryGroundAxiom& axiom)
    {
        auto& axiom_annotation = m_axiom_annotations[axiom.get_index()];

        self().update_and_annotation_impl(proposition, axiom);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(axiom_annotation);
        }

        if (get_num_unsatisfied_preconditions(axiom_annotation) == 0)
        {
            // std::cout << "Process satisfied axiom: " << axiom.get_index() << std::endl;

            const auto effect_proposition_index = axiom.get_polarity() ? get_positive_offsets<formalism::DerivedTag>()[axiom.get_derived_effect()] :
                                                                         get_negative_offsets<formalism::DerivedTag>()[axiom.get_derived_effect()];
            const auto& effect_proposition = this->m_propositions[effect_proposition_index];

            self().update_or_annotation_impl(axiom, effect_proposition);
        }
    }

    void dijksta()
    {
        m_num_unsat_goals = m_goal_propositions.size();

        while (!m_queue.empty())
        {
            const auto entry = m_queue.top();
            m_queue.pop();

            const auto& proposition = m_propositions[entry.proposition_index];
            const auto& annotation = m_proposition_annotations[entry.proposition_index];

            // std::cout << "Queue pop: " << entry.proposition_index << " " << entry.cost << std::endl;

            if (get_cost(annotation) < entry.cost)
            {
                continue;
            }

            if (proposition.is_goal() && --m_num_unsat_goals == 0)
            {
                return;
            }

            for (const auto action_index : proposition.is_precondition_of_actions())
            {
                const auto& action = m_unary_actions[action_index];

                self().on_process_action(proposition, action);
            }
            for (const auto axiom_index : proposition.is_precondition_of_axioms())
            {
                const auto& axiom = m_unary_axioms[axiom_index];

                self().on_process_axiom(proposition, axiom);
            }
        }

        // std::cout << "Num unsat goals: " << num_unsat_goals << std::endl;
    }

    HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag> m_positive_offsets;

    template<formalism::IsFluentOrDerivedTag P>
    IndexList& get_positive_offsets()
    {
        return boost::hana::at_key(m_positive_offsets, boost::hana::type<P> {});
    }

    HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag> m_negative_offsets;

    template<formalism::IsFluentOrDerivedTag P>
    IndexList& get_negative_offsets()
    {
        return boost::hana::at_key(m_negative_offsets, boost::hana::type<P> {});
    }

    HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag> m_atom_indices;

    template<formalism::IsFluentOrDerivedTag P>
    IndexList& get_atom_indices()
    {
        return boost::hana::at_key(m_atom_indices, boost::hana::type<P> {});
    }

    UnaryGroundActionList m_unary_actions;
    UnaryGroundAxiomList m_unary_axioms;

    AnnotationsList<DiscreteCost, size_t> m_action_annotations;
    AnnotationsList<DiscreteCost, size_t> m_axiom_annotations;

    static DiscreteCost& get_cost(Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static size_t& get_num_unsatisfied_preconditions(Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }
    static size_t get_num_unsatisfied_preconditions(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }

    PropositionList m_propositions;

    AnnotationsList<DiscreteCost> m_proposition_annotations;

    static DiscreteCost& get_cost(Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }

    IndexList m_goal_propositions;
    size_t m_num_unsat_goals;

    struct QueueEntry
    {
        Index proposition_index;
        DiscreteCost cost;
    };

    PriorityQueue<DiscreteCost, QueueEntry> m_queue;
};
}

#endif
