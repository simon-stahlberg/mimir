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


#include "action_schema.hpp"
#include "help_functions.hpp"

namespace formalism
{
    ActionSchemaImpl::ActionSchemaImpl(const std::string& name,
                                       const formalism::ParameterList& parameters,
                                       const formalism::LiteralList& precondition,
                                       const formalism::LiteralList& effect) :
        name(name),
        arity(parameters.size()),
        complete(true),
        parameters(parameters),
        precondition(precondition),
        effect(effect)
    {
    }

    ActionSchema create_action_schema(const std::string& name,
                                      const formalism::ParameterList& parameters,
                                      const formalism::LiteralList& precondition,
                                      const formalism::LiteralList& effect)
    {
        return std::make_shared<ActionSchemaImpl>(name, parameters, precondition, effect);
    }

    bool affects_predicate(const formalism::ActionSchema& action_schema, const formalism::Predicate& predicate)
    {
        return contains_predicate(action_schema->effect, predicate);
    }

    bool affect_predicate(const formalism::ActionSchemaList& action_schemas, const formalism::Predicate& predicate)
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

    std::ostream& operator<<(std::ostream& os, const formalism::ActionSchema& action_schema)
    {
        return os << action_schema->name << "/" << action_schema->arity;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ActionSchemaList& action_schemas)
    {
        print_vector(os, action_schemas);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::ActionSchema>::operator()(const formalism::ActionSchema& action_schema) const
    {
        return hash_combine(action_schema->name, action_schema->arity, action_schema->parameters, action_schema->precondition, action_schema->effect);
    }

    std::size_t hash<formalism::ActionSchemaList>::operator()(const formalism::ActionSchemaList& action_schemas) const { return hash_vector(action_schemas); }

    bool less<formalism::ActionSchema>::operator()(const formalism::ActionSchema& left_action_schema, const formalism::ActionSchema& right_action_schema) const
    {
        return less_combine(
            std::make_tuple(left_action_schema->name, left_action_schema->parameters, left_action_schema->precondition, left_action_schema->effect),
            std::make_tuple(right_action_schema->name, right_action_schema->parameters, right_action_schema->precondition, right_action_schema->effect));
    }

    bool equal_to<formalism::ActionSchema>::operator()(const formalism::ActionSchema& left_action_schema,
                                                       const formalism::ActionSchema& right_action_schema) const
    {
        return equal_to_combine(
            std::make_tuple(left_action_schema->name, left_action_schema->parameters, left_action_schema->precondition, left_action_schema->effect),
            std::make_tuple(right_action_schema->name, right_action_schema->parameters, right_action_schema->precondition, right_action_schema->effect));
    }
}  // namespace std
