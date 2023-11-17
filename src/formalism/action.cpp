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

#include "../../include/mimir/formalism/action.hpp"
#include "help_functions.hpp"

#include <algorithm>
#include <memory>
#include <vector>

namespace mimir::formalism
{
    mimir::formalism::ObjectList create_arguments(const mimir::formalism::ActionSchema& action_schema, const mimir::formalism::ParameterAssignment& assignment)
    {
        mimir::formalism::ObjectList arguments;

        for (const auto& parameter : action_schema->parameters)
        {
            arguments.emplace_back(assignment.at(parameter));
        }

        return arguments;
    }

    mimir::formalism::LiteralList create_precondition(const mimir::formalism::ActionSchema& action_schema, const mimir::formalism::ObjectList& arguments)
    {
        mimir::formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return mimir::formalism::ground_literal_list(action_schema->precondition, assignment);
    }

    mimir::formalism::LiteralList create_unconditional_effect(const mimir::formalism::ActionSchema& action_schema,
                                                              const mimir::formalism::ObjectList& arguments)
    {
        mimir::formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        return mimir::formalism::ground_literal_list(action_schema->unconditional_effect, assignment);
    }

    mimir::formalism::ImplicationList create_conditional_effect(const mimir::formalism::ActionSchema& action_schema,
                                                                const mimir::formalism::ObjectList& arguments)
    {
        mimir::formalism::ParameterAssignment assignment;

        for (uint32_t index = 0; index < arguments.size(); ++index)
        {
            const auto& parameter = action_schema->parameters.at(index);
            const auto& argument = arguments.at(index);
            assignment.insert(std::make_pair(parameter, argument));
        }

        mimir::formalism::ImplicationList implications;

        for (const auto& implication : action_schema->conditional_effect)
        {
            implications.emplace_back(mimir::formalism::ground_literal_list(implication.antecedent, assignment),
                                      mimir::formalism::ground_literal_list(implication.consequence, assignment));
        }

        return implications;
    }

    void convert_to_bitsets(const mimir::formalism::ProblemDescription& problem,
                            const mimir::formalism::LiteralList& literals,
                            mimir::formalism::Bitset& positive,
                            mimir::formalism::Bitset& negative)
    {
        for (const auto& literal : literals)
        {
            const auto& atom = literal->atom;
            const auto rank = problem->get_rank(atom);

            if (literal->negated)
            {
                negative.unset(rank);
            }
            else
            {
                positive.set(rank);
            }
        }
    }

    ActionImpl::ActionImpl(const mimir::formalism::ProblemDescription& problem,
                           const mimir::formalism::ActionSchema& schema,
                           mimir::formalism::ObjectList&& arguments,
                           mimir::formalism::LiteralList&& precondition,
                           mimir::formalism::LiteralList&& unconditional_effect,
                           mimir::formalism::ImplicationList&& conditional_effect,
                           double cost) :
        applicability_positive_precondition_bitset_(0, false),
        applicability_negative_precondition_bitset_(0, true),
        unconditional_positive_effect_bitset_(0, false),
        unconditional_negative_effect_bitset_(0, true),
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
            mimir::formalism::Bitset positive_precondition(0, false);
            mimir::formalism::Bitset negative_precondition(0, true);
            mimir::formalism::Bitset positive_effect(0, false);
            mimir::formalism::Bitset negative_effect(0, true);

            convert_to_bitsets(problem, antecedent, positive_precondition, negative_precondition);
            convert_to_bitsets(problem, consequence, positive_effect, negative_effect);

            conditional_positive_precondition_bitsets_.emplace_back(std::move(positive_precondition));
            conditional_negative_precondition_bitsets_.emplace_back(std::move(negative_precondition));
            conditional_positive_effect_bitsets_.emplace_back(std::move(positive_effect));
            conditional_negative_effect_bitsets_.emplace_back(std::move(negative_effect));
        }
    }

    ActionImpl::ActionImpl(const mimir::formalism::ProblemDescription& problem,
                           const mimir::formalism::ActionSchema& schema,
                           mimir::formalism::ObjectList&& arguments,
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

    ActionImpl::ActionImpl(const mimir::formalism::ProblemDescription& problem,
                           const mimir::formalism::ActionSchema& schema,
                           const mimir::formalism::ParameterAssignment& assignment) :
        ActionImpl(problem, schema, create_arguments(schema, assignment), 1)
    {
    }

    const mimir::formalism::ObjectList& ActionImpl::get_arguments() const { return arguments_; }

    const mimir::formalism::LiteralList& ActionImpl::get_precondition() const { return applicability_precondition_; }

    const mimir::formalism::LiteralList& ActionImpl::get_unconditional_effect() const { return unconditional_effect_; }

    const mimir::formalism::ImplicationList& ActionImpl::get_conditional_effect() const { return conditional_effect_; }

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         mimir::formalism::ObjectList&& arguments,
                         mimir::formalism::LiteralList&& precondition,
                         mimir::formalism::LiteralList&& unconditional_effect,
                         mimir::formalism::ImplicationList&& conditional_effect,
                         double cost)
    {
        return std::make_shared<mimir::formalism::ActionImpl>(problem,
                                                              schema,
                                                              std::move(arguments),
                                                              std::move(precondition),
                                                              std::move(unconditional_effect),
                                                              std::move(conditional_effect),
                                                              cost);
    }

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         mimir::formalism::ObjectList&& arguments,
                         double cost)
    {
        return std::make_shared<mimir::formalism::ActionImpl>(problem, schema, std::move(arguments), cost);
    }

    Action create_action(const mimir::formalism::ProblemDescription& problem,
                         const mimir::formalism::ActionSchema& schema,
                         const mimir::formalism::ParameterAssignment& assignment,
                         double cost)
    {
        return std::make_shared<mimir::formalism::ActionImpl>(problem, schema, assignment);
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionImpl& action)
    {
        os << action.schema->name;
        print_vector(os, action.get_arguments(), "(", ")", ", ");
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::Action& action) { return os << *action; }

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::Action>::operator()(const mimir::formalism::Action& action) const
    {
        return hash_combine(action->schema, action->get_arguments(), action->cost);
    }

    bool less<mimir::formalism::Action>::operator()(const mimir::formalism::Action& left_action, const mimir::formalism::Action& right_action) const
    {
        return less_combine(std::make_tuple(left_action->schema, left_action->get_arguments(), left_action->cost),
                            std::make_tuple(right_action->schema, right_action->get_arguments(), right_action->cost));
    }

    bool equal_to<mimir::formalism::Action>::operator()(const mimir::formalism::Action& left_action, const mimir::formalism::Action& right_action) const
    {
        return equal_to_combine(std::make_tuple(left_action->schema, left_action->get_arguments(), left_action->cost),
                                std::make_tuple(right_action->schema, right_action->get_arguments(), right_action->cost));
    }
}  // namespace std
