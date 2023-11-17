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

#include "../../include/mimir/formalism/action_schema.hpp"
#include "help_functions.hpp"

#include <algorithm>

namespace mimir::formalism
{
    ActionSchemaImpl::ActionSchemaImpl(const std::string& name,
                                       const mimir::formalism::ParameterList& parameters,
                                       const mimir::formalism::LiteralList& precondition,
                                       const mimir::formalism::LiteralList& unconditional_effect,
                                       const mimir::formalism::ImplicationList& conditional_effect,
                                       const mimir::formalism::Function& cost) :
        name(name),
        arity(parameters.size()),
        complete(true),
        parameters(parameters),
        precondition(precondition),
        unconditional_effect(unconditional_effect),
        conditional_effect(conditional_effect),
        cost(cost)
    {
    }

    ActionSchema create_action_schema(const std::string& name,
                                      const mimir::formalism::ParameterList& parameters,
                                      const mimir::formalism::LiteralList& precondition,
                                      const mimir::formalism::LiteralList& unconditional_effect,
                                      const mimir::formalism::ImplicationList& conditional_effect,
                                      const mimir::formalism::Function& cost)
    {
        return std::make_shared<ActionSchemaImpl>(name, parameters, precondition, unconditional_effect, conditional_effect, cost);
    }

    ActionSchema relax(const mimir::formalism::ActionSchema& action_schema, bool remove_negative_preconditions, bool remove_delete_list)
    {
        std::vector<mimir::formalism::Literal> relaxed_precondition;

        const auto positive_literal = [](const mimir::formalism::Literal& literal) { return !literal->negated; };

        if (remove_negative_preconditions)
        {
            std::copy_if(action_schema->precondition.cbegin(), action_schema->precondition.cend(), std::back_inserter(relaxed_precondition), positive_literal);
        }
        else
        {
            relaxed_precondition.insert(relaxed_precondition.end(), action_schema->precondition.cbegin(), action_schema->precondition.cend());
        }

        std::vector<mimir::formalism::Literal> relaxed_effect;

        if (remove_delete_list)
        {
            std::copy_if(action_schema->unconditional_effect.cbegin(),
                         action_schema->unconditional_effect.cend(),
                         std::back_inserter(relaxed_effect),
                         positive_literal);

            for (const auto& [_, consequence] : action_schema->conditional_effect)
            {
                std::copy_if(consequence.cbegin(), consequence.cend(), std::back_inserter(relaxed_effect), positive_literal);
            }
        }
        else
        {
            relaxed_effect.insert(relaxed_effect.end(), action_schema->unconditional_effect.cbegin(), action_schema->unconditional_effect.cend());
        }

        return create_action_schema(action_schema->name, action_schema->parameters, relaxed_precondition, relaxed_effect, {}, action_schema->cost);
    }

    bool affects_predicate(const mimir::formalism::ActionSchema& action_schema, const mimir::formalism::Predicate& predicate)
    {
        if (contains_predicate(action_schema->unconditional_effect, predicate))
        {
            return true;
        }

        for (const auto& [_, consequence] : action_schema->conditional_effect)
        {
            if (contains_predicate(consequence, predicate))
            {
                return true;
            }
        }

        return false;
    }

    bool affect_predicate(const mimir::formalism::ActionSchemaList& action_schemas, const mimir::formalism::Predicate& predicate)
    {
        for (const auto& action_schema : action_schemas)
        {
            if (affects_predicate(action_schema, predicate))
            {
                return true;
            }
        }

        return false;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchema& action_schema)
    {
        return os << action_schema->name << "/" << action_schema->arity;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchemaList& action_schemas)
    {
        print_vector(os, action_schemas);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& action_schema) const
    {
        return hash_combine(action_schema->name,
                            action_schema->arity,
                            action_schema->parameters,
                            action_schema->precondition,
                            action_schema->unconditional_effect,
                            action_schema->conditional_effect,
                            action_schema->cost);
    }

    std::size_t hash<mimir::formalism::ActionSchemaList>::operator()(const mimir::formalism::ActionSchemaList& action_schemas) const
    {
        return hash_vector(action_schemas);
    }

    bool less<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& left_action_schema,
                                                          const mimir::formalism::ActionSchema& right_action_schema) const
    {
        return less_combine(std::make_tuple(left_action_schema->name,
                                            left_action_schema->parameters,
                                            left_action_schema->precondition,
                                            left_action_schema->unconditional_effect,
                                            left_action_schema->conditional_effect,
                                            left_action_schema->cost),
                            std::make_tuple(right_action_schema->name,
                                            right_action_schema->parameters,
                                            right_action_schema->precondition,
                                            right_action_schema->unconditional_effect,
                                            right_action_schema->conditional_effect,
                                            right_action_schema->cost));
    }

    bool equal_to<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& left_action_schema,
                                                              const mimir::formalism::ActionSchema& right_action_schema) const
    {
        return equal_to_combine(std::make_tuple(left_action_schema->name,
                                                left_action_schema->parameters,
                                                left_action_schema->precondition,
                                                left_action_schema->unconditional_effect,
                                                left_action_schema->conditional_effect,
                                                left_action_schema->cost),
                                std::make_tuple(right_action_schema->name,
                                                right_action_schema->parameters,
                                                right_action_schema->precondition,
                                                right_action_schema->unconditional_effect,
                                                right_action_schema->conditional_effect,
                                                right_action_schema->cost));
    }
}  // namespace std
