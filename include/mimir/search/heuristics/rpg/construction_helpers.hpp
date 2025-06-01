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

#ifndef MIMIR_SEARCH_HEURISTICS_RPG_CONSTRUCTION_HELPERS_HPP_
#define MIMIR_SEARCH_HEURISTICS_RPG_CONSTRUCTION_HELPERS_HPP_

#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics/rpg/annotations.hpp"
#include "mimir/search/heuristics/rpg/proposition.hpp"
#include "mimir/search/heuristics/rpg/structures.hpp"

namespace mimir::search::rpg
{
using IsPreconditionOfContainer =
    HanaContainer<HanaContainer<IndexMap<IndexList>, formalism::FluentTag, formalism::DerivedTag>, formalism::PositiveTag, formalism::NegativeTag>;

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
IndexMap<IndexList>& get(IsPreconditionOfContainer& container)
{
    return boost::hana::at_key(boost::hana::at_key(container, boost::hana::type<R> {}), boost::hana::type<P> {});
}

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
void fill_precondition_of_structure(formalism::GroundConjunctiveCondition condition, IsPreconditionOfContainer& is_precondition_of, Index structure_index)
{
    for (const auto& pre_atom_index : condition->get_precondition<R, P>())
    {
        get<R, P>(is_precondition_of)[pre_atom_index].push_back(structure_index);
    }
}

inline std::tuple<ActionList, IsPreconditionOfContainer, IndexList> instantiate_actions(const DeleteRelaxedProblemExplorator& delete_relaxation)
{
    auto actions = ActionList {};
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
                fill_precondition_of_structure<formalism::PositiveTag, formalism::FluentTag>(action->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::DerivedTag>(action->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::FluentTag>(cond_effect->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::DerivedTag>(cond_effect->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);

                /* Negative */
                fill_precondition_of_structure<formalism::NegativeTag, formalism::FluentTag>(action->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::DerivedTag>(action->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::FluentTag>(cond_effect->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::DerivedTag>(cond_effect->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);

                actions.push_back(Action(unary_action_index, action, preconditions, conditional_preconditions, num_preconditions, eff_atom_index, true));

                if (actions.back().get_num_preconditions() == 0)
                {
                    trivial_unary_actions.push_back(unary_action_index);
                }
            }
            for (const auto& eff_atom_index : cond_effect->get_conjunctive_effect()->get_propositional_effects<formalism::NegativeTag>())
            {
                const auto unary_action_index = actions.size();

                /* Positive */
                fill_precondition_of_structure<formalism::PositiveTag, formalism::FluentTag>(action->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::DerivedTag>(action->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::FluentTag>(cond_effect->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::PositiveTag, formalism::DerivedTag>(cond_effect->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);

                /* Negative */
                fill_precondition_of_structure<formalism::NegativeTag, formalism::FluentTag>(action->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::DerivedTag>(action->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::FluentTag>(cond_effect->get_conjunctive_condition(),
                                                                                             is_precondition_of_action,
                                                                                             unary_action_index);
                fill_precondition_of_structure<formalism::NegativeTag, formalism::DerivedTag>(cond_effect->get_conjunctive_condition(),
                                                                                              is_precondition_of_action,
                                                                                              unary_action_index);

                actions.push_back(Action(unary_action_index, action, preconditions, conditional_preconditions, num_preconditions, eff_atom_index, false));

                if (actions.back().get_num_preconditions() == 0)
                {
                    trivial_unary_actions.push_back(unary_action_index);
                }
            }
        }
    }

    // std::cout << "[RPG] Number of unary actions: " << actions.size() << std::endl;

    return std::make_tuple(std::move(actions), std::move(is_precondition_of_action), std::move(trivial_unary_actions));
}

inline std::tuple<AxiomList, IsPreconditionOfContainer, IndexList> instantiate_axioms(const DeleteRelaxedProblemExplorator& delete_relaxation)
{
    auto axioms = AxiomList {};
    auto is_precondition_of_axiom = IsPreconditionOfContainer {};
    auto trivial_unary_axioms = IndexList {};

    for (const auto& axiom : delete_relaxation.create_ground_axioms())
    {
        const auto unary_axiom_index = axioms.size();

        /* Positive */
        fill_precondition_of_structure<formalism::PositiveTag, formalism::FluentTag>(axiom->get_conjunctive_condition(),
                                                                                     is_precondition_of_axiom,
                                                                                     unary_axiom_index);
        fill_precondition_of_structure<formalism::PositiveTag, formalism::DerivedTag>(axiom->get_conjunctive_condition(),
                                                                                      is_precondition_of_axiom,
                                                                                      unary_axiom_index);

        /* Negative */
        fill_precondition_of_structure<formalism::NegativeTag, formalism::FluentTag>(axiom->get_conjunctive_condition(),
                                                                                     is_precondition_of_axiom,
                                                                                     unary_axiom_index);
        fill_precondition_of_structure<formalism::NegativeTag, formalism::DerivedTag>(axiom->get_conjunctive_condition(),
                                                                                      is_precondition_of_axiom,
                                                                                      unary_axiom_index);

        const auto num_preconditions = axiom->get_conjunctive_condition()->get_num_preconditions<formalism::FluentTag, formalism::DerivedTag>();

        auto preconditions = axiom->get_conjunctive_condition()->get_hana_compressed_precondition<formalism::FluentTag, formalism::DerivedTag>();

        axioms.push_back(Axiom(unary_axiom_index, preconditions, num_preconditions, axiom->get_literal()->get_atom()->get_index(), true));

        if (axioms.back().get_num_preconditions() == 0)
        {
            trivial_unary_axioms.push_back(unary_axiom_index);
        }
    }

    // std::cout << "[RPG] Number of unary axioms: " << axioms.size() << std::endl;

    return std::make_tuple(std::move(axioms), std::move(is_precondition_of_axiom), std::move(trivial_unary_axioms));
}

template<formalism::IsFluentOrDerivedTag P>
IndexList instantiate_atoms(const formalism::ProblemImpl& problem)
{
    const auto& ground_atom_repository =
        boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<formalism::GroundAtomImpl<P>> {});

    auto atoms = IndexList {};
    atoms.resize(ground_atom_repository.size());
    std::iota(atoms.begin(), atoms.end(), 0);

    return atoms;
}

template<formalism::IsPolarity R, formalism::IsFluentOrDerivedTag P>
void instantiate_propositions_helper(const formalism::ProblemImpl& problem,
                                     IsPreconditionOfContainer& is_precondition_of_action,
                                     IsPreconditionOfContainer& is_precondition_of_axiom,
                                     PropositionList& propositions,
                                     IndexList& goal_propositions,
                                     PropositionOffsets& proposition_offsets)
{
    const auto& ground_atom_repository =
        boost::hana::at_key(problem.get_repositories().get_hana_repositories(), boost::hana::type<formalism::GroundAtomImpl<P>> {});

    auto& offsets = get<R, P>(proposition_offsets);

    for (Index atom_index = 0; atom_index < ground_atom_repository.size(); ++atom_index)
    {
        const auto proposition_index = propositions.size();

        if (atom_index >= offsets.size())
        {
            offsets.resize(atom_index + 1, MAX_INDEX);
        }
        offsets[atom_index] = proposition_index;

        const auto is_goal = problem.template get_goal_atoms_bitset<R, P>().get(atom_index);

        propositions.emplace_back(
            proposition_index,
            boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Action> {}, std::move(get<R, P>(is_precondition_of_action)[atom_index])),
                                  boost::hana::make_pair(boost::hana::type<Axiom> {}, std::move(get<R, P>(is_precondition_of_axiom)[atom_index]))),
            is_goal);

        if (is_goal)
            goal_propositions.push_back(proposition_index);
    }
}

inline std::tuple<PropositionList, IndexList, PropositionOffsets> instantiate_propositions(const formalism::ProblemImpl& problem,
                                                                                           IsPreconditionOfContainer&& is_precondition_of_action,
                                                                                           IsPreconditionOfContainer&& is_precondition_of_axiom,
                                                                                           IndexList&& trivial_unary_actions,
                                                                                           IndexList&& trivial_unary_axioms)
{
    auto propositions = PropositionList {};
    auto goal_propositions = IndexList {};
    auto proposition_offsets = PropositionOffsets {};

    // Create a dummy proposition with index 0 that contains all trivial unary actions and axioms without preconditions.
    propositions.emplace_back(propositions.size(),
                              boost::hana::make_map(boost::hana::make_pair(boost::hana::type<Action> {}, std::move(trivial_unary_actions)),
                                                    boost::hana::make_pair(boost::hana::type<Axiom> {}, std::move(trivial_unary_axioms))),
                              false);

    instantiate_propositions_helper<formalism::PositiveTag, formalism::FluentTag>(problem,
                                                                                  is_precondition_of_action,
                                                                                  is_precondition_of_axiom,
                                                                                  propositions,
                                                                                  goal_propositions,
                                                                                  proposition_offsets);
    instantiate_propositions_helper<formalism::NegativeTag, formalism::FluentTag>(problem,
                                                                                  is_precondition_of_action,
                                                                                  is_precondition_of_axiom,
                                                                                  propositions,
                                                                                  goal_propositions,
                                                                                  proposition_offsets);
    instantiate_propositions_helper<formalism::PositiveTag, formalism::DerivedTag>(problem,
                                                                                   is_precondition_of_action,
                                                                                   is_precondition_of_axiom,
                                                                                   propositions,
                                                                                   goal_propositions,
                                                                                   proposition_offsets);
    instantiate_propositions_helper<formalism::NegativeTag, formalism::DerivedTag>(problem,
                                                                                   is_precondition_of_action,
                                                                                   is_precondition_of_axiom,
                                                                                   propositions,
                                                                                   goal_propositions,
                                                                                   proposition_offsets);

    return std::make_tuple(std::move(propositions), std::move(goal_propositions), std::move(proposition_offsets));
}

}

#endif
