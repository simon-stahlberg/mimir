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


#include <mimir/common/hash.hpp>
#include <mimir/formalism/action_schema.hpp>
#include <mimir/formalism/function.hpp>
#include <mimir/formalism/literal.hpp>
#include <mimir/formalism/term.hpp>

#include <algorithm>
#include <deque>

namespace mimir::formalism
{
    ActionSchema::ActionSchema(loki::pddl::Action external_action) : external_(external_action) {}

    LiteralList ActionSchema::parse_conjunctive_literals(loki::pddl::Condition condition) const
    {
        throw std::runtime_error("not implemented");

        // LiteralList precondition;

        // std::deque<loki::pddl::Condition> conditions { condition };

        // while (conditions.size() > 0)
        // {
        //     const auto current_condition = conditions.front();
        //     conditions.pop_front();

        //     if (loki::pddl::ConditionLiteral literal = dynamic_cast<loki::pddl::ConditionLiteral>(current_condition))
        //     {
        //         precondition.emplace_back(Literal(literal->get_literal()));
        //     }
        //     if (loki::pddl::ConditionAnd conjunction = dynamic_cast<loki::pddl::ConditionAnd>(current_condition))
        //     {
        //         for (const auto& condition : conjunction->get_conditions())
        //         {
        //             conditions.emplace_back(condition);
        //         }
        //     }
        //     else
        //     {
        //         throw std::runtime_error("not implemented: we only support conjunction of literals");
        //     }
        // }

        // return precondition;
    }

    ActionSchema::ActionSchema() : external_(nullptr) {}

    uint32_t ActionSchema::get_id() const { return static_cast<uint32_t>(external_->get_identifier()); }

    const std::string& ActionSchema::get_name() const { return external_->get_name(); }

    std::size_t ActionSchema::get_arity() const { return external_->get_parameters().size(); }

    TermList ActionSchema::get_parameters() const
    {
        TermList parameters;

        for (const auto& parameter : external_->get_parameters())
        {
            parameters.emplace_back(Term(parameter->get_variable()));
        }

        return parameters;
    }

    LiteralList ActionSchema::get_precondition() const
    {
        auto external_precondition = external_->get_condition();

        if (external_precondition.has_value())
        {
            auto condition = external_precondition.value();
            return parse_conjunctive_literals(condition);
        }
        else
        {
            return LiteralList();
        }
    }

    LiteralList ActionSchema::get_effect() const { throw std::runtime_error("not implemented"); }

    ImplicationList ActionSchema::get_conditional_effect() const { throw std::runtime_error("not implemented"); }

    Function ActionSchema::get_cost() const { throw std::runtime_error("not implemented"); }

    bool ActionSchema::is_valid() const { return external_ != nullptr; }

    std::size_t ActionSchema::hash() const { throw std::runtime_error("not implemented"); }

    bool ActionSchema::operator<(const ActionSchema& other) const { return external_ < other.external_; }
    bool ActionSchema::operator>(const ActionSchema& other) const { return external_ > other.external_; }
    bool ActionSchema::operator==(const ActionSchema& other) const { return external_ == other.external_; }
    bool ActionSchema::operator!=(const ActionSchema& other) const { return !(*this == other); }
    bool ActionSchema::operator<=(const ActionSchema& other) const { return (*this < other) || (*this == other); }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ActionSchema& action_schema)
    {
        return os << action_schema.get_name() << "/" << action_schema.get_arity();
    }
}  // namespace mimir::formalism

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& action_schema) const { return action_schema.hash(); }

    std::size_t hash<mimir::formalism::ActionSchemaList>::operator()(const mimir::formalism::ActionSchemaList& action_schemas) const
    {
        return mimir::hash_container(action_schemas);
    }

    bool less<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& left_action_schema,
                                                          const mimir::formalism::ActionSchema& right_action_schema) const
    {
        return left_action_schema < right_action_schema;
    }

    bool equal_to<mimir::formalism::ActionSchema>::operator()(const mimir::formalism::ActionSchema& left_action_schema,
                                                              const mimir::formalism::ActionSchema& right_action_schema) const
    {
        return left_action_schema == right_action_schema;
    }
}  // namespace std
*/
