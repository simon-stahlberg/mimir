#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_

#include "../../formalism/problem/declarations.hpp"

#include <map>

namespace mimir
{

class ParameterIndexOrConstantId
{
private:
    size_t value;

public:
    ParameterIndexOrConstantId(size_t value, bool is_constant);

    bool is_constant() const;
    bool is_variable() const;
    size_t get_value() const;
};

class FlatLiteral
{
public:
    Literal source;
    std::vector<ParameterIndexOrConstantId> arguments;
    size_t predicate_id;
    size_t arity;
    bool negated;

    FlatLiteral(Literal literal, const std::map<Object, size_t>& parameter_indices);
};

class FlatActionSchema
{
private:
    std::map<Parameter, size_t> parameter_indices_;
    std::vector<Parameter> index_parameters_;

public:
    Action source;
    std::vector<FlatLiteral> static_precondition;
    std::vector<FlatLiteral> fluent_precondition;
    std::vector<FlatLiteral> unconditional_effect;
    std::vector<ParameterIndexOrConstantId> cost_arguments;
    size_t arity;

    FlatActionSchema(Domain domain, Action action_schema);

    const std::vector<Parameter>& get_parameters() const;
    size_t get_parameter_index(Parameter parameter) const;
};

}

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
