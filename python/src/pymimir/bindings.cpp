#include "mimir/datasets/state_space.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

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
 * IPyHeuristic
 *
 * Source: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
 */

class IPyHeuristic : public IHeuristic
{
public:
    /* Inherit the constructors */
    using IHeuristic::IHeuristic;

    /* Trampoline (need one for each virtual function) */
    double compute_heuristic(State state) override
    {
        PYBIND11_OVERRIDE_PURE(double,            /* Return type */
                               IHeuristic,        /* Parent class */
                               compute_heuristic, /* Name of function in C++ (must match Python name) */
                               state              /* Argument(s) */
        );
    }
};

/**
 * IPyDynamicAStarAlgorithmEventHandlerBase
 *
 * Source: https://pybind11.readthedocs.io/en/stable/advanced/classes.html#overriding-virtual-functions-in-python
 */

class IPyDynamicAStarAlgorithmEventHandlerBase : public DynamicAStarAlgorithmEventHandlerBase
{
public:
    /* Inherit the constructors */
    using DynamicAStarAlgorithmEventHandlerBase::DynamicAStarAlgorithmEventHandlerBase;

    /* Trampoline (need one for each virtual function) */
    void on_expand_state_impl(State state, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_expand_state_impl, state, problem, std::cref(pddl_factories));
    }

    void on_generate_state_impl(State state, GroundAction action, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_generate_state_impl, state, action, problem, std::cref(pddl_factories));
    }
    void on_generate_state_relaxed_impl(State state, GroundAction action, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_generate_state_relaxed_impl, state, action, problem, std::cref(pddl_factories));
    }
    void on_generate_state_not_relaxed_impl(State state, GroundAction action, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_generate_state_not_relaxed_impl, state, action, problem, std::cref(pddl_factories));
    }
    void on_close_state_impl(State state, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_close_state_impl, state, problem, std::cref(pddl_factories));
    }
    void on_finish_f_layer_impl(double f_value, uint64_t num_expanded_state, uint64_t num_generated_states) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_finish_f_layer_impl, f_value, num_expanded_state, num_generated_states);
    }
    void on_prune_state_impl(State state, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_prune_state_impl, state, problem, std::cref(pddl_factories));
    }
    void on_start_search_impl(State start_state, Problem problem, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_start_search_impl, start_state, problem, std::cref(pddl_factories));
    }
    /**
     * Note the trailing commas in the PYBIND11_OVERRIDE calls to name() and bark(). These are needed to portably implement a trampoline for a function that
     * does not take any arguments. For functions that take a nonzero number of arguments, the trailing comma must be omitted.
     */
    void on_end_search_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_end_search_impl, ); }
    void on_solved_impl(const GroundActionList& ground_action_plan) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_solved_impl, ground_action_plan);
    }
    void on_unsolvable_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_unsolvable_impl, ); }
    void on_exhausted_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_exhausted_impl, ); }
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

    py::class_<RequirementsImpl>(m, "Requirements")  //
        .def("__str__", &RequirementsImpl::str)
        .def("__repr__", &RequirementsImpl::str)
        .def("get_index", &RequirementsImpl::get_index)
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::reference_internal);

    py::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", &ObjectImpl::str)
        .def("__repr__", &ObjectImpl::str)
        .def("get_index", &ObjectImpl::get_index)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<ObjectList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<ObjectList>(m, "ObjectList");

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", &VariableImpl::str)
        .def("__repr__", &VariableImpl::str)
        .def("get_index", &VariableImpl::get_index)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<VariableList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<VariableList>(m, "VariableList");

    py::class_<TermObjectImpl>(m, "TermObject")  //
        .def("__str__", &TermObjectImpl::str)
        .def("__repr__", &TermObjectImpl::str)
        .def("get_index", &TermObjectImpl::get_index)
        .def("get_object", &TermObjectImpl::get_object, py::return_value_policy::reference_internal);

    py::class_<TermVariableImpl>(m, "TermVariable")  //
        .def("__str__", &TermVariableImpl::str)
        .def("__repr__", &TermVariableImpl::str)
        .def("get_index", &TermVariableImpl::get_index)
        .def("get_variable", &TermVariableImpl::get_variable, py::return_value_policy::reference_internal);

    py::class_<TermVariant>(m, "Term")  //
        .def(
            "get",
            [](const TermVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.term); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<TermVariantList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<TermVariantList>(m, "TermVariantList");

    py::class_<PredicateImpl<Static>>(m, "StaticPredicate")  //
        .def("__str__", &PredicateImpl<Static>::str)
        .def("__repr__", &PredicateImpl<Static>::str)
        .def("get_index", &PredicateImpl<Static>::get_index)
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
        .def("__str__", &PredicateImpl<Fluent>::str)
        .def("__repr__", &PredicateImpl<Fluent>::str)
        .def("get_index", &PredicateImpl<Fluent>::get_index)
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
        .def("__str__", &PredicateImpl<Derived>::str)
        .def("__repr__", &PredicateImpl<Derived>::str)
        .def("get_index", &PredicateImpl<Derived>::get_index)
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
        .def("__str__", &AtomImpl<Static>::str)
        .def("__repr__", &AtomImpl<Static>::str)
        .def("get_index", &AtomImpl<Static>::get_index)
        .def("get_predicate", &AtomImpl<Static>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Static>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Static>>(m, "StaticAtomList");

    py::class_<AtomImpl<Fluent>>(m, "FluentAtom")  //
        .def("__str__", &AtomImpl<Fluent>::str)
        .def("__repr__", &AtomImpl<Fluent>::str)
        .def("get_index", &AtomImpl<Fluent>::get_index)
        .def("get_predicate", &AtomImpl<Fluent>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Fluent>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Fluent>>(m, "FluentAtomList");

    py::class_<AtomImpl<Derived>>(m, "DerivedAtom")  //
        .def("__str__", &AtomImpl<Derived>::str)
        .def("__repr__", &AtomImpl<Derived>::str)
        .def("get_index", &AtomImpl<Derived>::get_index)
        .def("get_predicate", &AtomImpl<Derived>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const AtomImpl<Derived>& atom) { return to_term_variant_list(atom.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<AtomList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<AtomList<Derived>>(m, "DerivedAtomList");

    py::class_<FunctionSkeletonImpl>(m, "FunctionSkeleton")  //
        .def("__str__", &FunctionSkeletonImpl::str)
        .def("__repr__", &FunctionSkeletonImpl::str)
        .def("get_index", &FunctionSkeletonImpl::get_index)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference_internal)
        .def(
            "get_parameters",
            [](const FunctionSkeletonImpl& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionSkeletonList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<FunctionSkeletonList>(m, "FunctionSkeletonList");

    py::class_<FunctionImpl>(m, "Function")  //
        .def("__str__", &FunctionImpl::str)
        .def("__repr__", &FunctionImpl::str)
        .def("get_index", &FunctionImpl::get_index)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def(
            "get_terms",
            [](const FunctionImpl& self) { return to_term_variant_list(self.get_terms()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<FunctionList>(m, "FunctionList");

    py::class_<GroundFunctionImpl>(m, "GroundFunction")  //
        .def("__str__", &GroundFunctionImpl::str)
        .def("__repr__", &GroundFunctionImpl::str)
        .def("get_index", &GroundFunctionImpl::get_index)
        .def("get_function_skeleton", &GroundFunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundFunctionImpl& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundFunctionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundFunctionList>(m, "GroundFunctionList");

    py::class_<GroundAtomImpl<Static>>(m, "StaticGroundAtom")  //
        .def("__str__", &GroundAtomImpl<Static>::str)
        .def("__repr__", &GroundAtomImpl<Static>::str)
        .def("get_index", &GroundAtomImpl<Static>::get_index)
        .def("get_arity", &GroundAtomImpl<Static>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Static>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Static>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Static>>(m, "StaticGroundAtomList");

    py::class_<GroundAtomImpl<Fluent>>(m, "FluentGroundAtom")  //
        .def("__str__", &GroundAtomImpl<Fluent>::str)
        .def("__repr__", &GroundAtomImpl<Fluent>::str)
        .def("get_index", &GroundAtomImpl<Fluent>::get_index)
        .def("get_arity", &GroundAtomImpl<Fluent>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Fluent>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Fluent>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Fluent>>(m, "FluentGroundAtomList");

    py::class_<GroundAtomImpl<Derived>>(m, "DerivedGroundAtom")  //
        .def("__str__", &GroundAtomImpl<Derived>::str)
        .def("__repr__", &GroundAtomImpl<Derived>::str)
        .def("get_index", &GroundAtomImpl<Derived>::get_index)
        .def("get_arity", &GroundAtomImpl<Derived>::get_arity)
        .def("get_predicate", &GroundAtomImpl<Derived>::get_predicate, py::return_value_policy::reference_internal)
        .def(
            "get_objects",
            [](const GroundAtomImpl<Derived>& self) { return ObjectList(self.get_objects()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundAtomList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundAtomList<Derived>>(m, "DerivedGroundAtomList");

    py::class_<GroundLiteralImpl<Static>>(m, "StaticGroundLiteral")  //
        .def("__str__", &GroundLiteralImpl<Static>::str)
        .def("__repr__", &GroundLiteralImpl<Static>::str)
        .def("get_index", &GroundLiteralImpl<Static>::get_index)
        .def("get_atom", &GroundLiteralImpl<Static>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Static>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Static>>(m, "StaticGroundLiteralList");

    py::class_<GroundLiteralImpl<Fluent>>(m, "FluentGroundLiteral")  //
        .def("__str__", &GroundLiteralImpl<Fluent>::str)
        .def("__repr__", &GroundLiteralImpl<Fluent>::str)
        .def("get_index", &GroundLiteralImpl<Fluent>::get_index)
        .def("get_atom", &GroundLiteralImpl<Fluent>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Fluent>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Fluent>>(m, "FluentGroundLiteralList");

    py::class_<GroundLiteralImpl<Derived>>(m, "DerivedGroundLiteral")  //
        .def("__str__", &GroundLiteralImpl<Derived>::str)
        .def("__repr__", &GroundLiteralImpl<Derived>::str)
        .def("get_index", &GroundLiteralImpl<Derived>::get_index)
        .def("get_atom", &GroundLiteralImpl<Derived>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &GroundLiteralImpl<Derived>::is_negated);
    static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundLiteralList<Derived>>(m, "DerivedGroundLiteralList");

    py::class_<LiteralImpl<Static>>(m, "StaticLiteral")  //
        .def("__str__", &LiteralImpl<Static>::str)
        .def("__repr__", &LiteralImpl<Static>::str)
        .def("get_index", &LiteralImpl<Static>::get_index)
        .def("get_atom", &LiteralImpl<Static>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Static>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Static>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Static>>(m, "StaticLiteralList");

    py::class_<LiteralImpl<Fluent>>(m, "FluentLiteral")  //
        .def("__str__", &LiteralImpl<Fluent>::str)
        .def("__repr__", &LiteralImpl<Fluent>::str)
        .def("get_index", &LiteralImpl<Fluent>::get_index)
        .def("get_atom", &LiteralImpl<Fluent>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Fluent>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Fluent>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Fluent>>(m, "FluentLiteralList");

    py::class_<LiteralImpl<Derived>>(m, "DerivedLiteral")  //
        .def("__str__", &LiteralImpl<Derived>::str)
        .def("__repr__", &LiteralImpl<Derived>::str)
        .def("get_index", &LiteralImpl<Derived>::get_index)
        .def("get_atom", &LiteralImpl<Derived>::get_atom, py::return_value_policy::reference_internal)
        .def("is_negated", &LiteralImpl<Derived>::is_negated);
    static_assert(!py::detail::vector_needs_copy<LiteralList<Derived>>::value);  // Ensure return by reference + keep alive
    py::bind_vector<LiteralList<Derived>>(m, "DerivedLiteralList");

    py::class_<NumericFluentImpl>(m, "NumericFluent")  //
        .def("__str__", &NumericFluentImpl::str)
        .def("__repr__", &NumericFluentImpl::str)
        .def("get_index", &NumericFluentImpl::get_index)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference_internal)
        .def("get_number", &NumericFluentImpl::get_number);
    static_assert(!py::detail::vector_needs_copy<NumericFluentList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<NumericFluentList>(m, "NumericFluentList");

    py::class_<EffectSimpleImpl>(m, "EffectSimple")  //
        .def("__str__", &EffectSimpleImpl::str)
        .def("__repr__", &EffectSimpleImpl::str)
        .def("get_index", &EffectSimpleImpl::get_index)
        .def("get_effect", &EffectSimpleImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectSimpleList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectSimpleList>(m, "EffectSimpleList");

    py::class_<EffectConditionalImpl>(m, "EffectConditional")  //
        .def("__str__", &EffectConditionalImpl::str)
        .def("__repr__", &EffectConditionalImpl::str)
        .def("get_index", &EffectConditionalImpl::get_index)
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
        .def("__str__", &EffectUniversalImpl::str)
        .def("__repr__", &EffectUniversalImpl::str)
        .def("get_index", &EffectUniversalImpl::get_index)
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
        .def("__str__", &FunctionExpressionNumberImpl::str)
        .def("__repr__", &FunctionExpressionNumberImpl::str)
        .def("get_index", &FunctionExpressionNumberImpl::get_index)
        .def("get_number", &FunctionExpressionNumberImpl::get_number);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m, "FunctionExpressionBinaryOperator")  //
        .def("__str__", &FunctionExpressionBinaryOperatorImpl::str)
        .def("__repr__", &FunctionExpressionBinaryOperatorImpl::str)
        .def("get_index", &FunctionExpressionBinaryOperatorImpl::get_index)
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
        .def("__str__", &FunctionExpressionMultiOperatorImpl::str)
        .def("__repr__", &FunctionExpressionMultiOperatorImpl::str)
        .def("get_index", &FunctionExpressionMultiOperatorImpl::get_index)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const FunctionExpressionMultiOperatorImpl& function_expression)
            { return to_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::keep_alive<0, 1>());

    py::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", &FunctionExpressionMinusImpl::str)
        .def("__repr__", &FunctionExpressionMinusImpl::str)
        .def("get_index", &FunctionExpressionMinusImpl::get_index)
        .def(
            "get_function_expression",
            [](const FunctionExpressionMinusImpl& function_expression) { return FunctionExpressionVariant(function_expression.get_function_expression()); },
            py::keep_alive<0, 1>());
    ;

    py::class_<FunctionExpressionFunctionImpl>(m, "FunctionExpressionFunction")  //
        .def("__str__", &FunctionExpressionFunctionImpl::str)
        .def("__repr__", &FunctionExpressionFunctionImpl::str)
        .def("get_index", &FunctionExpressionFunctionImpl::get_index)
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
        .def("__str__", &GroundFunctionExpressionNumberImpl::str)
        .def("__repr__", &GroundFunctionExpressionNumberImpl::str)
        .def("get_index", &GroundFunctionExpressionNumberImpl::get_index)
        .def("get_number", &GroundFunctionExpressionNumberImpl::get_number);

    py::class_<GroundFunctionExpressionBinaryOperatorImpl>(m, "GroundFunctionExpressionBinaryOperator")  //
        .def("__str__", &GroundFunctionExpressionBinaryOperatorImpl::str)
        .def("__repr__", &GroundFunctionExpressionBinaryOperatorImpl::str)
        .def("get_index", &GroundFunctionExpressionBinaryOperatorImpl::get_index)
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
        .def("__str__", &GroundFunctionExpressionMultiOperatorImpl::str)
        .def("__repr__", &GroundFunctionExpressionMultiOperatorImpl::str)
        .def("get_index", &GroundFunctionExpressionMultiOperatorImpl::get_index)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def(
            "get_function_expressions",
            [](const GroundFunctionExpressionMultiOperatorImpl& function_expression)
            { return to_ground_function_expression_variant_list(function_expression.get_function_expressions()); },
            py::keep_alive<0, 1>());

    py::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", &GroundFunctionExpressionMinusImpl::str)
        .def("__repr__", &GroundFunctionExpressionMinusImpl::str)
        .def("get_index", &GroundFunctionExpressionMinusImpl::get_index)
        .def(
            "get_function_expression",
            [](const GroundFunctionExpressionMinusImpl& function_expression)
            { return GroundFunctionExpressionVariant(function_expression.get_function_expression()); },
            py::keep_alive<0, 1>());
    ;

    py::class_<GroundFunctionExpressionFunctionImpl>(m, "GroundFunctionExpressionFunction")  //
        .def("__str__", &GroundFunctionExpressionFunctionImpl::str)
        .def("__repr__", &GroundFunctionExpressionFunctionImpl::str)
        .def("get_index", &GroundFunctionExpressionFunctionImpl::get_index)
        .def("get_function", &GroundFunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference_internal);

    py::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", &OptimizationMetricImpl::str)
        .def("__repr__", &OptimizationMetricImpl::str)
        .def("get_index", &OptimizationMetricImpl::get_index)
        .def("get_function_expression", [](const OptimizationMetricImpl& metric) { return GroundFunctionExpressionVariant(metric.get_function_expression()); })
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference_internal);

    py::class_<ActionImpl>(m, "Action")  //
        .def("__str__", &ActionImpl::str)
        .def("__repr__", &ActionImpl::str)
        .def("get_index", &ActionImpl::get_index)
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
        .def("__str__", &AxiomImpl::str)
        .def("__repr__", &AxiomImpl::str)
        .def("get_index", &AxiomImpl::get_index)
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
        .def("__str__", &DomainImpl::str)
        .def("__repr__", &DomainImpl::str)
        .def("get_index", &DomainImpl::get_index)
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
        .def("__str__", &ProblemImpl::str)
        .def("__repr__", &ProblemImpl::str)
        .def("get_index", &ProblemImpl::get_index)
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
        .def("get_static_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_indices<Static, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_fluent_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_indices<Fluent, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_derived_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLFactories::get_ground_atoms_from_indices<Derived, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_object", &PDDLFactories::get_object, py::return_value_policy::reference_internal);

    py::class_<PDDLParser>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>())
        .def("get_domain", &PDDLParser::get_domain, py::return_value_policy::reference_internal)
        .def("get_problem", &PDDLParser::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &PDDLParser::get_pddl_factories);

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
        .value("UNSOLVABLE", SearchStatus::UNSOLVABLE)
        .export_values();

    /* State */
    py::class_<State>(m, "State")  //
        .def("__hash__", [](const State& self) { return std::hash<State>()(self); })
        .def("__eq__", &State::operator==)
        .def("__repr__",
             [](State self)
             {
                 std::stringstream ss;
                 ss << "State(#Fluent=";
                 ss << std::to_string(self.get_atoms<Fluent>().count());
                 ss << "; #Derived=";
                 ss << std::to_string(self.get_atoms<Derived>().count());
                 ss << ")";
                 return ss.str();
             })
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
        .def("contains", py::overload_cast<GroundAtom<Fluent>>(&State::contains<Fluent>, py::const_), py::arg("atom"))
        .def("contains", py::overload_cast<GroundAtom<Derived>>(&State::contains<Derived>, py::const_), py::arg("atom"))
        .def("superset_of", py::overload_cast<const GroundAtomList<Fluent>&>(&State::superset_of<Fluent>, py::const_), py::arg("atoms"))
        .def("superset_of", py::overload_cast<const GroundAtomList<Derived>&>(&State::superset_of<Derived>, py::const_), py::arg("atoms"))
        .def("literal_holds", py::overload_cast<GroundLiteral<Fluent>>(&State::literal_holds<Fluent>, py::const_), py::arg("literal"))
        .def("literal_holds", py::overload_cast<GroundLiteral<Derived>>(&State::literal_holds<Derived>, py::const_), py::arg("literal"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Fluent>&>(&State::literals_hold<Fluent>, py::const_), py::arg("literals"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Derived>&>(&State::literals_hold<Derived>, py::const_), py::arg("literals"))
        .def(
            "to_string",
            [](const State& self, Problem problem, const PDDLFactories& pddl_factories)
            {
                std::stringstream ss;
                ss << std::make_tuple(problem, self, std::cref(pddl_factories));
                return ss.str();
            },
            py::arg("problem"),
            py::arg("pddl_factories"))
        .def("get_index", &State::get_index);
    static_assert(!py::detail::vector_needs_copy<StateList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<StateList>(m, "StateList");
    bind_const_span<std::span<const State>>(m, "StateSpan");
    bind_const_index_grouped_vector<IndexGroupedVector<const State>>(m, "StateIndexGroupedVector");

    /* Action */
    // TODO: add missing functions here
    py::class_<StripsActionPrecondition>(m, "StripsActionPrecondition");
    py::class_<StripsActionEffect>(m, "StripsActionEffect");
    py::class_<ConditionalEffect>(m, "ConditionalEffect");
    py::class_<FlatSimpleEffect>(m, "FlatSimpleEffect");

    py::class_<GroundAction>(m, "GroundAction")  //
        .def("__hash__", [](const GroundAction& obj) { return std::hash<GroundAction>()(obj); })
        .def("__eq__", [](const GroundAction& a, const GroundAction& b) { return std::equal_to<GroundAction>()(a, b); })
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
             })
        .def("to_string",
             [](GroundAction self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("get_index", &GroundAction::get_index)
        .def("get_cost", &GroundAction::get_cost)
        .def("get_action", &GroundAction::get_action, py::return_value_policy::reference_internal)
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
        .def("get_strips_precondition", [](const GroundAction& self) { return StripsActionPrecondition(self.get_strips_precondition()); })
        .def("get_strips_effect", [](const GroundAction& self) { return StripsActionEffect(self.get_strips_effect()); })
        .def("get_conditional_effects",
             [](const GroundAction& self)
             {
                 auto conditional_effects = std::vector<ConditionalEffect> {};
                 for (const auto& flat_conditional_effect : self.get_conditional_effects())
                 {
                     conditional_effects.push_back(ConditionalEffect(flat_conditional_effect));
                 }
                 return conditional_effects;
             });
    static_assert(!py::detail::vector_needs_copy<GroundActionList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<GroundActionList>(m, "GroundActionList");
    bind_const_span<std::span<const GroundAction>>(m, "GroundActionSpan");

    /* ApplicableActionGenerators */

    py::class_<IApplicableActionGenerator, std::shared_ptr<IApplicableActionGenerator>>(m, "IApplicableActionGenerator")  //
        .def(
            "compute_applicable_actions",
            [](IApplicableActionGenerator& self, State state)
            {
                auto applicable_actions = GroundActionList();
                self.generate_applicable_actions(state, applicable_actions);
                return applicable_actions;
            },
            py::keep_alive<0, 1>(),
            py::arg("state"))
        .def(
            "get_ground_actions",
            [](const IApplicableActionGenerator& self)
            {
                auto actions = self.get_ground_actions();
                return actions;
            },
            py::keep_alive<0, 1>())
        .def("get_ground_action", &IApplicableActionGenerator::get_ground_action, py::keep_alive<0, 1>(), py::arg("action_index"))
        .def(
            "get_ground_axioms",
            [](const IApplicableActionGenerator& self)
            {
                auto axioms = self.get_ground_axioms();
                return axioms;
            },
            py::keep_alive<0, 1>())
        .def("get_ground_action", &IApplicableActionGenerator::get_ground_axiom, py::keep_alive<0, 1>(), py::arg("axiom_index"))
        .def("get_problem", &IApplicableActionGenerator::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &IApplicableActionGenerator::get_pddl_factories);

    // Lifted
    py::class_<ILiftedApplicableActionGeneratorEventHandler, std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler>>(
        m,
        "ILiftedApplicableActionGeneratorEventHandler");  //
    py::class_<DefaultLiftedApplicableActionGeneratorEventHandler,
               ILiftedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DefaultLiftedApplicableActionGeneratorEventHandler>>(m,
                                                                                    "DefaultLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedApplicableActionGeneratorEventHandler,
               ILiftedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DebugLiftedApplicableActionGeneratorEventHandler>>(m, "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<LiftedApplicableActionGenerator>>(
        m,
        "LiftedApplicableActionGenerator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>())
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler>>());

    // Grounded
    py::class_<IGroundedApplicableActionGeneratorEventHandler, std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler>>(
        m,
        "IGroundedApplicableActionGeneratorEventHandler");  //
    py::class_<DefaultGroundedApplicableActionGeneratorEventHandler,
               IGroundedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DefaultGroundedApplicableActionGeneratorEventHandler>>(m,
                                                                                      "DefaultGroundedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedApplicableActionGeneratorEventHandler,
               IGroundedApplicableActionGeneratorEventHandler,
               std::shared_ptr<DebugGroundedApplicableActionGeneratorEventHandler>>(m, "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<GroundedApplicableActionGenerator>>(
        m,
        "GroundedApplicableActionGenerator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>())
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler>>());

    /* StateRepository */
    py::class_<StateRepository, std::shared_ptr<StateRepository>>(m, "StateRepository")  //
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>>())
        .def("get_or_create_initial_state", &StateRepository::get_or_create_initial_state, py::keep_alive<0, 1>())    // keep_alive because value type
        .def("get_or_create_state", &StateRepository::get_or_create_state, py::keep_alive<0, 1>(), py::arg("atoms"))  // keep_alive because value type
        .def("get_or_create_successor_state",
             &StateRepository::get_or_create_successor_state,
             py::keep_alive<0, 1>(),
             py::arg("state"),
             py::arg("action"))  // keep_alive because value type
        .def("get_state_count", &StateRepository::get_state_count)
        .def("get_reached_fluent_ground_atoms",
             [](const StateRepository& self)
             {
                 const auto& atoms = self.get_reached_fluent_ground_atoms();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             })
        .def("get_reached_derived_ground_atoms",
             [](const StateRepository& self)
             {
                 const auto& atoms = self.get_reached_derived_ground_atoms();
                 return std::vector<size_t>(atoms.begin(), atoms.end());
             });

    /* Heuristics */
    py::class_<IHeuristic, IPyHeuristic, std::shared_ptr<IHeuristic>>(m, "IHeuristic").def(py::init<>());
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
    py::class_<AStarAlgorithmStatistics>(m, "AStarAlgorithmStatistics")  //
        .def("get_num_generated", &AStarAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &AStarAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &AStarAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &AStarAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_f_value", &AStarAlgorithmStatistics::get_num_generated_until_f_value)
        .def("get_num_expanded_until_f_value", &AStarAlgorithmStatistics::get_num_expanded_until_f_value)
        .def("get_num_deadends_until_f_value", &AStarAlgorithmStatistics::get_num_deadends_until_f_value)
        .def("get_num_pruned_until_f_value", &AStarAlgorithmStatistics::get_num_pruned_until_f_value);
    py::class_<IAStarAlgorithmEventHandler, std::shared_ptr<IAStarAlgorithmEventHandler>>(m, "IAStarAlgorithmEventHandler")  //
        .def("get_statistics", &IAStarAlgorithmEventHandler::get_statistics);
    py::class_<DefaultAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler, std::shared_ptr<DefaultAStarAlgorithmEventHandler>>(
        m,
        "DefaultAStarAlgorithmEventHandler")  //
        .def(py::init<bool>(), py::arg("quiet") = true);
    py::class_<DebugAStarAlgorithmEventHandler, IAStarAlgorithmEventHandler, std::shared_ptr<DebugAStarAlgorithmEventHandler>>(
        m,
        "DebugAStarAlgorithmEventHandler")  //
        .def(py::init<bool>(), py::arg("quiet") = true);
    py::class_<DynamicAStarAlgorithmEventHandlerBase,
               IAStarAlgorithmEventHandler,
               IPyDynamicAStarAlgorithmEventHandlerBase,
               std::shared_ptr<DynamicAStarAlgorithmEventHandlerBase>>(m,
                                                                       "AStarAlgorithmEventHandlerBase")  //
        .def(py::init<bool>(), py::arg("quiet") = true);
    py::class_<AStarAlgorithm, IAlgorithm, std::shared_ptr<AStarAlgorithm>>(m, "AStarAlgorithm")  //
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<IHeuristic>>())
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IHeuristic>,
                      std::shared_ptr<IAStarAlgorithmEventHandler>>());

    // BrFS
    py::class_<BrFSAlgorithmStatistics>(m, "BrFSAlgorithmStatistics")  //
        .def("get_num_generated", &BrFSAlgorithmStatistics::get_num_generated)
        .def("get_num_expanded", &BrFSAlgorithmStatistics::get_num_expanded)
        .def("get_num_deadends", &BrFSAlgorithmStatistics::get_num_deadends)
        .def("get_num_pruned", &BrFSAlgorithmStatistics::get_num_pruned)
        .def("get_num_generated_until_g_value", &BrFSAlgorithmStatistics::get_num_generated_until_g_value)
        .def("get_num_expanded_until_g_value", &BrFSAlgorithmStatistics::get_num_expanded_until_g_value)
        .def("get_num_deadends_until_g_value", &BrFSAlgorithmStatistics::get_num_deadends_until_g_value)
        .def("get_num_pruned_until_g_value", &BrFSAlgorithmStatistics::get_num_pruned_until_g_value);
    py::class_<IBrFSAlgorithmEventHandler, std::shared_ptr<IBrFSAlgorithmEventHandler>>(m, "IBrFSAlgorithmEventHandler")
        .def("get_statistics", &IBrFSAlgorithmEventHandler::get_statistics);
    py::class_<DefaultBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DefaultBrFSAlgorithmEventHandler>>(
        m,
        "DefaultBrFSAlgorithmEventHandler")  //
        .def(py::init<>());
    py::class_<DebugBrFSAlgorithmEventHandler, IBrFSAlgorithmEventHandler, std::shared_ptr<DebugBrFSAlgorithmEventHandler>>(
        m,
        "DebugBrFSAlgorithmEventHandler")  //
        .def(py::init<>());
    py::class_<BrFSAlgorithm, IAlgorithm, std::shared_ptr<BrFSAlgorithm>>(m, "BrFSAlgorithm")
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>>())
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>, std::shared_ptr<IBrFSAlgorithmEventHandler>>());

    // IW
    py::class_<TupleIndexMapper, std::shared_ptr<TupleIndexMapper>>(m, "TupleIndexMapper")  //
        .def("to_tuple_index", &TupleIndexMapper::to_tuple_index, py::arg("atom_indices"))
        .def(
            "to_atom_indices",
            [](const TupleIndexMapper& self, const TupleIndex tuple_index)
            {
                auto atom_indices = AtomIndexList {};
                self.to_atom_indices(tuple_index, atom_indices);
                return atom_indices;
            },
            py::arg("tuple_index"))
        .def("tuple_index_to_string", &TupleIndexMapper::tuple_index_to_string, py::arg("tuple_index"))
        .def("get_num_atoms", &TupleIndexMapper::get_num_atoms)
        .def("get_arity", &TupleIndexMapper::get_arity)
        .def("get_factors", &TupleIndexMapper::get_factors, py::return_value_policy::reference_internal)
        .def("get_max_tuple_index", &TupleIndexMapper::get_max_tuple_index)
        .def("get_empty_tuple_index", &TupleIndexMapper::get_empty_tuple_index);
    py::class_<FluentAndDerivedMapper, std::shared_ptr<FluentAndDerivedMapper>>(m, "FluentAndDerivedMapper")  //
        .def(
            "combine_and_sort",

            [](FluentAndDerivedMapper& self, const State state)
            {
                auto atom_indices = AtomIndexList {};
                self.combine_and_sort(state, atom_indices);
                return atom_indices;
            },
            py::arg("state"))
        .def(
            "combine_and_sort",

            [](FluentAndDerivedMapper& self, const State state, const State succ_state)
            {
                auto atom_indices = AtomIndexList {};
                auto add_atom_indices = AtomIndexList {};
                self.combine_and_sort(state, succ_state, atom_indices, add_atom_indices);
                return std::make_tuple(atom_indices, add_atom_indices);
            },
            py::arg("state"),
            py::arg("successor_state"))
        .def(
            "inverse_remap_and_separate",

            [](FluentAndDerivedMapper& self, const AtomIndexList& fluent_and_derived_atom_indices)
            {
                auto fluent_atom_indices = AtomIndexList {};
                auto derived_atom_indices = AtomIndexList {};
                self.inverse_remap_and_separate(fluent_and_derived_atom_indices, fluent_atom_indices, derived_atom_indices);
                return std::make_tuple(fluent_atom_indices, derived_atom_indices);
            },
            py::arg("fluent_and_derived_atom_indices"))
        .def("get_fluent_remap", &FluentAndDerivedMapper::get_fluent_remap, py::return_value_policy::reference_internal)
        .def("get_derived_remap", &FluentAndDerivedMapper::get_derived_remap, py::return_value_policy::reference_internal)
        .def("get_is_remapped_fluent", &FluentAndDerivedMapper::get_is_remapped_fluent, py::return_value_policy::reference_internal)
        .def("get_inverse_remap", &FluentAndDerivedMapper::get_inverse_remap, py::return_value_policy::reference_internal);

    py::class_<IWAlgorithmStatistics>(m, "IWAlgorithmStatistics")  //
        .def("get_effective_width", &IWAlgorithmStatistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &IWAlgorithmStatistics::get_brfs_statistics_by_arity);
    py::class_<IIWAlgorithmEventHandler, std::shared_ptr<IIWAlgorithmEventHandler>>(m, "IIWAlgorithmEventHandler")
        .def("get_statistics", &IIWAlgorithmEventHandler::get_statistics);
    py::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler, std::shared_ptr<DefaultIWAlgorithmEventHandler>>(m, "DefaultIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<IWAlgorithm, IAlgorithm, std::shared_ptr<IWAlgorithm>>(m, "IWAlgorithm")
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, int>())
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      int,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>>());

    // SIW
    py::class_<SIWAlgorithmStatistics>(m, "SIWAlgorithmStatistics")  //
        .def("get_maximum_effective_width", &SIWAlgorithmStatistics::get_maximum_effective_width)
        .def("get_average_effective_width", &SIWAlgorithmStatistics::get_average_effective_width)
        .def("get_iw_statistics_by_subproblem", &SIWAlgorithmStatistics::get_iw_statistics_by_subproblem);
    py::class_<ISIWAlgorithmEventHandler, std::shared_ptr<ISIWAlgorithmEventHandler>>(m, "ISIWAlgorithmEventHandler")
        .def("get_statistics", &ISIWAlgorithmEventHandler::get_statistics);
    py::class_<DefaultSIWAlgorithmEventHandler, ISIWAlgorithmEventHandler, std::shared_ptr<DefaultSIWAlgorithmEventHandler>>(m,
                                                                                                                             "DefaultSIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<SIWAlgorithm, IAlgorithm, std::shared_ptr<SIWAlgorithm>>(m, "SIWAlgorithm")
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, int>())
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      int,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>,
                      std::shared_ptr<ISIWAlgorithmEventHandler>>());

    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    // DataSets
    // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

    // StateVertex
    py::class_<StateVertex>(m, "StateVertex")  //
        .def("__eq__", &StateVertex::operator==)
        .def("__hash__", [](const StateVertex& self) { return std::hash<StateVertex>()(self); })
        .def("get_index", &StateVertex::get_index)
        .def(
            "get_state",
            [](const StateVertex& self) { return get_state(self); },
            py::keep_alive<0, 1>());

    // GroundActionEdge
    py::class_<GroundActionEdge>(m, "GroundActionEdge")  //
        .def("__eq__", &GroundActionEdge::operator==)
        .def("__hash__", [](const GroundActionEdge& self) { return std::hash<GroundActionEdge>()(self); })
        .def("get_index", &GroundActionEdge::get_index)
        .def("get_source", &GroundActionEdge::get_source)
        .def("get_target", &GroundActionEdge::get_target)
        .def("get_cost", [](const GroundActionEdge& self) { return get_cost(self); })
        .def(
            "get_creating_action",
            [](const GroundActionEdge& self) { return get_creating_action(self); },
            py::keep_alive<0, 1>());

    // GroundActionsEdge
    py::class_<GroundActionsEdge>(m, "GroundActionsEdge")  //
        .def("__eq__", &GroundActionsEdge::operator==)
        .def("__hash__", [](const GroundActionsEdge& self) { return std::hash<GroundActionsEdge>()(self); })
        .def("get_index", &GroundActionsEdge::get_index)
        .def("get_source", &GroundActionsEdge::get_source)
        .def("get_target", &GroundActionsEdge::get_target)
        .def("get_cost", [](const GroundActionsEdge& self) { return get_cost(self); })
        .def(
            "get_actions",
            [](const GroundActionsEdge& self) { return GroundActionList(get_actions(self).begin(), get_actions(self).end()); },
            py::keep_alive<0, 1>())
        .def(
            "get_representative_action",
            [](const GroundActionsEdge& self) { return get_representative_action(self); },
            py::keep_alive<0, 1>());

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
               std::shared_ptr<IApplicableActionGenerator> aag,
               std::shared_ptr<StateRepository> ssg,
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
            [](const std::vector<
                   std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const StateSpacesOptions& options) { return StateSpace::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = StateSpacesOptions())
        .def("compute_shortest_forward_distances_from_states", &StateSpace::compute_shortest_distances_from_states<ForwardTraversal>, py::arg("state_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &StateSpace::compute_shortest_distances_from_states<BackwardTraversal>,
             py::arg("state_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &StateSpace::compute_pairwise_shortest_state_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &StateSpace::compute_pairwise_shortest_state_distances<BackwardTraversal>)
        .def("get_problem", &StateSpace::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &StateSpace::get_pddl_factories)
        .def("get_aag", &StateSpace::get_aag)
        .def("get_ssg", &StateSpace::get_ssg)
        .def("get_state", &StateSpace::get_state, py::arg("state_index"))
        .def("get_states", &StateSpace::get_states, py::return_value_policy::reference_internal)
        .def("get_state_index", &StateSpace::get_state_index, py::arg("state"))
        .def("get_initial_state", &StateSpace::get_initial_state)
        .def("get_goal_states", &StateSpace::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &StateSpace::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_states",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_states", &StateSpace::get_num_states)
        .def("get_num_goal_states", &StateSpace::get_num_goal_states)
        .def("get_num_deadend_states", &StateSpace::get_num_deadend_states)
        .def("is_goal_state", &StateSpace::is_goal_state, py::arg("state_index"))
        .def("is_deadend_state", &StateSpace::is_deadend_state, py::arg("state_index"))
        .def("is_alive_state", &StateSpace::is_alive_state, py::arg("state_index"))
        .def("get_transitions", &StateSpace::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &StateSpace::get_transition_cost, py::arg("transition_index"))
        .def(
            "get_forward_adjacent_transitions",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const StateSpace& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_transitions", &StateSpace::get_num_transitions)
        .def("get_goal_distances", &StateSpace::get_goal_distances, py::return_value_policy::reference_internal)
        .def("get_max_goal_distance", &StateSpace::get_max_goal_distance)
        .def("sample_state_with_goal_distance",
             &StateSpace::sample_state_with_goal_distance,
             py::return_value_policy::reference_internal,
             py::arg("goal_distance"));

    // Certificate
    py::class_<Certificate, std::shared_ptr<Certificate>>(m, "Certificate")
        .def(py::init<size_t, size_t, std::string, ColorList>())
        .def("__eq__", &Certificate::operator==)
        .def("__hash__", [](const Certificate& self) { return std::hash<Certificate>()(self); })
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

    py::class_<FaithfulAbstractStateVertex>(m, "FaithfulAbstractStateVertex")
        .def("get_index", &FaithfulAbstractStateVertex::get_index)
        .def(
            "get_states",
            [](const FaithfulAbstractStateVertex& self) { return std::vector<State>(get_states(self).begin(), get_states(self).end()); },
            py::keep_alive<0, 1>())
        .def(
            "get_representative_state",
            [](const FaithfulAbstractStateVertex& self) { return get_representative_state(self); },
            py::keep_alive<0, 1>())
        .def(
            "get_certificate",
            [](const FaithfulAbstractStateVertex& self) { return get_certificate(self); },
            py::return_value_policy::reference_internal);

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
               std::shared_ptr<IApplicableActionGenerator> aag,
               std::shared_ptr<StateRepository> ssg,
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
            [](const std::vector<
                   std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return FaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionOptions())
        .def("compute_shortest_forward_distances_from_states",
             &FaithfulAbstraction::compute_shortest_distances_from_states<ForwardTraversal>,
             py::arg("state_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &FaithfulAbstraction::compute_shortest_distances_from_states<BackwardTraversal>,
             py::arg("state_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &FaithfulAbstraction::compute_pairwise_shortest_state_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &FaithfulAbstraction::compute_pairwise_shortest_state_distances<BackwardTraversal>)
        .def("get_problem", &FaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &FaithfulAbstraction::get_pddl_factories)
        .def("get_aag", &FaithfulAbstraction::get_aag)
        .def("get_ssg", &FaithfulAbstraction::get_ssg)
        .def("get_abstract_state_index", &FaithfulAbstraction::get_abstract_state_index, py::arg("state"))
        .def("get_states", &FaithfulAbstraction::get_states, py::return_value_policy::reference_internal)
        .def("get_concrete_to_abstract_state", &FaithfulAbstraction::get_concrete_to_abstract_state, py::return_value_policy::reference_internal)
        .def("get_initial_state", &FaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &FaithfulAbstraction::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &FaithfulAbstraction::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_states",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_states", &FaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &FaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &FaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &FaithfulAbstraction::is_goal_state, py::arg("state_index"))
        .def("is_deadend_state", &FaithfulAbstraction::is_deadend_state, py::arg("state_index"))
        .def("is_alive_state", &FaithfulAbstraction::is_alive_state, py::arg("state_index"))
        .def("get_transitions", &FaithfulAbstraction::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &FaithfulAbstraction::get_transition_cost, py::arg("transition_index"))
        .def(
            "get_forward_adjacent_transitions",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const FaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_transitions", &FaithfulAbstraction::get_num_transitions)
        .def("get_goal_distances", &FaithfulAbstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // GlobalFaithfulAbstraction

    py::class_<GlobalFaithfulAbstractState>(m, "GlobalFaithfulAbstractState")
        .def("__eq__", &GlobalFaithfulAbstractState::operator==)
        .def("__hash__", [](const GlobalFaithfulAbstractState& self) { return std::hash<GlobalFaithfulAbstractState>()(self); })
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
            [](const std::vector<
                   std::tuple<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return GlobalFaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionsOptions())
        .def("compute_shortest_forward_distances_from_states",
             &GlobalFaithfulAbstraction::compute_shortest_distances_from_states<ForwardTraversal>,
             py::arg("state_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &GlobalFaithfulAbstraction::compute_shortest_distances_from_states<BackwardTraversal>,
             py::arg("state_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &GlobalFaithfulAbstraction::compute_pairwise_shortest_state_distances<BackwardTraversal>)
        .def("get_index", &GlobalFaithfulAbstraction::get_index)
        .def("get_problem", &GlobalFaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_factories", &GlobalFaithfulAbstraction::get_pddl_factories)
        .def("get_aag", &GlobalFaithfulAbstraction::get_aag)
        .def("get_ssg", &GlobalFaithfulAbstraction::get_ssg)
        .def("get_abstractions", &GlobalFaithfulAbstraction::get_abstractions, py::return_value_policy::reference_internal)
        .def("get_abstract_state_index", py::overload_cast<State>(&GlobalFaithfulAbstraction::get_abstract_state_index, py::const_), py::arg("state"))
        .def("get_abstract_state_index",
             py::overload_cast<StateIndex>(&GlobalFaithfulAbstraction::get_abstract_state_index, py::const_),
             py::arg("state_index"))
        .def("get_states", &GlobalFaithfulAbstraction::get_states, py::return_value_policy::reference_internal)
        .def("get_concrete_to_abstract_state", &GlobalFaithfulAbstraction::get_concrete_to_abstract_state, py::return_value_policy::reference_internal)
        .def("get_global_state_index_to_state_index",
             &GlobalFaithfulAbstraction::get_global_state_index_to_state_index,
             py::return_value_policy::reference_internal)
        .def("get_initial_state", &GlobalFaithfulAbstraction::get_initial_state)
        .def("get_goal_states", &GlobalFaithfulAbstraction::get_goal_states, py::return_value_policy::reference_internal)
        .def("get_deadend_states", &GlobalFaithfulAbstraction::get_deadend_states, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_states",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_states", &GlobalFaithfulAbstraction::get_num_states)
        .def("get_num_goal_states", &GlobalFaithfulAbstraction::get_num_goal_states)
        .def("get_num_deadend_states", &GlobalFaithfulAbstraction::get_num_deadend_states)
        .def("is_goal_state", &GlobalFaithfulAbstraction::is_goal_state, py::arg("state_index"))
        .def("is_deadend_state", &GlobalFaithfulAbstraction::is_deadend_state, py::arg("state_index"))
        .def("is_alive_state", &GlobalFaithfulAbstraction::is_alive_state, py::arg("state_index"))
        .def("get_num_isomorphic_states", &GlobalFaithfulAbstraction::get_num_isomorphic_states)
        .def("get_num_non_isomorphic_states", &GlobalFaithfulAbstraction::get_num_non_isomorphic_states)
        .def("get_transitions", &GlobalFaithfulAbstraction::get_transitions, py::return_value_policy::reference_internal)
        .def("get_transition_cost", &GlobalFaithfulAbstraction::get_transition_cost)
        .def(
            "get_forward_adjacent_transitions",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const GlobalFaithfulAbstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
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
            "get_forward_adjacent_state_indices",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_state_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_states", &Abstraction::get_num_states)
        .def("get_num_goal_states", &Abstraction::get_num_goal_states)
        .def("get_num_deadend_states", &Abstraction::get_num_deadend_states)
        .def("is_goal_state", &Abstraction::is_goal_state, py::arg("state_index"))
        .def("is_deadend_state", &Abstraction::is_deadend_state, py::arg("state_index"))
        .def("is_alive_state", &Abstraction::is_alive_state, py::arg("state_index"))
        .def("get_transition_cost", &Abstraction::get_transition_cost, py::arg("transition_index"))
        .def(
            "get_forward_adjacent_transitions",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const Abstraction& self, StateIndex state)
            {
                auto iterator = self.get_adjacent_transition_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
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

    // EmptyVertex (used in StaticDigraph)
    py::class_<EmptyVertex>(m, "EmptyVertex")
        .def("__eq__", &EmptyVertex::operator==)
        .def("__hash__", [](const EmptyVertex& self) { return std::hash<EmptyVertex>()(self); })
        .def("get_index", &EmptyVertex::get_index);

    // EmptyEdge (used in StaticDigraph)
    py::class_<EmptyEdge>(m, "EmptyEdge")
        .def("__eq__", &EmptyEdge::operator==)
        .def("__hash__", [](const EmptyEdge& self) { return std::hash<EmptyEdge>()(self); })
        .def("get_index", &EmptyEdge::get_index)
        .def("get_source", &EmptyEdge::get_source)
        .def("get_target", &EmptyEdge::get_target);

    // StaticDigraph
    py::class_<StaticDigraph>(m, "StaticDigraph")  //
        .def(py::init<>())
        .def("__str__",
             [](const StaticDigraph& self)
             {
                 std::stringstream ss;
                 ss << self;
                 return ss.str();
             })
        .def("add_vertex", [](StaticDigraph& self) -> VertexIndex { return self.add_vertex(); })
        .def("add_directed_edge",
             [](StaticDigraph& self, VertexIndex source, VertexIndex target) -> EdgeIndex { return self.add_directed_edge(source, target); })
        .def("add_undirected_edge",
             [](StaticDigraph& self, VertexIndex source, VertexIndex target) -> std::pair<EdgeIndex, EdgeIndex>
             { return self.add_undirected_edge(source, target); })
        .def("clear", &StaticDigraph::clear)
        .def(
            "get_forward_adjacent_vertices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const StaticDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_vertices", &StaticDigraph::get_vertices, py::return_value_policy::reference_internal)
        .def("get_edges", &StaticDigraph::get_edges, py::return_value_policy::reference_internal)
        .def("get_num_vertices", &StaticDigraph::get_num_vertices)
        .def("get_num_edges", &StaticDigraph::get_num_edges);

    // ColorFunction
    py::class_<ColorFunction>(m, "ColorFunction")  //
        .def(
            "get_color_name",
            [](const ColorFunction& self, Color color) -> const std::string& { return self.get_color_name(color); },
            py::arg("color"));

    // ProblemColorFunction
    py::class_<ProblemColorFunction, ColorFunction>(m, "ProblemColorFunction")  //
        .def(py::init<Problem>(), py::arg("problem"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, Object object) -> Color { return self.get_color(object); },
            py::arg("object"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, GroundAtom<Static> atom, int pos) -> Color { return self.get_color(atom, pos); },
            py::arg("atom"),
            py::arg("position"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, GroundAtom<Fluent> atom, int pos) -> Color { return self.get_color(atom, pos); },
            py::arg("atom"),
            py::arg("position"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, GroundAtom<Derived> atom, int pos) -> Color { return self.get_color(atom, pos); },
            py::arg("atom"),
            py::arg("position"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, State state, GroundLiteral<Static> literal, int pos, bool mark_true_goal_literal) -> Color
            { return self.get_color(state, literal, pos, mark_true_goal_literal); },
            py::arg("state"),
            py::arg("literal"),
            py::arg("position"),
            py::arg("mark_true_goal_literal"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, State state, GroundLiteral<Fluent> literal, int pos, bool mark_true_goal_literal) -> Color
            { return self.get_color(state, literal, pos, mark_true_goal_literal); },
            py::arg("state"),
            py::arg("literal"),
            py::arg("position"),
            py::arg("mark_true_goal_literal"))
        .def(
            "get_color",
            [](const ProblemColorFunction& self, State state, GroundLiteral<Derived> literal, int pos, bool mark_true_goal_literal) -> Color
            { return self.get_color(state, literal, pos, mark_true_goal_literal); },
            py::arg("state"),
            py::arg("literal"),
            py::arg("position"),
            py::arg("mark_true_goal_literal"))
        .def("get_problem", &ProblemColorFunction::get_problem, py::return_value_policy::reference_internal)
        .def("get_name_to_color", &ProblemColorFunction::get_name_to_color, py::return_value_policy::reference_internal)
        .def("get_color_to_name", &ProblemColorFunction::get_color_to_name, py::return_value_policy::reference_internal);

    // ColoredVertex
    py::class_<ColoredVertex>(m, "ColoredVertex")
        .def("__eq__", &ColoredVertex::operator==)
        .def("__hash__", [](const ColoredVertex& self) { return std::hash<ColoredVertex>()(self); })
        .def("get_index", &ColoredVertex::get_index)
        .def("get_color", [](const ColoredVertex& self) { return get_color(self); });

    // StaticVertexColoredDigraph
    py::class_<StaticVertexColoredDigraph>(m, "StaticVertexColoredDigraph")  //
        .def(py::init<>())
        .def(
            "to_string",
            [](const StaticVertexColoredDigraph& self, const ColorFunction& color_function)
            {
                std::stringstream ss;
                ss << std::make_tuple(std::cref(self), std::cref(color_function));
                return ss.str();
            },
            py::arg("color_function"))
        .def(
            "add_vertex",
            [](StaticVertexColoredDigraph& self, Color color) -> VertexIndex { return self.add_vertex(color); },
            py::arg("color"))
        .def(
            "add_directed_edge",
            [](StaticVertexColoredDigraph& self, VertexIndex source, VertexIndex target) -> EdgeIndex { return self.add_directed_edge(source, target); },
            py::arg("source"),
            py::arg("target"))
        .def(
            "add_undirected_edge",
            [](StaticVertexColoredDigraph& self, VertexIndex source, VertexIndex target) -> std::pair<EdgeIndex, EdgeIndex>
            { return self.add_undirected_edge(source, target); },
            py::arg("source"),
            py::arg("target"))
        .def("clear", &StaticVertexColoredDigraph::clear)
        .def(
            "get_forward_adjacent_vertices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const StaticVertexColoredDigraph& self, VertexIndex vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_vertices", &StaticVertexColoredDigraph::get_vertices, py::return_value_policy::reference_internal)
        .def("get_edges", &StaticVertexColoredDigraph::get_edges, py::return_value_policy::reference_internal)
        .def("get_num_vertices", &StaticVertexColoredDigraph::get_num_vertices)
        .def("get_num_edges", &StaticVertexColoredDigraph::get_num_edges);

    m.def("compute_vertex_colors", &compute_vertex_colors, py::arg("vertex_colored_graph"));

    m.def("compute_sorted_vertex_colors", &compute_sorted_vertex_colors, py::arg("vertex_colored_graph"));

    // DenseNautyGraph
    py::class_<nauty_wrapper::DenseGraph>(m, "DenseNautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<StaticVertexColoredDigraph>())
        .def("add_edge", &nauty_wrapper::DenseGraph::add_edge, py::arg("source"), py::arg("target"))
        .def("compute_certificate", &nauty_wrapper::DenseGraph::compute_certificate)
        .def("clear", &nauty_wrapper::DenseGraph::clear, py::arg("num_vertices"));

    // SparseNautyGraph
    py::class_<nauty_wrapper::SparseGraph>(m, "SparseNautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>())
        .def(py::init<StaticVertexColoredDigraph>())
        .def("add_edge", &nauty_wrapper::SparseGraph::add_edge, py::arg("source"), py::arg("target"))
        .def("compute_certificate", &nauty_wrapper::SparseGraph::compute_certificate)
        .def("clear", &nauty_wrapper::SparseGraph::clear, py::arg("num_vertices"));

    // ObjectGraphPruningStrategy
    py::class_<ObjectGraphPruningStrategy>(m, "ObjectGraphPruningStrategy");

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
