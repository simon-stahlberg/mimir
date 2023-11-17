#include "../../include/mimir/generators/flat_action_schema.hpp"

#include <map>

namespace mimir::planners
{
    ParameterIndexOrConstantId::ParameterIndexOrConstantId(uint32_t value, bool is_constant) : value(is_constant ? ~value : value) {}

    bool ParameterIndexOrConstantId::is_constant() const { return value & 0x80000000; }

    bool ParameterIndexOrConstantId::is_variable() const { return !is_constant(); }

    uint32_t ParameterIndexOrConstantId::get_value() const { return is_constant() ? ~value : value; }

    FlatLiteral::FlatLiteral(const mimir::formalism::Literal& literal, const std::map<mimir::formalism::Object, uint32_t> parameter_indices) :
        source(literal),
        arguments(),
        predicate_id(literal->atom->predicate->id),
        arity(literal->atom->predicate->arity),
        negated(literal->negated)
    {
        arguments.reserve(literal->atom->arguments.size());

        for (const auto& obj : literal->atom->arguments)
        {
            const auto is_constant = obj->is_constant();
            const auto value = is_constant ? obj->id : parameter_indices.at(obj);
            arguments.push_back(ParameterIndexOrConstantId(value, is_constant));
        }
    }

    FlatImplication::FlatImplication(std::vector<FlatLiteral>&& antecedent, std::vector<FlatLiteral>&& consequence) :
        antecedent(std::move(antecedent)),
        consequence(std::move(consequence))
    {
    }

    FlatActionSchema::FlatActionSchema(const mimir::formalism::DomainDescription& domain, const mimir::formalism::ActionSchema& action_schema) :
        parameter_indices_(),
        index_parameters_(),
        source(action_schema),
        static_precondition(),
        fluent_precondition(),
        unconditional_effect(),
        conditional_effect(),
        cost_arguments(),
        arity(static_cast<uint32_t>(action_schema->arity))
    {
        for (const auto& parameter : action_schema->parameters)
        {
            parameter_indices_.emplace(parameter, static_cast<uint32_t>(parameter_indices_.size()));
            index_parameters_.emplace_back(parameter);
        }

        const mimir::formalism::PredicateSet static_predicates(domain->static_predicates.begin(), domain->static_predicates.end());

        for (const auto& literal : action_schema->precondition)
        {
            if (static_predicates.find(literal->atom->predicate) != static_predicates.end())
            {
                static_precondition.emplace_back(literal, parameter_indices_);
            }
            else
            {
                fluent_precondition.emplace_back(literal, parameter_indices_);
            }
        }

        for (const auto& literal : action_schema->unconditional_effect)
        {
            unconditional_effect.emplace_back(literal, parameter_indices_);
        }

        for (const auto& [antecedent, consequence] : action_schema->conditional_effect)
        {
            std::vector<FlatLiteral> flat_antecedent;
            std::vector<FlatLiteral> flat_consequence;

            for (const auto& literal : antecedent)
            {
                flat_antecedent.emplace_back(literal, parameter_indices_);
            }

            for (const auto& literal : consequence)
            {
                flat_consequence.emplace_back(literal, parameter_indices_);
            }

            conditional_effect.emplace_back(std::move(flat_antecedent), std::move(flat_consequence));
        }

        if (action_schema->cost->has_atom())
        {
            const auto cost_atom = action_schema->cost->get_atom();
            cost_arguments.reserve(cost_atom->arguments.size());

            for (const auto& parameter : cost_atom->arguments)
            {
                const auto is_constant = parameter->is_constant();
                const auto value = is_constant ? parameter->id : parameter_indices_.at(parameter);
                cost_arguments.emplace_back(ParameterIndexOrConstantId(value, is_constant));
            }
        }
    }

    const std::vector<mimir::formalism::Object>& FlatActionSchema::get_parameters() const { return index_parameters_; }

    uint32_t FlatActionSchema::get_parameter_index(const mimir::formalism::Object& parameter) const { return parameter_indices_.at(parameter); }
}  // namespace planners
