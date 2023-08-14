#include "flat_action_schema.hpp"

#include <map>

namespace planners
{
    ParameterIndexOrConstantId::ParameterIndexOrConstantId(uint32_t value, bool is_constant) : value(is_constant ? ~value : value) {}

    bool ParameterIndexOrConstantId::is_constant() const { return value & 0x80000000; }

    bool ParameterIndexOrConstantId::is_variable() const { return !is_constant(); }

    uint32_t ParameterIndexOrConstantId::get_value() const { return is_constant() ? ~value : value; }

    FlatLiteral::FlatLiteral(const formalism::Literal& literal, const std::map<formalism::Object, uint32_t> parameter_indices) :
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

    FlatActionSchema::FlatActionSchema(const formalism::DomainDescription& domain, const formalism::ActionSchema& action_schema) :
        parameter_indices_(),
        index_parameters_(),
        source(action_schema),
        static_precondition(),
        fluent_precondition(),
        effect(),
        arity(static_cast<uint32_t>(action_schema->arity))
    {
        for (const auto& parameter : action_schema->parameters)
        {
            parameter_indices_.emplace(parameter, static_cast<uint32_t>(parameter_indices_.size()));
            index_parameters_.emplace_back(parameter);
        }

        const formalism::PredicateSet static_predicates(domain->static_predicates.begin(), domain->static_predicates.end());

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

        for (const auto& literal : action_schema->effect)
        {
            effect.emplace_back(literal, parameter_indices_);
        }
    }

    const std::vector<formalism::Object>& FlatActionSchema::get_parameters() const { return index_parameters_; }

    uint32_t FlatActionSchema::get_parameter_index(const formalism::Object& parameter) const { return parameter_indices_.at(parameter); }
}  // namespace planners
