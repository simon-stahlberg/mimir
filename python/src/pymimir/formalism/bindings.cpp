#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

using namespace mimir;
namespace py = pybind11;

/**
 * Wrap variants to not expose them to the bindings.
 */

struct WrappedTerm
{
    Term term;
    explicit WrappedTerm(const Term& t) : term(t) {}
};

struct WrappedFunctionExpression
{
    FunctionExpression function_expression;
    explicit WrappedFunctionExpression(const FunctionExpression& e) : function_expression(e) {}
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

/**
 * Bindings
 */
void init_formalism(py::module_& m_formalism)
{
    py::enum_<loki::RequirementEnum>(m_formalism, "RequirementEnum")
        .value("STRIPS", loki::RequirementEnum::STRIPS)
        .value("TYPING", loki::RequirementEnum::TYPING)
        .value("NEGATIVE_PRECONDITIONS", loki::RequirementEnum::NEGATIVE_PRECONDITIONS)
        .value("DISJUNCTIVE_PRECONDITIONS", loki::RequirementEnum::DISJUNCTIVE_PRECONDITIONS)
        .value("EQUALITY", loki::RequirementEnum::EQUALITY)
        .value("EXISTENTIAL_PRECONDITIONS", loki::RequirementEnum::EXISTENTIAL_PRECONDITIONS)
        .value("UNIVERSAL_PRECONDITIONS", loki::RequirementEnum::UNIVERSAL_PRECONDITIONS)
        .value("QUANTIFIED_PRECONDITIONS", loki::RequirementEnum::QUANTIFIED_PRECONDITIONS)
        .value("CONDITIONAL_EFFECTS", loki::RequirementEnum::CONDITIONAL_EFFECTS)
        .value("FLUENTS", loki::RequirementEnum::FLUENTS)
        .value("OBJECT_FLUENTS", loki::RequirementEnum::OBJECT_FLUENTS)
        .value("NUMERIC_FLUENTS", loki::RequirementEnum::NUMERIC_FLUENTS)
        .value("ADL", loki::RequirementEnum::ADL)
        .value("DURATIVE_ACTIONS", loki::RequirementEnum::DURATIVE_ACTIONS)
        .value("DERIVED_PREDICATES", loki::RequirementEnum::DERIVED_PREDICATES)
        .value("TIMED_INITIAL_LITERALS", loki::RequirementEnum::TIMED_INITIAL_LITERALS)
        .value("PREFERENCES", loki::RequirementEnum::PREFERENCES)
        .value("CONSTRAINTS", loki::RequirementEnum::CONSTRAINTS)
        .value("ACTION_COSTS", loki::RequirementEnum::ACTION_COSTS)
        .export_values();

    py::enum_<loki::AssignOperatorEnum>(m_formalism, "AssignOperatorEnum")
        .value("ASSIGN", loki::AssignOperatorEnum::ASSIGN)
        .value("SCALE_UP", loki::AssignOperatorEnum::SCALE_UP)
        .value("SCALE_DOWN", loki::AssignOperatorEnum::SCALE_DOWN)
        .value("INCREASE", loki::AssignOperatorEnum::INCREASE)
        .value("DECREASE", loki::AssignOperatorEnum::DECREASE)
        .export_values();

    py::enum_<loki::BinaryOperatorEnum>(m_formalism, "BinaryOperatorEnum")
        .value("MUL", loki::BinaryOperatorEnum::MUL)
        .value("PLUS", loki::BinaryOperatorEnum::PLUS)
        .value("MINUS", loki::BinaryOperatorEnum::MINUS)
        .value("DIV", loki::BinaryOperatorEnum::DIV)
        .export_values();

    py::enum_<loki::MultiOperatorEnum>(m_formalism, "MultiOperatorEnum")
        .value("MUL", loki::MultiOperatorEnum::MUL)
        .value("PLUS", loki::MultiOperatorEnum::PLUS)
        .export_values();

    pybind11::enum_<loki::OptimizationMetricEnum>(m_formalism, "OptimizationMetricEnum")
        .value("MINIMIZE", loki::OptimizationMetricEnum::MINIMIZE)
        .value("MAXIMIZE", loki::OptimizationMetricEnum::MAXIMIZE)
        .export_values();

    py::class_<ActionImpl>(m_formalism, "Action")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ActionImpl>::str, py::const_))
        .def("get_identifier", &ActionImpl::get_identifier)
        .def("get_name", &ActionImpl::get_name, py::return_value_policy::reference)
        .def("get_arity", &ActionImpl::get_arity)
        .def("get_parameters", &ActionImpl::get_parameters, py::return_value_policy::reference)
        .def("get_conditions", &ActionImpl::get_conditions, py::return_value_policy::reference)
        .def("get_effects", &ActionImpl::get_effects, py::return_value_policy::reference);

    py::class_<AtomImpl>(m_formalism, "Atom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl>::str, py::const_))
        .def("get_identifier", &AtomImpl::get_identifier)
        .def("get_predicate", &AtomImpl::get_predicate, py::return_value_policy::reference)
        .def("get_terms", [](const AtomImpl& atom) { return wrap_terms(atom.get_terms()); });

    py::class_<AxiomImpl>(m_formalism, "Axiom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AxiomImpl>::str, py::const_))
        .def("get_identifier", &AxiomImpl::get_identifier)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference)
        .def("get_conditions", &AxiomImpl::get_conditions, py::return_value_policy::reference);

    py::class_<DomainImpl>(m_formalism, "Domain")  //
        .def("__str__", py::overload_cast<>(&loki::Base<DomainImpl>::str, py::const_))
        .def("get_identifier", &DomainImpl::get_identifier)
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::reference)
        .def("get_constants", &DomainImpl::get_constants, py::return_value_policy::reference)
        .def("get_predicates", &DomainImpl::get_predicates, py::return_value_policy::reference)
        .def("get_fluent_predicates", &DomainImpl::get_fluent_predicates, py::return_value_policy::reference)
        .def("get_static_predicates", &DomainImpl::get_static_predicates, py::return_value_policy::reference)
        .def("get_functions", &DomainImpl::get_functions, py::return_value_policy::reference)
        .def("get_actions", &DomainImpl::get_actions, py::return_value_policy::reference)
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::reference);

    py::class_<EffectImpl>(m_formalism, "Effect")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectImpl>::str, py::const_))
        .def("get_identifier", &EffectImpl::get_identifier)
        .def("get_parameters", &EffectImpl::get_parameters, py::return_value_policy::reference)
        .def("get_conditions", &EffectImpl::get_conditions, py::return_value_policy::reference)
        .def("get_effect", &EffectImpl::get_effect, py::return_value_policy::reference);

    py::class_<FunctionExpressionNumberImpl>(m_formalism, "FunctionExpressionNumber")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionNumberImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionNumberImpl::get_identifier)
        .def("get_number", &FunctionExpressionNumberImpl::get_number);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m_formalism, "FunctionExpressionBinaryOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionBinaryOperatorImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionBinaryOperatorImpl::get_identifier)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def("get_left_function_expression",
             [](const FunctionExpressionBinaryOperatorImpl& function_expression)
             { return WrappedFunctionExpression(function_expression.get_left_function_expression()); })
        .def("get_right_function_expression",
             [](const FunctionExpressionBinaryOperatorImpl& function_expression)
             { return WrappedFunctionExpression(function_expression.get_right_function_expression()); });

    py::class_<FunctionExpressionMultiOperatorImpl>(m_formalism, "FunctionExpressionMultiOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMultiOperatorImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def("get_function_expressions",
             [](const FunctionExpressionMultiOperatorImpl& function_expression)
             { return wrap_function_expressions(function_expression.get_function_expressions()); });

    py::class_<FunctionExpressionMinusImpl>(m_formalism, "FunctionExpressionMinus")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMinusImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMinusImpl::get_identifier)
        .def("get_function_expression",
             [](const FunctionExpressionMinusImpl& function_expression) { return WrappedFunctionExpression(function_expression.get_function_expression()); });
    ;

    py::class_<FunctionExpressionFunctionImpl>(m_formalism, "FunctionExpressionFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionFunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionFunctionImpl::get_identifier)
        .def("get_function", &FunctionExpressionFunctionImpl::get_function);

    py::class_<FunctionExpressionImpl>(m_formalism, "FunctionExpression")  //
        .def("get",
             [](const WrappedFunctionExpression& wrappedFunctionExpression) -> py::object
             { return std::visit(CastVisitor(), *wrappedFunctionExpression.function_expression); });
    ;

    py::class_<FunctionSkeletonImpl>(m_formalism, "FunctionSkeleton")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionSkeletonImpl>::str, py::const_))
        .def("get_identifier", &FunctionSkeletonImpl::get_identifier)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference)
        .def("get_parameters", &FunctionSkeletonImpl::get_parameters, py::return_value_policy::reference);

    py::class_<FunctionImpl>(m_formalism, "Function")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionImpl::get_identifier)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference)
        .def("get_terms", [](const FunctionImpl& function) { return wrap_terms(function.get_terms()); });

    py::class_<GroundAtomImpl>(m_formalism, "GroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl::get_identifier)
        .def("get_arity", &GroundAtomImpl::get_arity)
        .def("get_predicate", &GroundAtomImpl::get_predicate, py::return_value_policy::reference)
        .def("get_objects", &GroundAtomImpl::get_objects, py::return_value_policy::reference);

    py::class_<GroundLiteralImpl>(m_formalism, "GroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl::get_identifier)
        .def("get_atom", &GroundLiteralImpl::get_atom, py::return_value_policy::reference)
        .def("is_negated", &GroundLiteralImpl::is_negated);

    py::class_<LiteralImpl>(m_formalism, "Literal")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl>::str, py::const_))
        .def("get_identifier", &LiteralImpl::get_identifier)
        .def("get_atom", &LiteralImpl::get_atom, py::return_value_policy::reference)
        .def("is_negated", &LiteralImpl::is_negated);

    py::class_<OptimizationMetricImpl>(m_formalism, "OptimizationMetric")  //
        .def("__str__", py::overload_cast<>(&loki::Base<OptimizationMetricImpl>::str, py::const_))
        .def("get_identifier", &OptimizationMetricImpl::get_identifier)
        .def("get_function_expression", [](const OptimizationMetricImpl& metric) { return WrappedFunctionExpression(metric.get_function_expression()); })
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference);

    py::class_<NumericFluentImpl>(m_formalism, "NumericFluent")  //
        .def("__str__", py::overload_cast<>(&loki::Base<NumericFluentImpl>::str, py::const_))
        .def("get_identifier", &NumericFluentImpl::get_identifier)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference)
        .def("get_number", &NumericFluentImpl::get_number);

    py::class_<ObjectImpl>(m_formalism, "Object")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ObjectImpl>::str, py::const_))
        .def("get_identifier", &ObjectImpl::get_identifier)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::reference);

    py::class_<ParameterImpl>(m_formalism, "Parameter")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ParameterImpl>::str, py::const_))
        .def("get_identifier", &ParameterImpl::get_identifier, py::return_value_policy::reference)
        .def("get_variable", &ParameterImpl::get_variable, py::return_value_policy::reference);

    py::class_<PDDLFactories>(m_formalism, "PDDLFactories");

    py::class_<PDDLParser>(m_formalism, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain, py::return_value_policy::reference)
        .def("get_problem", &PDDLParser::get_problem, py::return_value_policy::reference)
        .def("get_factories", &PDDLParser::get_factories, py::return_value_policy::reference);

    py::class_<PredicateImpl>(m_formalism, "Predicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl>::str, py::const_))
        .def("get_identifier", &PredicateImpl::get_identifier)
        .def("get_name", &PredicateImpl::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl::get_parameters, py::return_value_policy::reference);

    py::class_<ProblemImpl>(m_formalism, "Problem")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ProblemImpl>::str, py::const_))
        .def("get_identifier", &ProblemImpl::get_identifier)
        .def("get_name", &ProblemImpl::get_name, py::return_value_policy::reference)
        .def("get_domain", &ProblemImpl::get_domain, py::return_value_policy::reference)
        .def("get_requirements", &ProblemImpl::get_requirements, py::return_value_policy::reference)
        .def("get_objects", &ProblemImpl::get_objects, py::return_value_policy::reference)
        .def("get_initial_literals", &ProblemImpl::get_initial_literals, py::return_value_policy::reference)
        .def("get_numeric_fluents", &ProblemImpl::get_numeric_fluents, py::return_value_policy::reference)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, py::return_value_policy::reference)
        .def("get_goal_condition", &ProblemImpl::get_goal_condition, py::return_value_policy::reference);

    py::class_<RequirementsImpl>(m_formalism, "Requirements")  //
        .def("__str__", py::overload_cast<>(&loki::Base<RequirementsImpl>::str, py::const_))
        .def("get_identifier", &RequirementsImpl::get_identifier)
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::reference);

    py::class_<TermObjectImpl>(m_formalism, "TermObject")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermObjectImpl>::str, py::const_))
        .def("get_identifier", &TermObjectImpl::get_identifier)
        .def("get_object", &TermObjectImpl::get_object, py::return_value_policy::reference);

    py::class_<TermVariableImpl>(m_formalism, "TermVariable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermVariableImpl>::str, py::const_))
        .def("get_identifier", &TermVariableImpl::get_identifier)
        .def("get_variable", &TermVariableImpl::get_variable, py::return_value_policy::reference);

    py::class_<WrappedTerm>(m_formalism, "Term")  //
        .def("get", [](const WrappedTerm& wrappedTerm) -> py::object { return std::visit(CastVisitor(), *wrappedTerm.term); });

    py::class_<VariableImpl>(m_formalism, "Variable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<VariableImpl>::str, py::const_))
        .def("get_identifier", &VariableImpl::get_identifier)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference);
}