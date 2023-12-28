#include "../../include/mimir/formalism/atom.hpp"
#include "../../include/mimir/formalism/domain.hpp"
#include "../../include/mimir/formalism/function.hpp"
#include "../../include/mimir/formalism/implication.hpp"
#include "../../include/mimir/formalism/literal.hpp"
#include "../../include/mimir/formalism/predicate.hpp"
#include "../../include/mimir/formalism/problem.hpp"
#include "../../include/mimir/generators/flat_action_schema.hpp"

#include <map>

namespace mimir::planners
{
    ParameterIndexOrConstantId::ParameterIndexOrConstantId(uint32_t value, bool is_constant) : value(is_constant ? ~value : value) {}

    bool ParameterIndexOrConstantId::is_constant() const { return value & 0x80000000; }

    bool ParameterIndexOrConstantId::is_variable() const { return !is_constant(); }

    uint32_t ParameterIndexOrConstantId::get_value() const { return is_constant() ? ~value : value; }

    FlatLiteral::FlatLiteral(const mimir::formalism::Literal& literal, const std::map<mimir::formalism::Term, uint32_t> parameter_indices) :
        source(literal),
        arguments(),
        predicate_id(literal.get_atom().get_predicate().get_id()),
        arity(literal.get_atom().get_predicate().get_arity()),
        negated(literal.is_negated())
    {
        arguments.reserve(literal.get_atom().get_terms().size());

        for (const auto& term : literal.get_atom().get_terms())
        {
            const auto is_constant = term.is_constant();
            const auto value = is_constant ? term.get_id() : parameter_indices.at(term);
            arguments.push_back(ParameterIndexOrConstantId(value, is_constant));
        }
    }

    FlatImplication::FlatImplication(std::vector<FlatLiteral>&& antecedent, std::vector<FlatLiteral>&& consequence) :
        antecedent(std::move(antecedent)),
        consequence(std::move(consequence))
    {
    }

    FlatActionSchema::FlatActionSchema(const mimir::formalism::Domain& domain, const mimir::formalism::ActionSchema& action_schema) :
        parameter_indices_(),
        index_parameters_(),
        source(action_schema),
        static_precondition(),
        fluent_precondition(),
        unconditional_effect(),
        conditional_effect(),
        cost_arguments(),
        arity(static_cast<uint32_t>(action_schema.get_arity()))
    {
        for (const auto& parameter : action_schema.get_parameters())
        {
            parameter_indices_.emplace(parameter, static_cast<uint32_t>(parameter_indices_.size()));
            index_parameters_.emplace_back(parameter);
        }

        const auto static_predicates_list = domain.get_static_predicates();
        const mimir::formalism::PredicateSet static_predicates(static_predicates_list.begin(), static_predicates_list.end());

        for (const auto& literal : action_schema.get_precondition())
        {
            if (static_predicates.find(literal.get_atom().get_predicate()) != static_predicates.end())
            {
                static_precondition.emplace_back(literal, parameter_indices_);
            }
            else
            {
                fluent_precondition.emplace_back(literal, parameter_indices_);
            }
        }

        for (const auto& literal : action_schema.get_effect())
        {
            unconditional_effect.emplace_back(literal, parameter_indices_);
        }

        for (const auto& [antecedent, consequence] : action_schema.get_conditional_effect())
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

        const auto cost = action_schema.get_cost();

        if (cost.is_defined())
        {
            throw std::runtime_error("not implemented");
            // const auto cost_atom = action_schema->cost->get_atom();
            // cost_arguments.reserve(cost_atom.get_terms().size());

            // for (const auto& parameter : cost_atom.get_terms())
            // {
            //     const auto is_constant = parameter->is_constant();
            //     const auto value = is_constant ? parameter->id : parameter_indices_.at(parameter);
            //     cost_arguments.emplace_back(ParameterIndexOrConstantId(value, is_constant));
            // }
        }
    }

    const std::vector<mimir::formalism::Term>& FlatActionSchema::get_parameters() const { return index_parameters_; }

    uint32_t FlatActionSchema::get_parameter_index(const mimir::formalism::Term& parameter) const { return parameter_indices_.at(parameter); }
}  // namespace planners
