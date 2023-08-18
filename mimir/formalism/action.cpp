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

    formalism::LiteralList create_effect(const formalism::ActionSchema& action_schema, const formalism::ObjectList& arguments)
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return formalism::ground_literal_list(action_schema->effect, assignment);
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           formalism::ObjectList&& arguments,
                           formalism::LiteralList&& precondition,
                           formalism::LiteralList&& effect,
                           int32_t cost) :
        positive_precondition_bitset_(0),
        negative_precondition_bitset_(0),
        positive_effect_bitset_(0),
        negative_effect_bitset_(0),
        arguments_(std::move(arguments)),
        precondition_(std::move(precondition)),
        effect_(std::move(effect)),
        problem(problem),
        schema(schema),
        cost(cost)
    {
        for (const auto& literal : get_precondition())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_precondition_bitset_.set(rank);
            }
            else
            {
                positive_precondition_bitset_.set(rank);
            }
        }

        for (const auto& literal : get_effect())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_effect_bitset_.set(rank);
            }
            else
            {
                positive_effect_bitset_.set(rank);
            }
        }
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           formalism::ObjectList&& arguments,
                           int32_t cost) :
        ActionImpl(problem, schema, std::move(arguments), create_precondition(schema, arguments), create_effect(schema, arguments), cost)
    {
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           const formalism::ParameterAssignment& assignment) :
        ActionImpl(problem, schema, create_arguments(schema, assignment), 1)
    {
    }

    const formalism::ObjectList& ActionImpl::get_arguments() const { return arguments_; }

    const formalism::LiteralList& ActionImpl::get_precondition() const { return precondition_; }

    const formalism::LiteralList& ActionImpl::get_effect() const { return effect_; }

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         formalism::ObjectList&& arguments,
                         formalism::LiteralList&& precondition,
                         formalism::LiteralList&& effect,
                         int32_t cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, std::move(arguments), std::move(precondition), std::move(effect), cost);
    }

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, formalism::ObjectList&& arguments, int32_t cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, std::move(arguments), cost);
    }

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, const formalism::ParameterAssignment& assignment)
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
