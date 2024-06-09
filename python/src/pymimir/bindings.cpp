#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/predicate_category.hpp"

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

#include "mimir/mimir.hpp"

using namespace mimir;
namespace py = pybind11;

/**
 * We cannot expose the variant types directly because they are not default constructible.
 */

struct TermVariant
{
    Term term;
    explicit TermVariant(const Term& t) : term(t) {}
};

struct FunctionExpressionVariant
{
    FunctionExpression function_expression;
    explicit FunctionExpressionVariant(const FunctionExpression& e) : function_expression(e) {}
};

struct GroundFunctionExpressionVariant
{
    GroundFunctionExpression function_expression;
    explicit GroundFunctionExpressionVariant(const GroundFunctionExpression& e) : function_expression(e) {}
};

std::vector<TermVariant> to_term_variant_list(const TermList& terms)
{
    auto result = std::vector<TermVariant> {};
    result.reserve(terms.size());
    for (const auto& term : terms)
    {
        result.push_back(TermVariant(term));
    }
    return result;
}

std::vector<FunctionExpressionVariant> to_function_expression_variant_list(const FunctionExpressionList& function_expressions)
{
    auto result = std::vector<FunctionExpressionVariant> {};
    result.reserve(function_expressions.size());
    for (const auto& function_expression : function_expressions)
    {
        result.push_back(FunctionExpressionVariant(function_expression));
    }
    return result;
}

std::vector<GroundFunctionExpressionVariant> to_ground_function_expression_variant_list(const GroundFunctionExpressionList& ground_function_expressions)
{
    auto result = std::vector<GroundFunctionExpressionVariant> {};
    result.reserve(ground_function_expressions.size());
    for (const auto& function_expression : ground_function_expressions)
    {
        result.push_back(GroundFunctionExpressionVariant(function_expression));
    }
    return result;
}

struct CastVisitor
{
    template<typename T>
    py::object operator()(const T& element) const
    {
        return py::cast(element);
    }
};

static std::shared_ptr<StateSpaceImpl>
create_state_space_helper(const std::string& domain_filepath, const std::string& problem_filepath, size_t max_num_states, size_t timeout_ms)
{
    return StateSpaceImpl::create(domain_filepath, problem_filepath, max_num_states, timeout_ms);
}

/**
 * Bindings
 */
void init_pymimir(py::module_& m)
{
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Formalism
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    py::enum_<loki::RequirementEnum>(m, "RequirementEnum")
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

    py::enum_<loki::AssignOperatorEnum>(m, "AssignOperatorEnum")
        .value("ASSIGN", loki::AssignOperatorEnum::ASSIGN)
        .value("SCALE_UP", loki::AssignOperatorEnum::SCALE_UP)
        .value("SCALE_DOWN", loki::AssignOperatorEnum::SCALE_DOWN)
        .value("INCREASE", loki::AssignOperatorEnum::INCREASE)
        .value("DECREASE", loki::AssignOperatorEnum::DECREASE)
        .export_values();

    py::enum_<loki::BinaryOperatorEnum>(m, "BinaryOperatorEnum")
        .value("MUL", loki::BinaryOperatorEnum::MUL)
        .value("PLUS", loki::BinaryOperatorEnum::PLUS)
        .value("MINUS", loki::BinaryOperatorEnum::MINUS)
        .value("DIV", loki::BinaryOperatorEnum::DIV)
        .export_values();

    py::enum_<loki::MultiOperatorEnum>(m, "MultiOperatorEnum")
        .value("MUL", loki::MultiOperatorEnum::MUL)
        .value("PLUS", loki::MultiOperatorEnum::PLUS)
        .export_values();

    pybind11::enum_<loki::OptimizationMetricEnum>(m, "OptimizationMetricEnum")
        .value("MINIMIZE", loki::OptimizationMetricEnum::MINIMIZE)
        .value("MAXIMIZE", loki::OptimizationMetricEnum::MAXIMIZE)
        .export_values();

    py::class_<RequirementsImpl>(m, "Requirements")  // s
        .def("__str__", py::overload_cast<>(&loki::Base<RequirementsImpl>::str, py::const_))
        .def("get_identifier", &RequirementsImpl::get_identifier)
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::reference);

    py::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ObjectImpl>::str, py::const_))
        .def("get_identifier", &ObjectImpl::get_identifier)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::reference);

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<VariableImpl>::str, py::const_))
        .def("get_identifier", &VariableImpl::get_identifier)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference);

    py::class_<TermObjectImpl>(m, "TermObject")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermObjectImpl>::str, py::const_))
        .def("get_identifier", &TermObjectImpl::get_identifier)
        .def("get_object", &TermObjectImpl::get_object, py::return_value_policy::reference);

    py::class_<TermVariableImpl>(m, "TermVariable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermVariableImpl>::str, py::const_))
        .def("get_identifier", &TermVariableImpl::get_identifier)
        .def("get_variable", &TermVariableImpl::get_variable, py::return_value_policy::reference);

    py::class_<TermVariant>(m, "Term")  //
        .def("get", [](const TermVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.term); });

    py::class_<PredicateImpl<Static>>(m, "StaticPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Static>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Static>::get_identifier)
        .def("get_name", &PredicateImpl<Static>::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl<Static>::get_parameters, py::return_value_policy::reference);

    py::class_<PredicateImpl<Fluent>>(m, "FluentPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Fluent>::get_identifier)
        .def("get_name", &PredicateImpl<Fluent>::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl<Fluent>::get_parameters, py::return_value_policy::reference);

    py::class_<PredicateImpl<Derived>>(m, "DerivedPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Derived>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Derived>::get_identifier)
        .def("get_name", &PredicateImpl<Derived>::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl<Derived>::get_parameters, py::return_value_policy::reference);

    py::class_<AtomImpl<Static>>(m, "StaticAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Static>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Static>::get_identifier)
        .def("get_predicate", &AtomImpl<Static>::get_predicate, py::return_value_policy::reference)
        .def(
            "get_terms",
            [](const AtomImpl<Static>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::return_value_policy::reference);

    py::class_<AtomImpl<Fluent>>(m, "FluentAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Fluent>::get_identifier)
        .def("get_predicate", &AtomImpl<Fluent>::get_predicate, py::return_value_policy::reference)
        .def(
            "get_terms",
            [](const AtomImpl<Fluent>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::return_value_policy::reference);

    py::class_<AtomImpl<Derived>>(m, "DerivedAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Derived>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Derived>::get_identifier)
        .def("get_predicate", &AtomImpl<Derived>::get_predicate, py::return_value_policy::reference)
        .def(
            "get_terms",
            [](const AtomImpl<Derived>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::return_value_policy::reference);

    py::class_<FunctionSkeletonImpl>(m, "FunctionSkeleton")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionSkeletonImpl>::str, py::const_))
        .def("get_identifier", &FunctionSkeletonImpl::get_identifier)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference)
        .def("get_parameters", &FunctionSkeletonImpl::get_parameters, py::return_value_policy::reference);

    py::class_<FunctionImpl>(m, "Function")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionImpl::get_identifier)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference)
        .def("get_terms", [](const FunctionImpl& function) { return to_term_variant_list(function.get_terms()); });

    py::class_<GroundFunctionImpl>(m, "GroundFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionImpl::get_identifier)
        .def("get_function_skeleton", &GroundFunctionImpl::get_function_skeleton, py::return_value_policy::reference)
        .def("get_objects", &GroundFunctionImpl::get_objects, py::return_value_policy::reference);

    py::class_<GroundAtomImpl<Static>>(m, "StaticGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Static>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Static>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Static>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Static>::get_predicate, py::return_value_policy::reference)
        .def("get_objects", &GroundAtomImpl<Static>::get_objects, py::return_value_policy::reference);

    py::class_<GroundAtomImpl<Fluent>>(m, "FluentGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Fluent>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Fluent>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Fluent>::get_predicate, py::return_value_policy::reference)
        .def("get_objects", &GroundAtomImpl<Fluent>::get_objects, py::return_value_policy::reference);

    py::class_<GroundAtomImpl<Derived>>(m, "DerivedGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Derived>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Derived>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Derived>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Derived>::get_predicate, py::return_value_policy::reference)
        .def("get_objects", &GroundAtomImpl<Derived>::get_objects, py::return_value_policy::reference);

    py::class_<GroundLiteralImpl<Static>>(m, "StaticGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Static>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Static>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Static>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &GroundLiteralImpl<Static>::is_negated);

    py::class_<GroundLiteralImpl<Fluent>>(m, "FluentGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Fluent>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Fluent>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &GroundLiteralImpl<Fluent>::is_negated);

    py::class_<GroundLiteralImpl<Derived>>(m, "DerivedGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Derived>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Derived>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Derived>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &GroundLiteralImpl<Derived>::is_negated);

    py::class_<LiteralImpl<Static>>(m, "StaticLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Static>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Static>::get_identifier)
        .def("get_atom", &LiteralImpl<Static>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &LiteralImpl<Static>::is_negated);

    py::class_<LiteralImpl<Fluent>>(m, "FluentLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Fluent>::get_identifier)
        .def("get_atom", &LiteralImpl<Fluent>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &LiteralImpl<Fluent>::is_negated);

    py::class_<LiteralImpl<Derived>>(m, "DerivedLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Derived>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Derived>::get_identifier)
        .def("get_atom", &LiteralImpl<Derived>::get_atom, py::return_value_policy::reference)
        .def("is_negated", &LiteralImpl<Derived>::is_negated);

    py::class_<NumericFluentImpl>(m, "NumericFluent")  //
        .def("__str__", py::overload_cast<>(&loki::Base<NumericFluentImpl>::str, py::const_))
        .def("get_identifier", &NumericFluentImpl::get_identifier)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference)
        .def("get_number", &NumericFluentImpl::get_number);

    py::class_<EffectSimpleImpl>(m, "SimpleEffect")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectSimpleImpl>::str, py::const_))
        .def("get_identifier", &EffectSimpleImpl::get_identifier)
        .def("get_effect", &EffectSimpleImpl::get_effect, py::return_value_policy::reference);

    py::class_<EffectConditionalImpl>(m, "ConditionalEffect")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectConditionalImpl>::str, py::const_))
        .def("get_identifier", &EffectConditionalImpl::get_identifier)
        .def("get_static_conditions", &EffectConditionalImpl::get_static_conditions, py::return_value_policy::reference)
        .def("get_fluent_conditions", &EffectConditionalImpl::get_fluent_conditions, py::return_value_policy::reference)
        .def("get_derived_conditions", &EffectConditionalImpl::get_derived_conditions, py::return_value_policy::reference)
        .def("get_effect", &EffectConditionalImpl::get_effect, py::return_value_policy::reference);

    py::class_<FunctionExpressionVariant>(m, "FunctionExpression")  //
        .def(
            "get",
            [](const FunctionExpressionVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.function_expression); },
            py::return_value_policy::reference);
    ;

    py::class_<EffectUniversalImpl>(m, "UniversalEffect")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectUniversalImpl>::str, py::const_))
        .def("get_identifier", &EffectUniversalImpl::get_identifier)
        .def("get_parameters", &EffectUniversalImpl::get_parameters, py::return_value_policy::reference)
        .def("get_static_conditions", &EffectUniversalImpl::get_static_conditions, py::return_value_policy::reference)
        .def("get_fluent_conditions", &EffectUniversalImpl::get_fluent_conditions, py::return_value_policy::reference)
        .def("get_derived_conditions", &EffectUniversalImpl::get_derived_conditions, py::return_value_policy::reference)
        .def("get_effect", &EffectUniversalImpl::get_effect, py::return_value_policy::reference);

    py::class_<FunctionExpressionNumberImpl>(m, "FunctionExpressionNumber")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionNumberImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionNumberImpl::get_identifier)
        .def("get_number", &FunctionExpressionNumberImpl::get_number);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m, "FunctionExpressionBinaryOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionBinaryOperatorImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionBinaryOperatorImpl::get_identifier)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def(
            "get_left_function_expression",
            [](const FunctionExpressionBinaryOperatorImpl& function_expression)
            { return FunctionExpressionVariant(function_expression.get_left_function_expression()); },
            py::return_value_policy::reference)
        .def(
            "get_right_function_expression",
            [](const FunctionExpressionBinaryOperatorImpl& function_expression)
            { return FunctionExpressionVariant(function_expression.get_right_function_expression()); },
            py::return_value_policy::reference);

    py::class_<FunctionExpressionMultiOperatorImpl>(m, "FunctionExpressionMultiOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMultiOperatorImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const FunctionExpressionMultiOperatorImpl& function_expression)
            { return to_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::return_value_policy::reference);

    py::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMinusImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMinusImpl::get_identifier)
        .def("get_function_expression",
             [](const FunctionExpressionMinusImpl& function_expression) { return FunctionExpressionVariant(function_expression.get_function_expression()); });
    ;

    py::class_<FunctionExpressionFunctionImpl>(m, "FunctionExpressionFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionFunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionFunctionImpl::get_identifier)
        .def("get_function", &FunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference);

    py::class_<GroundFunctionExpressionVariant>(m, "GroundFunctionExpression")  //
        .def(
            "get",
            [](const GroundFunctionExpressionVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.function_expression); },
            py::return_value_policy::reference);
    ;

    py::class_<GroundFunctionExpressionNumberImpl>(m, "GroundFunctionExpressionNumber")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionNumberImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionNumberImpl::get_identifier)
        .def("get_number", &GroundFunctionExpressionNumberImpl::get_number);

    py::class_<GroundFunctionExpressionBinaryOperatorImpl>(m, "GroundFunctionExpressionBinaryOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionBinaryOperatorImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionBinaryOperatorImpl::get_identifier)
        .def("get_binary_operator", &GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def(
            "get_left_function_expression",
            [](const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_left_function_expression()); },
            py::return_value_policy::reference)
        .def(
            "get_right_function_expression",
            [](const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_right_function_expression()); },
            py::return_value_policy::reference);

    py::class_<GroundFunctionExpressionMultiOperatorImpl>(m, "GroundFunctionExpressionMultiOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionMultiOperatorImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const GroundFunctionExpressionMultiOperatorImpl& function_expression)
            { return to_ground_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::return_value_policy::reference);

    py::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionMinusImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionMinusImpl::get_identifier)
        .def(
            "get_function_expression",
            [](const GroundFunctionExpressionMinusImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_function_expression()); },
            py::return_value_policy::reference);
    ;

    py::class_<GroundFunctionExpressionFunctionImpl>(m, "GroundFunctionExpressionFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionFunctionImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionFunctionImpl::get_identifier)
        .def("get_function", &GroundFunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference);

    py::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", py::overload_cast<>(&loki::Base<OptimizationMetricImpl>::str, py::const_))
        .def("get_identifier", &OptimizationMetricImpl::get_identifier)
        .def("get_function_expression", [](const OptimizationMetricImpl& metric) { return GroundFunctionExpressionVariant(metric.get_function_expression()); })
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference);

    py::class_<ActionImpl>(m, "Action")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ActionImpl>::str, py::const_))
        .def("get_identifier", &ActionImpl::get_identifier)
        .def("get_name", &ActionImpl::get_name, py::return_value_policy::reference)
        .def("get_arity", &ActionImpl::get_arity)
        .def("get_parameters", &ActionImpl::get_parameters, py::return_value_policy::reference)
        .def("get_static_conditions", &ActionImpl::get_static_conditions, py::return_value_policy::reference)
        .def("get_fluent_conditions", &ActionImpl::get_fluent_conditions, py::return_value_policy::reference)
        .def("get_derived_conditions", &ActionImpl::get_derived_conditions, py::return_value_policy::reference)
        .def("get_simple_effects", &ActionImpl::get_simple_effects, py::return_value_policy::reference)
        .def("get_conditional_effects", &ActionImpl::get_conditional_effects, py::return_value_policy::reference)
        .def("get_universal_effects", &ActionImpl::get_universal_effects, py::return_value_policy::reference);

    py::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AxiomImpl>::str, py::const_))
        .def("get_identifier", &AxiomImpl::get_identifier)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference)
        .def("get_static_conditions", &AxiomImpl::get_static_conditions, py::return_value_policy::reference)
        .def("get_fluent_conditions", &AxiomImpl::get_fluent_conditions, py::return_value_policy::reference)
        .def("get_derived_conditions", &AxiomImpl::get_derived_conditions, py::return_value_policy::reference);

    py::class_<DomainImpl>(m, "Domain")  //
        .def("__str__", py::overload_cast<>(&loki::Base<DomainImpl>::str, py::const_))
        .def("get_identifier", &DomainImpl::get_identifier)
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::reference)
        .def("get_constants", &DomainImpl::get_constants, py::return_value_policy::reference)
        .def("get_fluent_predicates", &DomainImpl::get_fluent_predicates, py::return_value_policy::reference)
        .def("get_static_predicates", &DomainImpl::get_static_predicates, py::return_value_policy::reference)
        .def("get_derived_predicates", &DomainImpl::get_derived_predicates, py::return_value_policy::reference)
        .def("get_functions", &DomainImpl::get_functions, py::return_value_policy::reference)
        .def("get_actions", &DomainImpl::get_actions, py::return_value_policy::reference)
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::reference);

    py::class_<ProblemImpl>(m, "Problem")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ProblemImpl>::str, py::const_))
        .def("get_identifier", &ProblemImpl::get_identifier)
        .def("get_name", &ProblemImpl::get_name, py::return_value_policy::reference)
        .def("get_domain", &ProblemImpl::get_domain, py::return_value_policy::reference)
        .def("get_requirements", &ProblemImpl::get_requirements, py::return_value_policy::reference)
        .def("get_objects", &ProblemImpl::get_objects, py::return_value_policy::reference)
        .def("get_static_initial_literals", &ProblemImpl::get_static_initial_literals, py::return_value_policy::reference)
        .def("get_fluent_initial_literals", &ProblemImpl::get_fluent_initial_literals, py::return_value_policy::reference)
        .def("get_numeric_fluents", &ProblemImpl::get_numeric_fluents, py::return_value_policy::reference)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, py::return_value_policy::reference)
        .def("get_static_goal_condition", &ProblemImpl::get_static_goal_condition, py::return_value_policy::reference)
        .def("get_fluent_goal_condition", &ProblemImpl::get_fluent_goal_condition, py::return_value_policy::reference)
        .def("get_derived_goal_condition", &ProblemImpl::get_derived_goal_condition, py::return_value_policy::reference);

    py::class_<PDDLFactories>(m, "PDDLFactories")  //
        .def("get_static_ground_atom", &PDDLFactories::get_ground_atom<Static>, py::return_value_policy::reference)
        .def("get_fluent_ground_atom", &PDDLFactories::get_ground_atom<Fluent>, py::return_value_policy::reference)
        .def("get_derived_ground_atom", &PDDLFactories::get_ground_atom<Derived>, py::return_value_policy::reference)
        .def("get_static_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Static, std::vector<size_t>>, py::const_),
             py::return_value_policy::reference)
        .def("get_fluent_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Fluent, std::vector<size_t>>, py::const_),
             py::return_value_policy::reference)
        .def("get_derived_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Derived, std::vector<size_t>>, py::const_),
             py::return_value_policy::reference)
        .def("get_object", &PDDLFactories::get_object, py::return_value_policy::reference)
        .def("get_objects_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_objects_from_ids<std::vector<size_t>>, py::const_),
             py::return_value_policy::reference);

    py::class_<PDDLParser>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain, py::return_value_policy::reference)
        .def("get_problem", &PDDLParser::get_problem, py::return_value_policy::reference)
        .def("get_factories", &PDDLParser::get_factories, py::return_value_policy::reference);

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Search
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    py::enum_<SearchNodeStatus>(m, "SearchNodeStatus")
        .value("NEW", SearchNodeStatus::NEW)
        .value("OPEN", SearchNodeStatus::OPEN)
        .value("CLOSED", SearchNodeStatus::CLOSED)
        .value("DEAD_END", SearchNodeStatus::DEAD_END)
        .export_values();

    py::enum_<SearchStatus>(m, "SearchStatus")
        .value("IN_PROGRESS", SearchStatus::IN_PROGRESS)
        .value("OUT_OF_TIME", SearchStatus::OUT_OF_TIME)
        .value("OUT_OF_MEMORY", SearchStatus::OUT_OF_MEMORY)
        .value("FAILED", SearchStatus::FAILED)
        .value("EXHAUSTED", SearchStatus::EXHAUSTED)
        .value("SOLVED", SearchStatus::SOLVED)
        .export_values();

    /* State */
    py::class_<State>(m, "State")  //
        .def("__hash__", &State::hash)
        .def("__eq__", &State::operator==)
        .def("get_fluent_atoms",
             [](const State& self)
             {
                 auto atoms = self.get_atoms<Fluent>();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("get_derived_atoms",
             [](const State& self)
             {
                 auto atoms = self.get_atoms<Derived>();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("contains", py::overload_cast<const GroundAtom<Fluent>&>(&State::contains<Fluent>, py::const_))
        .def("contains", py::overload_cast<const GroundAtom<Derived>&>(&State::contains<Derived>, py::const_))
        .def("superset_of", py::overload_cast<const GroundAtomList<Fluent>&>(&State::superset_of<Fluent>, py::const_))
        .def("superset_of", py::overload_cast<const GroundAtomList<Derived>&>(&State::superset_of<Derived>, py::const_))
        .def("literal_holds", py::overload_cast<const GroundLiteral<Fluent>&>(&State::literal_holds<Fluent>, py::const_))
        .def("literal_holds", py::overload_cast<const GroundLiteral<Derived>&>(&State::literal_holds<Derived>, py::const_))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Fluent>&>(&State::literals_hold<Fluent>, py::const_))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Derived>&>(&State::literals_hold<Derived>, py::const_))
        .def("to_string",
             [](const State& self, const Problem& problem, const PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(problem, self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_id", &State::get_id);

    /* Action */
    py::class_<GroundAction>(m, "GroundAction")  //
        .def("__hash__", &GroundAction::hash)
        .def("__eq__", &GroundAction::operator==)
        .def("to_string",
             [](const GroundAction& self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_id", &GroundAction::get_id);

    /* AAGs */

    py::class_<IAAG, std::shared_ptr<IAAG>>(m, "IAAG")  //
        .def(
            "compute_applicable_actions",
            [](IAAG& self, const State& state)
            {
                auto applicable_actions = GroundActionList {};
                self.generate_applicable_actions(state, applicable_actions);
                return applicable_actions;
            },
            py::return_value_policy::reference)
        .def("get_action", &IAAG::get_action, py::return_value_policy::reference)
        .def("get_problem", &IAAG::get_problem, py::return_value_policy::reference)
        .def("get_pddl_factories", py::overload_cast<>(&IAAG::get_pddl_factories), py::return_value_policy::reference);

    // Lifted
    py::class_<ILiftedAAGEventHandler, std::shared_ptr<ILiftedAAGEventHandler>>(m, "ILiftedAAGEventHandler");  //
    py::class_<DefaultLiftedAAGEventHandler, ILiftedAAGEventHandler, std::shared_ptr<DefaultLiftedAAGEventHandler>>(m,
                                                                                                                    "DefaultLiftedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedAAGEventHandler, ILiftedAAGEventHandler, std::shared_ptr<DebugLiftedAAGEventHandler>>(m, "DebugLiftedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedAAG, IAAG, std::shared_ptr<LiftedAAG>>(m, "LiftedAAG")  //
        .def(py::init<Problem, PDDLFactories&>())
        .def(py::init<Problem, PDDLFactories&, std::shared_ptr<ILiftedAAGEventHandler>>());

    // Grounded
    py::class_<IGroundedAAGEventHandler, std::shared_ptr<IGroundedAAGEventHandler>>(m, "IGroundedAAGEventHandler");  //
    py::class_<DefaultGroundedAAGEventHandler, IGroundedAAGEventHandler, std::shared_ptr<DefaultGroundedAAGEventHandler>>(m,
                                                                                                                          "DefaultGroundedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedAAGEventHandler, IGroundedAAGEventHandler, std::shared_ptr<DebugGroundedAAGEventHandler>>(m, "DebugGroundedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedAAG, IAAG, std::shared_ptr<GroundedAAG>>(m, "GroundedAAG")  //
        .def(py::init<Problem, PDDLFactories&>())
        .def(py::init<Problem, PDDLFactories&, std::shared_ptr<IGroundedAAGEventHandler>>());

    /* SSGs */
    py::class_<ISSG, std::shared_ptr<ISSG>>(m, "ISSG")  //
        .def("get_or_create_initial_state", &ISSG::get_or_create_initial_state)
        .def("get_or_create_state", &ISSG::get_or_create_state)
        .def("get_or_create_successor_state", &ISSG::get_or_create_successor_state)
        .def("get_state_count", &ISSG::get_state_count);
    py::class_<SuccessorStateGenerator, ISSG, std::shared_ptr<SuccessorStateGenerator>>(m, "SSG")  //
        .def(py::init<std::shared_ptr<IAAG>>());

    /* Heuristics */
    py::class_<IHeuristic, std::shared_ptr<IHeuristic>>(m, "IHeuristic");
    py::class_<BlindHeuristic, IHeuristic, std::shared_ptr<BlindHeuristic>>(m, "BlindHeuristic").def(py::init<>());

    /* Algorithms */
    py::class_<IAlgorithm, std::shared_ptr<IAlgorithm>>(m, "IAlgorithm")  //
        .def("find_solution",
             [](IAlgorithm& algorithm)
             {
                 auto out_actions = GroundActionList {};
                 auto search_status = algorithm.find_solution(out_actions);
                 return std::make_tuple(search_status, out_actions);
             });

    // AStar
    py::class_<AStarAlgorithm, IAlgorithm, std::shared_ptr<AStarAlgorithm>>(m, "AStarAlgorithm")  //
        .def(py::init<std::shared_ptr<IAAG>, std::shared_ptr<IHeuristic>>())
        .def(py::init<std::shared_ptr<IAAG>, std::shared_ptr<ISSG>, std::shared_ptr<IHeuristic>>());

    // BrFS
    py::class_<IBrFSAlgorithmEventHandler, std::shared_ptr<IBrFSAlgorithmEventHandler>>(m, "IBrFSAlgorithmEventHandler");
    py::class_<DefaultBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DefaultBrFSAlgorithmEventHandler>>(
        m,
        "DefaultBrFSAlgorithmEventHandler")  //
        .def(py::init<>());
    py::class_<DebugBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DebugBrFSAlgorithmEventHandler>>(
        m,
        "DebugBrFSAlgorithmEventHandler")  //
        .def(py::init<>());
    py::class_<BrFSAlgorithm, IAlgorithm, std::shared_ptr<BrFSAlgorithm>>(m, "BrFSAlgorithm")
        .def(py::init<std::shared_ptr<IAAG>>())
        .def(py::init<std::shared_ptr<IAAG>, std::shared_ptr<ISSG>, std::shared_ptr<IBrFSAlgorithmEventHandler>>());

    // IW
    py::class_<IIWAlgorithmEventHandler, std::shared_ptr<IIWAlgorithmEventHandler>>(m, "IIWAlgorithmEventHandler");
    py::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler, std::shared_ptr<DefaultIWAlgorithmEventHandler>>(m, "DefaultIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<IWAlgorithm, IAlgorithm, std::shared_ptr<IWAlgorithm>>(m, "IWAlgorithm")
        .def(py::init<std::shared_ptr<IAAG>, int>())
        .def(py::init<std::shared_ptr<IAAG>,
                      int,
                      std::shared_ptr<ISSG>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>>());

    // SIW
    py::class_<ISIWAlgorithmEventHandler, std::shared_ptr<ISIWAlgorithmEventHandler>>(m, "ISIWAlgorithmEventHandler");
    py::class_<DefaultSIWAlgorithmEventHandler, ISIWAlgorithmEventHandler, std::shared_ptr<DefaultSIWAlgorithmEventHandler>>(m,
                                                                                                                             "DefaultSIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<SIWAlgorithm, IAlgorithm, std::shared_ptr<SIWAlgorithm>>(m, "SIWAlgorithm")
        .def(py::init<std::shared_ptr<IAAG>, int>())
        .def(py::init<std::shared_ptr<IAAG>,
                      int,
                      std::shared_ptr<ISSG>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>,
                      std::shared_ptr<ISIWAlgorithmEventHandler>>());

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // DataSets
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // Transition
    py::class_<Transition>(m, "Transition")  //
        .def("__eq__", &Transition::operator==)
        .def("__hash__", &Transition::hash)
        .def("get_successor_state", &Transition::get_successor_state)
        .def("get_creating_action", &Transition::get_creating_action);

    // StateSpace
    py::class_<StateSpaceImpl, std::shared_ptr<StateSpaceImpl>>(m, "StateSpace")  //
        .def_static("create", &create_state_space_helper)
        .def("compute_shortest_distances_from_states",
             &StateSpaceImpl::compute_shortest_distances_from_states,
             pybind11::arg("states"),
             pybind11::arg("forward") = true)
        .def("compute_pairwise_shortest_state_distances", &StateSpaceImpl::compute_pairwise_shortest_state_distances, pybind11::arg("forward") = true)
        .def("get_states", &StateSpaceImpl::get_states, py::return_value_policy::reference)
        .def("get_initial_state", &StateSpaceImpl::get_initial_state)
        .def("get_forward_transitions", &StateSpaceImpl::get_forward_transitions, py::return_value_policy::reference)
        .def("get_backward_transitions", &StateSpaceImpl::get_backward_transitions, py::return_value_policy::reference)
        .def("get_problem", &StateSpaceImpl::get_problem, py::return_value_policy::reference)
        .def("get_goal_distances", &StateSpaceImpl::get_goal_distances, py::return_value_policy::reference)
        .def("get_goal_states", &StateSpaceImpl::get_goal_states, py::return_value_policy::reference)
        .def("get_deadend_states", &StateSpaceImpl::get_deadend_states, py::return_value_policy::reference)
        .def("get_num_states", &StateSpaceImpl::get_num_states)
        .def("get_num_transitions", &StateSpaceImpl::get_num_transitions)
        .def("get_num_goal_states", &StateSpaceImpl::get_num_goal_states)
        .def("get_num_deadend_states", &StateSpaceImpl::get_num_deadend_states)
        .def("get_goal_distance", &StateSpaceImpl::get_goal_distance)
        .def("get_max_goal_distance", &StateSpaceImpl::get_max_goal_distance)
        .def("is_deadend_state", &StateSpaceImpl::is_deadend_state)
        .def("sample_state_with_goal_distance", &StateSpaceImpl::sample_state_with_goal_distance, py::return_value_policy::reference)
        .def("get_aag", &StateSpaceImpl::get_aag)
        .def("get_ssg", &StateSpaceImpl::get_ssg)
        .def("get_pddl_parser", &StateSpaceImpl::get_pddl_parser, py::return_value_policy::reference)
        .def("get_factories", &StateSpaceImpl::get_factories, py::return_value_policy::reference);
}
