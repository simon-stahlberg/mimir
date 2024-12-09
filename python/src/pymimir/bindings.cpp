#include "init_declarations.hpp"

#include <iterator>
#include <pybind11/attr.h>
#include <pybind11/detail/common.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace mimir;

namespace py = pybind11;

/**
 * Type casters
 */

template<>
struct py::detail::type_caster<FlatIndexList>
{
public:
    PYBIND11_TYPE_CASTER(FlatIndexList, _("FlatIndexList"));

    static py::handle cast(const FlatIndexList& cpp_list, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_list)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};

template<>
struct py::detail::type_caster<FlatBitset>
{
public:
    PYBIND11_TYPE_CASTER(FlatBitset, _("FlatBitset"));

    static py::handle cast(const FlatBitset& cpp_bitset, py::return_value_policy, py::handle)
    {
        py::list py_list;
        for (const auto& cpp_item : cpp_bitset)
        {
            py_list.append(py::cast(cpp_item));
        }
        return py_list.release();
    }
};

/**
 * Opaque types
 */

/* Common */

/* Formalism */
PYBIND11_MAKE_OPAQUE(ActionList);
PYBIND11_MAKE_OPAQUE(AtomList<Static>);
PYBIND11_MAKE_OPAQUE(AtomList<Fluent>);
PYBIND11_MAKE_OPAQUE(AtomList<Derived>);
PYBIND11_MAKE_OPAQUE(AxiomList);
PYBIND11_MAKE_OPAQUE(DomainList);
PYBIND11_MAKE_OPAQUE(EffectConditionalList);
PYBIND11_MAKE_OPAQUE(FunctionExpressionList);
PYBIND11_MAKE_OPAQUE(FunctionSkeletonList);
PYBIND11_MAKE_OPAQUE(FunctionList);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Static>);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Fluent>);
PYBIND11_MAKE_OPAQUE(GroundAtomList<Derived>);
PYBIND11_MAKE_OPAQUE(GroundFunctionExpressionList);
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
PYBIND11_MAKE_OPAQUE(VariableList);
PYBIND11_MAKE_OPAQUE(TermList);

/* Search */
PYBIND11_MAKE_OPAQUE(StateList);
PYBIND11_MAKE_OPAQUE(std::vector<GroundEffectConditional>);  // cannot bind cista vector
PYBIND11_MAKE_OPAQUE(GroundActionList);
PYBIND11_MAKE_OPAQUE(GroundAxiomList);

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
    void on_expand_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_expand_state_impl, state, problem, std::cref(pddl_repositories));
    }

    void
    on_generate_state_impl(State state, GroundAction action, ContinuousCost action_cost, Problem problem, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void,
                          DynamicAStarAlgorithmEventHandlerBase,
                          on_generate_state_impl,
                          state,
                          action,
                          action_cost,
                          problem,
                          std::cref(pddl_repositories));
    }
    void on_generate_state_relaxed_impl(State state,
                                        GroundAction action,
                                        ContinuousCost action_cost,
                                        Problem problem,
                                        const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void,
                          DynamicAStarAlgorithmEventHandlerBase,
                          on_generate_state_relaxed_impl,
                          state,
                          action,
                          action_cost,
                          problem,
                          std::cref(pddl_repositories));
    }
    void on_generate_state_not_relaxed_impl(State state,
                                            GroundAction action,
                                            ContinuousCost action_cost,
                                            Problem problem,
                                            const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void,
                          DynamicAStarAlgorithmEventHandlerBase,
                          on_generate_state_not_relaxed_impl,
                          state,
                          action,
                          action_cost,
                          problem,
                          std::cref(pddl_repositories));
    }
    void on_close_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_close_state_impl, state, problem, std::cref(pddl_repositories));
    }
    void on_finish_f_layer_impl(double f_value, uint64_t num_expanded_state, uint64_t num_generated_states) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_finish_f_layer_impl, f_value, num_expanded_state, num_generated_states);
    }
    void on_prune_state_impl(State state, Problem problem, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_prune_state_impl, state, problem, std::cref(pddl_repositories));
    }
    void on_start_search_impl(State start_state, Problem problem, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_start_search_impl, start_state, problem, std::cref(pddl_repositories));
    }
    /**
     * Note the trailing commas in the PYBIND11_OVERRIDE calls to name() and bark(). These are needed to portably implement a trampoline for a function that
     * does not take any arguments. For functions that take a nonzero number of arguments, the trailing comma must be omitted.
     */
    void on_end_search_impl() override { PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_end_search_impl, ); }
    void on_solved_impl(const Plan& plan, const PDDLRepositories& pddl_repositories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_solved_impl, plan, pddl_repositories);
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
        .def("__str__", [](const RequirementsImpl& self) { return to_string(self); })
        .def("__repr__", [](const RequirementsImpl& self) { return to_string(self); })
        .def("get_index", &RequirementsImpl::get_index)
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::reference_internal);

    py::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", [](const ObjectImpl& self) { return to_string(self); })
        .def("__repr__", [](const ObjectImpl& self) { return to_string(self); })
        .def("get_index", &ObjectImpl::get_index)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<ObjectList>::value);  // Ensure return by reference + keep alive
    auto list_class = py::bind_vector<ObjectList>(m, "ObjectList");

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", [](const VariableImpl& self) { return to_string(self); })
        .def("__repr__", [](const VariableImpl& self) { return to_string(self); })
        .def("get_index", &VariableImpl::get_index)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<VariableList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<VariableList>(m, "VariableList");

    py::class_<TermImpl>(m, "Term")  //
        .def(
            "get",
            [](const TermImpl& term) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, term.get_variant()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<TermList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<TermList>(m, "TermList");

    auto bind_predicate = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<PredicateImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const PredicateImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const PredicateImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &PredicateImpl<Tag>::get_index)
            .def("get_name", &PredicateImpl<Tag>::get_name, py::return_value_policy::reference_internal)
            .def("get_parameters", &PredicateImpl<Tag>::get_parameters, py::keep_alive<0, 1>(), py::return_value_policy::copy)
            .def("get_arity", &PredicateImpl<Tag>::get_arity);

        static_assert(!py::detail::vector_needs_copy<PredicateList<Tag>>::value);
        auto predicate_list = py::bind_vector<PredicateList<Tag>>(m, class_name + "List");
        py::bind_map<ToPredicateMap<std::string, Tag>>(m, "StringTo" + class_name + "Map");
    };
    bind_predicate("StaticPredicate", Static {});
    bind_predicate("FluentPredicate", Fluent {});
    bind_predicate("DerivedPredicate", Derived {});

    auto bind_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<AtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const AtomImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const AtomImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &AtomImpl<Tag>::get_index)
            .def("get_predicate", &AtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_terms", &AtomImpl<Tag>::get_terms, py::keep_alive<0, 1>(), py::return_value_policy::copy)
            .def("get_variables", &AtomImpl<Tag>::get_variables);

        static_assert(!py::detail::vector_needs_copy<AtomList<Tag>>::value);
        auto list_cls = py::bind_vector<AtomList<Tag>>(m, class_name + "List");
    };
    bind_atom("StaticAtom", Static {});
    bind_atom("FluentAtom", Fluent {});
    bind_atom("DerivedAtom", Derived {});

    py::class_<FunctionSkeletonImpl>(m, "FunctionSkeleton")  //
        .def("__str__", [](const FunctionSkeletonImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionSkeletonImpl& self) { return to_string(self); })
        .def("get_index", &FunctionSkeletonImpl::get_index)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference_internal)
        .def("get_parameters", &FunctionSkeletonImpl::get_parameters, py::keep_alive<0, 1>(), py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<FunctionSkeletonList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionSkeletonList>(m, "FunctionSkeletonList");

    py::class_<FunctionImpl>(m, "Function")  //
        .def("__str__", [](const FunctionImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionImpl& self) { return to_string(self); })
        .def("get_index", &FunctionImpl::get_index)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def("get_terms", &FunctionImpl::get_terms, py::keep_alive<0, 1>(), py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<FunctionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionList>(m, "FunctionList");

    py::class_<GroundFunctionImpl>(m, "GroundFunction")  //
        .def("__str__", [](const GroundFunctionImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionImpl::get_index)
        .def("get_function_skeleton", &GroundFunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def("get_objects", &GroundFunctionImpl::get_objects, py::keep_alive<0, 1>(), py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<GroundFunctionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundFunctionList>(m, "GroundFunctionList");

    auto bind_ground_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundAtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundAtomImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundAtomImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundAtomImpl<Tag>::get_index)
            .def("get_arity", &GroundAtomImpl<Tag>::get_arity)
            .def("get_predicate", &GroundAtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_objects", &GroundAtomImpl<Tag>::get_objects, py::keep_alive<0, 1>(), py::return_value_policy::copy);

        static_assert(!py::detail::vector_needs_copy<GroundAtomList<Tag>>::value);
        list_class = py::bind_vector<GroundAtomList<Tag>>(m, class_name + "List")
                         .def(
                             "lift",
                             [](const GroundAtomList<Tag>& ground_atoms, PDDLRepositories& pddl_repositories) { return lift(ground_atoms, pddl_repositories); },
                             py::arg("pddl_repositories"));
    };
    bind_ground_atom("StaticGroundAtom", Static {});
    bind_ground_atom("FluentGroundAtom", Fluent {});
    bind_ground_atom("DerivedGroundAtom", Derived {});

    auto bind_ground_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundLiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundLiteralImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundLiteralImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundLiteralImpl<Tag>::get_index)
            .def("get_atom", &GroundLiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &GroundLiteralImpl<Tag>::is_negated);

        static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Tag>>::value);
        auto list =
            py::bind_vector<GroundLiteralList<Tag>>(m, class_name + "List")
                .def(
                    "lift",
                    [](const GroundLiteralList<Tag>& ground_literals, PDDLRepositories& pddl_repositories) { return lift(ground_literals, pddl_repositories); },
                    py::arg("pddl_repositories"));
    };
    bind_ground_literal("StaticGroundLiteral", Static {});
    bind_ground_literal("FluentGroundLiteral", Fluent {});
    bind_ground_literal("DerivedGroundLiteral", Derived {});

    auto bind_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<LiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const LiteralImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const LiteralImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &LiteralImpl<Tag>::get_index)
            .def("get_atom", &LiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &LiteralImpl<Tag>::is_negated);

        static_assert(!py::detail::vector_needs_copy<LiteralList<Tag>>::value);
        auto list = py::bind_vector<LiteralList<Tag>>(m, class_name + "List");
    };
    bind_literal("StaticLiteral", Static {});
    bind_literal("FluentLiteral", Fluent {});
    bind_literal("DerivedLiteral", Derived {});

    py::class_<NumericFluentImpl>(m, "NumericFluent")  //
        .def("__str__", [](const NumericFluentImpl& self) { return to_string(self); })
        .def("__repr__", [](const NumericFluentImpl& self) { return to_string(self); })
        .def("get_index", &NumericFluentImpl::get_index)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference_internal)
        .def("get_number", &NumericFluentImpl::get_number);
    static_assert(!py::detail::vector_needs_copy<NumericFluentList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<NumericFluentList>(m, "NumericFluentList");

    py::class_<EffectStripsImpl>(m, "EffectStrips")  //
        .def("__str__", [](const EffectStripsImpl& self) { return to_string(self); })
        .def("__repr__", [](const EffectStripsImpl& self) { return to_string(self); })
        .def("get_index", &EffectStripsImpl::get_index)
        .def("get_effects", &EffectStripsImpl::get_effects, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_function_expression", &EffectStripsImpl::get_function_expression, py::return_value_policy::reference_internal);

    py::class_<FunctionExpressionImpl>(m, "FunctionExpression")  //
        .def(
            "get",
            [](const FunctionExpressionImpl& fexpr) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, fexpr.get_variant()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionExpressionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionExpressionList>(m, "FunctionExpressionList");

    py::class_<EffectConditionalImpl>(m, "EffectConditional")  //
        .def("__str__", [](const EffectConditionalImpl& self) { return to_string(self); })
        .def("__repr__", [](const EffectConditionalImpl& self) { return to_string(self); })
        .def("get_index", &EffectConditionalImpl::get_index)
        .def("get_parameters", &EffectConditionalImpl::get_parameters, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_static_conditions", &EffectConditionalImpl::get_conditions<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_conditions", &EffectConditionalImpl::get_conditions<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_derived_conditions", &EffectConditionalImpl::get_conditions<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_effects", &EffectConditionalImpl::get_effects, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectConditionalList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectConditionalList>(m, "EffectConditionalList");

    py::class_<FunctionExpressionNumberImpl>(m, "FunctionExpressionNumber")  //
        .def("__str__", [](const FunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionNumberImpl::get_index)
        .def("get_number", &FunctionExpressionNumberImpl::get_number);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m, "FunctionExpressionBinaryOperator")  //
        .def("__str__", [](const FunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionBinaryOperatorImpl::get_index)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def("get_left_function_expression", &FunctionExpressionBinaryOperatorImpl::get_left_function_expression, py::return_value_policy::reference_internal)
        .def("get_right_function_expression",
             &FunctionExpressionBinaryOperatorImpl::get_right_function_expression,
             py::return_value_policy::reference_internal);

    py::class_<FunctionExpressionMultiOperatorImpl>(m, "FunctionExpressionMultiOperator")  //
        .def("__str__", [](const FunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionMultiOperatorImpl::get_index)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def("get_function_expressions", &FunctionExpressionMultiOperatorImpl::get_function_expressions, py::keep_alive<0, 1>(), py::return_value_policy::copy);

    py::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", [](const FunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionMinusImpl::get_index)
        .def("get_function_expression", &FunctionExpressionMinusImpl::get_function_expression, py::return_value_policy::reference_internal);

    py::class_<FunctionExpressionFunctionImpl>(m, "FunctionExpressionFunction")  //
        .def("__str__", [](const FunctionExpressionFunctionImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionFunctionImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionFunctionImpl::get_index)
        .def("get_function", &FunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference_internal);

    py::class_<GroundFunctionExpressionImpl>(m, "GroundFunctionExpression")  //
        .def(
            "get",
            [](const GroundFunctionExpressionImpl& fexpr) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, fexpr.get_variant()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundFunctionExpressionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundFunctionExpressionList>(m, "GroundFunctionExpressionList");

    py::class_<GroundFunctionExpressionNumberImpl>(m, "GroundFunctionExpressionNumber")  //
        .def("__str__", [](const GroundFunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionNumberImpl::get_index)
        .def("get_number", &GroundFunctionExpressionNumberImpl::get_number);

    py::class_<GroundFunctionExpressionBinaryOperatorImpl>(m, "GroundFunctionExpressionBinaryOperator")  //
        .def("__str__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionBinaryOperatorImpl::get_index)
        .def("get_binary_operator", &GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator)
        .def("get_left_function_expression",
             &GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression,
             py::return_value_policy::reference_internal)
        .def("get_right_function_expression",
             &GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression,
             py::return_value_policy::reference_internal);

    py::class_<GroundFunctionExpressionMultiOperatorImpl>(m, "GroundFunctionExpressionMultiOperator")  //
        .def("__str__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionMultiOperatorImpl::get_index)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator)
        .def("get_function_expressions",
             &GroundFunctionExpressionMultiOperatorImpl::get_function_expressions,
             py::keep_alive<0, 1>(),
             py::return_value_policy::copy);

    py::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", [](const GroundFunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionMinusImpl::get_index)
        .def("get_function_expression", &GroundFunctionExpressionMinusImpl::get_function_expression, py::return_value_policy::reference_internal);

    py::class_<GroundFunctionExpressionFunctionImpl>(m, "GroundFunctionExpressionFunction")  //
        .def("__str__", [](const GroundFunctionExpressionFunctionImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionFunctionImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionFunctionImpl::get_index)
        .def("get_function", &GroundFunctionExpressionFunctionImpl::get_function, py::return_value_policy::reference_internal);

    py::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", [](const OptimizationMetricImpl& self) { return to_string(self); })
        .def("__repr__", [](const OptimizationMetricImpl& self) { return to_string(self); })
        .def("get_index", &OptimizationMetricImpl::get_index)
        .def("get_function_expression", &OptimizationMetricImpl::get_function_expression, py::return_value_policy::reference_internal)
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference_internal);

    py::class_<ActionImpl>(m, "Action")  //
        .def("__str__", [](const ActionImpl& self) { return to_string(self); })
        .def("__repr__", [](const ActionImpl& self) { return to_string(self); })
        .def("get_index", &ActionImpl::get_index)
        .def("get_name", &ActionImpl::get_name, py::return_value_policy::copy)
        .def("get_parameters", &ActionImpl::get_parameters, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_static_conditions", &ActionImpl::get_conditions<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_conditions", &ActionImpl::get_conditions<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_derived_conditions", &ActionImpl::get_conditions<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_strips_effect", &ActionImpl::get_strips_effect, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_conditional_effects", &ActionImpl::get_conditional_effects, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_arity", &ActionImpl::get_arity);
    static_assert(!py::detail::vector_needs_copy<ActionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<ActionList>(m, "ActionList");

    py::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", [](const AxiomImpl& self) { return to_string(self); })
        .def("__repr__", [](const AxiomImpl& self) { return to_string(self); })
        .def("get_index", &AxiomImpl::get_index)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference_internal)
        .def("get_static_conditions", &AxiomImpl::get_conditions<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_conditions", &AxiomImpl::get_conditions<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_derived_conditions", &AxiomImpl::get_conditions<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_arity", &AxiomImpl::get_arity);
    static_assert(!py::detail::vector_needs_copy<AxiomList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<AxiomList>(m, "AxiomList");

    py::class_<DomainImpl>(m, "Domain")  //
        .def("__str__", [](const DomainImpl& self) { return to_string(self); })
        .def("__repr__", [](const DomainImpl& self) { return to_string(self); })
        .def("get_index", &DomainImpl::get_index)
        .def(
            "get_filepath",
            [](const DomainImpl& self) { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; },
            py::return_value_policy::copy)
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::copy)
        .def("get_constants", &DomainImpl::get_constants, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_static_predicates", &DomainImpl::get_predicates<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_predicates", &DomainImpl::get_predicates<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_derived_predicates", &DomainImpl::get_predicates<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_functions", &DomainImpl::get_functions, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_actions", &DomainImpl::get_actions, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::reference_internal)
        .def("get_name_to_static_predicate", &DomainImpl::get_name_to_predicate<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_name_to_fluent_predicate", &DomainImpl::get_name_to_predicate<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_name_to_derived_predicate", &DomainImpl::get_name_to_predicate<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<DomainList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<DomainList>(m, "DomainList");

    py::class_<ProblemImpl>(m, "Problem")  //
        .def("__str__", [](const ProblemImpl& self) { return to_string(self); })
        .def("__repr__", [](const ProblemImpl& self) { return to_string(self); })
        .def("get_index", &ProblemImpl::get_index)
        .def(
            "get_filepath",
            [](const ProblemImpl& self)
            { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; },
            py::return_value_policy::copy)
        .def("get_name", &ProblemImpl::get_name, py::return_value_policy::copy)
        .def("get_domain", &ProblemImpl::get_domain, py::return_value_policy::reference_internal)
        .def("get_requirements", &ProblemImpl::get_requirements, py::return_value_policy::reference_internal)
        .def("get_objects", &ProblemImpl::get_objects, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_static_initial_literals", &ProblemImpl::get_static_initial_literals, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_initial_literals", &ProblemImpl::get_fluent_initial_literals, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_numeric_fluents", &ProblemImpl::get_numeric_fluents, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, py::return_value_policy::reference_internal)
        .def("get_static_goal_condition", &ProblemImpl::get_goal_condition<Static>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_fluent_goal_condition", &ProblemImpl::get_goal_condition<Fluent>, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_derived_goal_condition", &ProblemImpl::get_goal_condition<Derived>, py::keep_alive<0, 1>(), py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<ProblemList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<ProblemList>(m, "ProblemList");

    py::class_<PDDLRepositories, std::shared_ptr<PDDLRepositories>>(m, "PDDLRepositories")  //
        .def(
            "get_static_ground_atoms",
            [](const PDDLRepositories& self) { return GroundAtomList<Static>(self.get_ground_atoms<Static>().begin(), self.get_ground_atoms<Static>().end()); },
            py::keep_alive<0, 1>())
        .def("get_static_ground_atom", &PDDLRepositories::get_ground_atom<Static>, py::return_value_policy::reference_internal)
        .def(
            "get_static_ground_atoms",
            [](const PDDLRepositories& self) { return GroundAtomList<Fluent>(self.get_ground_atoms<Fluent>().begin(), self.get_ground_atoms<Fluent>().end()); },
            py::keep_alive<0, 1>())
        .def("get_fluent_ground_atom", &PDDLRepositories::get_ground_atom<Fluent>, py::return_value_policy::reference_internal)
        .def(
            "get_static_ground_atoms",
            [](const PDDLRepositories& self)
            { return GroundAtomList<Derived>(self.get_ground_atoms<Derived>().begin(), self.get_ground_atoms<Derived>().end()); },
            py::keep_alive<0, 1>())
        .def("get_derived_ground_atom", &PDDLRepositories::get_ground_atom<Derived>, py::return_value_policy::reference_internal)
        .def("get_static_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Static, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_fluent_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Fluent, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_derived_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Derived, std::vector<size_t>>, py::const_),
             py::keep_alive<0, 1>())
        .def("get_object", &PDDLRepositories::get_object, py::return_value_policy::reference_internal);

    py::class_<PDDLParser>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>(), py::arg("domain_path"), py::arg("problem_path"))
        .def("get_domain", &PDDLParser::get_domain, py::return_value_policy::reference_internal)
        .def("get_problem", &PDDLParser::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &PDDLParser::get_pddl_repositories);

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
    py::class_<StateImpl>(m, "State")  //
        .def("__hash__", [](const StateImpl& self) { return self.get_index(); })
        .def("__eq__", [](const StateImpl& lhs, const StateImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("get_fluent_atoms", py::overload_cast<>(&StateImpl::get_atoms<Fluent>, py::const_), py::return_value_policy::copy)
        .def("get_derived_atoms", py::overload_cast<>(&StateImpl::get_atoms<Derived>, py::const_), py::return_value_policy::copy)
        .def("contains", py::overload_cast<GroundAtom<Fluent>>(&StateImpl::contains<Fluent>, py::const_), py::arg("atom"))
        .def("contains", py::overload_cast<GroundAtom<Derived>>(&StateImpl::contains<Derived>, py::const_), py::arg("atom"))
        .def("superset_of", py::overload_cast<const GroundAtomList<Fluent>&>(&StateImpl::superset_of<Fluent>, py::const_), py::arg("atoms"))
        .def("superset_of", py::overload_cast<const GroundAtomList<Derived>&>(&StateImpl::superset_of<Derived>, py::const_), py::arg("atoms"))
        .def("literal_holds", py::overload_cast<GroundLiteral<Fluent>>(&StateImpl::literal_holds<Fluent>, py::const_), py::arg("literal"))
        .def("literal_holds", py::overload_cast<GroundLiteral<Derived>>(&StateImpl::literal_holds<Derived>, py::const_), py::arg("literal"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Fluent>&>(&StateImpl::literals_hold<Fluent>, py::const_), py::arg("literals"))
        .def("literals_hold", py::overload_cast<const GroundLiteralList<Derived>&>(&StateImpl::literals_hold<Derived>, py::const_), py::arg("literals"))
        .def(
            "to_string",
            [](const StateImpl& self, Problem problem, const PDDLRepositories& pddl_repositories)
            {
                std::stringstream ss;
                ss << std::make_tuple(problem, State(&self), std::cref(pddl_repositories));
                return ss.str();
            },
            py::arg("problem"),
            py::arg("pddl_repositories"))
        .def("get_index", py::overload_cast<>(&StateImpl::get_index, py::const_));
    static_assert(!py::detail::vector_needs_copy<StateList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<StateList>(m, "StateList");
    bind_const_span<std::span<const State>>(m, "StateSpan");
    bind_const_index_grouped_vector<IndexGroupedVector<const State>>(m, "IndexGroupedVector");

    /* Action */
    py::class_<GroundEffectStrips>(m, "GroundEffectStrips")
        .def("get_positive_effects", py::overload_cast<>(&GroundEffectStrips::get_positive_effects, py::const_), py::return_value_policy::copy)
        .def("get_negative_effects", py::overload_cast<>(&GroundEffectStrips::get_negative_effects, py::const_), py::return_value_policy::copy)
        .def("get_cost", py::overload_cast<>(&GroundEffectStrips::get_cost, py::const_), py::return_value_policy::copy);

    py::class_<GroundEffectFluentLiteral>(m, "GroundEffectFluentLiteral")
        .def_readonly("is_negated", &GroundEffectFluentLiteral::is_negated)
        .def_readonly("atom_index", &GroundEffectFluentLiteral::atom_index);

    py::class_<GroundConditionStrips>(m, "GroundConditionStrips")
        .def("get_fluent_positive_condition",
             py::overload_cast<>(&GroundConditionStrips::get_positive_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_positive_condition",
             py::overload_cast<>(&GroundConditionStrips::get_positive_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_positive_condition",
             py::overload_cast<>(&GroundConditionStrips::get_positive_precondition<Derived>, py::const_),
             py::return_value_policy::copy)
        .def("get_fluent_negative_condition",
             py::overload_cast<>(&GroundConditionStrips::get_negative_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_negative_condition",
             py::overload_cast<>(&GroundConditionStrips::get_negative_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_negative_condition",
             py::overload_cast<>(&GroundConditionStrips::get_negative_precondition<Derived>, py::const_),
             py::return_value_policy::copy)
        .def("is_dynamically_applicable", &GroundConditionStrips::is_dynamically_applicable, py::arg("state"))
        .def(
            "is_applicable",
            [](const GroundConditionStrips& self, Problem problem, State state) { return self.is_applicable(problem, state); },
            py::arg("problem"),
            py::arg("state"));
    py::class_<GroundEffectConditional>(m, "GroundEffectConditional")
        .def("get_fluent_positive_condition",
             py::overload_cast<>(&GroundEffectConditional::get_positive_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_positive_condition",
             py::overload_cast<>(&GroundEffectConditional::get_positive_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_positive_condition",
             py::overload_cast<>(&GroundEffectConditional::get_positive_precondition<Derived>, py::const_),
             py::return_value_policy::copy)
        .def("get_fluent_negative_condition",
             py::overload_cast<>(&GroundEffectConditional::get_negative_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_negative_condition",
             py::overload_cast<>(&GroundEffectConditional::get_negative_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_negative_condition",
             py::overload_cast<>(&GroundEffectConditional::get_negative_precondition<Derived>, py::const_),
             py::return_value_policy::copy)
        .def("get_fluent_effect_literals",
             py::overload_cast<>(&GroundEffectConditional::get_fluent_effect_literals, py::const_),
             py::return_value_policy::copy);
    static_assert(!py::detail::vector_needs_copy<std::vector<GroundEffectConditional>>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<std::vector<GroundEffectConditional>>(m, "GroundEffectConditionalList");

    /* GroundAction */
    py::class_<GroundActionImpl>(m, "GroundAction")  //
        .def("__hash__", [](const GroundActionImpl& self) { return self.get_index(); })
        .def("__eq__", [](const GroundActionImpl& lhs, const GroundActionImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("to_string",
             [](const GroundActionImpl& self, const PDDLRepositories& pddl_repositories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAction(&self), std::cref(pddl_repositories), FullActionFormatterTag {});
                 return ss.str();
             })
        .def("to_string_for_plan",
             [](const GroundActionImpl& self, const PDDLRepositories& pddl_repositories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAction(&self), std::cref(pddl_repositories), PlanActionFormatterTag {});
                 return ss.str();
             })
        .def("get_index", py::overload_cast<>(&GroundActionImpl::get_index, py::const_), py::return_value_policy::copy)
        .def("get_action_index", py::overload_cast<>(&GroundActionImpl::get_action_index, py::const_), py::return_value_policy::copy)
        .def("get_object_indices", py::overload_cast<>(&GroundActionImpl::get_object_indices, py::const_), py::return_value_policy::copy)
        .def("get_strips_precondition",
             py::overload_cast<>(&GroundActionImpl::get_strips_precondition, py::const_),
             py::return_value_policy::reference_internal)
        .def("get_strips_effect", py::overload_cast<>(&GroundActionImpl::get_strips_effect, py::const_), py::return_value_policy::reference_internal)
        .def(
            "get_conditional_effects",
            [](const GroundActionImpl& self)
            { return std::vector<GroundEffectConditional>(self.get_conditional_effects().begin(), self.get_conditional_effects().end()); },
            py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundActionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundActionList>(m, "GroundActionList");
    bind_const_span<std::span<const GroundAction>>(m, "GroundActionSpan");

    /* GroundEffectDerivedLiteral */
    py::class_<GroundEffectDerivedLiteral>(m, "GroundEffectDerivedLiteral")
        .def_readonly("is_negated", &GroundEffectDerivedLiteral::is_negated)
        .def_readonly("atom_index", &GroundEffectDerivedLiteral::atom_index);

    /* GroundAxiom */
    py::class_<GroundAxiomImpl>(m, "GroundAxiom")  //
        .def("__hash__", [](const GroundAxiomImpl& self) { return self.get_index(); })
        .def("__eq__", [](const GroundAxiomImpl& lhs, const GroundAxiomImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("to_string",
             [](const GroundAxiomImpl& self, const PDDLRepositories& pddl_repositories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAxiom(&self), std::cref(pddl_repositories));
                 return ss.str();
             })
        .def("get_index", py::overload_cast<>(&GroundAxiomImpl::get_index, py::const_), py::return_value_policy::copy)
        .def("get_axiom_index", py::overload_cast<>(&GroundAxiomImpl::get_axiom_index, py::const_), py::return_value_policy::copy)
        .def("get_object_indices", py::overload_cast<>(&GroundAxiomImpl::get_object_indices, py::const_), py::return_value_policy::copy)
        .def("get_strips_precondition", py::overload_cast<>(&GroundAxiomImpl::get_strips_precondition, py::const_), py::return_value_policy::reference_internal)
        .def("get_derived_effect", py::overload_cast<>(&GroundAxiomImpl::get_derived_effect, py::const_), py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<GroundAxiomList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundAxiomList>(m, "GroundAxiomList");

    /* Plan */
    py::class_<Plan>(m, "Plan")  //
        .def("__len__", [](const Plan& arg) { return arg.get_actions().size(); })
        .def("get_actions", &Plan::get_actions)
        .def("get_cost", &Plan::get_cost);

    /* */
    auto bind_assignment_set = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<AssignmentSet<Tag>>(m, class_name.c_str())
            .def(py::init<Problem, PredicateList<Tag>, GroundAtomList<Tag>>(), py::arg("problem"), py::arg("predicates"), py::arg("ground_atoms"));
    };
    bind_assignment_set("StaticAssignmentSet", Static {});
    bind_assignment_set("FluentAssignmentSet", Fluent {});
    bind_assignment_set("DerivedAssignmentSet", Derived {});

    /* ConditionGrounder */
    py::class_<ConditionGrounder>(m, "ConditionGrounder")  //
        .def(py::init<Problem,
                      std::shared_ptr<PDDLRepositories>,
                      VariableList,
                      LiteralList<Static>,
                      LiteralList<Fluent>,
                      LiteralList<Derived>,
                      AssignmentSet<Static>>(),
             py::arg("problem"),
             py::arg("pddl_repositories"),
             py::arg("parameters"),
             py::arg("static_literals"),
             py::arg("fluent_literals"),
             py::arg("derived_literals"),
             py::arg("static_assignment_set"))
        .def("create_ground_conjunction_generator",  // we use the c++ name already for future update of pybind11 in regards of std::generator
             [](ConditionGrounder& self, State state, size_t max_num_groundings)
             {
                 auto result =
                     std::vector<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>> {};

                 auto count = size_t(0);
                 for (const auto& ground_conjunction : self.create_ground_conjunction_generator(state))
                 {
                     if (count >= max_num_groundings)
                     {
                         break;
                     }
                     result.push_back(ground_conjunction);
                     ++count;
                 }
                 return result;  // TODO: keep alive is not set. Lets leave it "buggy" for this specialized piece of code...
             });

    /* ActionGrounder */
    py::class_<ActionGrounder>(m, "ActionGrounder")  //
        .def("get_problem", &ActionGrounder::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &ActionGrounder::get_pddl_repositories, py::return_value_policy::copy)
        .def("get_action_precondition_grounders", &ActionGrounder::get_action_precondition_grounders, py::return_value_policy::copy)
        .def("ground_action", &ActionGrounder::ground_action, py::return_value_policy::reference_internal, py::arg("action"), py::arg("binding"))
        .def("get_ground_actions", &ActionGrounder::get_ground_actions, py::return_value_policy::reference_internal)
        .def("get_ground_action", &ActionGrounder::get_ground_action, py::return_value_policy::reference_internal, py::arg("action_index"))
        .def("get_num_ground_actions", &ActionGrounder::get_num_ground_actions, py::return_value_policy::reference_internal);

    /* ApplicableActionGenerators */
    py::class_<IApplicableActionGenerator, std::shared_ptr<IApplicableActionGenerator>>(m, "IApplicableActionGenerator")
        .def(
            "create_applicable_action_generator",  // we use the c++ name already for future update of pybind11 in regards of std::generator
            [](IApplicableActionGenerator& self, State state)
            {
                // TODO: pybind11 does not support std::generator. Is there a simple workaround WITHOUT introducing additional code?
                auto actions = GroundActionList {};
                for (const auto& action : self.create_applicable_action_generator(state))
                {
                    actions.push_back(action);
                }
                return actions;
            },
            py::keep_alive<0, 1>(),
            py::arg("state"))
        .def("get_action_grounder", py::overload_cast<>(&IApplicableActionGenerator::get_action_grounder), py::return_value_policy::reference_internal);

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
               std::shared_ptr<DebugLiftedApplicableActionGeneratorEventHandler>>(m,
                                                                                  "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<LiftedApplicableActionGenerator>>(
        m,
        "LiftedApplicableActionGenerator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLRepositories>>(), py::arg("problem"), py::arg("pddl_repositories"))
        .def(py::init<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler>>(),
             py::arg("problem"),
             py::arg("pddl_repositories"),
             py::arg("event_handler"));

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
               std::shared_ptr<DebugGroundedApplicableActionGeneratorEventHandler>>(m,
                                                                                    "DebugGroundedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<GroundedApplicableActionGenerator>>(
        m,
        "GroundedApplicableActionGenerator");

    /* AxiomGrounder */
    py::class_<AxiomGrounder>(m, "AxiomGrounder")  //
        .def("get_problem", &AxiomGrounder::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &AxiomGrounder::get_pddl_repositories, py::return_value_policy::copy)
        .def("get_axiom_precondition_grounders", &AxiomGrounder::get_axiom_precondition_grounders, py::return_value_policy::copy)
        .def("ground_axiom", &AxiomGrounder::ground_axiom, py::return_value_policy::reference_internal, py::arg("axiom"), py::arg("binding"))
        .def("get_ground_axioms", &AxiomGrounder::get_ground_axioms, py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("get_ground_axiom", &AxiomGrounder::get_ground_axiom, py::return_value_policy::reference_internal, py::arg("axiom_index"))
        .def("get_num_ground_axioms", &AxiomGrounder::get_num_ground_axioms, py::return_value_policy::reference_internal);

    /* IAxiomEvaluator */
    py::class_<IAxiomEvaluator, std::shared_ptr<IAxiomEvaluator>>(m, "IAxiomEvaluator")  //
        .def("get_axiom_grounder", py::overload_cast<>(&IAxiomEvaluator::get_axiom_grounder), py::return_value_policy::reference_internal);

    // Lifted
    py::class_<ILiftedAxiomEvaluatorEventHandler, std::shared_ptr<ILiftedAxiomEvaluatorEventHandler>>(m, "ILiftedAxiomEvaluatorEventHandler");  //
    py::class_<DefaultLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler, std::shared_ptr<DefaultLiftedAxiomEvaluatorEventHandler>>(
        m,
        "DefaultLiftedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugLiftedAxiomEvaluatorEventHandler, ILiftedAxiomEvaluatorEventHandler, std::shared_ptr<DebugLiftedAxiomEvaluatorEventHandler>>(
        m,
        "DebugLiftedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedAxiomEvaluator, IAxiomEvaluator, std::shared_ptr<LiftedAxiomEvaluator>>(m, "LiftedAxiomEvaluator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLRepositories>>(), py::arg("problem"), py::arg("pddl_repositories"))
        .def(py::init<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<ILiftedAxiomEvaluatorEventHandler>>(),
             py::arg("problem"),
             py::arg("pddl_repositories"),
             py::arg("event_handler"));

    // Grounded
    py::class_<IGroundedAxiomEvaluatorEventHandler, std::shared_ptr<IGroundedAxiomEvaluatorEventHandler>>(m, "IGroundedAxiomEvaluatorEventHandler");  //
    py::class_<DefaultGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler, std::shared_ptr<DefaultGroundedAxiomEvaluatorEventHandler>>(
        m,
        "DefaultGroundedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<DebugGroundedAxiomEvaluatorEventHandler, IGroundedAxiomEvaluatorEventHandler, std::shared_ptr<DebugGroundedAxiomEvaluatorEventHandler>>(
        m,
        "DebugGroundedAxiomEvaluatorEventHandler")  //
        .def(py::init<>());
    py::class_<GroundedAxiomEvaluator, IAxiomEvaluator, std::shared_ptr<GroundedAxiomEvaluator>>(m, "GroundedAxiomEvaluator")  //
        ;

    /* DeleteRelaxedProblemExplorator */
    py::class_<DeleteRelaxedProblemExplorator>(m, "DeleteRelaxedProblemExplorator")
        .def(py::init<Problem, std::shared_ptr<PDDLRepositories>>(), py::arg("problem"), py::arg("pddl_repositories"))
        .def("create_grounded_axiom_evaluator",
             &DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator,
             py::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedAxiomEvaluatorEventHandler>())
        .def("create_grounded_applicable_action_generator",
             &DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator,
             py::arg("axiom_evaluator_event_handler") = std::make_shared<DefaultGroundedApplicableActionGeneratorEventHandler>())
        .def("get_problem", &DeleteRelaxedProblemExplorator::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &DeleteRelaxedProblemExplorator::get_pddl_repositories, py::return_value_policy::copy);

    /* StateRepository */
    py::class_<StateRepository, std::shared_ptr<StateRepository>>(m, "StateRepository")  //
        .def(py::init<std::shared_ptr<IAxiomEvaluator>>(), py::arg("axiom_evaluator"))
        .def("get_or_create_initial_state", &StateRepository::get_or_create_initial_state, py::return_value_policy::reference_internal)
        .def("get_or_create_state", &StateRepository::get_or_create_state, py::return_value_policy::reference_internal, py::arg("atoms"))
        .def("get_or_create_successor_state",
             &StateRepository::get_or_create_successor_state,
             py::return_value_policy::copy,  // returns pair (State, ContinuousCost): TODO: we must ensure that State keeps StateRepository alive!
             py::arg("state"),
             py::arg("action"))
        .def("get_state_count", &StateRepository::get_state_count)
        .def("get_reached_fluent_ground_atoms_bitset", &StateRepository::get_reached_fluent_ground_atoms_bitset, py::return_value_policy::copy)
        .def("get_reached_derived_ground_atoms_bitset", &StateRepository::get_reached_derived_ground_atoms_bitset, py::return_value_policy::copy);

    /* Heuristics */
    py::class_<IHeuristic, IPyHeuristic, std::shared_ptr<IHeuristic>>(m, "IHeuristic").def(py::init<>());
    py::class_<BlindHeuristic, IHeuristic, std::shared_ptr<BlindHeuristic>>(m, "BlindHeuristic").def(py::init<>());

    /* Algorithms */

    // SearchResult
    py::class_<SearchResult>(m, "SearchResult")
        .def(py::init<>())
        .def_readwrite("status", &SearchResult::status)
        .def_readwrite("plan", &SearchResult::plan)
        .def_readwrite("goal_state", &SearchResult::goal_state);

    // IAlgorithm
    py::class_<IAlgorithm, std::shared_ptr<IAlgorithm>>(m, "IAlgorithm")  //
        .def("find_solution", py::overload_cast<>(&IAlgorithm::find_solution), py::keep_alive<0, 1>(), py::return_value_policy::copy)
        .def("find_solution",
             py::overload_cast<State>(&IAlgorithm::find_solution),
             py::arg("start_state"),
             py::keep_alive<0, 1>(),
             py::return_value_policy::copy);

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
    py::class_<IAStarAlgorithmEventHandler, std::shared_ptr<IAStarAlgorithmEventHandler>>(m,
                                                                                          "IAStarAlgorithmEventHandler")  //
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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>, std::shared_ptr<IHeuristic>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("heuristic"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IHeuristic>,
                      std::shared_ptr<IAStarAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("heuristic"),
             py::arg("event_handler"));

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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>, std::shared_ptr<IBrFSAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("event_handler"));

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

    py::class_<IWAlgorithmStatistics>(m, "IWAlgorithmStatistics")  //
        .def("get_effective_width", &IWAlgorithmStatistics::get_effective_width)
        .def("get_brfs_statistics_by_arity", &IWAlgorithmStatistics::get_brfs_statistics_by_arity);
    py::class_<IIWAlgorithmEventHandler, std::shared_ptr<IIWAlgorithmEventHandler>>(m, "IIWAlgorithmEventHandler")
        .def("get_statistics", &IIWAlgorithmEventHandler::get_statistics);
    py::class_<DefaultIWAlgorithmEventHandler, IIWAlgorithmEventHandler, std::shared_ptr<DefaultIWAlgorithmEventHandler>>(m, "DefaultIWAlgorithmEventHandler")
        .def(py::init<>());
    py::class_<IWAlgorithm, IAlgorithm, std::shared_ptr<IWAlgorithm>>(m, "IWAlgorithm")
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>, size_t>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("max_arity"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      std::shared_ptr<StateRepository>,
                      size_t,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("max_arity"),
             py::arg("brfs_event_handler"),
             py::arg("iw_event_handler"));

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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>, size_t>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("max_arity"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      std::shared_ptr<StateRepository>,
                      size_t,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>,
                      std::shared_ptr<ISIWAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("state_repository"),
             py::arg("max_arity"),
             py::arg("brfs_event_handler"),
             py::arg("iw_event_handler"),
             py::arg("siw_event_handler"));

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
            py::return_value_policy::reference_internal);

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
            py::return_value_policy::reference_internal);

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
            py::return_value_policy::reference_internal);

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
               std::shared_ptr<PDDLRepositories> factories,
               std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
               std::shared_ptr<StateRepository> state_repository,
               const StateSpaceOptions& options) { return StateSpace::create(problem, factories, applicable_action_generator, state_repository, options); },
            py::arg("problem"),
            py::arg("factories"),
            py::arg("applicable_action_generator"),
            py::arg("state_repository"),
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
                   std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const StateSpacesOptions& options) { return StateSpace::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = StateSpacesOptions())
        .def("compute_shortest_forward_distances_from_states",
             &StateSpace::compute_shortest_distances_from_vertices<ForwardTraversal>,
             py::arg("state_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &StateSpace::compute_shortest_distances_from_vertices<BackwardTraversal>,
             py::arg("state_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &StateSpace::compute_pairwise_shortest_vertex_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &StateSpace::compute_pairwise_shortest_vertex_distances<BackwardTraversal>)
        .def("get_problem", &StateSpace::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &StateSpace::get_pddl_repositories)
        .def("get_applicable_action_generator", &StateSpace::get_applicable_action_generator)
        .def("get_state_repository", &StateSpace::get_state_repository)
        .def("get_vertex", &StateSpace::get_vertex, py::arg("state_index"))
        .def("get_vertices", &StateSpace::get_vertices, py::return_value_policy::reference_internal)
        .def("get_vertex_index", &StateSpace::get_vertex_index, py::arg("state"))
        .def("get_initial_vertex_index", &StateSpace::get_initial_vertex_index)
        .def("get_goal_vertex_indices", &StateSpace::get_goal_vertex_indices, py::return_value_policy::reference_internal)
        .def("get_deadend_vertex_indices", &StateSpace::get_deadend_vertex_indices, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_states",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_vertices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_vertices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_vertices", &StateSpace::get_num_vertices)
        .def("get_num_goal_vertices", &StateSpace::get_num_goal_vertices)
        .def("get_num_deadend_vertices", &StateSpace::get_num_deadend_vertices)
        .def("is_goal_vertex", &StateSpace::is_goal_vertex, py::arg("state_index"))
        .def("is_deadend_vertex", &StateSpace::is_deadend_vertex, py::arg("state_index"))
        .def("is_alive_vertex", &StateSpace::is_alive_vertex, py::arg("state_index"))
        .def("get_edges", &StateSpace::get_edges, py::return_value_policy::reference_internal)  // TODO: make edgelist opague to avoid segfaults
        .def("get_edge", &StateSpace::get_edge, py::arg("edge_index"), py::return_value_policy::reference_internal)
        .def("get_edge_cost", &StateSpace::get_edge_cost, py::arg("edge_index"))
        .def(
            "get_forward_adjacent_transitions",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def("get_num_edges", &StateSpace::get_num_edges)
        .def("get_goal_distance", &StateSpace::get_goal_distance, py::arg("state_index"))
        .def("get_goal_distances", &StateSpace::get_goal_distances, py::return_value_policy::reference_internal)
        .def("get_max_goal_distance", &StateSpace::get_max_goal_distance)
        .def("sample_vertex_index_with_goal_distance",
             &StateSpace::sample_vertex_index_with_goal_distance,
             py::return_value_policy::reference_internal,
             py::arg("goal_distance"));

    // NautyCertificate
    py::class_<nauty_wrapper::Certificate, std::shared_ptr<nauty_wrapper::Certificate>>(m, "NautyCertificate")
        .def("__eq__", [](const nauty_wrapper::Certificate& lhs, const nauty_wrapper::Certificate& rhs) { return lhs == rhs; })
        .def("__hash__", [](const nauty_wrapper::Certificate& self) { return std::hash<nauty_wrapper::Certificate>()(self); })
        .def("get_canonical_graph", &nauty_wrapper::Certificate::get_canonical_graph, py::return_value_policy::reference_internal)
        .def("get_canonical_coloring", &nauty_wrapper::Certificate::get_canonical_coloring, py::return_value_policy::reference_internal);

    // NautyDenseGraph
    py::class_<nauty_wrapper::DenseGraph>(m, "NautyDenseGraph")  //
        .def(py::init<>())
        .def(py::init<int>(), py::arg("num_vertices"))
        .def(py::init<StaticVertexColoredDigraph>(), py::arg("digraph"))
        .def("add_edge", &nauty_wrapper::DenseGraph::add_edge, py::arg("source"), py::arg("target"))
        .def("compute_certificate", &nauty_wrapper::DenseGraph::compute_certificate)
        .def("clear", &nauty_wrapper::DenseGraph::clear, py::arg("num_vertices"));

    // NautySparseGraph
    py::class_<nauty_wrapper::SparseGraph>(m, "NautySparseGraph")  //
        .def(py::init<>())
        .def(py::init<int>(), py::arg("num_vertices"))
        .def(py::init<StaticVertexColoredDigraph>(), py::arg("digraph"))
        .def("add_edge", &nauty_wrapper::SparseGraph::add_edge, py::arg("source"), py::arg("target"))
        .def("compute_certificate", &nauty_wrapper::SparseGraph::compute_certificate)
        .def("clear", &nauty_wrapper::SparseGraph::clear, py::arg("num_vertices"));

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
               std::shared_ptr<PDDLRepositories> factories,
               std::shared_ptr<IApplicableActionGenerator> applicable_action_generator,
               std::shared_ptr<StateRepository> state_repository,
               const FaithfulAbstractionOptions& options)
            { return FaithfulAbstraction::create(problem, factories, applicable_action_generator, state_repository, options); },
            py::arg("problem"),
            py::arg("factories"),
            py::arg("applicable_action_generator"),
            py::arg("state_repository"),
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
                   std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return FaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionOptions())
        .def("compute_shortest_forward_distances_from_states",
             &FaithfulAbstraction::compute_shortest_distances_from_vertices<ForwardTraversal>,
             py::arg("vertex_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &FaithfulAbstraction::compute_shortest_distances_from_vertices<BackwardTraversal>,
             py::arg("vertex_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &FaithfulAbstraction::compute_pairwise_shortest_vertex_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &FaithfulAbstraction::compute_pairwise_shortest_vertex_distances<BackwardTraversal>)
        .def("get_problem", &FaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &FaithfulAbstraction::get_pddl_repositories)
        .def("get_applicable_action_generator", &FaithfulAbstraction::get_applicable_action_generator)
        .def("get_state_repository", &FaithfulAbstraction::get_state_repository)
        .def("get_vertex_index", &FaithfulAbstraction::get_vertex_index, py::arg("state"))
        .def("get_vertices", &FaithfulAbstraction::get_vertices, py::return_value_policy::reference_internal)
        .def("get_state_to_vertex_index", &FaithfulAbstraction::get_state_to_vertex_index, py::return_value_policy::reference_internal)
        .def("get_initial_vertex_index", &FaithfulAbstraction::get_initial_vertex_index)
        .def("get_goal_vertex_indices", &FaithfulAbstraction::get_goal_vertex_indices, py::return_value_policy::reference_internal)
        .def("get_deadend_vertex_indices", &FaithfulAbstraction::get_deadend_vertex_indices, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_vertices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_vertices", &FaithfulAbstraction::get_num_vertices)
        .def("get_num_goal_vertices", &FaithfulAbstraction::get_num_goal_vertices)
        .def("get_num_deadend_vertices", &FaithfulAbstraction::get_num_deadend_vertices)
        .def("is_goal_vertex", &FaithfulAbstraction::is_goal_vertex, py::arg("vertex_index"))
        .def("is_deadend_vertex", &FaithfulAbstraction::is_deadend_vertex, py::arg("vertex_index"))
        .def("is_alive_vertex", &FaithfulAbstraction::is_alive_vertex, py::arg("vertex_index"))
        .def("get_edges", &FaithfulAbstraction::get_edges, py::return_value_policy::reference_internal)  // TODO: make edgelist opague to avoid segfaults
        .def("get_edge", &FaithfulAbstraction::get_edge, py::arg("edge_index"), py::return_value_policy::reference_internal)
        .def("get_edge_cost", &FaithfulAbstraction::get_edge_cost, py::arg("edge_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const FaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_edges", &FaithfulAbstraction::get_num_edges)
        .def("get_goal_distance", &FaithfulAbstraction::get_goal_distance, py::arg("state_index"))
        .def("get_goal_distances", &FaithfulAbstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // GlobalFaithfulAbstraction

    py::class_<GlobalFaithfulAbstractState>(m, "GlobalFaithfulAbstractState")
        .def("__eq__", &GlobalFaithfulAbstractState::operator==)
        .def("__hash__", [](const GlobalFaithfulAbstractState& self) { return std::hash<GlobalFaithfulAbstractState>()(self); })
        .def("get_vertex_index", &GlobalFaithfulAbstractState::get_vertex_index)
        .def("get_global_index", &GlobalFaithfulAbstractState::get_global_index)
        .def("get_faithful_abstraction_index", &GlobalFaithfulAbstractState::get_faithful_abstraction_index)
        .def("get_faithful_abstraction_vertex_index", &GlobalFaithfulAbstractState::get_faithful_abstraction_vertex_index);

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
                   std::tuple<Problem, std::shared_ptr<PDDLRepositories>, std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<StateRepository>>>&
                   memories,
               const FaithfulAbstractionsOptions& options) { return GlobalFaithfulAbstraction::create(memories, options); },
            py::arg("memories"),
            py::arg("options") = FaithfulAbstractionsOptions())
        .def("compute_shortest_forward_distances_from_states",
             &GlobalFaithfulAbstraction::compute_shortest_distances_from_vertices<ForwardTraversal>,
             py::arg("vertex_indices"))
        .def("compute_shortest_backward_distances_from_states",
             &GlobalFaithfulAbstraction::compute_shortest_distances_from_vertices<BackwardTraversal>,
             py::arg("vertex_indices"))
        .def("compute_pairwise_shortest_forward_state_distances", &GlobalFaithfulAbstraction::compute_pairwise_shortest_vertex_distances<ForwardTraversal>)
        .def("compute_pairwise_shortest_backward_state_distances", &GlobalFaithfulAbstraction::compute_pairwise_shortest_vertex_distances<BackwardTraversal>)
        .def("get_index", &GlobalFaithfulAbstraction::get_index)
        .def("get_problem", &GlobalFaithfulAbstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &GlobalFaithfulAbstraction::get_pddl_repositories)
        .def("get_applicable_action_generator", &GlobalFaithfulAbstraction::get_applicable_action_generator)
        .def("get_state_repository", &GlobalFaithfulAbstraction::get_state_repository)
        .def("get_abstractions", &GlobalFaithfulAbstraction::get_abstractions, py::return_value_policy::reference_internal)
        .def("get_vertex_index", py::overload_cast<State>(&GlobalFaithfulAbstraction::get_vertex_index, py::const_), py::arg("state"))
        .def("get_vertex_index", py::overload_cast<Index>(&GlobalFaithfulAbstraction::get_vertex_index, py::const_), py::arg("global_state_index"))
        .def("get_vertices", &GlobalFaithfulAbstraction::get_vertices, py::return_value_policy::reference_internal)
        .def("get_state_to_vertex_index", &GlobalFaithfulAbstraction::get_state_to_vertex_index, py::return_value_policy::reference_internal)
        .def("get_global_vertex_index_to_vertex_index",
             &GlobalFaithfulAbstraction::get_global_vertex_index_to_vertex_index,
             py::return_value_policy::reference_internal)
        .def("get_initial_vertex_index", &GlobalFaithfulAbstraction::get_initial_vertex_index)
        .def("get_goal_vertex_indices", &GlobalFaithfulAbstraction::get_goal_vertex_indices, py::return_value_policy::reference_internal)
        .def("get_deadend_vertex_indices", &GlobalFaithfulAbstraction::get_deadend_vertex_indices, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_vertices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_vertices", &GlobalFaithfulAbstraction::get_num_vertices)
        .def("get_num_goal_vertices", &GlobalFaithfulAbstraction::get_num_goal_vertices)
        .def("get_num_deadend_vertices", &GlobalFaithfulAbstraction::get_num_deadend_vertices)
        .def("is_goal_vertex", &GlobalFaithfulAbstraction::is_goal_vertex, py::arg("vertex_index"))
        .def("is_deadend_vertex", &GlobalFaithfulAbstraction::is_deadend_vertex, py::arg("vertex_index"))
        .def("is_alive_vertex", &GlobalFaithfulAbstraction::is_alive_vertex, py::arg("vertex_index"))
        .def("get_num_isomorphic_states", &GlobalFaithfulAbstraction::get_num_isomorphic_states)
        .def("get_num_non_isomorphic_states", &GlobalFaithfulAbstraction::get_num_non_isomorphic_states)
        .def("get_edges", &GlobalFaithfulAbstraction::get_edges, py::return_value_policy::reference_internal)  // TODO: make edgelist opague to avoid segfaults
        .def("get_edge", &GlobalFaithfulAbstraction::get_edge, py::arg("edge_index"), py::return_value_policy::reference_internal)
        .def("get_edge_cost", &GlobalFaithfulAbstraction::get_edge_cost, py::arg("edge_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edges",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const GlobalFaithfulAbstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_edges", &GlobalFaithfulAbstraction::get_num_edges)
        .def("get_goal_distance", &GlobalFaithfulAbstraction::get_goal_distance, py::arg("state_index"))
        .def("get_goal_distances", &GlobalFaithfulAbstraction::get_goal_distances, py::return_value_policy::reference_internal);

    // Abstraction
    py::class_<Abstraction, std::shared_ptr<Abstraction>>(m, "Abstraction")  //
        .def(py::init<FaithfulAbstraction>(), py::arg("faithful_abstraction"))
        .def(py::init<GlobalFaithfulAbstraction>(), py::arg("global_faithful_abstraction"))
        .def("get_problem", &Abstraction::get_problem, py::return_value_policy::reference_internal)
        .def("get_pddl_repositories", &Abstraction::get_pddl_repositories)
        .def("get_applicable_action_generator", &Abstraction::get_applicable_action_generator)
        .def("get_state_repository", &Abstraction::get_state_repository)
        .def("get_vertex_index", &Abstraction::get_vertex_index)
        .def("get_initial_vertex_index", &Abstraction::get_initial_vertex_index)
        .def("get_goal_vertex_indices", &Abstraction::get_goal_vertex_indices, py::return_value_policy::reference_internal)
        .def("get_deadend_vertex_indices", &Abstraction::get_deadend_vertex_indices, py::return_value_policy::reference_internal)
        .def(
            "get_forward_adjacent_vertex_indices",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_vertex_indices",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_vertex_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_vertices", &Abstraction::get_num_vertices)
        .def("get_num_goal_vertices", &Abstraction::get_num_goal_vertices)
        .def("get_num_deadend_vertices", &Abstraction::get_num_deadend_vertices)
        .def("is_goal_vertex", &Abstraction::is_goal_vertex, py::arg("vertex_index"))
        .def("is_deadend_vertex", &Abstraction::is_deadend_vertex, py::arg("vertex_index"))
        .def("is_alive_vertex", &Abstraction::is_alive_vertex, py::arg("vertex_index"))
        .def("get_edge_cost", &Abstraction::get_edge_cost, py::arg("edge_index"))
        .def(
            "get_forward_adjacent_edges",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edges<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_forward_adjacent_edge_indices",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<ForwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def(
            "get_backward_adjacent_edge_indices",
            [](const Abstraction& self, Index vertex)
            {
                auto iterator = self.get_adjacent_edge_indices<BackwardTraversal>(vertex);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("vertex_index"))
        .def("get_num_edges", &Abstraction::get_num_edges)
        .def("get_goal_distance", &Abstraction::get_goal_distance, py::arg("state_index"))
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
        .def("compute_admissible_chain", py::overload_cast<const GroundAtomList<Fluent>&>(&TupleGraph::compute_admissible_chain))
        .def("compute_admissible_chain", py::overload_cast<const StateList&>(&TupleGraph::compute_admissible_chain))
        .def("get_state_space", &TupleGraph::get_state_space)
        .def("get_tuple_index_mapper", &TupleGraph::get_tuple_index_mapper)
        .def("get_root_state", &TupleGraph::get_root_state, py::keep_alive<0, 1>())
        .def("get_vertices_grouped_by_distance", &TupleGraph::get_vertices_grouped_by_distance, py::return_value_policy::reference_internal)
        .def("get_digraph", &TupleGraph::get_digraph, py::return_value_policy::reference_internal)
        .def("get_states_grouped_by_distance", &TupleGraph::get_states_grouped_by_distance, py::return_value_policy::reference_internal);

    // TupleGraphFactory
    py::class_<TupleGraphFactory>(m, "TupleGraphFactory")  //
        .def(py::init<std::shared_ptr<StateSpace>, int, bool>(), py::arg("state_space"), py::arg("arity"), py::arg("prune_dominated_tuples") = false)
        .def("create", &TupleGraphFactory::create)
        .def("get_state_space", &TupleGraphFactory::get_state_space)
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

    // ObjectGraphPruningStrategy
    py::class_<ObjectGraphPruningStrategy>(m, "ObjectGraphPruningStrategy");

    // ObjectGraph
    m.def("create_object_graph",
          &create_object_graph,
          py::arg("color_function"),
          py::arg("pddl_repositories"),
          py::arg("problem"),
          py::arg("state"),
          py::arg("state_index") = 0,
          py::arg("mark_true_goal_literals") = false,
          py::arg("pruning_strategy") = ObjectGraphPruningStrategy(),
          "Creates an object graph based on the provided parameters");

    // Color Refinement
    py::class_<color_refinement::Certificate>(m, "CertificateColorRefinement")
        .def("__eq__", [](const color_refinement::Certificate& lhs, const color_refinement::Certificate& rhs) { return lhs == rhs; })
        .def("__hash__", [](const color_refinement::Certificate& self) { return std::hash<color_refinement::Certificate>()(self); })
        .def("__str__",
             [](const color_refinement::Certificate& self)
             {
                 auto os = std::stringstream();
                 os << self;
                 return os.str();
             })
        // Returning canonical compression functions does not work due to unhashable type list.
        //.def("get_canonical_configuration_compression_function", &color_refinement::Certificate::get_canonical_compression_function)
        .def("get_canonical_coloring", &color_refinement::Certificate::get_canonical_coloring);

    m.def("compute_certificate_color_refinement",
          &color_refinement::compute_certificate<StaticVertexColoredDigraph>,
          py::arg("graph"),
          "Creates color refinement certificate");

    // K-FWL
    auto bind_kfwl_certificate = [&]<size_t K>(const std::string& class_name, std::integral_constant<size_t, K>)
    {
        using CertificateK = kfwl::Certificate<K>;

        py::class_<CertificateK>(m, class_name.c_str())
            .def("__eq__", [](const CertificateK& lhs, const CertificateK& rhs) { return lhs == rhs; })
            .def("__hash__", [](const CertificateK& self) { return std::hash<CertificateK>()(self); })
            .def("__str__",
                 [](const CertificateK& self)
                 {
                     auto os = std::stringstream();
                     os << self;
                     return os.str();
                 })
            // Returning canonical compression functions does not work due to unhashable type list.
            //.def("get_canonical_isomorphic_type_compression_function", &CertificateK::get_canonical_isomorphic_type_compression_function)
            //.def("get_canonical_configuration_compression_function", &CertificateK::get_canonical_configuration_compression_function)
            .def("get_canonical_coloring", &CertificateK::get_canonical_coloring);
    };
    bind_kfwl_certificate("Certificate2FWL", std::integral_constant<size_t, 2> {});
    bind_kfwl_certificate("Certificate3FWL", std::integral_constant<size_t, 3> {});
    bind_kfwl_certificate("Certificate4FWL", std::integral_constant<size_t, 4> {});

    py::class_<kfwl::IsomorphismTypeCompressionFunction>(m, "IsomorphismTypeCompressionFunction")  //
        .def(py::init<>());

    auto bind_compute_kfwl_certificate = [&]<size_t K>(const std::string& function_name, std::integral_constant<size_t, K>)
    {
        m.def(
            function_name.c_str(),
            [](const StaticVertexColoredDigraph& graph, kfwl::IsomorphismTypeCompressionFunction& iso_type_function)
            { return kfwl::compute_certificate<K>(graph, iso_type_function); },
            py::arg("static_vertex_colored_digraph"),
            py::arg("isomorphism_type_compression_function"));
    };
    bind_compute_kfwl_certificate("compute_certificate_2fwl", std::integral_constant<size_t, 2> {});
    bind_compute_kfwl_certificate("compute_certificate_3fwl", std::integral_constant<size_t, 3> {});
    bind_compute_kfwl_certificate("compute_certificate_4fwl", std::integral_constant<size_t, 4> {});
}
