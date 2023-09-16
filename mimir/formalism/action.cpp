/*
 * Copyright (C) 2023 Simon Stahlberg
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


#include "action.hpp"
#include "help_functions.hpp"

#include <algorithm>
#include <memory>
#include <vector>

namespace formalism
{
    formalism::ObjectList create_arguments(const formalism::ActionSchema& action_schema, const formalism::ParameterAssignment& assignment)
    {
        formalism::ObjectList arguments;

        for (const auto& parameter : action_schema->parameters)
        {
            arguments.emplace_back(assignment.at(parameter));
        }

        return arguments;
    }

    formalism::LiteralList create_precondition(const formalism::ActionSchema& action_schema, const formalism::ObjectList& arguments)
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return formalism::ground_literal_list(action_schema->precondition, assignment);
    }

    formalism::LiteralList create_unconditional_effect(const formalism::ActionSchema& action_schema, const formalism::ObjectList& arguments)
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return formalism::ground_literal_list(action_schema->unconditional_effect, assignment);
    }

    formalism::ImplicationList create_conditional_effect(const formalism::ActionSchema& action_schema, const formalism::ObjectList& arguments)
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        formalism::ImplicationList implications;

        for (const auto& implication : action_schema->conditional_effect)
        {
            implications.emplace_back(formalism::ground_literal_list(implication.antecedent, assignment),
                                      formalism::ground_literal_list(implication.consequence, assignment));
        }

        return implications;
    }

    void convert_to_bitsets(const formalism::ProblemDescription& problem,
                            const formalism::LiteralList& literals,
                            formalism::Bitset& positive,
                            formalism::Bitset& negative)
    {
        for (const auto& literal : literals)
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative.set(rank);
            }
            else
            {
                positive.set(rank);
            }
        }
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           formalism::ObjectList&& arguments,
                           formalism::LiteralList&& precondition,
                           formalism::LiteralList&& unconditional_effect,
                           formalism::ImplicationList&& conditional_effect,
                           double cost) :
        applicability_positive_precondition_bitset_(0),
        applicability_negative_precondition_bitset_(0),
        unconditional_positive_effect_bitset_(0),
        unconditional_negative_effect_bitset_(0),
        conditional_positive_precondition_bitsets_(),
        conditional_negative_precondition_bitsets_(),
        conditional_positive_effect_bitsets_(),
        conditional_negative_effect_bitsets_(),
        arguments_(std::move(arguments)),
        applicability_precondition_(std::move(precondition)),
        unconditional_effect_(std::move(unconditional_effect)),
        conditional_effect_(std::move(conditional_effect)),
        problem(problem),
        schema(schema),
        cost(cost)
    {
        convert_to_bitsets(problem, applicability_precondition_, applicability_positive_precondition_bitset_, applicability_negative_precondition_bitset_);
        convert_to_bitsets(problem, unconditional_effect_, unconditional_positive_effect_bitset_, unconditional_negative_effect_bitset_);

        for (const auto& [antecedent, consequence] : conditional_effect_)
        {
            formalism::Bitset positive_precondition(0);
            formalism::Bitset negative_precondition(0);
            formalism::Bitset positive_effect(0);
            formalism::Bitset negative_effect(0);

            convert_to_bitsets(problem, antecedent, positive_precondition, negative_precondition);
            convert_to_bitsets(problem, consequence, positive_effect, negative_effect);

            conditional_positive_precondition_bitsets_.emplace_back(std::move(positive_precondition));
            conditional_negative_precondition_bitsets_.emplace_back(std::move(negative_precondition));
            conditional_positive_effect_bitsets_.emplace_back(std::move(positive_effect));
            conditional_negative_effect_bitsets_.emplace_back(std::move(negative_effect));
        }
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           formalism::ObjectList&& arguments,
                           int32_t cost) :
        ActionImpl(problem,
                   schema,
                   std::move(arguments),
                   create_precondition(schema, arguments),
                   create_unconditional_effect(schema, arguments),
                   create_conditional_effect(schema, arguments),
                   cost)
    {
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           const formalism::ParameterAssignment& assignment) :
        ActionImpl(problem, schema, create_arguments(schema, assignment), 1)
    {
    }

    const formalism::ObjectList& ActionImpl::get_arguments() const { return arguments_; }

    const formalism::LiteralList& ActionImpl::get_precondition() const { return applicability_precondition_; }

    const formalism::LiteralList& ActionImpl::get_unconditional_effect() const { return unconditional_effect_; }

    const formalism::ImplicationList& ActionImpl::get_conditional_effect() const { return conditional_effect_; }

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         formalism::ObjectList&& arguments,
                         formalism::LiteralList&& precondition,
                         formalism::LiteralList&& unconditional_effect,
                         formalism::ImplicationList&& conditional_effect,
                         double cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem,
                                                       schema,
                                                       std::move(arguments),
                                                       std::move(precondition),
                                                       std::move(unconditional_effect),
                                                       std::move(conditional_effect),
                                                       cost);
    }

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, formalism::ObjectList&& arguments, double cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, std::move(arguments), cost);
    }

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         const formalism::ParameterAssignment& assignment,
                         double cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, assignment);
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ActionImpl& action)
    {
        os << action.schema->name;
        print_vector(os, action.get_arguments(), "(", ")", ", ");
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::Action& action) { return os << *action; }

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Action>::operator()(const formalism::Action& action) const
    {
        return hash_combine(action->schema, action->get_arguments(), action->cost);
    }

    bool less<formalism::Action>::operator()(const formalism::Action& left_action, const formalism::Action& right_action) const
    {
        return less_combine(std::make_tuple(left_action->schema, left_action->get_arguments(), left_action->cost),
                            std::make_tuple(right_action->schema, right_action->get_arguments(), right_action->cost));
    }

    bool equal_to<formalism::Action>::operator()(const formalism::Action& left_action, const formalism::Action& right_action) const
    {
        return equal_to_combine(std::make_tuple(left_action->schema, left_action->get_arguments(), left_action->cost),
                                std::make_tuple(right_action->schema, right_action->get_arguments(), right_action->cost));
    }
}  // namespace std
