#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

using namespace mimir;
namespace py = pybind11;

struct WrappedTerm
{
    Term term;
};

struct WrappedCondition
{
    Condition condition;
};

struct WrappedEffect
{
    Effect effect;
};

struct WrappedFunctionExpression
{
    FunctionExpression function_expression;
};

std::vector<WrappedTerm> wrap_terms(const TermList& terms)
{
    std::vector<WrappedTerm> wrapped_terms;
    wrapped_terms.reserve(terms.size());
    for (const auto& term : terms)
    {
        wrapped_terms.push_back(WrappedTerm(term));
    }
    return wrapped_terms;
}

std::vector<WrappedCondition> wrap_conditions(const ConditionList& conditions)
{
    std::vector<WrappedCondition> wrapped_conditions;
    wrapped_conditions.reserve(conditions.size());
    for (const auto& condition : conditions)
    {
        wrapped_conditions.push_back(WrappedCondition(condition));
    }
    return wrapped_conditions;
}

std::vector<WrappedEffect> wrap_effects(const EffectList& effects)
{
    std::vector<WrappedEffect> wrapped_effects;
    wrapped_effects.reserve(effects.size());
    for (const auto& effect : effects)
    {
        wrapped_effects.push_back(WrappedEffect(effect));
    }
    return wrapped_effects;
}

std::vector<WrappedFunctionExpression> wrap_function_expressions(const FunctionExpressionList& function_expressions)
{
    std::vector<WrappedFunctionExpression> wrapped_function_expressions;
    wrapped_function_expressions.reserve(function_expressions.size());
    for (const auto& function_expression : function_expressions)
    {
        wrapped_function_expressions.push_back(WrappedFunctionExpression(function_expression));
    }
    return wrapped_function_expressions;
}

struct CastVisitor
{
    template<typename T>
    py::object operator()(const T& element) const
    {
        return py::cast(element);
    }
};

void init_formalism(py::module_& m_formalism)
{
    py::class_<ActionImpl>(m_formalism, "Action")  //
        .def("get_identifier", &ActionImpl::get_identifier)
        .def("get_arity", &ActionImpl::get_arity)
        .def("get_condition", &ActionImpl::get_condition)
        .def("get_effect", &ActionImpl::get_effect)
        .def("get_name", &ActionImpl::get_name)
        .def("get_parameters", &ActionImpl::get_parameters);

    py::class_<AtomImpl>(m_formalism, "Atom")  //
        .def("get_identifier", &AtomImpl::get_identifier)
        .def("get_predicate", &AtomImpl::get_predicate)
        .def("get_terms", [](const AtomImpl& atom) { return wrap_terms(atom.get_terms()); });

    py::class_<ConditionLiteralImpl>(m_formalism, "ConditionLiteral")  //
        .def("get_identifier", &ConditionLiteralImpl::get_identifier)
        .def("get_literal", &ConditionLiteralImpl::get_literal);

    py::class_<ConditionAndImpl>(m_formalism, "ConditionAnd")  //
        .def("get_identifier", &ConditionAndImpl::get_identifier)
        .def("get_conditions", [](const ConditionAndImpl& condition) { return wrap_conditions(condition.get_conditions()); });

    py::class_<ConditionOrImpl>(m_formalism, "ConditionOr")  //
        .def("get_identifier", &ConditionOrImpl::get_identifier)
        .def("get_conditions", [](const ConditionOrImpl& condition) { return wrap_conditions(condition.get_conditions()); });

    py::class_<ConditionNotImpl>(m_formalism, "ConditionNot")  //
        .def("get_identifier", &ConditionNotImpl::get_identifier)
        .def("get_condition", [](const ConditionNotImpl& condition) { return WrappedCondition(condition.get_condition()); });

    py::class_<ConditionImplyImpl>(m_formalism, "ConditionImply")  //
        .def("get_identifier", &ConditionImplyImpl::get_identifier)
        .def("get_condition_left", [](const ConditionImplyImpl& condition) { return WrappedCondition(condition.get_condition_left()); })
        .def("get_condition_right", [](const ConditionImplyImpl& condition) { return WrappedCondition(condition.get_condition_right()); });

    py::class_<ConditionExistsImpl>(m_formalism, "ConditionExists")  //
        .def("get_identifier", &ConditionExistsImpl::get_identifier)
        .def("get_parameters", &ConditionExistsImpl::get_parameters)
        .def("get_condition", [](const ConditionExistsImpl& condition) { return WrappedCondition(condition.get_condition()); });

    py::class_<ConditionForallImpl>(m_formalism, "ConditionForall")  //
        .def("get_identifier", &ConditionForallImpl::get_identifier)
        .def("get_parameters", &ConditionForallImpl::get_parameters)
        .def("get_condition", [](const ConditionForallImpl& condition) { return WrappedCondition(condition.get_condition()); });

    py::class_<ConditionImpl>(m_formalism, "Condition")  //
        .def("get", [](const WrappedCondition& wrappedCondition) -> py::object { return std::visit(CastVisitor(), *wrappedCondition.condition); });
    ;

    py::class_<DomainImpl>(m_formalism, "Domain")  //
        .def("get_identifier", &DomainImpl::get_identifier)
        .def("get_name", &DomainImpl::get_name)
        .def("get_types", &DomainImpl::get_types)
        .def("get_constants", &DomainImpl::get_constants)
        .def("get_predicates", &DomainImpl::get_predicates)
        .def("get_fluent_predicates", &DomainImpl::get_fluent_predicates)
        .def("get_static_predicates", &DomainImpl::get_static_predicates)
        .def("get_functions", &DomainImpl::get_functions)
        .def("get_actions", &DomainImpl::get_actions)
        .def("get_requirements", &DomainImpl::get_requirements);

    py::class_<EffectLiteralImpl>(m_formalism, "EffectLiteral")  //
        .def("get_identifier", &EffectLiteralImpl::get_identifier)
        .def("get_literal", &EffectLiteralImpl::get_literal);

    py::class_<EffectAndImpl>(m_formalism, "EffectAnd")  //
        .def("get_identifier", &EffectAndImpl::get_identifier)
        .def("get_effects", [](const EffectAndImpl& effect) { return wrap_effects(effect.get_effects()); });

    py::class_<EffectNumericImpl>(m_formalism, "EffectNumeric")  //
        .def("get_identifier", &EffectNumericImpl::get_identifier)
        .def("get_assign_operator", &EffectNumericImpl::get_assign_operator)
        .def("get_function", &EffectNumericImpl::get_function)
        .def("get_function_expression", [](const EffectNumericImpl& effect) { return WrappedFunctionExpression(effect.get_function_expression()); });

    py::class_<EffectConditionalForallImpl>(m_formalism, "EffectConditionalForall")  //
        .def("get_identifier", &EffectConditionalForallImpl::get_identifier)
        .def("get_effect", [](const EffectConditionalForallImpl& effect) { return WrappedEffect(effect.get_effect()); })
        .def("get_parameters", &EffectConditionalForallImpl::get_parameters);

    py::class_<EffectConditionalWhenImpl>(m_formalism, "EffectConditionalWhen")  //
        .def("get_identifier", &EffectConditionalWhenImpl::get_identifier)
        .def("get_condition", [](const EffectConditionalWhenImpl& effect) { return WrappedCondition(effect.get_condition()); })
        .def("get_effect", [](const EffectConditionalWhenImpl& effect) { return WrappedEffect(effect.get_effect()); });
    ;

    py::class_<EffectImpl>(m_formalism, "Effect")  //
        .def("get", [](const WrappedEffect& wrappedEffect) -> py::object { return std::visit(CastVisitor(), *wrappedEffect.effect); });

    py::class_<FunctionExpressionNumberImpl>(m_formalism, "FunctionExpressionNumber")  //
        .def("get_identifier", &FunctionExpressionNumberImpl::get_identifier)
        .def("get_number", &FunctionExpressionNumberImpl::get_number);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m_formalism, "FunctionExpressionBinaryOperator")  //
        .def("get_identifier", &FunctionExpressionBinaryOperatorImpl::get_identifier)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def("get_left_function_expression",
             [](const FunctionExpressionBinaryOperatorImpl& function_expression)
             { return WrappedFunctionExpression(function_expression.get_left_function_expression()); })
        .def("get_right_function_expression",
             [](const FunctionExpressionBinaryOperatorImpl& function_expression)
             { return WrappedFunctionExpression(function_expression.get_right_function_expression()); });

    py::class_<FunctionExpressionMultiOperatorImpl>(m_formalism, "FunctionExpressionMultiOperator")  //
        .def("get_identifier", &FunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def("get_function_expressions",
             [](const FunctionExpressionMultiOperatorImpl& function_expression)
             { return wrap_function_expressions(function_expression.get_function_expressions()); });

    py::class_<FunctionExpressionMinusImpl>(m_formalism, "FunctionExpressionMinus")  //
        .def("get_identifier", &FunctionExpressionMinusImpl::get_identifier)
        .def("get_function_expression",
             [](const FunctionExpressionMinusImpl& function_expression) { return WrappedFunctionExpression(function_expression.get_function_expression()); });
    ;

    py::class_<FunctionExpressionFunctionImpl>(m_formalism, "FunctionExpressionFunction")  //
        .def("get_identifier", &FunctionExpressionFunctionImpl::get_identifier);

    py::class_<FunctionExpressionImpl>(m_formalism, "FunctionExpression")  //
        .def("get",
             [](const WrappedFunctionExpression& wrappedFunctionExpression) -> py::object
             { return std::visit(CastVisitor(), *wrappedFunctionExpression.function_expression); });
    ;

    py::class_<FunctionSkeletonImpl>(m_formalism, "FunctionSkeleton")  //
        .def("get_identifier", &FunctionSkeletonImpl::get_identifier)
        .def("get_name", &FunctionSkeletonImpl::get_name)
        .def("get_parameters", &FunctionSkeletonImpl::get_parameters)
        .def("get_type", &FunctionSkeletonImpl::get_type);

    py::class_<FunctionImpl>(m_formalism, "Function")  //
        .def("get_identifier", &FunctionImpl::get_identifier)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton)
        .def("get_terms", [](const FunctionImpl& function) { return wrap_terms(function.get_terms()); });

    py::class_<GroundAtomImpl>(m_formalism, "GroundAtom")  //
        .def("get_identifier", &GroundAtomImpl::get_identifier)
        .def("get_arity", &GroundAtomImpl::get_arity)
        .def("get_predicate", &GroundAtomImpl::get_predicate)
        .def("get_objects", &GroundAtomImpl::get_objects);

    py::class_<GroundLiteralImpl>(m_formalism, "GroundLiteral")  //
        .def("get_identifier", &GroundLiteralImpl::get_identifier)
        .def("get_atom", &GroundLiteralImpl::get_atom)
        .def("is_negated", &GroundLiteralImpl::is_negated);

    py::class_<LiteralImpl>(m_formalism, "Literal")  //
        .def("get_identifier", &LiteralImpl::get_identifier)
        .def("get_atom", &LiteralImpl::get_atom)
        .def("is_negated", &LiteralImpl::is_negated);

    py::class_<OptimizationMetricImpl>(m_formalism, "OptimizationMetric")  //
        .def("get_identifier", &OptimizationMetricImpl::get_identifier)
        .def("get_function_expression", [](const OptimizationMetricImpl& metric) { return WrappedFunctionExpression(metric.get_function_expression()); })
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric);

    py::class_<NumericFluentImpl>(m_formalism, "NumericFluent")  //
        .def("get_identifier", &NumericFluentImpl::get_identifier)
        .def("get_function", &NumericFluentImpl::get_function)
        .def("get_number", &NumericFluentImpl::get_number);

    py::class_<ObjectImpl>(m_formalism, "Object")  //
        .def("get_identifier", &ObjectImpl::get_identifier)
        .def("get_name", &ObjectImpl::get_name)
        .def("get_bases", &ObjectImpl::get_bases);

    py::class_<ParameterImpl>(m_formalism, "Parameter")  //
        .def("get_identifier", &ParameterImpl::get_identifier)
        .def("get_variable", &ParameterImpl::get_variable)
        .def("get_bases", &ParameterImpl::get_bases);

    py::class_<PDDLFactories>(m_formalism, "PDDLFactories");

    py::class_<PDDLParser>(m_formalism, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain)
        .def("get_problem", &PDDLParser::get_problem)
        .def("get_factories", &PDDLParser::get_factories);

    py::class_<PredicateImpl>(m_formalism, "Predicate")  //
        .def("get_identifier", &PredicateImpl::get_identifier)
        .def("get_name", &PredicateImpl::get_name)
        .def("get_parameters", &PredicateImpl::get_parameters);

    py::class_<ProblemImpl>(m_formalism, "Problem")  //
        .def("get_identifier", &ProblemImpl::get_identifier)
        .def("get_name", &ProblemImpl::get_name)
        .def("get_domain", &ProblemImpl::get_domain)
        .def("get_requirements", &ProblemImpl::get_requirements)
        .def("get_objects", &ProblemImpl::get_objects)
        .def("get_initial_literals", &ProblemImpl::get_initial_literals)
        .def("get_numeric_fluents", &ProblemImpl::get_numeric_fluents)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric)
        .def("get_goal_condition", &ProblemImpl::get_goal_condition);

    py::class_<RequirementsImpl>(m_formalism, "Requirements")  //
        .def("get_identifier", &RequirementsImpl::get_identifier)
        .def("get_requirements", &RequirementsImpl::get_requirements);

    py::class_<TermObjectImpl>(m_formalism, "TermObject")  //
        .def("get_identifier", &TermObjectImpl::get_identifier)
        .def("get_object", &TermObjectImpl::get_object);

    py::class_<TermVariableImpl>(m_formalism, "TermVariable")  //
        .def("get_identifier", &TermVariableImpl::get_identifier)
        .def("get_variable", &TermVariableImpl::get_variable);

    py::class_<WrappedTerm>(m_formalism, "Term")  //
        .def("get", [](const WrappedTerm& wrappedTerm) -> py::object { return std::visit(CastVisitor(), *wrappedTerm.term); });

    py::class_<TypeImpl>(m_formalism, "Type")  //
        .def("get_identifier", &TypeImpl::get_identifier)
        .def("get_name", &TypeImpl::get_name)
        .def("get_bases", &TypeImpl::get_bases);

    py::class_<VariableImpl>(m_formalism, "Variable")  //
        .def("get_identifier", &VariableImpl::get_identifier)
        .def("get_name", &VariableImpl::get_name);
}
