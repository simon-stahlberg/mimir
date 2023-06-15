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

#include <memory>
#include <vector>

namespace formalism
{
    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           const formalism::ObjectList& arguments,
                           const int32_t cost) :
        positive_precondition_ranks_(),
        negative_precondition_ranks_(),
        positive_effect_ranks_(),
        negative_effect_ranks_(),
        problem(problem),
        schema(schema),
        arguments(arguments),
        cost(cost)
    {
        for (const auto& literal : get_precondition())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_precondition_ranks_.emplace_back(rank);
            }
            else
            {
                positive_precondition_ranks_.emplace_back(rank);
            }
        }

        for (const auto& literal : get_effect())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_effect_ranks_.emplace_back(rank);
            }
            else
            {
                positive_effect_ranks_.emplace_back(rank);
            }
        }

        std::sort(positive_precondition_ranks_.begin(), positive_precondition_ranks_.end());
        std::sort(negative_precondition_ranks_.begin(), negative_precondition_ranks_.end());
        std::sort(positive_effect_ranks_.begin(), positive_effect_ranks_.end());
        std::sort(negative_effect_ranks_.begin(), negative_effect_ranks_.end());
    }

    ActionImpl::ActionImpl(const formalism::ProblemDescription& problem,
                           const formalism::ActionSchema& schema,
                           const formalism::ParameterAssignment& assignment) :
        positive_precondition_ranks_(),
        negative_precondition_ranks_(),
        positive_effect_ranks_(),
        negative_effect_ranks_(),
        problem(problem),
        schema(schema),
        arguments(),
        cost(1)
    {
        for (const auto& parameter : schema->parameters)
        {
            arguments.push_back(assignment.at(parameter));
        }

        for (const auto& literal : get_precondition())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_precondition_ranks_.emplace_back(rank);
            }
            else
            {
                positive_precondition_ranks_.emplace_back(rank);
            }
        }

        for (const auto& literal : get_effect())
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative_effect_ranks_.emplace_back(rank);
            }
            else
            {
                positive_effect_ranks_.emplace_back(rank);
            }
        }

        std::sort(positive_precondition_ranks_.begin(), positive_precondition_ranks_.end());
        std::sort(negative_precondition_ranks_.begin(), negative_precondition_ranks_.end());
        std::sort(positive_effect_ranks_.begin(), positive_effect_ranks_.end());
        std::sort(negative_effect_ranks_.begin(), negative_effect_ranks_.end());
    }

    formalism::LiteralList ActionImpl::get_precondition() const
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return formalism::ground_literal_list(schema->precondition, assignment);
    }

    formalism::LiteralList ActionImpl::get_effect() const
    {
        formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return formalism::ground_literal_list(schema->effect, assignment);
    }

    Action create_action(const formalism::ProblemDescription& problem,
                         const formalism::ActionSchema& schema,
                         const formalism::ObjectList& arguments,
                         const int32_t cost)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, arguments, cost);
    }

    Action create_action(const formalism::ProblemDescription& problem, const formalism::ActionSchema& schema, const formalism::ParameterAssignment& assignment)
    {
        return std::make_shared<formalism::ActionImpl>(problem, schema, assignment);
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ActionImpl& action)
    {
        os << action.schema->name;
        print_vector(os, action.arguments, "(", ")", ", ");
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::Action& action) { return os << *action; }

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::Action>::operator()(const formalism::Action& action) const
    {
        return hash_combine(action->schema, action->arguments, action->cost);
    }

    bool less<formalism::Action>::operator()(const formalism::Action& left_action, const formalism::Action& right_action) const
    {
        return less_combine(std::make_tuple(left_action->schema, left_action->arguments, left_action->cost),
                            std::make_tuple(right_action->schema, right_action->arguments, right_action->cost));
    }

    bool equal_to<formalism::Action>::operator()(const formalism::Action& left_action, const formalism::Action& right_action) const
    {
        return equal_to_combine(std::make_tuple(left_action->schema, left_action->arguments, left_action->cost),
                                std::make_tuple(right_action->schema, right_action->arguments, right_action->cost));
    }
}  // namespace std
