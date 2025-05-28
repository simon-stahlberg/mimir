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

#include <range/v3/view/all.hpp>
#include <range/v3/view/concat.hpp>

namespace mimir::search::rpg
{

/**
 * Action
 */

using ConditionsContainer =
    HanaContainer<HanaContainer<const FlatIndexList*, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag>;

class Action
{
public:
    Action(Index index,
           ConditionsContainer preconditions,
           ConditionsContainer conditional_preconditions,
           size_t num_preconditions,
           Index fluent_effect,
           bool polarity) :
        m_index(index),
        m_preconditions(preconditions),
        m_conditional_preconditions(conditional_preconditions),
        m_num_preconditions(num_preconditions),
        m_fluent_effect(fluent_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_conditional_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_conditional_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_all_preconditions_range() const
    {
        return ::ranges::views::concat(
            ::ranges::subrange(get_preconditions<R, P>()->compressed_begin(), get_preconditions<R, P>()->compressed_end()),
            ::ranges::subrange(get_conditional_preconditions<R, P>()->compressed_begin(), get_conditional_preconditions<R, P>()->compressed_end()));
    }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_effect() const { return m_fluent_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    ConditionsContainer m_preconditions;
    ConditionsContainer m_conditional_preconditions;
    size_t m_num_preconditions;
    Index m_fluent_effect;
    bool m_polarity;
};
using ActionList = std::vector<Action>;

/**
 * Axiom
 */

class Axiom
{
public:
    Axiom(Index index, ConditionsContainer preconditions, size_t num_preconditions, Index derived_effect, bool polarity) :
        m_index(index),
        m_num_preconditions(num_preconditions),
        m_derived_effect(derived_effect),
        m_polarity(polarity)
    {
    }

    Index get_index() const { return m_index; }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_preconditions() const
    {
        return boost::hana::at_key(boost::hana::at_key(m_preconditions, boost::hana::type<R> {}), boost::hana::type<P> {});
    }
    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto get_all_preconditions_range() const
    {
        return get_preconditions<R, P>()->compressed_range();
    }
    size_t get_num_preconditions() const { return m_num_preconditions; }
    Index get_effect() const { return m_derived_effect; }
    bool get_polarity() const { return m_polarity; }

private:
    Index m_index;
    ConditionsContainer m_preconditions;
    size_t m_num_preconditions;
    Index m_derived_effect;
    bool m_polarity;
};
using AxiomList = std::vector<Axiom>;

template<typename T>
concept IsStructure = requires(T a) {
    { a.get_index() } -> std::convertible_to<Index>;
    { a.template get_all_preconditions_range<formalism::PositiveTag, formalism::FluentTag>() };   // TODO add return value check, i.e., RangeV3Over
    { a.template get_all_preconditions_range<formalism::PositiveTag, formalism::DerivedTag>() };  // TODO add return value check, i.e., RangeV3Over
    { a.template get_all_preconditions_range<formalism::NegativeTag, formalism::FluentTag>() };   // TODO add return value check, i.e., RangeV3Over
    { a.template get_all_preconditions_range<formalism::NegativeTag, formalism::DerivedTag>() };  // TODO add return value check, i.e., RangeV3Over
    { a.get_num_preconditions() } -> std::convertible_to<size_t>;
    { a.get_effect() } -> std::convertible_to<Index>;
    { a.get_polarity() } -> std::convertible_to<bool>;
};

using StructuresContainer = boost::hana::map<boost::hana::pair<boost::hana::type<Action>, ActionList>, boost::hana::pair<boost::hana::type<Axiom>, AxiomList>>;

/**
 * Proposition
 */

class Proposition
{
public:
    Proposition(Index index, HanaContainer<IndexList, Action, Axiom> is_precondition_of, bool is_goal) :
        m_index(index),
        m_is_precondition_of(std::move(is_precondition_of)),
        m_is_goal(is_goal)
    {
    }

    Index get_index() const { return m_index; }
    template<IsStructure S>
    const IndexList& is_precondition_of() const
    {
        return boost::hana::at_key(m_is_precondition_of, boost::hana::type<S> {});
    }
    const HanaContainer<IndexList, Action, Axiom>& is_precondition_of() const { return m_is_precondition_of; }
    bool is_goal() const { return m_is_goal; }

private:
    Index m_index;
    HanaContainer<IndexList, Action, Axiom> m_is_precondition_of;
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
/// For simplicity, we follow this approach.
/// However, we could provide a stronger but cheaper approximation which is not y <- not phi_i for each i=1,...,k.
/// In other words, we derive not y if at least one y <- phi_i fails to derive y.
/// This is very cheap to represent and compute but requires a bit of thinking on how to implement it because it requires instantiating new conditions.
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
    using IsPreconditionOfContainer =
        HanaContainer<HanaContainer<IndexMap<IndexList>, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag>;

    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    IndexMap<IndexList>& get(IsPreconditionOfContainer& container)
    {
        return boost::hana::at_key(boost::hana::at_key(container, boost::hana::type<R> {}), boost::hana::type<P> {});
    }

    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) : m_offsets(), m_atom_indices(), m_structures(), m_num_unsat_goals(0)
    {
        /**
         * Instantiate actions.
         */

        auto [is_precondition_of_action, trivial_unary_actions] = instantiate_actions(delete_relaxation);

        /**
         * Instantiate axioms
         */

        auto [is_precondition_of_axiom, trivial_unary_axioms] = instantiate_axioms(delete_relaxation);

        /**
         * Instantiate atoms.
         */

        instantiate_atoms<formalism::FluentTag>(*delete_relaxation.get_problem());
        instantiate_atoms<formalism::DerivedTag>(*delete_relaxation.get_problem());

        /**
         * Instantiate propositions.
         */

        // Create a dummy proposition with index 0 that contains all trivial unary actions and axioms without preconditions.
        m_propositions.emplace_back(m_propositions.size(),
                                    boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Action> {}, std::move(trivial_unary_actions)),
                                                          boost::hana::make_pair(boost::hana::type<Axiom> {}, std::move(trivial_unary_axioms))),
                                    false);

        instantiate_propositions<formalism::PositiveTag, formalism::FluentTag>(*delete_relaxation.get_problem(),
                                                                               is_precondition_of_action,
                                                                               is_precondition_of_axiom);
        instantiate_propositions<formalism::NegativeTag, formalism::FluentTag>(*delete_relaxation.get_problem(),
                                                                               is_precondition_of_action,
                                                                               is_precondition_of_axiom);
        instantiate_propositions<formalism::PositiveTag, formalism::DerivedTag>(*delete_relaxation.get_problem(),
                                                                                is_precondition_of_action,
                                                                                is_precondition_of_axiom);
        instantiate_propositions<formalism::NegativeTag, formalism::DerivedTag>(*delete_relaxation.get_problem(),
                                                                                is_precondition_of_action,
                                                                                is_precondition_of_axiom);

        /**
         * Instantiate cost annotations.
         */

        get_structures_annotations<Action>().resize(get_structures<Action>().size());
        get_structures_annotations<Axiom>().resize(get_structures<Axiom>().size());
        get_proposition_annotations().resize(get_propositions().size());
    }

    std::tuple<IsPreconditionOfContainer, IndexList> instantiate_actions(const DeleteRelaxedProblemExplorator& delete_relaxation)
    {
        auto& actions = get_structures<Action>();
        auto is_precondition_of_action = IsPreconditionOfContainer {};
        auto trivial_unary_actions = IndexList {};

        for (const auto& action : delete_relaxation.create_ground_actions())
        {
            for (const auto& cond_effect : action->get_conditional_effects())
            {
                const auto num_preconditions = action->get_conjunctive_condition()->get_num_preconditions<formalism::FluentTag, formalism::DerivedTag>()
                                               + cond_effect->get_conjunctive_condition()->get_num_preconditions<formalism::FluentTag, formalism::DerivedTag>();

                auto preconditions = action->get_conjunctive_condition()->get_hana_compressed_precondition<formalism::FluentTag, formalism::DerivedTag>();
                auto conditional_preconditions =
                    cond_effect->get_conjunctive_condition()->get_hana_compressed_precondition<formalism::FluentTag, formalism::DerivedTag>();

                for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_propositional_effects<formalism::PositiveTag>())
                {
                    const auto unary_action_index = actions.size();

                    /* Positive */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        get<formalism::PositiveTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        get<formalism::PositiveTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        get<formalism::PositiveTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        get<formalism::PositiveTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    /* Negative */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        get<formalism::NegativeTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        get<formalism::NegativeTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        get<formalism::NegativeTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        get<formalism::NegativeTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }

                    actions.push_back(Action(unary_action_index, preconditions, conditional_preconditions, num_preconditions, eff_atom_index, true));

                    if (actions.back().get_num_preconditions() == 0)
                    {
                        trivial_unary_actions.push_back(unary_action_index);
                    }
                }
                for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_propositional_effects<formalism::NegativeTag>())
                {
                    const auto unary_action_index = actions.size();

                    /* Positive */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        get<formalism::PositiveTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        get<formalism::PositiveTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
                    {
                        get<formalism::PositiveTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
                    {
                        get<formalism::PositiveTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    /* Negative */
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        get<formalism::NegativeTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index : action->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        get<formalism::NegativeTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
                    {
                        get<formalism::NegativeTag, formalism::FluentTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }
                    for (const auto& pre_atom_index :
                         cond_effect->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
                    {
                        get<formalism::NegativeTag, formalism::DerivedTag>(is_precondition_of_action)[pre_atom_index].push_back(unary_action_index);
                    }

                    actions.push_back(Action(unary_action_index, preconditions, conditional_preconditions, num_preconditions, eff_atom_index, false));

                    if (actions.back().get_num_preconditions() == 0)
                    {
                        trivial_unary_actions.push_back(unary_action_index);
                    }
                }
            }
        }

        return std::make_tuple(std::move(is_precondition_of_action), std::move(trivial_unary_actions));
    }

    std::tuple<IsPreconditionOfContainer, IndexList> instantiate_axioms(const DeleteRelaxedProblemExplorator& delete_relaxation)
    {
        auto& axioms = get_structures<Axiom>();
        auto is_precondition_of_axiom = IsPreconditionOfContainer {};
        auto trivial_unary_axioms = IndexList {};

        for (const auto& axiom : delete_relaxation.create_ground_axioms())
        {
            const auto unary_axiom_index = axioms.size();

            /* Positive */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::FluentTag>())
            {
                get<formalism::PositiveTag, formalism::FluentTag>(is_precondition_of_axiom)[pre_atom_index].push_back(unary_axiom_index);
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::PositiveTag, formalism::DerivedTag>())
            {
                get<formalism::PositiveTag, formalism::DerivedTag>(is_precondition_of_axiom)[pre_atom_index].push_back(unary_axiom_index);
            }
            /* Negative */
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::FluentTag>())
            {
                get<formalism::NegativeTag, formalism::FluentTag>(is_precondition_of_axiom)[pre_atom_index].push_back(unary_axiom_index);
            }
            for (const auto& pre_atom_index : axiom->get_conjunctive_condition()->get_precondition<formalism::NegativeTag, formalism::DerivedTag>())
            {
                get<formalism::NegativeTag, formalism::DerivedTag>(is_precondition_of_axiom)[pre_atom_index].push_back(unary_axiom_index);
            }

            const auto num_preconditions = axiom->get_conjunctive_condition()->get_num_preconditions<formalism::FluentTag, formalism::DerivedTag>();

            auto preconditions = axiom->get_conjunctive_condition()->get_hana_compressed_precondition<formalism::FluentTag, formalism::DerivedTag>();

            axioms.push_back(Axiom(unary_axiom_index, preconditions, num_preconditions, axiom->get_literal()->get_atom()->get_index(), true));

            if (axioms.back().get_num_preconditions() == 0)
            {
                trivial_unary_axioms.push_back(unary_axiom_index);
            }
        }

        return std::make_tuple(std::move(is_precondition_of_axiom), std::move(trivial_unary_axioms));
    }

    template<formalism::IsFluentOrDerivedTag P>
    void instantiate_atoms(const formalism::ProblemImpl& problem)
    {
        const auto& ground_atom_repository =
            boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<formalism::GroundAtomImpl<P>> {});

        auto& atoms = get_atom_indices<P>();
        atoms.resize(ground_atom_repository.size());
        std::iota(atoms.begin(), atoms.end(), 0);
    }

    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    void instantiate_propositions(const formalism::ProblemImpl& problem,
                                  IsPreconditionOfContainer& is_precondition_of_action,
                                  IsPreconditionOfContainer& is_precondition_of_axiom)
    {
        const auto& ground_atom_repository =
            boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<formalism::GroundAtomImpl<P>> {});

        auto& offsets = get_offsets<R, P>();

        for (Index atom_index = 0; atom_index < ground_atom_repository.size(); ++atom_index)
        {
            const auto proposition_index = m_propositions.size();

            if (atom_index >= offsets.size())
            {
                offsets.resize(atom_index + 1, MAX_INDEX);
            }
            offsets[atom_index] = proposition_index;

            const auto is_goal = problem.template get_goal_atoms_bitset<R, P>().get(atom_index);

            m_propositions.emplace_back(
                proposition_index,
                boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Action> {}, std::move(get<R, P>(is_precondition_of_action)[atom_index])),
                                      boost::hana::make_pair(boost::hana::type<Axiom> {}, std::move(get<R, P>(is_precondition_of_axiom)[atom_index]))),
                is_goal);

            if (is_goal)
                m_goal_propositions.push_back(proposition_index);
        }
    }

    void initialize_and_annotations()
    {
        boost::hana::for_each(m_structures,
                              [this](auto&& pair)
                              {
                                  const auto& structures = boost::hana::second(pair);
                                  for (const auto& structure : structures)
                                  {
                                      self().initialize_and_annotations_impl(structure);
                                  }
                              });
    }

    void initialize_or_annotations()
    {
        for (const auto& proposition : m_propositions)
        {
            self().initialize_or_annotations_impl(proposition);
        }
    }

    template<formalism::IsFluentOrDerivedTag P>
    void initialize_or_annotations_and_queue_helper(State state)
    {
        if constexpr (std::is_same_v<P, formalism::DerivedTag>)
        {
            // Derive all negative derived propositions immediately, i.e., not y <- T for all y.

            auto& positive_offsets = get_offsets<formalism::PositiveTag, P>();
            auto& negative_offsets = get_offsets<formalism::NegativeTag, P>();

            for (const auto& atom_index : state->get_atoms<P>())
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[positive_offsets[atom_index]]);
            }

            for (const auto& atom_index : get_atom_indices<P>())
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[negative_offsets[atom_index]]);
            }
        }
        else
        {
            auto& positive_offsets = get_offsets<formalism::PositiveTag, P>();
            auto& negative_offsets = get_offsets<formalism::NegativeTag, P>();
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
    }

    void initialize_or_annotations_and_queue(State state)
    {
        this->m_queue.clear();

        initialize_or_annotations_and_queue_helper<formalism::FluentTag>(state);
        initialize_or_annotations_and_queue_helper<formalism::DerivedTag>(state);

        // Trivial dummy proposition to trigger actions and axioms without preconditions
        self().initialize_or_annotations_and_queue_impl(m_propositions[0]);
    }

    template<IsStructure S>
    void on_process_structure(const Proposition& proposition, const S& structure)
    {
        auto& structure_annotation = get_structures_annotations<S>()[structure.get_index()];

        self().update_and_annotation_impl(proposition, structure);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(structure_annotation);
        }

        if (get_num_unsatisfied_preconditions(structure_annotation) == 0)
        {
            // std::cout << "Process satisfied structure: " << structure.get_index() << std::endl;

            const auto effect_proposition_index = structure.get_polarity() ?
                                                      get_offsets<formalism::PositiveTag, formalism::FluentTag>()[structure.get_effect()] :
                                                      get_offsets<formalism::NegativeTag, formalism::FluentTag>()[structure.get_effect()];

            const auto& effect_proposition = get_propositions()[effect_proposition_index];

            self().update_or_annotation_impl(structure, effect_proposition);
        }
    }

    void dijksta()
    {
        m_num_unsat_goals = m_goal_propositions.size();

        while (!m_queue.empty())
        {
            const auto entry = m_queue.top();
            m_queue.pop();

            const auto& proposition = get_propositions()[entry.proposition_index];
            const auto& annotation = get_proposition_annotations()[entry.proposition_index];

            // std::cout << "Queue pop: " << entry.proposition_index << " " << entry.cost << std::endl;

            if (get_cost(annotation) < entry.cost)
            {
                continue;
            }

            if (proposition.is_goal() && --m_num_unsat_goals == 0)
            {
                return;
            }

            boost::hana::for_each(proposition.is_precondition_of(),
                                  [this, &proposition](auto&& pair)
                                  {
                                      using KeyType = typename decltype(+boost::hana::first(pair))::type;
                                      const auto& structure_indices = boost::hana::second(pair);
                                      const auto& structures = get_structures<KeyType>();

                                      for (const auto structure_index : structure_indices)
                                      {
                                          const auto& structure = structures[structure_index];

                                          self().on_process_structure(proposition, structure);
                                      }
                                  });
        }

        // std::cout << "Num unsat goals: " << num_unsat_goals << std::endl;
    }

    HanaContainer<HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag> m_offsets;

    template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
    auto& get_offsets()
    {
        return boost::hana::at_key(boost::hana::at_key(m_offsets, boost::hana::type<R> {}), boost::hana::type<P> {});
    }

    HanaContainer<IndexList, formalism::FluentTag, formalism::DerivedTag> m_atom_indices;

    template<formalism::IsFluentOrDerivedTag P>
    auto& get_atom_indices()
    {
        return boost::hana::at_key(m_atom_indices, boost::hana::type<P> {});
    }

    StructuresContainer m_structures;

    template<IsStructure S>
    auto& get_structures()
    {
        return boost::hana::at_key(m_structures, boost::hana::type<S> {});
    }

    HanaContainer<AnnotationsList<DiscreteCost, size_t>, Action, Axiom> m_structure_annotations;

    template<IsStructure S>
    auto& get_structures_annotations()
    {
        return boost::hana::at_key(m_structure_annotations, boost::hana::type<S> {});
    }

    static DiscreteCost& get_cost(Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<0>(annotation); }
    static size_t& get_num_unsatisfied_preconditions(Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }
    static size_t get_num_unsatisfied_preconditions(const Annotations<DiscreteCost, size_t>& annotation) { return std::get<1>(annotation); }

    PropositionList m_propositions;

    auto& get_propositions() { return m_propositions; }

    AnnotationsList<DiscreteCost> m_proposition_annotations;

    auto& get_proposition_annotations() { return m_proposition_annotations; }

    static DiscreteCost& get_cost(Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }
    static DiscreteCost get_cost(const Annotations<DiscreteCost>& annotation) { return std::get<0>(annotation); }

    IndexList m_goal_propositions;

    auto& get_goal_propositions() { return m_goal_propositions; }

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
