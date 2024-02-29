#include <algorithm>
#include <mimir/formalism/domain/action.hpp>
#include <mimir/formalism/domain/atom.hpp>
#include <mimir/formalism/domain/conditions.hpp>
#include <mimir/formalism/domain/domain.hpp>
#include <mimir/formalism/domain/effects.hpp>
#include <mimir/formalism/domain/literal.hpp>
#include <mimir/formalism/domain/parameter.hpp>
#include <mimir/formalism/domain/predicate.hpp>
#include <mimir/formalism/domain/term.hpp>
#include <mimir/search/applicable_action_generators/internal_representation.hpp>

namespace mimir
{

ParameterIndexOrConstantId::ParameterIndexOrConstantId(size_t value, bool is_constant) : value(is_constant ? ~value : value) {};

bool ParameterIndexOrConstantId::is_constant() const
{
    const size_t shift = sizeof(size_t) * 8 - 1;
    const size_t mask = 1UL << shift;
    return value & mask;
}

bool ParameterIndexOrConstantId::is_variable() const { return !is_constant(); }

size_t ParameterIndexOrConstantId::get_value() const { return is_constant() ? ~value : value; }

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

static void conjunctive_precondition(Condition precondition, LiteralList& ref_literals)
{
    if (const auto* precondition_literal = std::get_if<ConditionLiteralImpl>(precondition))
    {
        ref_literals.emplace_back(precondition_literal->get_literal());
    }
    else if (const auto* precondition_and = std::get_if<ConditionAndImpl>(precondition))
    {
        for (const auto& inner_precondition : precondition_and->get_conditions())
        {
            conjunctive_precondition(inner_precondition, ref_literals);
        }
    }
    else
    {
        throw std::runtime_error("only conjunctive preconditions are supported");
    }
}

static void conjunctive_effect(Effect effect, LiteralList& ref_literals)
{
    if (const auto* effect_literal = std::get_if<EffectLiteralImpl>(effect))
    {
        ref_literals.emplace_back(effect_literal->get_literal());
    }
    else if (const auto* effect_and = std::get_if<EffectAndImpl>(effect))
    {
        for (const auto& inner_effect : effect_and->get_effects())
        {
            conjunctive_effect(inner_effect, ref_literals);
        }
    }
    else
    {
        throw std::runtime_error("only conjunctive effects are supported");
    }
}

FlatAction::FlatAction(Domain domain, Action action_schema) :
    to_index_(),
    index_parameters_(),
    source(action_schema),
    static_precondition(),
    fluent_precondition(),
    unconditional_effect(),
    cost_arguments(),
    arity(static_cast<uint32_t>(action_schema->get_arity()))
{
    for (const auto& parameter : action_schema->get_parameters())
    {
        to_index_.emplace(parameter, static_cast<uint32_t>(to_index_.size()));
        to_parameter_.emplace(parameter->get_variable(), parameter);
        index_parameters_.emplace_back(parameter);
    }

    const auto& static_predicates = domain->get_static_predicates();
    const auto& precondition = action_schema->get_condition();
    const auto& effect = action_schema->get_effect();

    if (precondition.has_value())
    {
        LiteralList precondition_literals;
        conjunctive_precondition(precondition.value(), precondition_literals);

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
    }

    if (effect.has_value())
    {
        LiteralList effect_literals;
        conjunctive_effect(effect.value(), effect_literals);

        for (const auto& literal : effect_literals)
        {
            unconditional_effect.emplace_back(literal, to_index_, to_parameter_);
        }
    }
}

const std::vector<Parameter>& FlatAction::get_parameters() const { return index_parameters_; }

size_t FlatAction::get_parameter_index(Parameter parameter) const { return to_index_.at(parameter); }

}
