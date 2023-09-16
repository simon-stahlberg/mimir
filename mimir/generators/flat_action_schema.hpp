#if !defined(PLANNERS_ACTION_SCHEMA_FLAT_REPRESENTATION_HPP_)
#define PLANNERS_ACTION_SCHEMA_FLAT_REPRESENTATION_HPP_

#include "../formalism/action.hpp"
#include "../formalism/action_schema.hpp"

#include <vector>

namespace planners
{
    class ParameterIndexOrConstantId
    {
      private:
        uint32_t value;

      public:
        ParameterIndexOrConstantId(uint32_t value, bool is_constant);

        bool is_constant() const;

        bool is_variable() const;

        uint32_t get_value() const;
    };

    class FlatLiteral
    {
      public:
        formalism::Literal source;
        std::vector<ParameterIndexOrConstantId> arguments;
        uint32_t predicate_id;
        uint32_t arity;
        bool negated;

        FlatLiteral(const formalism::Literal& literal, const std::map<formalism::Object, uint32_t> parameter_indices);
    };

    class FlatImplication
    {
      public:
        std::vector<FlatLiteral> antecedent;
        std::vector<FlatLiteral> consequence;

        FlatImplication(std::vector<FlatLiteral>&& antecedent, std::vector<FlatLiteral>&& consequence);
    };

    class FlatActionSchema
    {
      private:
        std::map<formalism::Object, uint32_t> parameter_indices_;
        std::vector<formalism::Object> index_parameters_;

      public:
        formalism::ActionSchema source;
        std::vector<FlatLiteral> static_precondition;
        std::vector<FlatLiteral> fluent_precondition;
        std::vector<FlatLiteral> unconditional_effect;
        std::vector<FlatImplication> conditional_effect;
        std::vector<ParameterIndexOrConstantId> cost_arguments;
        uint32_t arity;

        FlatActionSchema(const formalism::DomainDescription& domain, const formalism::ActionSchema& action_schema);

        const std::vector<formalism::Object>& get_parameters() const;

        uint32_t get_parameter_index(const formalism::Object& parameter) const;
    };
}  // namespace planners

#endif  // PLANNERS_ACTION_SCHEMA_FLAT_REPRESENTATION_HPP_
