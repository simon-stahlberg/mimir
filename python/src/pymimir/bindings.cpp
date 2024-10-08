#include "init_declarations.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/search/condition_grounders/conjunction_grounder.hpp"

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
struct ReprVisitor
{
    template<typename T>
    std::string operator()(const T& element) const
    {
        return element.str();
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
PYBIND11_MAKE_OPAQUE(EffectComplexList);
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
    void on_solved_impl(const GroundActionList& ground_action_plan, const PDDLFactories& pddl_factories) override
    {
        PYBIND11_OVERRIDE(void, DynamicAStarAlgorithmEventHandlerBase, on_solved_impl, ground_action_plan, pddl_factories);
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
    auto list_class = py::bind_vector<ObjectList>(m, "ObjectList");
    def_opaque_vector_repr<ObjectList>(list_class, "ObjectList");

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", &VariableImpl::str)
        .def("__repr__", &VariableImpl::str)
        .def("get_index", &VariableImpl::get_index)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<VariableList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<VariableList>(m, "VariableList");
    def_opaque_vector_repr<VariableList>(list_class, "VariableList");

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
        .def("get", [](const TermVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.term); }, py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<TermVariantList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<TermVariantList>(m, "TermVariantList");
    def_opaque_vector_repr<TermVariantList>(list_class, "TermVariantList", [](const TermVariant& elem) { return std::visit(ReprVisitor {}, *elem.term); });

    auto bind_predicate = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<PredicateImpl<Tag>>(m, class_name.c_str())
            .def("__str__", &PredicateImpl<Tag>::str)
            .def("__repr__", &PredicateImpl<Tag>::str)
            .def("get_index", &PredicateImpl<Tag>::get_index)
            .def("get_name", &PredicateImpl<Tag>::get_name, py::return_value_policy::reference_internal)
            .def(
                "get_parameters",
                [](const PredicateImpl<Tag>& self) { return VariableList(self.get_parameters()); },
                py::keep_alive<0, 1>())
            .def("get_arity", &PredicateImpl<Tag>::get_arity);

        static_assert(!py::detail::vector_needs_copy<PredicateList<Tag>>::value);
        auto predicate_list = py::bind_vector<PredicateList<Tag>>(m, class_name + "List");
        def_opaque_vector_repr<PredicateList<Tag>>(predicate_list, class_name + "List");
        py::bind_map<ToPredicateMap<std::string, Tag>>(m, "StringTo" + class_name + "Map");
    };
    bind_predicate("StaticPredicate", Static {});
    bind_predicate("FluentPredicate", Fluent {});
    bind_predicate("DerivedPredicate", Derived {});

    auto bind_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<AtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", &AtomImpl<Tag>::str)
            .def("__repr__", &AtomImpl<Tag>::str)
            .def("get_index", &AtomImpl<Tag>::get_index)
            .def("get_predicate", &AtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_terms", [](const AtomImpl<Tag>& atom) { return atom.get_terms(); }, py::keep_alive<0, 1>());

        static_assert(!py::detail::vector_needs_copy<AtomList<Tag>>::value);
        auto list_cls = py::bind_vector<AtomList<Tag>>(m, class_name + "List");
        def_opaque_vector_repr<AtomList<Tag>>(list_cls, class_name + "List");
    };
    bind_atom("StaticAtom", Static {});
    bind_atom("FluentAtom", Fluent {});
    bind_atom("DerivedAtom", Derived {});

    py::class_<FunctionSkeletonImpl>(m, "FunctionSkeleton")  //
        .def("__str__", &FunctionSkeletonImpl::str)
        .def("__repr__", &FunctionSkeletonImpl::str)
        .def("get_index", &FunctionSkeletonImpl::get_index)
        .def("get_name", &FunctionSkeletonImpl::get_name, py::return_value_policy::reference_internal)
        .def("get_parameters", [](const FunctionSkeletonImpl& self) { return VariableList(self.get_parameters()); }, py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionSkeletonList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionSkeletonList>(m, "FunctionSkeletonList");
    def_opaque_vector_repr<FunctionSkeletonList>(list_class, "FunctionSkeletonList");

    py::class_<FunctionImpl>(m, "Function")  //
        .def("__str__", &FunctionImpl::str)
        .def("__repr__", &FunctionImpl::str)
        .def("get_index", &FunctionImpl::get_index)
        .def("get_function_skeleton", &FunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def("get_terms", [](const FunctionImpl& self) { return to_term_variant_list(self.get_terms()); }, py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<FunctionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionList>(m, "FunctionList");
    def_opaque_vector_repr<FunctionSkeletonList>(list_class, "FunctionList");

    py::class_<GroundFunctionImpl>(m, "GroundFunction")  //
        .def("__str__", &GroundFunctionImpl::str)
        .def("__repr__", &GroundFunctionImpl::str)
        .def("get_index", &GroundFunctionImpl::get_index)
        .def("get_function_skeleton", &GroundFunctionImpl::get_function_skeleton, py::return_value_policy::reference_internal)
        .def("get_objects", [](const GroundFunctionImpl& self) { return ObjectList(self.get_objects()); }, py::keep_alive<0, 1>());
    static_assert(!py::detail::vector_needs_copy<GroundFunctionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundFunctionList>(m, "GroundFunctionList");
    def_opaque_vector_repr<FunctionSkeletonList>(list_class, "GroundFunctionList");

    auto bind_ground_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundAtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", &GroundAtomImpl<Tag>::str)
            .def("__repr__", &GroundAtomImpl<Tag>::str)
            .def("get_index", &GroundAtomImpl<Tag>::get_index)
            .def("get_arity", &GroundAtomImpl<Tag>::get_arity)
            .def("get_predicate", &GroundAtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_objects", [](const GroundAtomImpl<Tag>& self) { return ObjectList(self.get_objects()); }, py::keep_alive<0, 1>());

        static_assert(!py::detail::vector_needs_copy<GroundAtomList<Tag>>::value);
        list_class = py::bind_vector<GroundAtomList<Tag>>(m, class_name + "List")
            .def("lift", [](const GroundAtomList<Tag>& ground_atoms, PDDLFactories& pddl_factories) { return lift(ground_atoms, pddl_factories); }, py::arg("pddl_factories"));
        def_opaque_vector_repr<GroundAtomList<Tag>>(list_class, class_name + "List");
    };
    bind_ground_atom("StaticGroundAtom", Static {});
    bind_ground_atom("FluentGroundAtom", Fluent {});
    bind_ground_atom("DerivedGroundAtom", Derived {});

    auto bind_ground_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundLiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", &GroundLiteralImpl<Tag>::str)
            .def("__repr__", &GroundLiteralImpl<Tag>::str)
            .def("get_index", &GroundLiteralImpl<Tag>::get_index)
            .def("get_atom", &GroundLiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &GroundLiteralImpl<Tag>::is_negated);

        static_assert(!py::detail::vector_needs_copy<GroundLiteralList<Tag>>::value);
        auto list = py::bind_vector<GroundLiteralList<Tag>>(m, class_name + "List")
            .def("lift", [](const GroundLiteralList<Tag>& ground_literals, PDDLFactories& pddl_factories) { return lift(ground_literals, pddl_factories); }, py::arg("pddl_factories"));
        def_opaque_vector_repr<GroundLiteralList<Tag>>(list, class_name + "List");
    };
    bind_ground_literal("StaticGroundLiteral", Static {});
    bind_ground_literal("FluentGroundLiteral", Fluent {});
    bind_ground_literal("DerivedGroundLiteral", Derived {});

    auto bind_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<LiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", &LiteralImpl<Tag>::str)
            .def("__repr__", &LiteralImpl<Tag>::str)
            .def("get_index", &LiteralImpl<Tag>::get_index)
            .def("get_atom", &LiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &LiteralImpl<Tag>::is_negated);

        static_assert(!py::detail::vector_needs_copy<LiteralList<Tag>>::value);
        auto list = py::bind_vector<LiteralList<Tag>>(m, class_name + "List");
        def_opaque_vector_repr<GroundLiteralList<Tag>>(list, "List");
    };
    bind_literal("StaticLiteral", Static {});
    bind_literal("FluentLiteral", Fluent {});
    bind_literal("DerivedLiteral", Derived {});

    py::class_<NumericFluentImpl>(m, "NumericFluent")  //
        .def("__str__", &NumericFluentImpl::str)
        .def("__repr__", &NumericFluentImpl::str)
        .def("get_index", &NumericFluentImpl::get_index)
        .def("get_function", &NumericFluentImpl::get_function, py::return_value_policy::reference_internal)
        .def("get_number", &NumericFluentImpl::get_number);
    static_assert(!py::detail::vector_needs_copy<NumericFluentList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<NumericFluentList>(m, "NumericFluentList");
    def_opaque_vector_repr<NumericFluentList>(list_class, "NumericFluentList");

    py::class_<EffectSimpleImpl>(m, "EffectSimple")  //
        .def("__str__", &EffectSimpleImpl::str)
        .def("__repr__", &EffectSimpleImpl::str)
        .def("get_index", &EffectSimpleImpl::get_index)
        .def("get_effect", &EffectSimpleImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectSimpleList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<EffectSimpleList>(m, "EffectSimpleList");
    def_opaque_vector_repr<EffectSimpleList>(list_class, "EffectSimpleList");

    py::class_<FunctionExpressionVariant>(m, "FunctionExpression")  //
        .def(
            "get",
            [](const FunctionExpressionVariant& arg) -> py::object { return std::visit(CastVisitor(), *arg.function_expression); },
            py::keep_alive<0, 1>());
    ;
    static_assert(!py::detail::vector_needs_copy<FunctionExpressionVariantList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<FunctionExpressionVariantList>(m, "FunctionExpressionVariantList");
    def_opaque_vector_repr<FunctionExpressionVariantList>(list_class, "FunctionExpressionVariantList");

    py::class_<EffectComplexImpl>(m, "EffectComplex")  //
        .def("__str__", &EffectComplexImpl::str)
        .def("__repr__", &EffectComplexImpl::str)
        .def("get_index", &EffectComplexImpl::get_index)
        .def(
            "get_parameters",
            [](const EffectComplexImpl& self) { return VariableList(self.get_parameters()); },
            py::keep_alive<0, 1>())
        .def(
            "get_static_conditions",
            [](const EffectComplexImpl& self) { return LiteralList<Static>(self.get_conditions<Static>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_fluent_conditions",
            [](const EffectComplexImpl& self) { return LiteralList<Fluent>(self.get_conditions<Fluent>()); },
            py::keep_alive<0, 1>())
        .def(
            "get_derived_conditions",
            [](const EffectComplexImpl& self) { return LiteralList<Derived>(self.get_conditions<Derived>()); },
            py::keep_alive<0, 1>())
        .def("get_effect", &EffectComplexImpl::get_effect, py::return_value_policy::reference_internal);
    static_assert(!py::detail::vector_needs_copy<EffectComplexList>::value);  // Ensure return by reference + keep alive
    py::bind_vector<EffectComplexList>(m, "EffectComplexList");
    def_opaque_vector_repr<EffectComplexList>(list_class, "EffectComplexList");

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
    list_class = py::bind_vector<GroundFunctionExpressionVariantList>(m, "GroundFunctionExpressionVariantList");
    def_opaque_vector_repr<GroundFunctionExpressionVariantList>(list_class, "GroundFunctionExpressionVariantList");

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
            "get_complex_effects",
            [](const ActionImpl& self) { return EffectComplexList(self.get_complex_effects()); },
            py::keep_alive<0, 1>())
        .def("get_arity", &ActionImpl::get_arity);
    static_assert(!py::detail::vector_needs_copy<ActionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<ActionList>(m, "ActionList");
    def_opaque_vector_repr<ActionList>(list_class, "ActionList");

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
    list_class = py::bind_vector<AxiomList>(m, "AxiomList");
    def_opaque_vector_repr<AxiomList>(list_class, "AxiomList");

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
        .def("get_predicates",
             [](const py::object& py_domain)
             {
                 const auto& self = py::cast<const DomainImpl&>(py_domain);
                 size_t n_preds = self.get_predicates<Static>().size() + self.get_predicates<Fluent>().size() + self.get_predicates<Derived>().size();
                 py::list all_predicates(n_preds);
                 int i = 0;
                 auto append = [&](const auto& preds)
                 {
                     for (const auto& pred : preds)
                     {
                         auto py_pred = py::cast(pred);
                         if (!py_pred)
                         {
                             throw py::error_already_set();
                         }
                         py::detail::keep_alive_impl(py_pred, py_domain);
                         all_predicates[i] = py_pred;
                         ++i;
                     }
                 };
                 append(self.get_predicates<Static>());
                 append(self.get_predicates<Fluent>());
                 append(self.get_predicates<Derived>());
                 return all_predicates;
             })
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
    list_class = py::bind_vector<DomainList>(m, "DomainList");
    def_opaque_vector_repr<DomainList>(list_class, "DomainList");

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
            py::keep_alive<0, 1>())
        .def("get_goal_condition",
             [](const py::object& py_problem)
             {
                 const auto& self = py::cast<const ProblemImpl&>(py_problem);
                 size_t n_goals =
                     self.get_goal_condition<Static>().size() + self.get_goal_condition<Fluent>().size() + self.get_goal_condition<Derived>().size();
                 py::list all_goal_literals(n_goals);
                 int i = 0;
                 auto append = [&](const auto& goals)
                 {
                     for (const auto& goal : goals)
                     {
                         auto py_goal = py::cast(goal);
                         if (!py_goal)
                         {
                             throw py::error_already_set();
                         }
                         py::detail::keep_alive_impl(py_goal, py_problem);
                         all_goal_literals[i] = py_goal;
                         ++i;
                     }
                 };
                 append(self.get_goal_condition<Static>());
                 append(self.get_goal_condition<Fluent>());
                 append(self.get_goal_condition<Derived>());
                 return all_goal_literals;
             });
    static_assert(!py::detail::vector_needs_copy<ProblemList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<ProblemList>(m, "ProblemList");
    def_opaque_vector_repr<ProblemList>(list_class, "ProblemList");

    py::class_<PDDLFactories, std::shared_ptr<PDDLFactories>> pddl_factories(m, "PDDLFactories");  //

    pddl_factories
        .def("get_ground_atoms",
             [](py::object py_factory)  // we need an object handle to keep the factory alive for each atom in the list
             {
                 const auto& factory = py::cast<const PDDLFactories&>(py_factory);
                 const auto& static_atom_factory = factory.get_factory<GroundAtomFactory<Static>>();
                 const auto& fluent_atom_factory = factory.get_factory<GroundAtomFactory<Fluent>>();
                 const auto& derived_atom_factory = factory.get_factory<GroundAtomFactory<Derived>>();

                 py::list all_atoms(static_atom_factory.size() + fluent_atom_factory.size() + derived_atom_factory.size());
                 size_t i = 0;
                 auto append = [&](const auto& atoms)
                 {
                     for (const auto& atom : atoms)
                     {
                         py::object py_atom = py::cast(atom);
                         if (!py_atom)
                         {
                             throw py::error_already_set();
                         }
                         py::detail::keep_alive_impl(py_atom, py_factory);
                         all_atoms[i] = py::object { py_atom };
                         i++;
                     }
                 };
                 append(static_atom_factory);
                 append(fluent_atom_factory);
                 append(derived_atom_factory);
                 return all_atoms;
             })
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

    auto bind_ground_atoms_range = [&]<typename Tag>(std::string_view func_name, Tag)
    {
        pddl_factories.def(
            func_name.data(),
            [=](const PDDLFactories& factory)
            {
                GroundAtomList<Tag> atoms;
                for (const auto& elem : factory.get_factory<GroundAtomFactory<Tag>>())
                {
                    atoms.emplace_back(elem);
                }
                return atoms;
            },
            py::keep_alive<0, 1>());
    };
    bind_ground_atoms_range("get_static_ground_atoms", Static {});
    bind_ground_atoms_range("get_fluent_ground_atoms", Fluent {});
    bind_ground_atoms_range("get_derived_ground_atoms", Derived {});

    py::class_<PDDLParser>(m, "PDDLParser")  //
        .def(py::init<std::string, std::string>(), py::arg("domain_path"), py::arg("problem_path"))
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
        .def("__eq__", [](const State& lhs, const State& rhs) { return lhs == rhs; })
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
    list_class = py::bind_vector<StateList>(m, "StateList");
    def_opaque_vector_repr<StateList>(list_class, "StateList");
    bind_const_span<std::span<const State>>(m, "StateSpan");
    bind_const_index_grouped_vector<IndexGroupedVector<const State>>(m, "IndexGroupedVector");

    /* Action */
    // TODO: add missing functions here
    py::class_<StripsActionPrecondition>(m, "StripsActionPrecondition");
    py::class_<StripsActionEffect>(m, "StripsActionEffect");
    py::class_<ConditionalEffect>(m, "ConditionalEffect");
    py::class_<FlatSimpleEffect>(m, "FlatSimpleEffect");

    py::class_<GroundAction>(m, "GroundAction")  //
        .def("__hash__", [](const GroundAction& obj) { return std::hash<GroundAction>()(obj); })
        .def("__eq__", [](const GroundAction& a, const GroundAction& b) { return std::equal_to<GroundAction>()(a, b); })
        .def("to_string",
             [](GroundAction self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(self, std::cref(pddl_factories));
                 return ss.str();
             })
        .def("to_string_for_plan",
             [](GroundAction self, PDDLFactories& pddl_factories)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(std::cref(pddl_factories), self);
                 return ss.str();
             })
        .def("get_index", &GroundAction::get_index)
        .def("get_cost", &GroundAction::get_cost)
        .def("get_action_index", &GroundAction::get_action_index, py::return_value_policy::reference_internal)
        .def("get_object_indices", &GroundAction::get_object_indices, py::return_value_policy::copy)
        .def("get_strips_precondition", [](const GroundAction& self) { return StripsActionPrecondition(self.get_strips_precondition()); })
        .def("get_strips_effect", [](const GroundAction& self) { return StripsActionEffect(self.get_strips_effect()); })
        .def("get_conditional_effects",
             [](const GroundAction& self)
             {
                 auto conditional_effects = std::vector<ConditionalEffect> {};
                 for (const auto& flat_conditional_effect : self.get_conditional_effects())
                 {
                     conditional_effects.emplace_back(flat_conditional_effect);
                 }
                 return conditional_effects;
             });
    static_assert(!py::detail::vector_needs_copy<GroundActionList>::value);  // Ensure return by reference + keep alive
    list_class = py::bind_vector<GroundActionList>(m, "GroundActionList");
    def_opaque_vector_repr<GroundActionList>(list_class, "GroundActionList");
    bind_const_span<std::span<const GroundAction>>(m, "GroundActionSpan");


    /* ConjunctionGrounder */

    py::class_<LiftedConjunctionGrounder, std::shared_ptr<LiftedConjunctionGrounder>>(m,
                                                                                      "LiftedConjunctionGrounder")  //
        .def(py::init<Problem, VariableList, LiteralList<Static>, LiteralList<Fluent>, LiteralList<Derived>, std::shared_ptr<PDDLFactories>>(),
             py::arg("problem"),
             py::arg("variables"),
             py::arg("static_literals"),
             py::arg("fluent_literals"),
             py::arg("derived_literals"),
             py::arg("pddl_factories"))
        .def("ground", &LiftedConjunctionGrounder::ground, py::arg("state"));

    /* ApplicableActionGenerators */

    py::class_<IApplicableActionGenerator, std::shared_ptr<IApplicableActionGenerator>>(m,
                                                                                        "IApplicableActionGenerator")  //
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
               std::shared_ptr<DebugLiftedApplicableActionGeneratorEventHandler>>(m,
                                                                                  "DebugLiftedApplicableActionGeneratorEventHandler")  //
        .def(py::init<>());
    py::class_<LiftedApplicableActionGenerator, IApplicableActionGenerator, std::shared_ptr<LiftedApplicableActionGenerator>>(
        m,
        "LiftedApplicableActionGenerator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>(), py::arg("problem"), py::arg("pddl_factories"))
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<ILiftedApplicableActionGeneratorEventHandler>>(), py::arg("problem"), py::arg("pddl_factories"), py::arg("event_handler"));

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
        "GroundedApplicableActionGenerator")  //
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>>(), py::arg("problem"), py::arg("pddl_factories"))
        .def(py::init<Problem, std::shared_ptr<PDDLFactories>, std::shared_ptr<IGroundedApplicableActionGeneratorEventHandler>>(), py::arg("problem"), py::arg("pddl_factories"), py::arg("event_handler"));

    /* StateRepository */
    py::class_<StateRepository, std::shared_ptr<StateRepository>>(m, "StateRepository")  //
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>>(), py::arg("applicable_action_generator"))
        .def("get_or_create_initial_state", &StateRepository::get_or_create_initial_state, py::keep_alive<0, 1>())  // keep_alive because value type
        .def("get_or_create_state",
             &StateRepository::get_or_create_state,
             py::keep_alive<0, 1>(),
             py::arg("atoms"))  // keep_alive because value type
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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, std::shared_ptr<IHeuristic>>(), py::arg("applicable_action_generator"), py::arg("heuristic"))
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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>>(), py::arg("applicable_action_generator"))
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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, size_t>(), py::arg("applicable_action_generator"), py::arg("max_arity"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      size_t,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("max_arity"),
             py::arg("state_repository"),
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
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>, size_t>(), py::arg("applicable_action_generator"), py::arg("max_arity"))
        .def(py::init<std::shared_ptr<IApplicableActionGenerator>,
                      size_t,
                      std::shared_ptr<StateRepository>,
                      std::shared_ptr<IBrFSAlgorithmEventHandler>,
                      std::shared_ptr<IIWAlgorithmEventHandler>,
                      std::shared_ptr<ISIWAlgorithmEventHandler>>(),
             py::arg("applicable_action_generator"),
             py::arg("max_arity"),
             py::arg("state_repository"),
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
        .def("get_state", [](const StateVertex& self) { return get_state(self); }, py::keep_alive<0, 1>());

    // GroundActionEdge
    py::class_<GroundActionEdge>(m, "GroundActionEdge")  //
        .def("__eq__", &GroundActionEdge::operator==)
        .def("__hash__", [](const GroundActionEdge& self) { return std::hash<GroundActionEdge>()(self); })
        .def("get_index", &GroundActionEdge::get_index)
        .def("get_source", &GroundActionEdge::get_source)
        .def("get_target", &GroundActionEdge::get_target)
        .def("get_cost", [](const GroundActionEdge& self) { return get_cost(self); })
        .def("get_creating_action", [](const GroundActionEdge& self) { return get_creating_action(self); }, py::keep_alive<0, 1>());

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
        .def("get_representative_action", [](const GroundActionsEdge& self) { return get_representative_action(self); }, py::keep_alive<0, 1>());

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
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const StateSpace& self, Index state)
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
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const StateSpace& self, Index state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const StateSpace& self, Index state)
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
        .def(py::init<size_t, size_t, std::string, ColorList>(),
             py::arg("num_vertices"),
             py::arg("num_edges"),
             py::arg("nauty_certificate"),
             py::arg("canonical_initial_coloring"))
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
        .def("get_certificate", [](const FaithfulAbstractStateVertex& self) { return get_certificate(self); }, py::return_value_policy::reference_internal);

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
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const FaithfulAbstraction& self, Index state)
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
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const FaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const FaithfulAbstraction& self, Index state)
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
        .def("get_abstract_state_index", py::overload_cast<Index>(&GlobalFaithfulAbstraction::get_abstract_state_index, py::const_), py::arg("state_index"))
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
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_states<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_states",
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_states<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_state_indices",
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const GlobalFaithfulAbstraction& self, Index state)
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
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const GlobalFaithfulAbstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const GlobalFaithfulAbstraction& self, Index state)
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
        .def(py::init<FaithfulAbstraction>(), py::arg("faithful_abstraction"))
        .def(py::init<GlobalFaithfulAbstraction>(), py::arg("global_faithful_abstraction"))
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
            [](const Abstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_state_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_state_indices",
            [](const Abstraction& self, Index state)
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
            [](const Abstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transitions",
            [](const Abstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transitions<BackwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_forward_adjacent_transition_indices",
            [](const Abstraction& self, Index state)
            {
                auto iterator = self.get_adjacent_transition_indices<ForwardTraversal>(state);
                return py::make_iterator(iterator.begin(), iterator.end());
            },
            py::keep_alive<0, 1>(),
            py::arg("state_index"))
        .def(
            "get_backward_adjacent_transition_indices",
            [](const Abstraction& self, Index state)
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
        .def("get_states", [](const TupleGraphVertex& self) { return StateList(self.get_states()); }, py::keep_alive<0, 1>());
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
        .def("get_color_name", [](const ColorFunction& self, Color color) -> const std::string& { return self.get_color_name(color); }, py::arg("color"));

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
        .def(py::init<int>(), py::arg("num_vertices"))
        .def(py::init<StaticVertexColoredDigraph>(), py::arg("digraph"))
        .def("add_edge", &nauty_wrapper::DenseGraph::add_edge, py::arg("source"), py::arg("target"))
        .def("compute_certificate", &nauty_wrapper::DenseGraph::compute_certificate)
        .def("clear", &nauty_wrapper::DenseGraph::clear, py::arg("num_vertices"));

    // SparseNautyGraph
    py::class_<nauty_wrapper::SparseGraph>(m, "SparseNautyGraph")  //
        .def(py::init<>())
        .def(py::init<int>(), py::arg("num_vertices"))
        .def(py::init<StaticVertexColoredDigraph>(), py::arg("digraph"))
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
