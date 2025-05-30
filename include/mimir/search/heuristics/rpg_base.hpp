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
#include "mimir/search/heuristics/rpg/annotations.hpp"
#include "mimir/search/heuristics/rpg/construction_helpers.hpp"
#include "mimir/search/heuristics/rpg/proposition.hpp"
#include "mimir/search/heuristics/rpg/structures.hpp"
#include "mimir/search/openlists/priority_queue.hpp"

namespace mimir::search::rpg
{

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
/// I also tried pruning irrelevant unary actions and axioms in the exact way but it is too costly and the amound of pruned actions is too low.
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
        if (is_goal_state)
            return 0.;

        self().initialize_and_annotations();
        self().initialize_or_annotations();
        self().initialize_or_annotations_and_queue(state);
        dijksta();
        return (m_num_unsat_goals > 0) ? INFINITY_CONTINUOUS_COST : self().extract_impl(state);
    }

private:
    explicit RelaxedPlanningGraph(const DeleteRelaxedProblemExplorator& delete_relaxation) :
        m_problem(delete_relaxation.get_problem()),
        m_offsets(),
        m_atom_indices(),
        m_structures(),
        m_structure_annotations(),
        m_propositions(),
        m_proposition_annotations(),
        m_goal_propositions(),
        m_num_unsat_goals(0),
        m_queue()
    {
        /**
         * Instantiate actions.
         */

        auto [actions, is_precondition_of_action, trivial_unary_actions] = instantiate_actions(delete_relaxation);
        get<Action>(get_structures()) = std::move(actions);

        /**
         * Instantiate axioms
         */

        auto [axioms, is_precondition_of_axiom, trivial_unary_axioms] = instantiate_axioms(delete_relaxation);
        get<Axiom>(get_structures()) = std::move(axioms);

        /**
         * Instantiate atoms.
         */

        auto fluent_atoms_indices = instantiate_atoms<formalism::FluentTag>(*delete_relaxation.get_problem());
        get<formalism::FluentTag>(get_atom_indices()) = std::move(fluent_atoms_indices);
        auto derived_atom_indices = instantiate_atoms<formalism::DerivedTag>(*delete_relaxation.get_problem());
        get<formalism::DerivedTag>(get_atom_indices()) = std::move(derived_atom_indices);

        /**
         * Instantiate propositions.
         */

        auto [propositions, goal_propositions, proposition_offsets] = instantiate_propositions(*delete_relaxation.get_problem(),
                                                                                               std::move(is_precondition_of_action),
                                                                                               std::move(is_precondition_of_axiom),
                                                                                               std::move(trivial_unary_actions),
                                                                                               std::move(trivial_unary_axioms));
        get_propositions() = std::move(propositions);
        get_goal_propositions() = std::move(goal_propositions);
        get_offsets() = std::move(proposition_offsets);

        /**
         * Instantiate cost annotations.
         */

        get<Action>(get_structures_annotations()).resize(get<Action>(get_structures()).size());
        get<Axiom>(get_structures_annotations()).resize(get<Axiom>(get_structures()).size());
        get_proposition_annotations().resize(get_propositions().size());
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
        for (const auto& proposition : get_propositions())
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

            auto& positive_offsets = get<formalism::PositiveTag, P>(get_offsets());
            auto& negative_offsets = get<formalism::NegativeTag, P>(get_offsets());

            for (const auto& atom_index : state->get_atoms<P>())
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[positive_offsets[atom_index]]);
            }

            for (const auto& atom_index : get<P>(get_atom_indices()))
            {
                self().initialize_or_annotations_and_queue_impl(m_propositions[negative_offsets[atom_index]]);
            }
        }
        else
        {
            auto& positive_offsets = get<formalism::PositiveTag, P>(get_offsets());
            auto& negative_offsets = get<formalism::NegativeTag, P>(get_offsets());
            const auto& all_atoms = get<P>(get_atom_indices());
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
        auto& structure_annotation = get<S>(get_structures_annotations())[structure.get_index()];

        self().update_and_annotation_impl(proposition, structure);

        if (proposition.get_index() != DUMMY_PROPOSITION_INDEX)
        {
            --get_num_unsatisfied_preconditions(structure_annotation);
        }

        if (get_num_unsatisfied_preconditions(structure_annotation) == 0)
        {
            // std::cout << "Process satisfied structure: " << structure.get_index() << std::endl;

            const auto effect_proposition_index = structure.get_polarity() ?
                                                      get<formalism::PositiveTag, formalism::FluentTag>(get_offsets())[structure.get_effect()] :
                                                      get<formalism::NegativeTag, formalism::FluentTag>(get_offsets())[structure.get_effect()];

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
                                      const auto& structures = get<KeyType>(get_structures());

                                      for (const auto structure_index : structure_indices)
                                      {
                                          const auto& structure = structures[structure_index];

                                          self().on_process_structure(proposition, structure);
                                      }
                                  });
        }

        // std::cout << "Num unsat goals: " << num_unsat_goals << std::endl;
    }

    formalism::Problem m_problem;

    const formalism::ProblemImpl& get_problem() const { return *m_problem; }

    PropositionOffsets m_offsets;

    auto& get_offsets() { return m_offsets; }

    AtomIndicesContainer m_atom_indices;

    auto& get_atom_indices() { return m_atom_indices; }

    StructuresContainer m_structures;

    auto& get_structures() { return m_structures; }

    StructuresAnnotations m_structure_annotations;

    auto& get_structures_annotations() { return m_structure_annotations; }

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
