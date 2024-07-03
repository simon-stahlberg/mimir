#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/object.hpp"
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
        .def("get_parameters", &PredicateImpl<Static>::get_parameters, py::return_value_policy::reference)
        .def("get_arity", &PredicateImpl<Static>::get_arity);

    py::class_<PredicateImpl<Fluent>>(m, "FluentPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Fluent>::get_identifier)
        .def("get_name", &PredicateImpl<Fluent>::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl<Fluent>::get_parameters, py::return_value_policy::reference)
        .def("get_arity", &PredicateImpl<Fluent>::get_arity);

    py::class_<PredicateImpl<Derived>>(m, "DerivedPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Derived>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Derived>::get_identifier)
        .def("get_name", &PredicateImpl<Derived>::get_name, py::return_value_policy::reference)
        .def("get_parameters", &PredicateImpl<Derived>::get_parameters, py::return_value_policy::reference)
        .def("get_arity", &PredicateImpl<Derived>::get_arity);

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
        .def(
            "get_static_conditions",
            [](const EffectConditionalImpl& self) { return self.get_conditions<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_conditions",
            [](const EffectConditionalImpl& self) { return self.get_conditions<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_conditions",
            [](const EffectConditionalImpl& self) { return self.get_conditions<Derived>(); },
            py::return_value_policy::reference)
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
        .def(
            "get_static_conditions",
            [](const EffectUniversalImpl& self) { return self.get_conditions<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_conditions",
            [](const EffectUniversalImpl& self) { return self.get_conditions<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_conditions",
            [](const EffectUniversalImpl& self) { return self.get_conditions<Derived>(); },
            py::return_value_policy::reference)
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
        .def("get_parameters", &ActionImpl::get_parameters, py::return_value_policy::reference)
        .def(
            "get_static_conditions",
            [](const ActionImpl& self) { return self.get_conditions<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_conditions",
            [](const ActionImpl& self) { return self.get_conditions<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_conditions",
            [](const ActionImpl& self) { return self.get_conditions<Derived>(); },
            py::return_value_policy::reference)
        .def("get_simple_effects", &ActionImpl::get_simple_effects, py::return_value_policy::reference)
        .def("get_conditional_effects", &ActionImpl::get_conditional_effects, py::return_value_policy::reference)
        .def("get_universal_effects", &ActionImpl::get_universal_effects, py::return_value_policy::reference)
        .def("get_arity", &ActionImpl::get_arity);

    py::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AxiomImpl>::str, py::const_))
        .def("get_identifier", &AxiomImpl::get_identifier)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference)
        .def(
            "get_static_conditions",
            [](const AxiomImpl& self) { return self.get_conditions<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_conditions",
            [](const AxiomImpl& self) { return self.get_conditions<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_conditions",
            [](const AxiomImpl& self) { return self.get_conditions<Derived>(); },
            py::return_value_policy::reference)
        .def("get_arity", &AxiomImpl::get_arity);

    py::class_<DomainImpl>(m, "Domain")  //
        .def("__str__", py::overload_cast<>(&loki::Base<DomainImpl>::str, py::const_))
        .def("get_identifier", &DomainImpl::get_identifier)
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::reference)
        .def("get_constants", &DomainImpl::get_constants, py::return_value_policy::reference)
        .def(
            "get_static_predicates",
            [](const DomainImpl& self) { return self.get_predicates<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_predicates",
            [](const DomainImpl& self) { return self.get_predicates<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_predicates",
            [](const DomainImpl& self) { return self.get_predicates<Derived>(); },
            py::return_value_policy::reference)
        .def("get_functions", &DomainImpl::get_functions, py::return_value_policy::reference)
        .def("get_actions", &DomainImpl::get_actions, py::return_value_policy::reference)
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::reference)
        .def(
            "get_name_to_static_predicate",
            [](const DomainImpl& self) { return self.get_name_to_predicate<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_name_to_fluent_predicate",
            [](const DomainImpl& self) { return self.get_name_to_predicate<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_name_to_derived_predicate",
            [](const DomainImpl& self) { return self.get_name_to_predicate<Derived>(); },
            py::return_value_policy::reference);

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
        .def(
            "get_static_goal_condition",
            [](const ProblemImpl& self) { return self.get_goal_condition<Static>(); },
            py::return_value_policy::reference)
        .def(
            "get_fluent_goal_condition",
            [](const ProblemImpl& self) { return self.get_goal_condition<Fluent>(); },
            py::return_value_policy::reference)
        .def(
            "get_derived_goal_condition",
            [](const ProblemImpl& self) { return self.get_goal_condition<Derived>(); },
            py::return_value_policy::reference);

    py::class_<PDDLFactories, std::shared_ptr<PDDLFactories>>(m, "PDDLFactories")  //
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

    py::class_<PDDLParser, std::shared_ptr<PDDLParser>>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain_filepath", [](const PDDLParser& self) { return std::string(self.get_domain_filepath()); })
        .def("get_problem_filepath", [](const PDDLParser& self) { return std::string(self.get_problem_filepath()); })
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
             [](State self)
             {
                 auto atoms = self.get_atoms<Fluent>();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("get_derived_atoms",
             [](State self)
             {
                 auto atoms = self.get_atoms<Derived>();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("contains", py::overload_cast<GroundAtom<Fluent>>(&State::contains<Fluent>, py::const_))
        .def("contains", py::overload_cast<GroundAtom<Derived>>(&State::contains<Derived>, py::const_))
        .def("superset_of", py::overload_cast<const GroundAtomList<Fluent>&>(&State::superset_of<Fluent>, py::const_))
        .def("superset_of", py::overload_cast<const GroundAtomList<Derived>&>(&State::superset_of<Derived>, py::const_))
        .def("literal_holds", py::overload_cast<GroundLiteral<Fluent>>(&State::literal_holds<Fluent>, py::const_))
        .def("literal_holds", py::overload_cast<GroundLiteral<Derived>>(&State::literal_holds<Derived>, py::const_))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Fluent>&>(&State::literals_hold<Fluent>, py::const_))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Derived>&>(&State::literals_hold<Derived>, py::const_))
        .def("to_string",
             [](const State& self, Problem problem, const PDDLFactories& pddl_factories)
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
        .def("get_name", [](GroundAction self) { return self.get_action()->get_name(); })
        .def(
            "get_terms",
            [](GroundAction self)
            {
                ObjectList terms;
                for (const auto& object : self.get_objects())
                {
                    terms.emplace_back(object);
                }
                return terms;
            },
            py::return_value_policy::reference_internal)
        .def("to_string",
             [](GroundAction self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_id", &GroundAction::get_id)
        .def("__repr__",
             [](GroundAction self)
             {
                 std::stringstream ss;
                 ss << "(";
                 ss << self.get_action()->get_name();
                 for (const auto& object : self.get_objects())
                 {
                     ss << " " << object->get_name();
                 }
                 ss << ")";
                 return ss.str();
             });

    /* AAGs */

    py::class_<IAAG, std::shared_ptr<IAAG>>(m, "IAAG")  //
        .def(
            "compute_applicable_actions",
            [](IAAG& self, State state)
            {
                auto applicable_actions = GroundActionList {};
                self.generate_applicable_actions(state, applicable_actions);
                return applicable_actions;
            },
            py::return_value_policy::reference)
        .def("get_action", &IAAG::get_action, py::return_value_policy::reference)
        .def("get_problem", &IAAG::get_problem, py::return_value_policy::reference)
        .def("get_pddl_factories", &IAAG::get_pddl_factories);

    // Lifted
    py::class_<ILiftedAAGEventHandler, std::shared_ptr<ILiftedAAGEventHandler>>(m, "ILiftedAAGEventHandler");  //
    py::class_<DefaultLiftedAAGEventHandler, ILiftedAAGEventHandler, std::shared_ptr<DefaultLiftedAAGEventHandler>>(m,
                                                                                                                    "DefaultLiftedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedAAGEventHandler, ILiftedAAGEventHandler, std::shared_ptr<DebugLiftedAAGEventHandler>>(m, "DebugLiftedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedAAG, IAAG, std::shared_ptr<LiftedAAG>>(m, "LiftedAAG")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>())
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<ILiftedAAGEventHandler>>());

    // Grounded
    py::class_<IGroundedAAGEventHandler, std::shared_ptr<IGroundedAAGEventHandler>>(m, "IGroundedAAGEventHandler");  //
    py::class_<DefaultGroundedAAGEventHandler, IGroundedAAGEventHandler, std::shared_ptr<DefaultGroundedAAGEventHandler>>(m,
                                                                                                                          "DefaultGroundedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedAAGEventHandler, IGroundedAAGEventHandler, std::shared_ptr<DebugGroundedAAGEventHandler>>(m, "DebugGroundedAAGEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedAAG, IAAG, std::shared_ptr<GroundedAAG>>(m, "GroundedAAG")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>())
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IGroundedAAGEventHandler>>());

    /* SuccessorStateGenerator */
    py::class_<SuccessorStateGenerator, std::shared_ptr<SuccessorStateGenerator>>(m, "SuccessorStateGenerator")  //
        .def(py::init<std::shared_ptr<IAAG>>())
        .def("get_or_create_initial_state", &SuccessorStateGenerator::get_or_create_initial_state)
        .def("get_or_create_state", &SuccessorStateGenerator::get_or_create_state)
        .def("get_or_create_successor_state", &SuccessorStateGenerator::get_or_create_successor_state)
        .def("get_state_count", &SuccessorStateGenerator::get_state_count)
        .def("get_reached_fluent_ground_atoms",
             [](const SuccessorStateGenerator& self)
             {
                 const auto& atoms = self.get_reached_fluent_ground_atoms();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("get_reached_derived_ground_atoms",
             [](const SuccessorStateGenerator& self)
             {
                 const auto& atoms = self.get_reached_derived_ground_atoms();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             });

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
        .def(py::init<std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>, std::shared_ptr<IHeuristic>>());

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
        .def(py::init<std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>, std::shared_ptr<IBrFSAlgorithmEventHandler>>());

    // IW
    py::class_<TupleIndexMapper, std::shared_ptr<TupleIndexMapper>>(m, "TupleIndexMapper")  //
        .def("to_tuple_index", &TupleIndexMapper::to_tuple_index)
        .def("to_atom_indices",
             [](const TupleIndexMapper& self, const TupleIndex tuple_index)
             {
                 auto atom_indices = AtomIndexList {};
                 self.to_atom_indices(tuple_index, atom_indices);
                 return atom_indices;
             })
        .def("tuple_index_to_string", &TupleIndexMapper::tuple_index_to_string)
        .def("get_num_atoms", &TupleIndexMapper::get_num_atoms)
        .def("get_arity", &TupleIndexMapper::get_arity)
        .def("get_factors", &TupleIndexMapper::get_factors, py::return_value_policy::reference)
        .def("get_max_tuple_index", &TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &TupleIndexMapper::get_empty_tuple_index);
    py::class_<FluentAndDerivedMapper, std::shared_ptr<FluentAndDerivedMapper>>(m, "FluentAndDerivedMapper")  //
        .def("combine_and_sort",

             [](FluentAndDerivedMapper& self, const State state)
             {
                 auto atom_indices = AtomIndexList {};
                 self.combine_and_sort(state, atom_indices);
                 return atom_indices;
             })
        .def("combine_and_sort",

             [](FluentAndDerivedMapper& self, const State state, const State succ_state)
             {
                 auto atom_indices = AtomIndexList {};
                 auto add_atom_indices = AtomIndexList {};
                 self.combine_and_sort(state, succ_state, atom_indices, add_atom_indices);
                 return std::make_tuple(atom_indices, add_atom_indices);
             })
        .def("inverse_remap_and_separate",

             [](FluentAndDerivedMapper& self, const AtomIndexList& fluent_and_derived_atom_indices)
             {
                 auto fluent_atom_indices = AtomIndexList {};
                 auto derived_atom_indices = AtomIndexList {};
                 self.inverse_remap_and_separate(fluent_and_derived_atom_indices, fluent_atom_indices, derived_atom_indices);
                 return std::make_tuple(fluent_atom_indices, derived_atom_indices);
             })
        .def("get_fluent_remap", &FluentAndDerivedMapper::get_fluent_remap, py::return_value_policy::reference)
        .def("get_derived_remap", &FluentAndDerivedMapper::get_derived_remap, py::return_value_policy::reference)
        .def("get_is_remapped_fluent", &FluentAndDerivedMapper::get_is_remapped_fluent, py::return_value_policy::reference)
        .def("get_inverse_remap", &FluentAndDerivedMapper::get_inverse_remap, py::return_value_policy::reference);
    py::class_<IIWAlgorithmEventHandler, std::shared_ptr<IIWAlgorithmEventHandler>>(m, "IIWAlgorithmEventHandler");
    py::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler, std::shared_ptr<DefaultIWAlgorithmEventHandler>>(m, "DefaultIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<IWAlgorithm, IAlgorithm, std::shared_ptr<IWAlgorithm>>(m, "IWAlgorithm")
        .def(py::init<std::shared_ptr<IAAG>, int>())
        .def(py::init<std::shared_ptr<IAAG>,
                      int,
                      std::shared_ptr<SuccessorStateGenerator>,
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
                      std::shared_ptr<SuccessorStateGenerator>,
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
        .def("get_forward_successor", &Transition::get_forward_successor)
        .def("get_backward_successor", &Transition::get_backward_successor)
        .def("get_creating_action", &Transition::get_creating_action);

    // StateSpace
    py::class_<StateSpace, std::shared_ptr<StateSpace>>(m, "StateSpace")  //
        .def_static(
            "create",
            [](const std::string& domain_filepath,
               const std::string& problem_filepath,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               uint32_t max_num_states,
               uint32_t timeout_ms)
            { return StateSpace::create(domain_filepath, problem_filepath, use_unit_cost_one, remove_if_unsolvable, max_num_states, timeout_ms); },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max())
        .def_static(
            "create",
            [](std::shared_ptr<PDDLParser> parser,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<SuccessorStateGenerator> ssg,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               uint32_t max_num_states,
               uint32_t timeout_ms) { return StateSpace::create(parser, aag, ssg, use_unit_cost_one, remove_if_unsolvable, max_num_states, timeout_ms); },
            py::arg("parser"),
            py::arg("aag"),
            py::arg("ssg"),
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max())
        .def_static(
            "create",
            [](const std::string& domain_filepath,
               const std::vector<std::string>& problem_filepaths,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads)
            {
                auto problem_filepaths_ = std::vector<fs::path>(problem_filepaths.begin(), problem_filepaths.end());
                return StateSpace::create(domain_filepath,
                                          problem_filepaths_,
                                          use_unit_cost_one,
                                          remove_if_unsolvable,
                                          sort_ascending_by_num_states,
                                          max_num_states,
                                          timeout_ms,
                                          num_threads);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def_static(
            "create",
            [](const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads) {
                return StateSpace::create(memories,
                                          use_unit_cost_one,
                                          remove_if_unsolvable,
                                          sort_ascending_by_num_states,
                                          max_num_states,
                                          timeout_ms,
                                          num_threads);
            },
            py::arg("memories"),
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def("compute_shortest_distances_from_states",
             &StateSpace::compute_shortest_distances_from_states,
             pybind11::arg("states"),
             pybind11::arg("forward") = true)
        .def("compute_pairwise_shortest_state_distances", &StateSpace::compute_pairwise_shortest_state_distances, pybind11::arg("forward") = true)
        .def("get_pddl_parser", &StateSpace::get_pddl_parser)
        .def("get_aag", &StateSpace::get_aag)
        .def("get_ssg", &StateSpace::get_ssg)
        .def("get_states", &StateSpace::get_states, py::return_value_policy::reference)
        .def("get_state_index", &StateSpace::get_state_index)
        .def("get_initial_state", &StateSpace::get_initial_state)
        .def("get_goal_states", &StateSpace::get_goal_states, py::return_value_policy::reference)
        .def("get_deadend_states", &StateSpace::get_deadend_states, py::return_value_policy::reference)
        .def("get_forward_successor_adjacency_lists", &StateSpace::get_forward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_successor_adjacency_lists", &StateSpace::get_backward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_states", &StateSpace::get_num_states)
        .def("get_num_goal_states", &StateSpace::get_num_goal_states)
        .def("get_num_deadend_states", &StateSpace::get_num_deadend_states)
        .def("is_goal_state", &StateSpace::is_goal_state)
        .def("is_deadend_state", &StateSpace::is_deadend_state)
        .def("is_alive_state", &StateSpace::is_alive_state)
        .def("get_transitions", &StateSpace::get_transitions, py::return_value_policy::reference)
        .def("get_transition_cost", &StateSpace::get_transition_cost)
        .def("get_forward_transition_adjacency_lists", &StateSpace::get_forward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_transition_adjacency_lists", &StateSpace::get_backward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_transitions", &StateSpace::get_num_transitions)
        .def("get_goal_distances", &StateSpace::get_goal_distances, py::return_value_policy::reference)
        .def("get_goal_distance", &StateSpace::get_goal_distance)
        .def("get_max_goal_distance", &StateSpace::get_max_goal_distance)
        .def("sample_state_with_goal_distance", &StateSpace::sample_state_with_goal_distance, py::return_value_policy::reference);

    // Abstraction
    py::class_<Abstraction, std::shared_ptr<Abstraction>>(m, "Abstraction")  //
        .def(py::init<FaithfulAbstraction>())
        .def(py::init<GlobalFaithfulAbstraction>())
        .def("get_pddl_parser", &Abstraction::get_pddl_parser)
        .def("get_aag", &Abstraction::get_aag)
        .def("get_ssg", &Abstraction::get_ssg)
        .def("get_abstract_state_index", &Abstraction::get_abstract_state_index)
        .def("get_initial_state", &Abstraction::get_initial_state)
        .def("get_goal_states", &Abstraction::get_goal_states, py::return_value_policy::reference)
        .def("get_deadend_states", &Abstraction::get_deadend_states, py::return_value_policy::reference)
        .def("get_forward_successor_adjacency_lists", &Abstraction::get_forward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_successor_adjacency_lists", &Abstraction::get_backward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_states", &Abstraction::get_num_states)
        .def("get_num_goal_states", &Abstraction::get_num_goal_states)
        .def("get_num_deadend_states", &Abstraction::get_num_deadend_states)
        .def("is_goal_state", &Abstraction::is_goal_state)
        .def("is_deadend_state", &Abstraction::is_deadend_state)
        .def("is_alive_state", &Abstraction::is_alive_state)
        .def("get_transition_cost", &Abstraction::get_transition_cost)
        .def("get_forward_transition_adjacency_lists", &Abstraction::get_forward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_transition_adjacency_lists", &Abstraction::get_backward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_transitions", &Abstraction::get_num_transitions)
        .def("get_goal_distances", &Abstraction::get_goal_distances, py::return_value_policy::reference);

    // FaithfulAbstraction
    py::class_<FaithfulAbstractState>(m, "FaithfulAbstractState")
        .def("get_index", &FaithfulAbstractState::get_index)
        .def("get_state", &FaithfulAbstractState::get_state)
        .def("get_certificate", &FaithfulAbstractState::get_certificate);

    py::class_<FaithfulAbstraction, std::shared_ptr<FaithfulAbstraction>>(m, "FaithfulAbstraction")
        .def_static(
            "create",
            [](const std::string& domain_filepath,
               const std::string& problem_filepath,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               uint32_t max_num_states,
               uint32_t timeout_ms)
            {
                return FaithfulAbstraction::create(domain_filepath,
                                                   problem_filepath,
                                                   mark_true_goal_literals,
                                                   use_unit_cost_one,
                                                   remove_if_unsolvable,
                                                   compute_complete_abstraction_mapping,
                                                   max_num_states,
                                                   timeout_ms);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepath"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max())
        .def_static(
            "create",
            [](std::shared_ptr<PDDLParser> parser,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<SuccessorStateGenerator> ssg,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               uint32_t max_num_states,
               uint32_t timeout_ms) {
                return FaithfulAbstraction::create(parser,
                                                   aag,
                                                   ssg,
                                                   mark_true_goal_literals,
                                                   use_unit_cost_one,
                                                   remove_if_unsolvable,
                                                   max_num_states,
                                                   timeout_ms);
            },
            py::arg("parser"),
            py::arg("aag"),
            py::arg("ssg"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max())
        .def_static(
            "create",
            [](const std::string& domain_filepath,
               const std::vector<std::string>& problem_filepaths,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads)
            {
                auto problem_filepaths_ = std::vector<fs::path>(problem_filepaths.begin(), problem_filepaths.end());
                return FaithfulAbstraction::create(domain_filepath,
                                                   problem_filepaths_,
                                                   mark_true_goal_literals,
                                                   use_unit_cost_one,
                                                   remove_if_unsolvable,
                                                   compute_complete_abstraction_mapping,
                                                   sort_ascending_by_num_states,
                                                   max_num_states,
                                                   timeout_ms,
                                                   num_threads);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def_static(
            "create",
            [](const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads)
            {
                return FaithfulAbstraction::create(memories,
                                                   mark_true_goal_literals,
                                                   use_unit_cost_one,
                                                   remove_if_unsolvable,
                                                   compute_complete_abstraction_mapping,
                                                   sort_ascending_by_num_states,
                                                   max_num_states,
                                                   timeout_ms,
                                                   num_threads);
            },
            py::arg("memories"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def("compute_shortest_distances_from_states", &FaithfulAbstraction::compute_shortest_distances_from_states)
        .def("get_pddl_parser", &FaithfulAbstraction::get_pddl_parser)
        .def("get_aag", &FaithfulAbstraction::get_aag)
        .def("get_ssg", &FaithfulAbstraction::get_ssg)
        .def("get_abstract_state_index", &FaithfulAbstraction::get_abstract_state_index)
        .def("get_states", &FaithfulAbstraction::get_states, py::return_value_policy::reference)
        .def("get_states_by_certificate", &FaithfulAbstraction::get_states_by_certificate, py::return_value_policy::reference)
        .def("get_initial_state", &FaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &FaithfulAbstraction::get_goal_states, py::return_value_policy::reference)
        .def("get_deadend_states", &FaithfulAbstraction::get_deadend_states, py::return_value_policy::reference)
        .def("get_forward_successor_adjacency_lists", &FaithfulAbstraction::get_forward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_successor_adjacency_lists", &FaithfulAbstraction::get_backward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_states", &FaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &FaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &FaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &FaithfulAbstraction::is_goal_state)
        .def("is_deadend_state", &FaithfulAbstraction::is_deadend_state)
        .def("is_alive_state", &FaithfulAbstraction::is_alive_state)
        .def("get_transitions", &FaithfulAbstraction::get_transitions, py::return_value_policy::reference)
        .def("get_transition_cost", &FaithfulAbstraction::get_transition_cost)
        .def("get_forward_transition_adjacency_lists", &FaithfulAbstraction::get_forward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_transition_adjacency_lists", &FaithfulAbstraction::get_backward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_transitions", &FaithfulAbstraction::get_num_transitions)
        .def("get_goal_distances", &FaithfulAbstraction::get_goal_distances, py::return_value_policy::reference);

    // GlobalFaithfulAbstraction

    py::class_<GlobalFaithfulAbstractState>(m, "GlobalFaithfulAbstractState")
        .def("__eq__", &GlobalFaithfulAbstractState::operator==)
        .def("__hash__", &GlobalFaithfulAbstractState::hash)
        .def("get_id", &GlobalFaithfulAbstractState::get_id)
        .def("get_abstraction_index", &GlobalFaithfulAbstractState::get_abstraction_index)
        .def("get_abstract_state_id", &GlobalFaithfulAbstractState::get_abstract_state_id);

    py::class_<GlobalFaithfulAbstraction, std::shared_ptr<GlobalFaithfulAbstraction>>(m, "GlobalFaithfulAbstraction")
        .def_static(
            "create",
            [](const std::string& domain_filepath,
               const std::vector<std::string>& problem_filepaths,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads)
            {
                std::vector<fs::path> problem_filepaths_(problem_filepaths.begin(), problem_filepaths.end());
                return GlobalFaithfulAbstraction::create(domain_filepath,
                                                         problem_filepaths_,
                                                         mark_true_goal_literals,
                                                         use_unit_cost_one,
                                                         remove_if_unsolvable,
                                                         compute_complete_abstraction_mapping,
                                                         sort_ascending_by_num_states,
                                                         max_num_states,
                                                         timeout_ms,
                                                         num_threads);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def_static(
            "create",
            [](const std::vector<std::tuple<std::shared_ptr<PDDLParser>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>& memories,
               bool mark_true_goal_literals,
               bool use_unit_cost_one,
               bool remove_if_unsolvable,
               bool compute_complete_abstraction_mapping,
               bool sort_ascending_by_num_states,
               uint32_t max_num_states,
               uint32_t timeout_ms,
               uint32_t num_threads)
            {
                return GlobalFaithfulAbstraction::create(memories,
                                                         mark_true_goal_literals,
                                                         use_unit_cost_one,
                                                         remove_if_unsolvable,
                                                         compute_complete_abstraction_mapping,
                                                         sort_ascending_by_num_states,
                                                         max_num_states,
                                                         timeout_ms,
                                                         num_threads);
            },
            py::arg("memories"),
            py::arg("mark_true_goal_literals") = false,
            py::arg("use_unit_cost_one") = true,
            py::arg("remove_if_unsolvable") = true,
            py::arg("compute_complete_abstraction_mapping") = false,
            py::arg("sort_ascending_by_num_states") = true,
            py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
            py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
            py::arg("num_threads") = std::thread::hardware_concurrency())
        .def("get_index", &GlobalFaithfulAbstraction::get_index)
        .def("get_pddl_parser", &GlobalFaithfulAbstraction::get_pddl_parser)
        .def("get_aag", &GlobalFaithfulAbstraction::get_aag)
        .def("get_ssg", &GlobalFaithfulAbstraction::get_ssg)
        .def("get_abstractions", &GlobalFaithfulAbstraction::get_abstractions, py::return_value_policy::reference)
        .def("get_abstract_state_index", &GlobalFaithfulAbstraction::get_abstract_state_index)
        .def("get_states", &GlobalFaithfulAbstraction::get_states, py::return_value_policy::reference)
        .def("get_state_index", &GlobalFaithfulAbstraction::get_state_index)
        .def("get_states_by_certificate", &GlobalFaithfulAbstraction::get_states_by_certificate, py::return_value_policy::reference)
        .def("get_initial_state", &GlobalFaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &GlobalFaithfulAbstraction::get_goal_states, py::return_value_policy::reference)
        .def("get_deadend_states", &GlobalFaithfulAbstraction::get_deadend_states, py::return_value_policy::reference)
        .def("get_forward_successor_adjacency_lists", &GlobalFaithfulAbstraction::get_forward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_successor_adjacency_lists", &GlobalFaithfulAbstraction::get_backward_successor_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_states", &GlobalFaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &GlobalFaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &GlobalFaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &GlobalFaithfulAbstraction::is_goal_state)
        .def("is_deadend_state", &GlobalFaithfulAbstraction::is_deadend_state)
        .def("is_alive_state", &GlobalFaithfulAbstraction::is_alive_state)
        .def("get_num_isomorphic_states", &GlobalFaithfulAbstraction::get_num_isomorphic_states)
        .def("get_num_non_isomorphic_states", &GlobalFaithfulAbstraction::get_num_non_isomorphic_states)

        .def("get_transitions", &GlobalFaithfulAbstraction::get_transitions, py::return_value_policy::reference)
        .def("get_transition_cost", &GlobalFaithfulAbstraction::get_transition_cost)
        .def("get_forward_transition_adjacency_lists", &GlobalFaithfulAbstraction::get_forward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_backward_transition_adjacency_lists", &GlobalFaithfulAbstraction::get_backward_transition_adjacency_lists, py::return_value_policy::reference)
        .def("get_num_transitions", &GlobalFaithfulAbstraction::get_num_transitions)
        .def("get_goal_distances", &GlobalFaithfulAbstraction::get_goal_distances, py::return_value_policy::reference);

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Graphs
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // TupleGraphVertex
    py::class_<TupleGraphVertex>(m, "TupleGraphVertex")  //
        .def("get_identifier", &TupleGraphVertex::get_index)
        .def("get_tuple_index", &TupleGraphVertex::get_tuple_index)
        .def("get_states", &TupleGraphVertex::get_states, py::return_value_policy::reference);

    // TupleGraph
    py::class_<TupleGraph>(m, "TupleGraph")  //
        .def("__str__",
             [](const TupleGraph& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def("compute_admissible_chain",
             py::overload_cast<const GroundAtomList<Fluent>&, const GroundAtomList<Derived>&>(&TupleGraph::compute_admissible_chain))
        .def("compute_admissible_chain", py::overload_cast<const StateList&>(&TupleGraph::compute_admissible_chain))
        .def("get_state_space", &TupleGraph::get_state_space)
        .def("get_tuple_index_mapper", &TupleGraph::get_tuple_index_mapper)
        .def("get_atom_index_mapper", &TupleGraph::get_atom_index_mapper)
        .def("get_root_state", &TupleGraph::get_root_state)
        .def("get_vertices", &TupleGraph::get_vertices, py::return_value_policy::reference)
        .def("get_forward_successors", &TupleGraph::get_forward_successors, py::return_value_policy::reference)
        .def("get_backward_successors", &TupleGraph::get_backward_successors, py::return_value_policy::reference)
        .def("get_vertex_indices_by_distances", &TupleGraph::get_vertex_indices_by_distances, py::return_value_policy::reference)
        .def("get_states_by_distance", &TupleGraph::get_states_by_distance, py::return_value_policy::reference);

    // TupleGraphFactory
    py::class_<TupleGraphFactory>(m, "TupleGraphFactory")  //
        .def(py::init<std::shared_ptr<StateSpace>, int, bool>(), py::arg("state_space"), py::arg("arity"), py::arg("prune_dominated_tuples") = false)
        .def("create", &TupleGraphFactory::create, py::return_value_policy::copy)
        .def("get_state_space", &TupleGraphFactory::get_state_space)
        .def("get_atom_index_mapper", &TupleGraphFactory::get_atom_index_mapper)
        .def("get_tuple_index_mapper", &TupleGraphFactory::get_tuple_index_mapper);

    // Partitioning
    py::class_<Partitioning>(m, "Partitioning")
        .def("get_vertex_index_permutation", &Partitioning::get_vertex_index_permutation, py::return_value_policy::reference)
        .def("get_partitioning", &Partitioning::get_partitioning, py::return_value_policy::reference)
        .def("get_partition", &Partitioning::get_partition, py::return_value_policy::reference);

    // Certificate
    py::class_<Certificate>(m, "Certificate")
        .def(py::init<std::string, ColorList>())
        .def("__eq__", &Certificate::operator==)
        .def("__hash__", &Certificate::hash)
        .def("get_nauty_certificate", &Certificate::get_nauty_certificate, py::return_value_policy::reference)
        .def("get_canonical_initial_coloring", &Certificate::get_canonical_initial_coloring, py::return_value_policy::reference);

    // NautyGraph
    py::class_<nauty_wrapper::Graph>(m, "NautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>())
        .def("add_edge", &nauty_wrapper::Graph::add_edge)
        .def("compute_certificate", &nauty_wrapper::Graph::compute_certificate)
        .def("reset", &nauty_wrapper::Graph::reset);

    // Digraph
    py::class_<Digraph>(m, "Digraph")  //
        .def(py::init<bool>(), py::arg("is_directed") = false)
        .def(py::init<int, bool>(), py::arg("num_vertices"), py::arg("is_directed") = false)
        .def("add_edge", &Digraph::add_edge)
        .def("to_nauty_graph", &Digraph::to_nauty_graph)
        .def("reset", &Digraph::reset, py::arg("num_vertices"), py::arg("is_directed") = false)
        .def("get_num_vertices", &Digraph::get_num_vertices)
        .def("get_num_edges", &Digraph::get_num_edges)
        .def("get_forward_successors", &Digraph::get_forward_successors)
        .def("get_backward_successors", &Digraph::get_backward_successors);

    // ProblemColorFunction
    py::class_<ProblemColorFunction, std::shared_ptr<ProblemColorFunction>>(m, "ProblemColorFunction")  //
        .def(py::init<Problem>(), py::arg("problem"));

    // ObjectGraph
    py::class_<ObjectGraph>(m, "ObjectGraph")  //
        .def(py::init<std::shared_ptr<ProblemColorFunction>>())
        .def("get_coloring_function", &ObjectGraph::get_coloring_function)
        .def("get_digraph", &ObjectGraph::get_digraph, py::return_value_policy::reference)
        .def("get_vertex_colors", &ObjectGraph::get_vertex_colors, py::return_value_policy::reference)
        .def("get_sorted_vertex_colors", &ObjectGraph::get_sorted_vertex_colors, py::return_value_policy::reference)
        .def("get_partitioning", &ObjectGraph::get_partitioning, py::return_value_policy::reference);

    // ObjectGraph
    py::class_<ObjectGraphFactory>(m, "ObjectGraphFactory")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, bool>(),
             py::arg("problem"),
             py::arg("pddl_factories"),
             py::arg("mark_true_goal_literals") = false)
        .def("create", &ObjectGraphFactory::create, py::return_value_policy::copy);
}
