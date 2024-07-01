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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/lifted.hpp"

#include "mimir/common/itertools.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/condition_grounders.hpp"
#include "mimir/search/translations.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>
#include <vector>

using namespace std::string_literals;

namespace mimir
{

class GroundAndEvaluateFunctionExpressionVisitor
{
private:
    const GroundFunctionToValue& m_ground_function_value_costs;
    const ObjectList& m_binding;
    PDDLFactories& m_pddl_factories;

    GroundFunction ground_function(const Function& function)
    {
        auto grounded_terms = ObjectList {};
        m_pddl_factories.ground_variables(function->get_terms(), m_binding, grounded_terms);
        return m_pddl_factories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);
    }

public:
    GroundAndEvaluateFunctionExpressionVisitor(const GroundFunctionToValue& ground_function_value_costs,
                                               const ObjectList& binding,
                                               PDDLFactories& ref_pddl_factories) :

        m_ground_function_value_costs(ground_function_value_costs),
        m_binding(binding),
        m_pddl_factories(ref_pddl_factories)
    {
    }

    double operator()(const FunctionExpressionNumberImpl& expr) { return expr.get_number(); }

    double operator()(const FunctionExpressionBinaryOperatorImpl& expr)
    {
        return evaluate_binary(expr.get_binary_operator(),
                               std::visit(*this, *expr.get_left_function_expression()),
                               std::visit(*this, *expr.get_right_function_expression()));
    }

    double operator()(const FunctionExpressionMultiOperatorImpl& expr)
    {
        assert(!expr.get_function_expressions().empty());

        auto it = expr.get_function_expressions().begin();
        auto result = std::visit(*this, **it);
        for (; it != expr.get_function_expressions().end(); ++it)
        {
            result = evaluate_multi(expr.get_multi_operator(), result, std::visit(*this, **it));
        }

        return result;
    }

    double operator()(const FunctionExpressionMinusImpl& expr) { return -std::visit(*this, *expr.get_function_expression()); }

    double operator()(const FunctionExpressionFunctionImpl& expr)
    {
        auto grounded_function = ground_function(expr.get_function());

        auto it = m_ground_function_value_costs.find(grounded_function);
        if (it == m_ground_function_value_costs.end())
        {
            throw std::runtime_error("No numeric fluent available to determine cost for ground function "s + grounded_function->str());
        }
        const auto cost = it->second;

        return cost;
    }
};

const std::vector<AxiomPartition>& LiftedApplicableActionGenerator::get_axiom_partitioning() const { return m_axiom_evaluator.get_axiom_partitioning(); }

GroundAxiom LiftedApplicableActionGenerator::ground_axiom(Axiom axiom, ObjectList&& binding)
{
    return m_axiom_evaluator.ground_axiom(axiom, std::move(binding));
}

GroundAction LiftedApplicableActionGenerator::ground_action(Action action, ObjectList&& binding)
{
    /* 1. Check if grounding is cached */

    auto& groundings = m_action_groundings[action];
    auto it = groundings.find(binding);
    if (it != groundings.end())
    {
        m_event_handler->on_ground_action_cache_hit(action, binding);

        return it->second;
    }

    m_event_handler->on_ground_action_cache_miss(action, binding);

    /* 2. Ground the action */

    m_event_handler->on_ground_action(action, binding);

    const auto fill_effect = [this](const Literal<Fluent>& literal, FlatSimpleEffect& ref_effect, const auto& binding)
    {
        const auto grounded_literal = m_pddl_factories->ground_literal(literal, binding);
        ref_effect.is_negated = grounded_literal->is_negated();
        ref_effect.atom_id = grounded_literal->get_atom()->get_identifier();
    };

    /* Header */

    m_action_builder.get_id() = m_flat_actions.size();
    m_action_builder.get_cost() =
        std::visit(GroundAndEvaluateFunctionExpressionVisitor(m_ground_function_value_costs, binding, *m_pddl_factories), *action->get_function_expression());
    m_action_builder.get_action() = action;
    auto& objects = m_action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj);
    }

    /* Precondition */
    auto strips_precondition_proxy = StripsActionPreconditionBuilderProxy(m_action_builder.get_strips_precondition());
    auto& positive_fluent_precondition = strips_precondition_proxy.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = strips_precondition_proxy.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = strips_precondition_proxy.get_positive_precondition<Static>();
    auto& negative_static_precondition = strips_precondition_proxy.get_negative_precondition<Static>();
    auto& positive_derived_precondition = strips_precondition_proxy.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = strips_precondition_proxy.get_negative_precondition<Derived>();
    positive_fluent_precondition.unset_all();
    negative_fluent_precondition.unset_all();
    positive_static_precondition.unset_all();
    negative_static_precondition.unset_all();
    positive_derived_precondition.unset_all();
    negative_derived_precondition.unset_all();
    m_pddl_factories->ground_and_fill_bitset(action->get_conditions<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    m_pddl_factories->ground_and_fill_bitset(action->get_conditions<Static>(), positive_static_precondition, negative_static_precondition, binding);
    m_pddl_factories->ground_and_fill_bitset(action->get_conditions<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);

    /* Simple effects */
    auto strips_effect_proxy = StripsActionEffectBuilderProxy(m_action_builder.get_strips_effect());
    auto& positive_effect = strips_effect_proxy.get_positive_effects();
    auto& negative_effect = strips_effect_proxy.get_negative_effects();
    positive_effect.unset_all();
    negative_effect.unset_all();
    auto effect_literals = LiteralList<Fluent> {};
    for (const auto& effect : action->get_simple_effects())
    {
        effect_literals.push_back(effect->get_effect());
    }
    m_pddl_factories->ground_and_fill_bitset(effect_literals, positive_effect, negative_effect, binding);

    /* Conditional effects */
    // Fetch data
    auto& conditional_effects = m_action_builder.get_conditional_effects();

    // Resize builders.
    // Note: flatmemory tracks "actual" size to avoid deallocation of nested types
    const auto num_conditional_effects = action->get_conditional_effects().size();
    conditional_effects.resize(num_conditional_effects);
    if (num_conditional_effects > 0)
    {
        for (size_t i = 0; i < num_conditional_effects; ++i)
        {
            auto cond_effect_proxy_i = ConditionalEffectBuilderProxy(conditional_effects[i]);
            auto& cond_positive_fluent_precondition_i = cond_effect_proxy_i.get_positive_precondition<Fluent>();
            auto& cond_negative_fluent_precondition_i = cond_effect_proxy_i.get_negative_precondition<Fluent>();
            auto& cond_positive_static_precondition_i = cond_effect_proxy_i.get_positive_precondition<Static>();
            auto& cond_negative_static_precondition_i = cond_effect_proxy_i.get_negative_precondition<Static>();
            auto& cond_positive_derived_precondition_i = cond_effect_proxy_i.get_positive_precondition<Derived>();
            auto& cond_negative_derived_precondition_i = cond_effect_proxy_i.get_negative_precondition<Derived>();
            auto& cond_simple_effect_i = cond_effect_proxy_i.get_simple_effect();
            cond_positive_fluent_precondition_i.clear();
            cond_negative_fluent_precondition_i.clear();
            cond_positive_static_precondition_i.clear();
            cond_negative_static_precondition_i.clear();
            cond_positive_derived_precondition_i.clear();
            cond_negative_derived_precondition_i.clear();
            m_pddl_factories->ground_and_fill_vector(action->get_conditional_effects().at(i)->get_conditions<Fluent>(),
                                                     cond_positive_fluent_precondition_i,
                                                     cond_negative_fluent_precondition_i,
                                                     binding);
            m_pddl_factories->ground_and_fill_vector(action->get_conditional_effects().at(i)->get_conditions<Static>(),
                                                     cond_positive_static_precondition_i,
                                                     cond_negative_static_precondition_i,
                                                     binding);
            m_pddl_factories->ground_and_fill_vector(action->get_conditional_effects().at(i)->get_conditions<Derived>(),
                                                     cond_positive_derived_precondition_i,
                                                     cond_negative_derived_precondition_i,
                                                     binding);

            fill_effect(action->get_conditional_effects().at(i)->get_effect(), cond_simple_effect_i, binding);
        }
    }

    /* Universal effects */

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_ext = binding;

    const auto num_universal_effects = action->get_universal_effects().size();
    if (num_universal_effects > 0)
    {
        const auto& universal_effect_consistency_graphs = m_action_universal_effects.at(action);
        const auto binding_ext_size = binding_ext.size();
        for (size_t i = 0; i < num_universal_effects; ++i)
        {
            // Fetch data
            const auto& universal_effect = action->get_universal_effects().at(i);
            const auto& consistency_graph = universal_effect_consistency_graphs.at(i);
            const auto& objects_by_parameter_index = consistency_graph.get_objects_by_parameter_index();

            // Resize builders.
            const auto num_conditional_effects = CartesianProduct(objects_by_parameter_index).num_combinations();
            const auto old_size = conditional_effects.size();
            conditional_effects.resize(old_size + num_conditional_effects);

            // Create binding and ground conditions and effect
            binding_ext.resize(binding_ext_size + universal_effect->get_arity());

            // The position to place the conditional precondition + effect
            auto j = old_size;
            assert(!objects_by_parameter_index.empty());
            for (const auto& combination : CartesianProduct(objects_by_parameter_index))
            {
                // Create binding
                for (size_t pos = 0; pos < universal_effect->get_arity(); ++pos)
                {
                    const auto object_id = *combination[pos];
                    binding_ext[binding_ext_size + pos] = m_pddl_factories->get_object(object_id);
                }

                auto cond_effect_proxy_j = ConditionalEffectBuilderProxy(conditional_effects[j]);
                auto& cond_positive_fluent_precondition_j = cond_effect_proxy_j.get_positive_precondition<Fluent>();
                auto& cond_negative_fluent_precondition_j = cond_effect_proxy_j.get_negative_precondition<Fluent>();
                auto& cond_positive_static_precondition_j = cond_effect_proxy_j.get_positive_precondition<Static>();
                auto& cond_negative_static_precondition_j = cond_effect_proxy_j.get_negative_precondition<Static>();
                auto& cond_positive_derived_precondition_j = cond_effect_proxy_j.get_positive_precondition<Derived>();
                auto& cond_negative_derived_precondition_j = cond_effect_proxy_j.get_negative_precondition<Derived>();
                auto& cond_simple_effect_j = cond_effect_proxy_j.get_simple_effect();
                cond_positive_fluent_precondition_j.clear();
                cond_negative_fluent_precondition_j.clear();
                cond_positive_static_precondition_j.clear();
                cond_negative_static_precondition_j.clear();
                cond_positive_derived_precondition_j.clear();
                cond_negative_derived_precondition_j.clear();
                m_pddl_factories->ground_and_fill_vector(universal_effect->get_conditions<Fluent>(),
                                                         cond_positive_fluent_precondition_j,
                                                         cond_negative_fluent_precondition_j,
                                                         binding_ext);
                m_pddl_factories->ground_and_fill_vector(universal_effect->get_conditions<Static>(),
                                                         cond_positive_static_precondition_j,
                                                         cond_negative_static_precondition_j,
                                                         binding_ext);
                m_pddl_factories->ground_and_fill_vector(universal_effect->get_conditions<Derived>(),
                                                         cond_positive_derived_precondition_j,
                                                         cond_negative_derived_precondition_j,
                                                         binding_ext);

                fill_effect(universal_effect->get_effect(), cond_simple_effect_j, binding_ext);

                ++j;
            }
        }
    }

    auto& flatmemory_builder = m_action_builder.get_flatmemory_builder();
    flatmemory_builder.finish();

    const auto [iter, inserted] = m_flat_actions.insert(flatmemory_builder);
    const auto grounded_action = GroundAction(*iter);
    if (inserted)
    {
        m_actions_by_index.push_back(grounded_action);
    }

    // Ensure that buffer is interpretable back to same data as builder
    assert(flatmemory_builder == *iter);

    /* 3. Insert to groundings table */

    groundings.emplace(std::move(binding), GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

void LiftedApplicableActionGenerator::generate_applicable_actions(State state, GroundActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_event_handler->on_start_generating_applicable_actions();

    // Create the assignment sets that are shared by all action schemas.

    auto& fluent_predicates = m_problem->get_domain()->get_predicates<Fluent>();
    auto fluent_atoms = m_pddl_factories->get_ground_atoms_from_ids<Fluent>(state.get_atoms<Fluent>());
    auto fluent_assignment_set = AssignmentSet<Fluent>(m_problem, fluent_predicates, fluent_atoms);

    auto& derived_predicates = m_problem->get_problem_and_domain_derived_predicates();
    auto derived_atoms = m_pddl_factories->get_ground_atoms_from_ids<Derived>(state.get_atoms<Derived>());
    auto derived_assignment_set = AssignmentSet<Derived>(m_problem, derived_predicates, derived_atoms);

    // Get all applicable ground actions.
    // This is done by getting bindings in the given state using the precondition.
    // These bindings are then used to ground the actual action schemas.

    std::vector<ObjectList> bindings;
    for (auto& [action, condition_grounder] : m_action_precondition_grounders)
    {
        condition_grounder.compute_bindings(state, fluent_assignment_set, derived_assignment_set, bindings);

        for (auto& binding : bindings)
        {
            out_applicable_actions.emplace_back(ground_action(action, std::move(binding)));
        }
    }

    m_event_handler->on_end_generating_applicable_actions(out_applicable_actions, *m_pddl_factories);
}

void LiftedApplicableActionGenerator::generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms,
                                                                FlatBitsetBuilder<Derived>& ref_derived_state_atoms)
{
    // In the lifted case, we use the axiom evaluator.
    m_axiom_evaluator.generate_and_apply_axioms(fluent_state_atoms, ref_derived_state_atoms);
}

void LiftedApplicableActionGenerator::on_finish_f_layer() const { m_event_handler->on_finish_f_layer(); }

void LiftedApplicableActionGenerator::on_end_search() const { m_event_handler->on_end_search(); }

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories) :
    LiftedApplicableActionGenerator(problem, std::move(pddl_factories), std::make_shared<DefaultLiftedAAGEventHandler>())
{
}

LiftedApplicableActionGenerator::LiftedApplicableActionGenerator(Problem problem,
                                                                 std::shared_ptr<PDDLFactories> pddl_factories,
                                                                 std::shared_ptr<ILiftedAAGEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_factories(std::move(pddl_factories)),
    m_event_handler(std::move(event_handler)),
    m_axiom_evaluator(problem, m_pddl_factories, m_event_handler),
    m_action_precondition_grounders(),
    m_action_universal_effects(),
    m_ground_function_value_costs()
{
    /* 1. Initialize ground function costs. */

    for (const auto numeric_fluent : problem->get_numeric_fluents())
    {
        m_ground_function_value_costs.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
    }

    /* 2. Initialize the condition grounders for each action schema. */

    auto static_initial_atoms = to_ground_atoms(m_problem->get_static_initial_literals());
    auto static_assignment_set = AssignmentSet<Static>(m_problem, m_problem->get_domain()->get_predicates<Static>(), static_initial_atoms);

    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        m_action_precondition_grounders.emplace(action,
                                                ConditionGrounder<State>(m_problem,
                                                                         action->get_parameters(),
                                                                         action->get_conditions<Static>(),
                                                                         action->get_conditions<Fluent>(),
                                                                         action->get_conditions<Derived>(),
                                                                         static_assignment_set,
                                                                         m_pddl_factories));
        auto universal_effects = std::vector<consistency_graph::StaticConsistencyGraph>();
        universal_effects.reserve(action->get_universal_effects().size());

        for (const auto& universal_effect : action->get_universal_effects())
        {
            universal_effects.push_back(consistency_graph::StaticConsistencyGraph(problem,
                                                                                  action->get_arity(),
                                                                                  action->get_arity() + universal_effect->get_arity(),
                                                                                  universal_effect->get_conditions<Static>(),
                                                                                  static_assignment_set));
        }

        m_action_universal_effects.emplace(action, std::move(universal_effects));
    }
}

const GroundAxiomList& LiftedApplicableActionGenerator::get_ground_axioms() const { return m_axiom_evaluator.get_ground_axioms(); }

const GroundActionList& LiftedApplicableActionGenerator::get_ground_actions() const { return m_actions_by_index; }

GroundAction LiftedApplicableActionGenerator::get_action(size_t action_id) const { return m_actions_by_index.at(action_id); }

Problem LiftedApplicableActionGenerator::get_problem() const { return m_problem; }

[[nodiscard]] const std::shared_ptr<PDDLFactories>& LiftedApplicableActionGenerator::get_pddl_factories() const { return m_pddl_factories; }

std::ostream& operator<<(std::ostream& out, const LiftedApplicableActionGenerator& lifted_aag)
{
    out << "Lifted AAG:" << std::endl;

    for (const auto& [action, grounder] : lifted_aag.m_action_precondition_grounders)
    {
        out << " - Action: " << action->get_name() << std::endl;
        out << grounder << std::endl;
    }

    return out;
}

}
