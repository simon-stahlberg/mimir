#include <mimir/formalism/domain/action.hpp>
#include <mimir/formalism/domain/atom.hpp>
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

FlatLiteral::FlatLiteral(Literal literal, const std::map<Object, size_t>& parameter_indices) :
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
        throw std::runtime_error("not implemented");
        // const auto is_constant = term->is_constant();
        //  const auto value = is_constant ? term->id : parameter_indices.at(term);
        //  arguments.push_back(ParameterIndexOrConstantId(value, is_constant));
    }
}

FlatActionSchema::FlatActionSchema(Domain domain, Action action_schema) :
    parameter_indices_(),
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
        parameter_indices_.emplace(parameter, static_cast<uint32_t>(parameter_indices_.size()));
        index_parameters_.emplace_back(parameter);
    }

    throw std::runtime_error("not implemented");

    // const std::unordered_set<Predicate> static_predicates(domain->static_predicates.begin(), domain->static_predicates.end());

    // for (const auto& literal : action_schema->precondition)
    // {
    //     if (static_predicates.find(literal->atom->predicate) != static_predicates.end())
    //     {
    //         static_precondition.emplace_back(literal, parameter_indices_);
    //     }
    //     else
    //     {
    //         fluent_precondition.emplace_back(literal, parameter_indices_);
    //     }
    // }

    // for (const auto& literal : action_schema->unconditional_effect)
    // {
    //     unconditional_effect.emplace_back(literal, parameter_indices_);
    // }

    // if (action_schema->cost->has_atom())
    // {
    //     const auto cost_atom = action_schema->cost->get_atom();
    //     cost_arguments.reserve(cost_atom->arguments.size());

    //     for (const auto& parameter : cost_atom->arguments)
    //     {
    //         const auto is_constant = parameter->is_constant();
    //         const auto value = is_constant ? parameter->id : parameter_indices_.at(parameter);
    //         cost_arguments.emplace_back(ParameterIndexOrConstantId(value, is_constant));
    //     }
    // }
}

const std::vector<Parameter>& FlatActionSchema::get_parameters() const { return index_parameters_; }

size_t FlatActionSchema::get_parameter_index(Parameter parameter) const { return parameter_indices_.at(parameter); }

}
