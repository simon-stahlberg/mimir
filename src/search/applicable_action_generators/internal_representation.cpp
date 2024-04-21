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

#include "mimir/search/applicable_action_generators/internal_representation.hpp"

#include "mimir/common/translations.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effect.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/term.hpp"

#include <algorithm>

namespace mimir
{

/*
 * Helper functions
 */

size_t get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (first_position + 1)) + (second * (second_position + 1)) + (third * (first_object + 1)) + (fourth * (second_object + 1));
    return (size_t) rank;
}

size_t num_assignments(int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity) + (second * arity) + (third * num_objects) + (fourth * num_objects);
    return (size_t) (max + 1);
}

std::vector<std::vector<bool>> build_assignment_sets(Problem problem, const std::vector<size_t>& atom_identifiers, const PDDLFactories& factories)
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    std::vector<std::vector<bool>> assignment_sets;
    assignment_sets.resize(predicates.size());

    for (const auto& predicate : predicates)
    {
        auto& assignment_set = assignment_sets[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }

    for (const auto& identifier : atom_identifiers)
    {
        const auto& ground_atom = factories.get_ground_atom(identifier);
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = assignment_sets[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];
            assignment_set[get_assignment_position(first_position, first_object->get_identifier(), -1, -1, arity, num_objects)] = true;

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[get_assignment_position(second_position, second_object->get_identifier(), -1, -1, arity, num_objects)] = true;
                assignment_set[get_assignment_position(first_position,
                                                       first_object->get_identifier(),
                                                       second_position,
                                                       second_object->get_identifier(),
                                                       arity,
                                                       num_objects)] = true;
            }
        }
    }

    return assignment_sets;
}

bool literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                            const std::vector<FlatLiteral>& literals,
                            const Assignment& first_assignment,
                            const Assignment& second_assignment,
                            Problem problem)
{
    for (const auto& literal : literals)
    {
        int32_t first_position = -1;
        int32_t second_position = -1;
        int32_t first_object_id = -1;
        int32_t second_object_id = -1;
        bool empty_assignment = true;

        for (std::size_t index = 0; index < literal.arity; ++index)
        {
            const auto& term = literal.arguments[index];

            if (term.is_constant())
            {
                if (literal.arity <= 2)
                {
                    const auto term_id = term.get_value();

                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(term_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(term_id);
                    }

                    empty_assignment = false;
                }
            }
            else
            {
                const auto term_index = term.get_value();

                if (first_assignment.parameter_index == term_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(first_assignment.object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(first_assignment.object_id);
                        break;
                    }

                    empty_assignment = false;
                }
                else if (second_assignment.parameter_index == term_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(second_assignment.object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(second_assignment.object_id);
                        break;
                    }

                    empty_assignment = false;
                }
            }
        }

        if (!empty_assignment)
        {
            const auto& assignment_set = assignment_sets[literal.predicate_id];
            const auto assignment_rank = get_assignment_position(first_position,
                                                                 first_object_id,
                                                                 second_position,
                                                                 second_object_id,
                                                                 static_cast<int32_t>(literal.arity),
                                                                 static_cast<int32_t>(problem->get_objects().size()));

            const auto consistent_with_state = assignment_set[assignment_rank];

            if (!literal.negated && !consistent_with_state)
            {
                return false;
            }
            else if (literal.negated && consistent_with_state && ((literal.arity == 1) || ((literal.arity == 2) && (second_position >= 0))))
            {
                return false;
            }
        }
    }

    return true;
}

/*
 * Struct Assignment and AssignmentPair
 */

Assignment::Assignment(size_t parameter_index, size_t object_id) : parameter_index(parameter_index), object_id(object_id) {}

AssignmentPair::AssignmentPair(size_t first_position, const Assignment& first_assignment, size_t second_position, const Assignment& second_assignment) :
    first_position(first_position),
    second_position(second_position),
    first_assignment(first_assignment),
    second_assignment(second_assignment)
{
}

/*
 * Class ParameterIndexOrConstantId
 */

ParameterIndexOrConstantId::ParameterIndexOrConstantId(size_t value, bool is_constant) : value(is_constant ? ~value : value) {};

bool ParameterIndexOrConstantId::is_constant() const
{
    const size_t shift = sizeof(size_t) * 8 - 1;
    const size_t mask = 1UL << shift;
    return value & mask;
}

bool ParameterIndexOrConstantId::is_variable() const { return !is_constant(); }

size_t ParameterIndexOrConstantId::get_value() const { return is_constant() ? ~value : value; }

/*
 * Class FlatLiteral
 */

FlatLiteral::FlatLiteral(Literal literal, const std::map<Parameter, size_t>& to_index, const std::map<Variable, Parameter>& to_parameter) :
    source(literal),
    arguments(),
    predicate_id(literal->get_atom()->get_predicate()->get_identifier()),
    arity(literal->get_atom()->get_predicate()->get_arity()),
    negated(literal->is_negated())
{
    const auto literal_terms = literal->get_atom()->get_terms();

    arguments.reserve(literal_terms.size());

    for (const auto& term : literal_terms)
    {
        if (const auto* object = std::get_if<TermObjectImpl>(term))
        {
            const auto constant_identifier = object->get_identifier();
            arguments.push_back(ParameterIndexOrConstantId(constant_identifier, true));
        }
        else if (const auto* variable = std::get_if<TermVariableImpl>(term))
        {
            const auto parameter_index = to_index.at(to_parameter.at(variable->get_variable()));
            arguments.push_back(ParameterIndexOrConstantId(parameter_index, false));
        }
        else
        {
            throw std::runtime_error("internal error");
        }
    }
}

FlatAction::FlatAction(Domain domain, Action action_schema) :
    to_index_(),
    index_parameters_(),
    source(action_schema),
    static_precondition(),
    fluent_precondition(),
    unconditional_effect(),
    arity(static_cast<uint32_t>(action_schema->get_arity()))
{
    for (const auto& parameter : action_schema->get_parameters())
    {
        to_index_.emplace(parameter, static_cast<uint32_t>(to_index_.size()));
        to_parameter_.emplace(parameter->get_variable(), parameter);
        index_parameters_.emplace_back(parameter);
    }

    const auto& static_predicates = domain->get_static_predicates();
    const auto& precondition_literals = action_schema->get_conditions();
    const auto& effect = action_schema->get_effects();

    for (const auto& literal : precondition_literals)
    {
        const auto& literal_predicate = literal->get_atom()->get_predicate();

        if (std::find(static_predicates.begin(), static_predicates.end(), literal_predicate) != static_predicates.end())
        {
            static_precondition.emplace_back(literal, to_index_, to_parameter_);
        }
        else
        {
            fluent_precondition.emplace_back(literal, to_index_, to_parameter_);
        }
    }

    LiteralList effect_literals;
    to_literals(effect, effect_literals);

    for (const auto& literal : effect_literals)
    {
        unconditional_effect.emplace_back(literal, to_index_, to_parameter_);
    }
}

const std::vector<Parameter>& FlatAction::get_parameters() const { return index_parameters_; }

size_t FlatAction::get_parameter_index(Parameter parameter) const { return to_index_.at(parameter); }

}
