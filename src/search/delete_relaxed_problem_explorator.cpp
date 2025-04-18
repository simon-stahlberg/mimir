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

#include "mimir/search/delete_relaxed_problem_explorator.hpp"

#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/translator/delete_relax.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/grounded.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/axiom_evaluators/grounded.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers/default.hpp"
#include "mimir/search/axiom_evaluators/lifted.hpp"
#include "mimir/search/dense_state.hpp"
#include "mimir/search/match_tree/match_tree.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

DeleteRelaxedProblemExplorator::DeleteRelaxedProblemExplorator(Problem problem) :
    m_problem(problem),
    m_delete_relax_transformer(),
    m_delete_free_problem(),
    m_delete_free_object_to_unrelaxed_object()
{
    // std::cout << "[DeleteRelaxedProblemExplorator] Started delete relaxed exploration." << std::endl;
    // const auto start_time = std::chrono::high_resolution_clock::now();

    auto domain_delete_free_builder = DomainBuilder();
    auto delete_free_domain = m_delete_relax_transformer.translate_level_0(m_problem->get_domain(), domain_delete_free_builder);

    auto delete_relax_builder = ProblemBuilder(delete_free_domain);
    m_delete_free_problem = m_delete_relax_transformer.translate_level_0(m_problem, delete_relax_builder);

    auto delete_free_applicable_action_generator = LiftedApplicableActionGeneratorImpl(m_delete_free_problem);
    auto delete_free_axiom_evalator = std::make_shared<LiftedAxiomEvaluatorImpl>(m_delete_free_problem);
    auto delete_free_state_repository = StateRepositoryImpl(std::static_pointer_cast<IAxiomEvaluator>(delete_free_axiom_evalator));

    auto unrelaxed_objects_by_name = std::unordered_map<std::string, Object> {};
    for (const auto& object : m_problem->get_problem_and_domain_objects())
    {
        unrelaxed_objects_by_name.emplace(object->get_name(), object);
    }
    for (const auto& object : m_delete_free_problem->get_problem_and_domain_objects())
    {
        m_delete_free_object_to_unrelaxed_object.emplace(object, unrelaxed_objects_by_name.at(object->get_name()));
    }

    auto [initial_state, initial_metric_value] = delete_free_state_repository.get_or_create_initial_state();

    auto dense_state = DenseState(initial_state);
    auto state = initial_state;

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    do
    {
        reached_delete_free_explore_fixpoint = true;

        auto num_atoms_before = delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count();

        // Create and all applicable actions and apply them
        // Attention1: we cannot just apply newly generated actions because conditional effects might trigger later.
        // Attention2: we incrementally keep growing the atoms in the dense state.
        for (const auto& action : delete_free_applicable_action_generator.create_applicable_action_generator(dense_state))
        {
            // Note that get_or_create_successor_state already modifies dense_state to be the successor state.
            // TODO(numeric): in the delete relaxation, we have to remove all numeric constraints and effects.
            const auto [successor_state, metric_value] = delete_free_state_repository.get_or_create_successor_state(state, dense_state, action, 0);
            state = successor_state;
        }

        // Create and all applicable axioms and apply them
        delete_free_axiom_evalator->generate_and_apply_axioms(dense_state);

        // Note: checking fluent atoms suffices because derived are implied by those.
        auto num_atoms_after = delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);

    // const auto end_time = std::chrono::high_resolution_clock::now();
    // const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    // std::cout << "[DeleteRelaxedProblemExplorator] Total time for delete relaxed exploration: " << total_time.count() << "\n"
    //           << "[DeleteRelaxedProblemExplorator] Number of fluent grounded atoms reachable in delete-free problem: "
    //           << delete_free_state_repository.get_reached_fluent_ground_atoms_bitset().count() << "\n"
    //           << "[DeleteRelaxedProblemExplorator] Number of derived grounded atoms reachable in delete-free problem: "
    //           << delete_free_state_repository.get_reached_derived_ground_atoms_bitset().count() << std::endl;
}

static ObjectList translate_from_delete_free_to_unrelaxed_problem(const ObjectList& objects, const ToObjectMap<Object>& delete_free_object_to_unrelaxed_object)
{
    auto result = ObjectList {};
    result.reserve(objects.size());
    for (const auto& object : objects)
    {
        result.push_back(delete_free_object_to_unrelaxed_object.at(object));
    }
    return result;
}

GroundedAxiomEvaluator DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator(const match_tree::Options& options,
                                                                                       GroundedAxiomEvaluatorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_axiom_instantiation();
    auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& pddl_repositories = problem.get_repositories();

    /* Initialize bookkeeping to map ground axioms into corresponding partition. */
    const auto num_partitions = problem.get_problem_and_domain_axiom_partitioning().size();
    auto axiom_to_partition = std::unordered_map<Axiom, size_t> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        const auto& partition = problem.get_problem_and_domain_axiom_partitioning()[i];
        for (const auto& axiom : partition.get_axioms())
        {
            axiom_to_partition.emplace(axiom, i);
        }
    }

    /* Store ground axioms in corresponding partition. */
    auto ground_axiom_partitioning = std::vector<GroundAxiomList>(num_partitions);

    for (const auto& delete_free_ground_axiom :
         boost::hana::at_key(m_delete_free_problem->get_repositories().get_hana_repositories(), boost::hana::type<GroundAxiomImpl> {}))
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& axiom : m_delete_relax_transformer.get_unrelaxed_axioms(delete_free_ground_axiom->get_axiom()))
        {
            auto binding = translate_from_delete_free_to_unrelaxed_problem(delete_free_ground_axiom->get_objects(), m_delete_free_object_to_unrelaxed_object);

            auto ground_axiom = problem.ground(axiom, std::move(binding));

            if (is_statically_applicable(ground_axiom->get_conjunctive_condition(), problem.get_static_initial_positive_atoms_bitset()))
            {
                ground_axiom_partitioning.at(axiom_to_partition.at(axiom)).push_back(ground_axiom);
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_axiom_instantiation(total_time);

    event_handler->on_start_build_axiom_match_trees();
    start_time = std::chrono::high_resolution_clock::now();

    /* Create a MatchTree for each partition. */
    auto match_tree_partitioning = std::vector<std::unique_ptr<match_tree::MatchTreeImpl<GroundAxiomImpl>>> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        event_handler->on_start_build_axiom_match_tree(i);

        const auto& ground_axioms = ground_axiom_partitioning.at(i);

        auto match_tree = match_tree::MatchTreeImpl<GroundAxiomImpl>::create(pddl_repositories, ground_axioms, options);

        event_handler->on_finish_build_axiom_match_tree(*match_tree);

        match_tree_partitioning.push_back(std::move(match_tree));
    }

    end_time = std::chrono::high_resolution_clock::now();
    total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_build_axiom_match_trees(total_time);

    return GroundedAxiomEvaluatorImpl::create(m_problem, std::move(match_tree_partitioning), std::move(event_handler));
}

GroundedApplicableActionGenerator
DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator(const match_tree::Options& options,
                                                                            GroundedApplicableActionGeneratorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_action_instantiation();
    const auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& pddl_repositories = problem.get_repositories();

    auto ground_actions = GroundActionList {};
    for (const auto& delete_free_ground_action :
         boost::hana::at_key(m_delete_free_problem->get_repositories().get_hana_repositories(), boost::hana::type<GroundActionImpl> {}))
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& action : m_delete_relax_transformer.get_unrelaxed_actions(delete_free_ground_action->get_action()))
        {
            auto binding = translate_from_delete_free_to_unrelaxed_problem(delete_free_ground_action->get_objects(), m_delete_free_object_to_unrelaxed_object);

            auto grounded_action = problem.ground(action, std::move(binding));

            if (is_statically_applicable(grounded_action->get_conjunctive_condition(), problem.get_static_initial_positive_atoms_bitset()))
            {
                ground_actions.push_back(grounded_action);
            }
        }
    }

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_action_instantiation(total_time);

    event_handler->on_start_build_action_match_tree();

    auto match_tree = match_tree::MatchTreeImpl<GroundActionImpl>::create(pddl_repositories, ground_actions, options);

    event_handler->on_finish_build_action_match_tree(*match_tree);

    return GroundedApplicableActionGeneratorImpl::create(m_problem, std::move(match_tree), std::move(event_handler));
}

const Problem& DeleteRelaxedProblemExplorator::get_problem() const { return m_problem; }

}  // namespace mimir
