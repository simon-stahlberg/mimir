#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"

#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>  // Necessary for automatic conversion of e.g. std::vectors
#include <pybind11/stl_bind.h>

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

using TermVariantList = std::vector<TermVariant>;

struct FunctionExpressionVariant
{
    FunctionExpression function_expression;
    explicit FunctionExpressionVariant(const FunctionExpression& e) : function_expression(e) {}
};

using FunctionExpressionVariantList = std::vector<FunctionExpressionVariant>;

struct GroundFunctionExpressionVariant
{
    GroundFunctionExpression function_expression;
    explicit GroundFunctionExpressionVariant(const GroundFunctionExpression& e) : function_expression(e) {}
};

using GroundFunctionExpressionVariantList = std::vector<GroundFunctionExpressionVariant>;

TermVariantList to_term_variant_list(const TermList& terms)
{
    auto result = TermVariantList {};
    result.reserve(terms.size());
    for (const auto& term : terms)
    {
        result.push_back(TermVariant(term));
    }
    return result;
}

FunctionExpressionVariantList to_function_expression_variant_list(const FunctionExpressionList& function_expressions)
{
    auto result = FunctionExpressionVariantList {};
    result.reserve(function_expressions.size());
    for (const auto& function_expression : function_expressions)
    {
        result.push_back(FunctionExpressionVariant(function_expression));
    }
    return result;
}

GroundFunctionExpressionVariantList to_ground_function_expression_variant_list(const GroundFunctionExpressionList& ground_function_expressions)
{
    auto result = GroundFunctionExpressionVariantList {};
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
 * Opaque types
 */

/* Formalism */
PYBIND11_MAKE_OPAQUE(ActionList);
PYBIND11_MAKE_OPAQUE(AtomList<Static>);
PYBIND11_MAKE_OPAQUE(AtomList<Fluent>);
PYBIND11_MAKE_OPAQUE(AtomList<Derived>);
PYBIND11_MAKE_OPAQUE(AxiomList);
PYBIND11_MAKE_OPAQUE(DomainList);
PYBIND11_MAKE_OPAQUE(EffectSimpleList);
PYBIND11_MAKE_OPAQUE(EffectConditionalList);
PYBIND11_MAKE_OPAQUE(EffectUniversalList);
PYBIND11_MAKE_OPAQUE(FunctionExpressionVariantList);
PYBIND11_MAKE_OPAQUE(FunctionSkeletonList);
PYBIND11_MAKE_OPAQUE(FunctionList);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Static>);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Fluent>);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Derived>);
PYBIND11_MAKE_OPAQUE(GroundFunctionExpressionVariantList);
PYBIND11_MAKE_OPAQUE(GroundLiteralList<Static>);
PYBIND11_MAKE_OPAQUE(GroundLiteralList<Fluent>);
PYBIND11_MAKE_OPAQUE(GroundLiteralList<Derived>);
PYBIND11_MAKE_OPAQUE(LiteralList<Static>);
PYBIND11_MAKE_OPAQUE(LiteralList<Fluent>);
PYBIND11_MAKE_OPAQUE(LiteralList<Derived>);
PYBIND11_MAKE_OPAQUE(NumericFluentList);
PYBIND11_MAKE_OPAQUE(ObjectList);
PYBIND11_MAKE_OPAQUE(PredicateList<Static>);
PYBIND11_MAKE_OPAQUE(PredicateList<Fluent>);
PYBIND11_MAKE_OPAQUE(PredicateList<Derived>);
PYBIND11_MAKE_OPAQUE(ToPredicateMap<std::string, Static>);
PYBIND11_MAKE_OPAQUE(ToPredicateMap<std::string, Fluent>);
PYBIND11_MAKE_OPAQUE(ToPredicateMap<std::string, Derived>);
PYBIND11_MAKE_OPAQUE(ProblemList);
PYBIND11_MAKE_OPAQUE(TermList);
PYBIND11_MAKE_OPAQUE(VariableList);

/* Search */
PYBIND11_MAKE_OPAQUE(StateList);
PYBIND11_MAKE_OPAQUE(GroundActionList);

/**
 * Common
 */

/// @brief Binds a std::span<T> as an unmodifiable python object.
/// Modifiable std::span are more complicated.
/// Hence, we use std::span exclusively for unmodifiable data,
/// and std::vector for modifiable data.
template<typename Span, typename holder_type = std::unique_ptr<Span>>
py::class_<Span, holder_type> bind_const_span(py::handle m, const std::string& name)
{
    py::class_<Span, holder_type> cl(m, name.c_str());

    using T = typename Span::value_type;
    using SizeType = typename Span::size_type;
    using DiffType = typename Span::difference_type;
    using ItType = typename Span::iterator;

    auto wrap_i = [](DiffType i, SizeType n)
    {
        if (i < 0)
        {
            i += n;
        }
        if (i < 0 || (SizeType) i >= n)
        {
            throw py::index_error();
        }
        return i;
    };

    cl.def(
        "__getitem__",
        [wrap_i](const Span& v, DiffType i) -> const T&
        {
            i = wrap_i(i, v.size());
            return v[(SizeType) i];
        },
        py::return_value_policy::reference_internal  // ref + keepalive
    );

    cl.def(
        "__iter__",
        [](const Span& v) { return py::make_iterator<py::return_value_policy::reference_internal, ItType, ItType, const T&>(v.begin(), v.end()); },
        py::keep_alive<0, 1>() /* Essential: keep span alive while iterator exists */
    );

    cl.def("__len__", &Span::size);

    return cl;
}

/// @brief Binds a IndexGroupedVector as an unmodifiable python object.
/// Modifiable IndexGroupedVector are more complicated because they use std::span.
/// See section regarding bind_const_span.
template<typename IndexGroupedVector, typename holder_type = std::unique_ptr<IndexGroupedVector>>
py::class_<IndexGroupedVector, holder_type> bind_const_index_grouped_vector(py::handle m, const std::string& name)
{
    py::class_<IndexGroupedVector, holder_type> cl(m, name.c_str());

    using T = typename IndexGroupedVector::value_type;
    using SizeType = typename IndexGroupedVector::size_type;
    using DiffType = typename IndexGroupedVector::difference_type;
    using ItType = typename IndexGroupedVector::const_iterator;

    auto wrap_i = [](DiffType i, SizeType n)
    {
        if (i < 0)
        {
            i += n;
        }
        if (i < 0 || (SizeType) i >= n)
        {
            throw py::index_error();
        }
        return i;
    };

    /**
     * Accessors
     */

    /* This requires std::vector<T> to be an opague type
       because weak references to python lists are not allowed.
    */
    cl.def(
        "__getitem__",
        [wrap_i](const IndexGroupedVector& v, DiffType i) -> T
        {
            i = wrap_i(i, v.size());
            return v[(SizeType) i];
        },
        py::return_value_policy::copy,
        py::keep_alive<0, 1>()); /* Essential keep span alive while copy of element exists */

    cl.def(
        "__iter__",
        [](const IndexGroupedVector& v)
        {
            return py::make_iterator<py::return_value_policy::copy, ItType, ItType, T>(
                v.begin(),
                v.end(),
                py::return_value_policy::copy,
                py::keep_alive<0, 1>()); /* Essential: keep iterator alive while copy of element exists. */
        },
        py::keep_alive<0, 1>() /* Essential: keep index grouped vector alive while iterator exists */
    );

    cl.def("__len__", &IndexGroupedVector::size);

    return cl;
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
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::reference_internal);

    py::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ObjectImpl>::str, py::const_))
        .def("get_identifier", &ObjectImpl::get_identifier)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<ObjectList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<ObjectList>(m, "ObjectList");

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<VariableImpl>::str, py::const_))
        .def("get_identifier", &VariableImpl::get_identifier)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<VariableList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<VariableList>(m, "VariableList");

    py::class_<TermObjectImpl>(m, "TermObject")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermObjectImpl>::str, py::const_))
        .def("get_identifier", &TermObjectImpl::get_identifier)
        .def("get_object", &TermObjectImpl::get_object, py::return_value_policy::reference_internal);

    py::class_<TermVariableImpl>(m, "TermVariable")  //
        .def("__str__", py::overload_cast<>(&loki::Base<TermVariableImpl>::str, py::const_))
        .def("get_identifier", &TermVariableImpl::get_identifier)
        .def("get_variable", &TermVariableImpl::get_variable, py::return_value_policy::reference_internal);

    py::class_<TermVariant>(m, "Term")  //
        .def(
            "get",
            [](const TermVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.term); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<TermVariantList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<TermVariantList>(m, "TermVariantList");

    py::class_<PredicateImpl<Static>>(m, "StaticPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Static>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Static>::get_identifier)
        .def("get_name", &PredicateImpl<Static>::get_name, py::return_value_policy::reference_internal)
        .def(
            "get_parameters",
            [](const PredicateImpl<Static>& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &PredicateImpl<Static>::get_arity);
    static_assert(!py::detail::vector_needs_copy<PredicateList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<PredicateList<Static>>(m, "StaticPredicateList");
    py::bind_map<ToPredicateMap<std::string, Static>>(m, "StringToStaticPredicateMap");

    py::class_<PredicateImpl<Fluent>>(m, "FluentPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Fluent>::get_identifier)
        .def("get_name", &PredicateImpl<Fluent>::get_name, py::return_value_policy::reference_internal)
        .def(
            "get_parameters",
            [](const PredicateImpl<Fluent>& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &PredicateImpl<Fluent>::get_arity);
    static_assert(!py::detail::vector_needs_copy<PredicateList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<PredicateList<Fluent>>(m, "FluentPredicateList");
    py::bind_map<ToPredicateMap<std::string, Fluent>>(m, "StringToFluentPredicateMap");

    py::class_<PredicateImpl<Derived>>(m, "DerivedPredicate")  //
        .def("__str__", py::overload_cast<>(&loki::Base<PredicateImpl<Derived>>::str, py::const_))
        .def("get_identifier", &PredicateImpl<Derived>::get_identifier)
        .def("get_name", &PredicateImpl<Derived>::get_name, py::return_value_policy::reference_internal)
        .def(
            "get_parameters",
            [](const PredicateImpl<Derived>& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &PredicateImpl<Derived>::get_arity);
    static_assert(!py::detail::vector_needs_copy<PredicateList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<PredicateList<Derived>>(m, "DerivedPredicateList");
    py::bind_map<ToPredicateMap<std::string, Derived>>(m, "StringToDerivedPredicateMap");

    py::class_<AtomImpl<Static>>(m, "StaticAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Static>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Static>::get_identifier)
        .def("get_predicate", &AtomImpl<Static>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Static>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Static>>(m, "StaticAtomList");

    py::class_<AtomImpl<Fluent>>(m, "FluentAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Fluent>::get_identifier)
        .def("get_predicate", &AtomImpl<Fluent>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Fluent>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Fluent>>(m, "FluentAtomList");

    py::class_<AtomImpl<Derived>>(m, "DerivedAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AtomImpl<Derived>>::str, py::const_))
        .def("get_identifier", &AtomImpl<Derived>::get_identifier)
        .def("get_predicate", &AtomImpl<Derived>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Derived>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Derived>>(m, "DerivedAtomList");

    py::class_<FunctionSkeletonImpl>(m, "FunctionSkeleton")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionSkeletonImpl>::str, py::const_))
        .def("get_identifier", &FunctionSkeletonImpl::get_identifier)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference_internal)
        .def(
            "get_parameters",
            [](const FunctionSkeletonImpl& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionSkeletonList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<FunctionSkeletonList>(m, "FunctionSkeletonList");

    py::class_<FunctionImpl>(m, "Function")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionImpl::get_identifier)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const FunctionImpl& self) { return to_term_variant_list(self.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<FunctionList>(m, "FunctionList");

    py::class_<GroundFunctionImpl>(m, "GroundFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionImpl::get_identifier)
        .def("get_function_skeleton", &GroundFunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundFunctionImpl& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundFunctionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundFunctionList>(m, "GroundFunctionList");

    py::class_<GroundAtomImpl<Static>>(m, "StaticGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Static>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Static>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Static>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Static>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Static>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Static>>(m, "StaticGroundAtomList");

    py::class_<GroundAtomImpl<Fluent>>(m, "FluentGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Fluent>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Fluent>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Fluent>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Fluent>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Fluent>>(m, "FluentGroundAtomList");

    py::class_<GroundAtomImpl<Derived>>(m, "DerivedGroundAtom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundAtomImpl<Derived>>::str, py::const_))
        .def("get_identifier", &GroundAtomImpl<Derived>::get_identifier)
        .def("get_arity", &GroundAtomImpl<Derived>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Derived>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Derived>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Derived>>(m, "DerivedGroundAtomList");

    py::class_<GroundLiteralImpl<Static>>(m, "StaticGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Static>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Static>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Static>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Static>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Static>>(m, "StaticGroundLiteralList");

    py::class_<GroundLiteralImpl<Fluent>>(m, "FluentGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Fluent>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Fluent>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Fluent>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Fluent>>(m, "FluentGroundLiteralList");

    py::class_<GroundLiteralImpl<Derived>>(m, "DerivedGroundLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundLiteralImpl<Derived>>::str, py::const_))
        .def("get_identifier", &GroundLiteralImpl<Derived>::get_identifier)
        .def("get_atom", &GroundLiteralImpl<Derived>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Derived>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Derived>>(m, "DerivedGroundLiteralList");

    py::class_<LiteralImpl<Static>>(m, "StaticLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Static>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Static>::get_identifier)
        .def("get_atom", &LiteralImpl<Static>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Static>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Static>>(m, "StaticLiteralList");

    py::class_<LiteralImpl<Fluent>>(m, "FluentLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Fluent>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Fluent>::get_identifier)
        .def("get_atom", &LiteralImpl<Fluent>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Fluent>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Fluent>>(m, "FluentLiteralList");

    py::class_<LiteralImpl<Derived>>(m, "DerivedLiteral")  //
        .def("__str__", py::overload_cast<>(&loki::Base<LiteralImpl<Derived>>::str, py::const_))
        .def("get_identifier", &LiteralImpl<Derived>::get_identifier)
        .def("get_atom", &LiteralImpl<Derived>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Derived>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Derived>>(m, "DerivedLiteralList");

    py::class_<NumericFluentImpl>(m, "NumericFluent")  //
        .def("__str__", py::overload_cast<>(&loki::Base<NumericFluentImpl>::str, py::const_))
        .def("get_identifier", &NumericFluentImpl::get_identifier)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference_internal)
        .def("get_number", &NumericFluentImpl::get_number);
    static_assert(!py::detail::vector_needs_copy<NumericFluentList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<NumericFluentList>(m, "NumericFluentList");

    py::class_<EffectSimpleImpl>(m, "EffectSimple")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectSimpleImpl>::str, py::const_))
        .def("get_identifier", &EffectSimpleImpl::get_identifier)
        .def("get_effect", &EffectSimpleImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectSimpleList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectSimpleList>(m, "EffectSimpleList");

    py::class_<EffectConditionalImpl>(m, "EffectConditional")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectConditionalImpl>::str, py::const_))
        .def("get_identifier", &EffectConditionalImpl::get_identifier)
        .def(
            "get_static_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Static>(self.get_conditions<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Fluent>(self.get_conditions<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Derived>(self.get_conditions<Derived>()); },
            py::keep_alive<0, 1>())
        .def("get_effect", &EffectConditionalImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectConditionalList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectConditionalList>(m, "EffectConditionalList");

    py::class_<FunctionExpressionVariant>(m, "FunctionExpression")  //
        .def(
            "get",
            [](const FunctionExpressionVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.function_expression); },
            py::keep_alive<0, 1>());
    ;
    static_assert(!py::detail::vector_needs_copy<FunctionExpressionVariantList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<FunctionExpressionVariantList>(m, "FunctionExpressionVariantList");

    py::class_<EffectUniversalImpl>(m, "EffectUniversal")  //
        .def("__str__", py::overload_cast<>(&loki::Base<EffectUniversalImpl>::str, py::const_))
        .def("get_identifier", &EffectUniversalImpl::get_identifier)
        .def(
            "get_parameters",
            [](const EffectUniversalImpl& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def(
            "get_static_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Static>(self.get_conditions<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Fluent>(self.get_conditions<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_conditions",
            [](const EffectUniversalImpl& self) { return LiteralList<Derived>(self.get_conditions<Derived>()); },
            py::keep_alive<0, 1>())
        .def("get_effect", &EffectUniversalImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectUniversalList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectUniversalList>(m, "EffectUniversalList");

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
            py::keep_alive<0, 1>())
        .def(
            "get_right_function_expression",
            [](const FunctionExpressionBinaryOperatorImpl& function_expression)
            { return FunctionExpressionVariant(function_expression.get_right_function_expression()); },
            py::keep_alive<0, 1>());

    py::class_<FunctionExpressionMultiOperatorImpl>(m, "FunctionExpressionMultiOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMultiOperatorImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const FunctionExpressionMultiOperatorImpl& function_expression)
            { return to_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::keep_alive<0, 1>());

    py::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionMinusImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionMinusImpl::get_identifier)
        .def(
            "get_function_expression",
            [](const FunctionExpressionMinusImpl& function_expression) { return FunctionExpressionVariant(function_expression.get_function_expression()); },
            py::keep_alive<0, 1>());
    ;

    py::class_<FunctionExpressionFunctionImpl>(m, "FunctionExpressionFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<FunctionExpressionFunctionImpl>::str, py::const_))
        .def("get_identifier", &FunctionExpressionFunctionImpl::get_identifier)
        .def("get_function", &FunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference_internal);

    py::class_<GroundFunctionExpressionVariant>(m, "GroundFunctionExpression")  //
        .def(
            "get",
            [](const GroundFunctionExpressionVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.function_expression); },
            py::keep_alive<0, 1>());
    ;
    static_assert(!py::detail::vector_needs_copy<GroundFunctionExpressionVariantList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundFunctionExpressionVariantList>(m, "GroundFunctionExpressionVariantList");

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
            py::keep_alive<0, 1>())
        .def(
            "get_right_function_expression",
            [](const GroundFunctionExpressionBinaryOperatorImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_right_function_expression()); },
            py::keep_alive<0, 1>());

    py::class_<GroundFunctionExpressionMultiOperatorImpl>(m, "GroundFunctionExpressionMultiOperator")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionMultiOperatorImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionMultiOperatorImpl::get_identifier)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const GroundFunctionExpressionMultiOperatorImpl& function_expression)
            { return to_ground_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::keep_alive<0, 1>());

    py::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionMinusImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionMinusImpl::get_identifier)
        .def(
            "get_function_expression",
            [](const GroundFunctionExpressionMinusImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_function_expression()); },
            py::keep_alive<0, 1>());
    ;

    py::class_<GroundFunctionExpressionFunctionImpl>(m, "GroundFunctionExpressionFunction")  //
        .def("__str__", py::overload_cast<>(&loki::Base<GroundFunctionExpressionFunctionImpl>::str, py::const_))
        .def("get_identifier", &GroundFunctionExpressionFunctionImpl::get_identifier)
        .def("get_function", &GroundFunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference_internal);

    py::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", py::overload_cast<>(&loki::Base<OptimizationMetricImpl>::str, py::const_))
        .def("get_identifier", &OptimizationMetricImpl::get_identifier)
        .def("get_function_expression", [](const OptimizationMetricImpl& metric) { return GroundFunctionExpressionVariant(metric.get_function_expression()); })
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference_internal);

    py::class_<ActionImpl>(m, "Action")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ActionImpl>::str, py::const_))
        .def("get_identifier", &ActionImpl::get_identifier)
        .def("get_name", &ActionImpl::get_name, py::return_value_policy::copy)
        .def(
            "get_parameters",
            [](const ActionImpl& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def(
            "get_static_conditions",
            [](const ActionImpl& self) { return LiteralList<Static>(self.get_conditions<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_conditions",
            [](const ActionImpl& self) { return LiteralList<Fluent>(self.get_conditions<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_conditions",
            [](const ActionImpl& self) { return LiteralList<Derived>(self.get_conditions<Derived>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_simple_effects",
            [](const ActionImpl& self) { return EffectSimpleList(self.get_simple_effects()); },
            py::keep_alive<0, 1>())
        .def(
            "get_conditional_effects",
            [](const ActionImpl& self) { return EffectConditionalList(self.get_conditional_effects()); },
            py::keep_alive<0, 1>())
        .def(
            "get_universal_effects",
            [](const ActionImpl& self) { return EffectUniversalList(self.get_universal_effects()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &ActionImpl::get_arity);
    static_assert(!py::detail::vector_needs_copy<ActionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<ActionList>(m, "ActionList");

    py::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", py::overload_cast<>(&loki::Base<AxiomImpl>::str, py::const_))
        .def("get_identifier", &AxiomImpl::get_identifier)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference_internal)
        .def(
            "get_static_conditions",
            [](const AxiomImpl& self) { return LiteralList<Static>(self.get_conditions<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_conditions",
            [](const AxiomImpl& self) { return LiteralList<Fluent>(self.get_conditions<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_conditions",
            [](const AxiomImpl& self) { return LiteralList<Derived>(self.get_conditions<Derived>()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &AxiomImpl::get_arity);
    static_assert(!py::detail::vector_needs_copy<AxiomList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AxiomList>(m, "AxiomList");

    py::class_<DomainImpl>(m, "Domain")  //
        .def("__str__", py::overload_cast<>(&loki::Base<DomainImpl>::str, py::const_))
        .def("get_identifier", &DomainImpl::get_identifier)
        .def(
            "get_filepath",
            [](const DomainImpl& self) { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; },
            py::return_value_policy::copy)
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::copy)
        .def(
            "get_constants",
            [](const DomainImpl& self) { return ObjectList(self.get_constants()); },
            py::keep_alive<0, 1>())
        .def(
            "get_static_predicates",
            [](const DomainImpl& self) { return PredicateList<Static>(self.get_predicates<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_predicates",
            [](const DomainImpl& self) { return PredicateList<Fluent>(self.get_predicates<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_predicates",
            [](const DomainImpl& self) { return PredicateList<Derived>(self.get_predicates<Derived>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_functions",
            [](const DomainImpl& self) { return FunctionSkeletonList(self.get_functions()); },
            py::keep_alive<0, 1>())
        .def(
            "get_actions",
            [](const DomainImpl& self) { return ActionList(self.get_actions()); },
            py::keep_alive<0, 1>())
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::reference_internal)
        .def(
            "get_name_to_static_predicate",
            [](const DomainImpl& self) { return ToPredicateMap<std::string, Static>(self.get_name_to_predicate<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_name_to_fluent_predicate",
            [](const DomainImpl& self) { return ToPredicateMap<std::string, Fluent>(self.get_name_to_predicate<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_name_to_derived_predicate",
            [](const DomainImpl& self) { return ToPredicateMap<std::string, Derived>(self.get_name_to_predicate<Derived>()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<DomainList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<DomainList>(m, "DomainList");

    py::class_<ProblemImpl>(m, "Problem")  //
        .def("__str__", py::overload_cast<>(&loki::Base<ProblemImpl>::str, py::const_))
        .def("get_identifier", &ProblemImpl::get_identifier)
        .def(
            "get_filepath",
            [](const ProblemImpl& self)
            { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; },
            py::return_value_policy::copy)
        .def("get_name", &ProblemImpl::get_name, py::return_value_policy::copy)
        .def("get_domain", &ProblemImpl::get_domain, py::return_value_policy::reference_internal)
        .def("get_requirements", &ProblemImpl::get_requirements, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const ProblemImpl& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>())
        .def(
            "get_static_initial_literals",
            [](const ProblemImpl& self) { return GroundLiteralList<Static>(self.get_static_initial_literals()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_initial_literals",
            [](const ProblemImpl& self) { return GroundLiteralList<Fluent>(self.get_fluent_initial_literals()); },
            py::keep_alive<0, 1>())
        .def(
            "get_numeric_fluents",
            [](const ProblemImpl& self) { return NumericFluentList(self.get_numeric_fluents()); },
            py::keep_alive<0, 1>())
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, py::return_value_policy::reference_internal)
        .def(
            "get_static_goal_condition",
            [](const ProblemImpl& self) { return GroundLiteralList<Static>(self.get_goal_condition<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_goal_condition",
            [](const ProblemImpl& self) { return GroundLiteralList<Fluent>(self.get_goal_condition<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_goal_condition",
            [](const ProblemImpl& self) { return GroundLiteralList<Derived>(self.get_goal_condition<Derived>()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<ProblemList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<ProblemList>(m, "ProblemList");

    py::class_<PDDLFactories, std::shared_ptr<PDDLFactories>>(m, "PDDLFactories")  //
        .def("get_static_ground_atom", &PDDLFactories::get_ground_atom<Static>, py::return_value_policy::reference_internal)
        .def("get_fluent_ground_atom", &PDDLFactories::get_ground_atom<Fluent>, py::return_value_policy::reference_internal)
        .def("get_derived_ground_atom", &PDDLFactories::get_ground_atom<Derived>, py::return_value_policy::reference_internal)
        .def("get_static_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Static, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_fluent_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Fluent, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_derived_ground_atoms_from_ids",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_ids<Derived, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_object", &PDDLFactories::get_object, py::return_value_policy::reference_internal);

    py::class_<PDDLParser>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain, py::return_value_policy::reference_internal)
        .def("get_problem", &PDDLParser::get_problem, py::return_value_policy::reference_internal)
        .def("get_factories", &PDDLParser::get_factories);

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
    static_assert(!py::detail::vector_needs_copy<StateList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<StateList>(m, "StateList");
    bind_const_span<std::span<const State>>(m, "StateSpan");
    bind_const_index_grouped_vector<IndexGroupedVector<const State>>(m, "StateIndexGroupedVector");

    /* Action */
    py::class_<GroundAction>(m, "GroundAction")  //
        .def("__hash__", &GroundAction::hash)
        .def("__eq__", &GroundAction::operator==)
        .def("get_name", [](GroundAction self) { return self.get_action()->get_name(); })
        .def(
            "get_objects",
            [](GroundAction self)
            {
                ObjectList terms;
                for (const auto& object : self.get_objects())
                {
                    terms.emplace_back(object);
                }
                return terms;
            },
            py::keep_alive<0, 1>())
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
    static_assert(!py::detail::vector_needs_copy<GroundActionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundActionList>(m, "GroundActionList");
    bind_const_span<std::span<const GroundAction>>(m, "GroundActionSpan");
    /* AAGs */

    py::class_<IAAG, std::shared_ptr<IAAG>>(m, "IAAG")  //
        .def(
            "compute_applicable_actions",
            [](IAAG& self, State state)
            {
                auto applicable_actions = GroundActionList();
                self.generate_applicable_actions(state, applicable_actions);
                return applicable_actions;
            },
            py::keep_alive<0, 1>())
        .def("get_action", &IAAG::get_action, py::keep_alive<0, 1>())
        .def("get_problem", &IAAG::get_problem, py::return_value_policy::reference_internal)
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
        .def("get_or_create_initial_state", &SuccessorStateGenerator::get_or_create_initial_state, py::keep_alive<0, 1>())      // keep_alive because value type
        .def("get_or_create_state", &SuccessorStateGenerator::get_or_create_state, py::keep_alive<0, 1>())                      // keep_alive because value type
        .def("get_or_create_successor_state", &SuccessorStateGenerator::get_or_create_successor_state, py::keep_alive<0, 1>())  // keep_alive because value type
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
        .def("get_factors", &TupleIndexMapper::get_factors, py::return_value_policy::reference_internal)
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
        .def("get_fluent_remap", &FluentAndDerivedMapper::get_fluent_remap, py::return_value_policy::reference_internal)
        .def("get_derived_remap", &FluentAndDerivedMapper::get_derived_remap, py::return_value_policy::reference_internal)
        .def("get_is_remapped_fluent", &FluentAndDerivedMapper::get_is_remapped_fluent, py::return_value_policy::reference_internal)
        .def("get_inverse_remap", &FluentAndDerivedMapper::get_inverse_remap, py::return_value_policy::reference_internal);
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
        .def("get_index", &Transition::get_index)
        .def("get_source_state", &Transition::get_source_state)
        .def("get_target_state", &Transition::get_target_state)
        .def("get_cost", &Transition::get_cost)
        .def("get_creating_action", &Transition::get_creating_action, py::keep_alive<0, 1>());

    // AbstractTransition
    py::class_<AbstractTransition>(m, "AbstractTransition")  //
        .def("__eq__", &AbstractTransition::operator==)
        .def("__hash__", &AbstractTransition::hash)
        .def("get_index", &AbstractTransition::get_index)
        .def("get_source_state", &AbstractTransition::get_source_state)
        .def("get_target_state", &AbstractTransition::get_target_state)
        .def("get_cost", &AbstractTransition::get_cost)
        .def(
            "get_actions",
            [](const AbstractTransition& self) { return GroundActionList(self.get_actions().begin(), self.get_actions().end()); },
            py::keep_alive<0, 1>())
        .def("get_representative_action", &AbstractTransition::get_representative_action, py::keep_alive<0, 1>());

    // StateSpace
    py::class_<StateSpaceOptions>(m, "StateSpaceOptions")
        .def(py::init<bool, bool, uint32_t, uint32_t>(),
             py::arg("use_unit_cost_one") = true,
             py::arg("remove_if_unsolvable") = true,
             py::arg("max_num_states") = std::numeric_limits<uint32_t>::max(),
             py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max())
        .def_readwrite("use_unit_cost_one", &StateSpaceOptions::use_unit_cost_one)
        .def_readwrite("remove_if_unsolvable", &StateSpaceOptions::remove_if_unsolvable)
        .def_readwrite("max_num_states", &StateSpaceOptions::max_num_states)
        .def_readwrite("timeout_ms", &StateSpaceOptions::timeout_ms);

    py::class_<StateSpacesOptions>(m, "StateSpacesOptions")
        .def(py::init<StateSpaceOptions, bool, uint32_t>(),
             py::arg("state_space_options") = StateSpaceOptions(),
             py::arg("sort_ascending_by_num_states") = true,
             py::arg("num_threads") = std::thread::hardware_concurrency())
        .def_readwrite("state_space_options", &StateSpacesOptions::state_space_options)
        .def_readwrite("sort_ascending_by_num_states", &StateSpacesOptions::sort_ascending_by_num_states)
        .def_readwrite("num_threads", &StateSpacesOptions::num_threads);

    py::class_<StateSpace, std::shared_ptr<StateSpace>>(m, "StateSpace")  //
        .def("__str__",
             [](const StateSpace& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::string& problem_filepath, const StateSpaceOptions& options)
            { return StateSpace::create(domain_filepath, problem_filepath, options); },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("options") = StateSpaceOptions())
        .def_static(
            "create",
            [](Problem problem,
               std::shared_ptr<PDDLFactories> factories,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<SuccessorStateGenerator> ssg,
               const StateSpaceOptions& options) { return StateSpace::create(problem, factories, aag, ssg, options); },
            py::arg("problem"),
            py::arg("factories"),
            py::arg("aag"),
            py::arg("ssg"),
            py::arg("options") = StateSpaceOptions())
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::vector<std::string>& problem_filepaths, const StateSpacesOptions& options)
            {
                auto problem_filepaths_ = std::vector<fs::path>(problem_filepaths.begin(), problem_filepaths.end());
                return StateSpace::create(domain_filepath, problem_filepaths_, options);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("options") = StateSpacesOptions())
        .def_static(
            "create",
            [](const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>&
                   memories,
               const StateSpacesOptions& options) { return StateSpace::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = StateSpacesOptions())
        .def("compute_shortest_distances_from_states",
             &StateSpace::compute_shortest_distances_from_states,
             pybind11::arg("states"),
             pybind11::arg("forward") = true)
        .def("compute_pairwise_shortest_state_distances", &StateSpace::compute_pairwise_shortest_state_distances, pybind11::arg("forward") = true)
        .def("get_problem", &StateSpace::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &StateSpace::get_pddl_factories)
        .def("get_aag", &StateSpace::get_aag)
        .def("get_ssg", &StateSpace::get_ssg)
        .def("get_states", &StateSpace::get_states, py::return_value_policy::reference_internal)
        .def("get_state_index", &StateSpace::get_state_index)
        .def("get_initial_state", &StateSpace::get_initial_state)
        .def("get_goal_states", &StateSpace::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &StateSpace::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_target_states",
            [](const StateSpace& self, StateIndex source)
            {
                auto iterator = self.get_target_states(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_source_states",
            [](const StateSpace& self, StateIndex target)
            {
                auto iterator = self.get_source_states(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_states", &StateSpace::get_num_states)
        .def("get_num_goal_states", &StateSpace::get_num_goal_states)
        .def("get_num_deadend_states", &StateSpace::get_num_deadend_states)
        .def("is_goal_state", &StateSpace::is_goal_state)
        .def("is_deadend_state", &StateSpace::is_deadend_state)
        .def("is_alive_state", &StateSpace::is_alive_state)
        .def("get_transitions", &StateSpace::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &StateSpace::get_transition_cost)
        .def(
            "get_forward_transition_indices",
            [](const StateSpace& self, StateIndex source)
            {
                auto iterator = self.get_forward_transition_indices(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transition_indices",
            [](const StateSpace& self, StateIndex target)
            {
                auto iterator = self.get_backward_transition_indices(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_transitions",
            [](const StateSpace& self, StateIndex source)
            {
                auto iterator = self.get_forward_transitions(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transitions",
            [](const StateSpace& self, StateIndex target)
            {
                auto iterator = self.get_backward_transitions(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_transitions", &StateSpace::get_num_transitions)
        .def("get_goal_distances", &StateSpace::get_goal_distances, py::return_value_policy::reference_internal)
        .def("get_goal_distance", &StateSpace::get_goal_distance)
        .def("get_max_goal_distance", &StateSpace::get_max_goal_distance)
        .def("sample_state_with_goal_distance", &StateSpace::sample_state_with_goal_distance, py::return_value_policy::reference_internal);

    // Certificate
    py::class_<Certificate, std::shared_ptr<Certificate>>(m, "Certificate")
        .def(py::init<size_t, size_t, std::string, ColorList>())
        .def("__eq__", &Certificate::operator==)
        .def("__hash__", &Certificate::hash)
        .def("get_num_vertices", &Certificate::get_num_vertices)
        .def("get_num_edges", &Certificate::get_num_edges)
        .def("get_nauty_certificate", &Certificate::get_nauty_certificate, py::return_value_policy::reference_internal)
        .def("get_canonical_initial_coloring", &Certificate::get_canonical_initial_coloring, py::return_value_policy::reference_internal);

    // FaithfulAbstraction
    py::enum_<ObjectGraphPruningStrategyEnum>(m, "ObjectGraphPruningStrategyEnum")
        .value("None", ObjectGraphPruningStrategyEnum::None)
        .value("StaticScc", ObjectGraphPruningStrategyEnum::StaticScc)
        .export_values();

    py::class_<FaithfulAbstractionOptions>(m, "FaithfulAbstractionOptions")
        .def(py::init<bool, bool, bool, bool, uint32_t, uint32_t, uint32_t, ObjectGraphPruningStrategyEnum>(),
             py::arg("mark_true_goal_literals") = false,
             py::arg("use_unit_cost_one") = true,
             py::arg("remove_if_unsolvable") = true,
             py::arg("compute_complete_abstraction_mapping") = false,
             py::arg("max_num_concrete_states") = std::numeric_limits<uint32_t>::max(),
             py::arg("max_num_abstract_states") = std::numeric_limits<uint32_t>::max(),
             py::arg("timeout_ms") = std::numeric_limits<uint32_t>::max(),
             py::arg("pruning_strategy") = ObjectGraphPruningStrategyEnum::None)
        .def_readwrite("mark_true_goal_literals", &FaithfulAbstractionOptions::mark_true_goal_literals)
        .def_readwrite("use_unit_cost_one", &FaithfulAbstractionOptions::use_unit_cost_one)
        .def_readwrite("remove_if_unsolvable", &FaithfulAbstractionOptions::remove_if_unsolvable)
        .def_readwrite("compute_complete_abstraction_mapping", &FaithfulAbstractionOptions::compute_complete_abstraction_mapping)
        .def_readwrite("max_num_concrete_states", &FaithfulAbstractionOptions::max_num_concrete_states)
        .def_readwrite("max_num_abstract_states", &FaithfulAbstractionOptions::max_num_abstract_states)
        .def_readwrite("timeout_ms", &FaithfulAbstractionOptions::timeout_ms)
        .def_readwrite("pruning_strategy", &FaithfulAbstractionOptions::pruning_strategy);

    py::class_<FaithfulAbstractionsOptions>(m, "FaithfulAbstractionsOptions")
        .def(py::init<FaithfulAbstractionOptions, bool, uint32_t>(),
             py::arg("fa_options") = FaithfulAbstractionOptions(),
             py::arg("sort_ascending_by_num_states") = true,
             py::arg("num_threads") = std::thread::hardware_concurrency())
        .def_readwrite("fa_options", &FaithfulAbstractionsOptions::fa_options)
        .def_readwrite("sort_ascending_by_num_states", &FaithfulAbstractionsOptions::sort_ascending_by_num_states)
        .def_readwrite("num_threads", &FaithfulAbstractionsOptions::num_threads);

    py::class_<FaithfulAbstractState>(m, "FaithfulAbstractState")
        .def("get_index", &FaithfulAbstractState::get_index)
        .def(
            "get_states",
            [](const FaithfulAbstractState& self) { return std::vector<State>(self.get_states().begin(), self.get_states().end()); },
            py::keep_alive<0, 1>())
        .def("get_representative_state", &FaithfulAbstractState::get_representative_state, py::keep_alive<0, 1>())
        .def("get_certificate", &FaithfulAbstractState::get_certificate, py::return_value_policy::reference_internal);

    py::class_<FaithfulAbstraction, std::shared_ptr<FaithfulAbstraction>>(m, "FaithfulAbstraction")
        .def("__str__",
             [](const FaithfulAbstraction& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::string& problem_filepath, const FaithfulAbstractionOptions& options)
            { return FaithfulAbstraction::create(domain_filepath, problem_filepath, options); },
            py::arg("domain_filepath"),
            py::arg("problem_filepath"),
            py::arg("options") = FaithfulAbstractionOptions())
        .def_static(
            "create",
            [](Problem problem,
               std::shared_ptr<PDDLFactories> factories,
               std::shared_ptr<IAAG> aag,
               std::shared_ptr<SuccessorStateGenerator> ssg,
               const FaithfulAbstractionOptions& options) { return FaithfulAbstraction::create(problem, factories, aag, ssg, options); },
            py::arg("problem"),
            py::arg("factories"),
            py::arg("aag"),
            py::arg("ssg"),
            py::arg("options") = FaithfulAbstractionOptions())
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::vector<std::string>& problem_filepaths, const FaithfulAbstractionsOptions& options)
            {
                auto problem_filepaths_ = std::vector<fs::path>(problem_filepaths.begin(), problem_filepaths.end());
                return FaithfulAbstraction::create(domain_filepath, problem_filepaths_, options);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("options") = FaithfulAbstractionsOptions())
        .def_static(
            "create",
            [](const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return FaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionOptions())
        .def("compute_shortest_distances_from_states", &FaithfulAbstraction::compute_shortest_distances_from_states)
        .def("get_problem", &FaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &FaithfulAbstraction::get_pddl_factories)
        .def("get_aag", &FaithfulAbstraction::get_aag)
        .def("get_ssg", &FaithfulAbstraction::get_ssg)
        .def("get_abstract_state_index", &FaithfulAbstraction::get_abstract_state_index)
        .def("get_states", &FaithfulAbstraction::get_states, py::return_value_policy::reference_internal)
        .def("get_concrete_to_abstract_state", &FaithfulAbstraction::get_concrete_to_abstract_state, py::return_value_policy::reference_internal)
        .def("get_initial_state", &FaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &FaithfulAbstraction::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &FaithfulAbstraction::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_target_states",
            [](const FaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_target_states(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_source_states",
            [](const FaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_source_states(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_states", &FaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &FaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &FaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &FaithfulAbstraction::is_goal_state)
        .def("is_deadend_state", &FaithfulAbstraction::is_deadend_state)
        .def("is_alive_state", &FaithfulAbstraction::is_alive_state)
        .def("get_transitions", &FaithfulAbstraction::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &FaithfulAbstraction::get_transition_cost)
        .def(
            "get_forward_transition_indices",
            [](const FaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transition_indices(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transition_indices",
            [](const FaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transition_indices(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_transitions",
            [](const FaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transitions(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transitions",
            [](const FaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transitions(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_transitions", &FaithfulAbstraction::get_num_transitions)
        .def("get_goal_distances", &FaithfulAbstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // GlobalFaithfulAbstraction

    py::class_<GlobalFaithfulAbstractState>(m, "GlobalFaithfulAbstractState")
        .def("__eq__", &GlobalFaithfulAbstractState::operator==)
        .def("__hash__", &GlobalFaithfulAbstractState::hash)
        .def("get_index", &GlobalFaithfulAbstractState::get_index)
        .def("get_global_index", &GlobalFaithfulAbstractState::get_global_index)
        .def("get_faithful_abstraction_index", &GlobalFaithfulAbstractState::get_faithful_abstraction_index)
        .def("get_faithful_abstract_state_index", &GlobalFaithfulAbstractState::get_faithful_abstract_state_index);

    py::class_<GlobalFaithfulAbstraction, std::shared_ptr<GlobalFaithfulAbstraction>>(m, "GlobalFaithfulAbstraction")
        .def("__str__",
             [](const GlobalFaithfulAbstraction& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::vector<std::string>& problem_filepaths, const FaithfulAbstractionsOptions& options)
            {
                auto problem_filepaths_ = std::vector<fs::path>(problem_filepaths.begin(), problem_filepaths.end());
                return GlobalFaithfulAbstraction::create(domain_filepath, problem_filepaths_, options);
            },
            py::arg("domain_filepath"),
            py::arg("problem_filepaths"),
            py::arg("options") = FaithfulAbstractionsOptions())
        .def_static(
            "create",
            [](const std::vector<std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IAAG>, std::shared_ptr<SuccessorStateGenerator>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return GlobalFaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionsOptions())
        .def("get_index", &GlobalFaithfulAbstraction::get_index)
        .def("get_problem", &GlobalFaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &GlobalFaithfulAbstraction::get_pddl_factories)
        .def("get_aag", &GlobalFaithfulAbstraction::get_aag)
        .def("get_ssg", &GlobalFaithfulAbstraction::get_ssg)
        .def("get_abstractions", &GlobalFaithfulAbstraction::get_abstractions, py::return_value_policy::reference_internal)
        .def("get_abstract_state_index", py::overload_cast<State>(&GlobalFaithfulAbstraction::get_abstract_state_index, py::const_), py::arg("concrete_state"))
        .def("get_abstract_state_index",
             py::overload_cast<StateIndex>(&GlobalFaithfulAbstraction::get_abstract_state_index, py::const_),
             py::arg("global_state_index"))
        .def("get_states", &GlobalFaithfulAbstraction::get_states, py::return_value_policy::reference_internal)
        .def("get_concrete_to_abstract_state", &GlobalFaithfulAbstraction::get_concrete_to_abstract_state, py::return_value_policy::reference_internal)
        .def("get_global_state_index_to_state_index",
             &GlobalFaithfulAbstraction::get_global_state_index_to_state_index,
             py::return_value_policy::reference_internal)
        .def("get_initial_state", &GlobalFaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &GlobalFaithfulAbstraction::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &GlobalFaithfulAbstraction::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_target_states",
            [](const GlobalFaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_target_states(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_source_states",
            [](const GlobalFaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_source_states(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_states", &GlobalFaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &GlobalFaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &GlobalFaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &GlobalFaithfulAbstraction::is_goal_state)
        .def("is_deadend_state", &GlobalFaithfulAbstraction::is_deadend_state)
        .def("is_alive_state", &GlobalFaithfulAbstraction::is_alive_state)
        .def("get_num_isomorphic_states", &GlobalFaithfulAbstraction::get_num_isomorphic_states)
        .def("get_num_non_isomorphic_states", &GlobalFaithfulAbstraction::get_num_non_isomorphic_states)
        .def("get_transitions", &GlobalFaithfulAbstraction::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &GlobalFaithfulAbstraction::get_transition_cost)
        .def(
            "get_forward_transition_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transition_indices(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transition_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transition_indices(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_transitions",
            [](const GlobalFaithfulAbstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transitions(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transitions",
            [](const GlobalFaithfulAbstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transitions(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_transitions", &GlobalFaithfulAbstraction::get_num_transitions)
        .def("get_goal_distances", &GlobalFaithfulAbstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // Abstraction
    py::class_<Abstraction, std::shared_ptr<Abstraction>>(m, "Abstraction")  //
        .def(py::init<FaithfulAbstraction>())
        .def(py::init<GlobalFaithfulAbstraction>())
        .def("get_problem", &Abstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &Abstraction::get_pddl_factories)
        .def("get_aag", &Abstraction::get_aag)
        .def("get_ssg", &Abstraction::get_ssg)
        .def("get_abstract_state_index", &Abstraction::get_abstract_state_index)
        .def("get_initial_state", &Abstraction::get_initial_state)
        .def("get_goal_states", &Abstraction::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &Abstraction::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_target_states",
            [](const Abstraction& self, StateIndex source)
            {
                auto iterator = self.get_target_states(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_source_states",
            [](const Abstraction& self, StateIndex target)
            {
                auto iterator = self.get_source_states(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_states", &Abstraction::get_num_states)
        .def("get_num_goal_states", &Abstraction::get_num_goal_states)
        .def("get_num_deadend_states", &Abstraction::get_num_deadend_states)
        .def("is_goal_state", &Abstraction::is_goal_state)
        .def("is_deadend_state", &Abstraction::is_deadend_state)
        .def("is_alive_state", &Abstraction::is_alive_state)
        .def("get_transition_cost", &Abstraction::get_transition_cost)
        .def(
            "get_forward_transition_indices",
            [](const Abstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transition_indices(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transition_indices",
            [](const Abstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transition_indices(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_transitions",
            [](const Abstraction& self, StateIndex source)
            {
                auto iterator = self.get_forward_transitions(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_transitions",
            [](const Abstraction& self, StateIndex target)
            {
                auto iterator = self.get_backward_transitions(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_num_transitions", &Abstraction::get_num_transitions)
        .def("get_goal_distances", &Abstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // Graphs
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // TupleGraphVertex
    py::class_<TupleGraphVertex>(m, "TupleGraphVertex")  //
        .def("get_index", &TupleGraphVertex::get_index)
        .def("get_tuple_index", &TupleGraphVertex::get_tuple_index)
        .def(
            "get_states",
            [](const TupleGraphVertex& self) { return StateList(self.get_states()); },
            py::keep_alive<0, 1>());
    bind_const_span<std::span<const TupleGraphVertex>>(m, "TupleGraphVertexSpan");
    bind_const_index_grouped_vector<IndexGroupedVector<const TupleGraphVertex>>(m, "TupleGraphVertexIndexGroupedVector");

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
        .def("get_root_state", &TupleGraph::get_root_state, py::keep_alive<0, 1>())
        .def("get_vertices_grouped_by_distance", &TupleGraph::get_vertices_grouped_by_distance, py::return_value_policy::reference_internal)
        .def("get_digraph", &TupleGraph::get_digraph, py::return_value_policy::reference_internal)
        .def("get_states_grouped_by_distance", &TupleGraph::get_states_grouped_by_distance, py::return_value_policy::reference_internal);

    // TupleGraphFactory
    py::class_<TupleGraphFactory>(m, "TupleGraphFactory")  //
        .def(py::init<std::shared_ptr<StateSpace>, int, bool>(), py::arg("state_space"), py::arg("arity"), py::arg("prune_dominated_tuples") = false)
        .def("create", &TupleGraphFactory::create)
        .def("get_state_space", &TupleGraphFactory::get_state_space)
        .def("get_atom_index_mapper", &TupleGraphFactory::get_atom_index_mapper)
        .def("get_tuple_index_mapper", &TupleGraphFactory::get_tuple_index_mapper);

    // DigraphVertex
    py::class_<DigraphVertex>(m, "DigraphVertex")
        .def("__eq__", &DigraphVertex::operator==)
        .def("__hash__", &DigraphVertex::hash)
        .def("get_index", &DigraphVertex::get_index);

    // DigraphEdge
    py::class_<DigraphEdge>(m, "DigraphEdge")
        .def("__eq__", &DigraphEdge::operator==)
        .def("__hash__", &DigraphEdge::hash)
        .def("get_index", &DigraphEdge::get_index)
        .def("get_source", &DigraphEdge::get_source)
        .def("get_target", &DigraphEdge::get_target);

    // Digraph
    py::class_<Digraph>(m, "Digraph")  //
        .def(py::init<>())
        .def("__str__",
             [](const Digraph& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def("add_vertex", [](Digraph& self) -> VertexIndex { return self.add_vertex(); })
        .def("add_directed_edge", [](Digraph& self, VertexIndex source, VertexIndex target) -> EdgeIndex { return self.add_directed_edge(source, target); })
        .def("add_undirected_edge",
             [](Digraph& self, VertexIndex source, VertexIndex target) -> std::pair<EdgeIndex, EdgeIndex> { return self.add_undirected_edge(source, target); })
        .def("reset", &Digraph::reset)
        .def(
            "get_targets",
            [](const Digraph& self, StateIndex source)
            {
                auto iterator = self.get_targets(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_sources",
            [](const Digraph& self, StateIndex target)
            {
                auto iterator = self.get_sources(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_edges",
            [](const Digraph& self, StateIndex source)
            {
                auto iterator = self.get_forward_edges(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_edges",
            [](const Digraph& self, StateIndex target)
            {
                auto iterator = self.get_backward_edges(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_vertices", &Digraph::get_vertices, py::return_value_policy::reference_internal)
        .def("get_edges", &Digraph::get_edges, py::return_value_policy::reference_internal)
        .def("get_num_vertices", &Digraph::get_num_vertices)
        .def("get_num_edges", &Digraph::get_num_edges);

    // ColorFunction
    py::class_<ColorFunction>(m, "ColorFunction")  //
        .def("get_color_name", [](const ColorFunction& self, Color color) -> const std::string& { return self.get_color_name(color); });

    // ProblemColorFunction
    py::class_<ProblemColorFunction, ColorFunction>(m, "ProblemColorFunction")  //
        .def(py::init<Problem>(), py::arg("problem"))
        .def("get_color", [](const ProblemColorFunction& self, Object object) -> Color { return self.get_color(object); })
        .def("get_color", [](const ProblemColorFunction& self, GroundAtom<Static> atom, int pos) -> Color { return self.get_color(atom, pos); })
        .def("get_color", [](const ProblemColorFunction& self, GroundAtom<Fluent> atom, int pos) -> Color { return self.get_color(atom, pos); })
        .def("get_color", [](const ProblemColorFunction& self, GroundAtom<Derived> atom, int pos) -> Color { return self.get_color(atom, pos); })
        .def("get_color",
             [](const ProblemColorFunction& self, State state, GroundLiteral<Static> literal, int pos, bool mark_true_goal_literal) -> Color
             { return self.get_color(state, literal, pos, mark_true_goal_literal); })
        .def("get_color",
             [](const ProblemColorFunction& self, State state, GroundLiteral<Fluent> literal, int pos, bool mark_true_goal_literal) -> Color
             { return self.get_color(state, literal, pos, mark_true_goal_literal); })
        .def("get_color",
             [](const ProblemColorFunction& self, State state, GroundLiteral<Derived> literal, int pos, bool mark_true_goal_literal) -> Color
             { return self.get_color(state, literal, pos, mark_true_goal_literal); })
        .def("get_problem", &ProblemColorFunction::get_problem, py::return_value_policy::reference_internal)
        .def("get_name_to_color", &ProblemColorFunction::get_name_to_color, py::return_value_policy::reference_internal)
        .def("get_color_to_name", &ProblemColorFunction::get_color_to_name, py::return_value_policy::reference_internal);

    // ColoredDigraphVertex
    py::class_<ColoredDigraphVertex>(m, "ColoredDigraphVertex")
        .def("__eq__", &ColoredDigraphVertex::operator==)
        .def("__hash__", &ColoredDigraphVertex::hash)
        .def("get_index", &ColoredDigraphVertex::get_index)
        .def("get_color", &ColoredDigraphVertex::get_color);

    // VertexColoredDigraph
    py::class_<VertexColoredDigraph>(m, "VertexColoredDigraph")  //
        .def(py::init<>())
        .def("to_string",
             [](const VertexColoredDigraph& self, const ColorFunction& color_function)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(std::cref(self), std::cref(color_function));
                 return ss.str();
             })
        .def("add_vertex", [](VertexColoredDigraph& self, Color color) -> VertexIndex { return self.add_vertex(color); })
        .def("add_directed_edge",
             [](VertexColoredDigraph& self, VertexIndex source, VertexIndex target) -> EdgeIndex { return self.add_directed_edge(source, target); })
        .def("add_undirected_edge",
             [](VertexColoredDigraph& self, VertexIndex source, VertexIndex target) -> std::pair<EdgeIndex, EdgeIndex>
             { return self.add_undirected_edge(source, target); })
        .def("reset", &VertexColoredDigraph::reset)
        .def(
            "get_targets",
            [](const VertexColoredDigraph& self, StateIndex source)
            {
                auto iterator = self.get_targets(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_sources",
            [](const VertexColoredDigraph& self, StateIndex target)
            {
                auto iterator = self.get_sources(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_forward_edges",
            [](const VertexColoredDigraph& self, StateIndex source)
            {
                auto iterator = self.get_forward_edges(source);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def(
            "get_backward_edges",
            [](const VertexColoredDigraph& self, StateIndex target)
            {
                auto iterator = self.get_backward_edges(target);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>())
        .def("get_vertices", &VertexColoredDigraph::get_vertices, py::return_value_policy::reference_internal)
        .def("get_edges", &VertexColoredDigraph::get_edges, py::return_value_policy::reference_internal)
        .def("get_num_vertices", &VertexColoredDigraph::get_num_vertices)
        .def("get_num_edges", &VertexColoredDigraph::get_num_edges);

    m.def("compute_vertex_colors", &compute_vertex_colors, py::arg("vertex_colored_graph"));

    m.def("compute_sorted_vertex_colors", &compute_sorted_vertex_colors, py::arg("vertex_colored_graph"));

    // DenseNautyGraph
    py::class_<nauty_wrapper::DenseGraph>(m, "DenseNautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<VertexColoredDigraph>())
        .def("add_edge", &nauty_wrapper::DenseGraph::add_edge)
        .def("compute_certificate", &nauty_wrapper::DenseGraph::compute_certificate)
        .def("reset", &nauty_wrapper::DenseGraph::reset);

    // SparseNautyGraph
    py::class_<nauty_wrapper::SparseGraph>(m, "SparseNautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<VertexColoredDigraph>())
        .def("add_edge", &nauty_wrapper::SparseGraph::add_edge)
        .def("compute_certificate", &nauty_wrapper::SparseGraph::compute_certificate)
        .def("reset", &nauty_wrapper::SparseGraph::reset);

    // ObjectGraphPruningStrategy
    py::class_<ObjectGraphPruningStrategy>(m, "ObjectGraphPruningStrategy")  //
        ;

    // ObjectGraph
    m.def("create_object_graph",
          &create_object_graph,
          py::arg("color_function"),
          py::arg("pddl_factories"),
          py::arg("problem"),
          py::arg("state"),
          py::arg("state_index") = 0,
          py::arg("mark_true_goal_literals") = false,
          py::arg("pruning_strategy") = ObjectGraphPruningStrategy(),
          "Creates an object graph based on the provided parameters");
}
