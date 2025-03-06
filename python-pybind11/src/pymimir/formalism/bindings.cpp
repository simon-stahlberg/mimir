
#include "init_declarations.hpp"

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

using namespace mimir;

namespace py = pybind11;

void init_formalism(py::module_& m)
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

    py::class_<loki::Options>(m, "ParserOptions")
        .def(py::init<>())
        .def_readwrite("strict", &loki::Options::strict, "Enable strict mode")
        .def_readwrite("quiet", &loki::Options::quiet, "Suppress output");

    py::class_<RequirementsImpl>(m, "Requirements")  //
        .def("__str__", [](const RequirementsImpl& self) { return to_string(self); })
        .def("__repr__", [](const RequirementsImpl& self) { return to_string(self); })
        .def("get_index", &RequirementsImpl::get_index, py::return_value_policy::copy)
        .def("get_requirements", &RequirementsImpl::get_requirements, py::return_value_policy::copy);

    py::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", [](const ObjectImpl& self) { return to_string(self); })
        .def("__repr__", [](const ObjectImpl& self) { return to_string(self); })
        .def("get_index", &ObjectImpl::get_index, py::return_value_policy::copy)
        .def("get_name", &ObjectImpl::get_name, py::return_value_policy::copy);
    py::bind_vector<ObjectList>(m, "ObjectList");

    py::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", [](const VariableImpl& self) { return to_string(self); })
        .def("__repr__", [](const VariableImpl& self) { return to_string(self); })
        .def("get_index", &VariableImpl::get_index, py::return_value_policy::copy)
        .def("get_name", &VariableImpl::get_name, py::return_value_policy::copy);
    py::bind_vector<VariableList>(m, "VariableList");

    py::class_<TermImpl>(m, "Term")  //
        .def(
            "get",
            [](const TermImpl& term) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, term.get_variant()); },
            py::keep_alive<0, 1>());
    py::bind_vector<TermList>(m, "TermList");

    auto bind_predicate = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<PredicateImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const PredicateImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const PredicateImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &PredicateImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_name", &PredicateImpl<Tag>::get_name, py::return_value_policy::copy)
            .def("get_parameters", &PredicateImpl<Tag>::get_parameters, py::return_value_policy::copy)
            .def("get_arity", &PredicateImpl<Tag>::get_arity, py::return_value_policy::copy);
        py::bind_vector<PredicateList<Tag>>(m, class_name + "List");
        py::bind_map<ToPredicateMap<std::string, Tag>>(m, class_name + "Map");
    };
    bind_predicate("StaticPredicate", Static {});
    bind_predicate("FluentPredicate", Fluent {});
    bind_predicate("DerivedPredicate", Derived {});

    auto bind_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<AtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const AtomImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const AtomImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &AtomImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_predicate", &AtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_terms", &AtomImpl<Tag>::get_terms, py::return_value_policy::copy)
            .def("get_variables", &AtomImpl<Tag>::get_variables, py::return_value_policy::reference_internal);
        py::bind_vector<AtomList<Tag>>(m, class_name + "List");
    };
    bind_atom("StaticAtom", Static {});
    bind_atom("FluentAtom", Fluent {});
    bind_atom("DerivedAtom", Derived {});

    auto bind_function_skeleton = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<FunctionSkeletonImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionSkeletonImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const FunctionSkeletonImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &FunctionSkeletonImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_name", &FunctionSkeletonImpl<Tag>::get_name, py::return_value_policy::copy)
            .def("get_parameters", &FunctionSkeletonImpl<Tag>::get_parameters, py::return_value_policy::copy);
        py::bind_vector<FunctionSkeletonList<Tag>>(m, class_name + "List");
    };
    bind_function_skeleton("StaticFunctionSkeleton", Static {});
    bind_function_skeleton("FluentFunctionSkeleton", Fluent {});
    bind_function_skeleton("AuxiliaryFunctionSkeleton", Auxiliary {});

    auto bind_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<FunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const FunctionImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &FunctionImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_function_skeleton", &FunctionImpl<Tag>::get_function_skeleton, py::return_value_policy::reference_internal)
            .def("get_terms", &FunctionImpl<Tag>::get_terms, py::return_value_policy::copy);
        py::bind_vector<FunctionList<Tag>>(m, class_name + "List");
    };
    bind_function("StaticFunction", Static {});
    bind_function("FluentFunction", Fluent {});
    bind_function("AuxiliaryFunction", Auxiliary {});

    auto bind_ground_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundFunctionImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundFunctionImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundFunctionImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_function_skeleton", &GroundFunctionImpl<Tag>::get_function_skeleton, py::return_value_policy::reference_internal)
            .def("get_objects", &GroundFunctionImpl<Tag>::get_objects, py::return_value_policy::copy);
        py::bind_vector<GroundFunctionList<Tag>>(m, class_name + "List");
    };
    bind_ground_function("StaticGroundFunction", Static {});
    bind_ground_function("FluentGroundFunction", Fluent {});
    bind_ground_function("AuxiliaryGroundFunction", Auxiliary {});

    auto bind_ground_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundAtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundAtomImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundAtomImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundAtomImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_arity", &GroundAtomImpl<Tag>::get_arity, py::return_value_policy::copy)
            .def("get_predicate", &GroundAtomImpl<Tag>::get_predicate, py::return_value_policy::reference_internal)
            .def("get_objects", &GroundAtomImpl<Tag>::get_objects, py::return_value_policy::copy);
        py::bind_vector<GroundAtomList<Tag>>(m, class_name + "List");
    };
    bind_ground_atom("StaticGroundAtom", Static {});
    bind_ground_atom("FluentGroundAtom", Fluent {});
    bind_ground_atom("DerivedGroundAtom", Derived {});

    auto bind_ground_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundLiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundLiteralImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundLiteralImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundLiteralImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_atom", &GroundLiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &GroundLiteralImpl<Tag>::is_negated, py::return_value_policy::copy);
        py::bind_vector<GroundLiteralList<Tag>>(m, class_name + "List");
    };
    bind_ground_literal("StaticGroundLiteral", Static {});
    bind_ground_literal("FluentGroundLiteral", Fluent {});
    bind_ground_literal("DerivedGroundLiteral", Derived {});

    auto bind_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<LiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const LiteralImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const LiteralImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &LiteralImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_atom", &LiteralImpl<Tag>::get_atom, py::return_value_policy::reference_internal)
            .def("is_negated", &LiteralImpl<Tag>::is_negated, py::return_value_policy::copy);
        py::bind_vector<LiteralList<Tag>>(m, class_name + "List");
    };
    bind_literal("StaticLiteral", Static {});
    bind_literal("FluentLiteral", Fluent {});
    bind_literal("DerivedLiteral", Derived {});

    auto bind_ground_function_value = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundFunctionValueImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundFunctionValueImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundFunctionValueImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundFunctionValueImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_function", &GroundFunctionValueImpl<Tag>::get_function, py::return_value_policy::reference_internal)
            .def("get_number", &GroundFunctionValueImpl<Tag>::get_number, py::return_value_policy::copy);
        py::bind_vector<GroundFunctionValueList<Tag>>(m, class_name + "List");
    };
    bind_ground_function_value("StaticGroundFunctionValue", Static {});
    bind_ground_function_value("FluentGroundFunctionValue", Fluent {});
    bind_ground_function_value("AuxiliaryGroundFunctionValue", Auxiliary {});

    py::class_<FunctionExpressionImpl>(m, "FunctionExpression")  //
        .def("get",
             [](const FunctionExpressionImpl& fexpr) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, fexpr.get_variant()); });
    py::bind_vector<FunctionExpressionList>(m, "FunctionExpressionList");

    py::class_<FunctionExpressionNumberImpl>(m, "FunctionExpressionNumber")  //
        .def("__str__", [](const FunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionNumberImpl::get_index, py::return_value_policy::copy)
        .def("get_number", &FunctionExpressionNumberImpl::get_number, py::return_value_policy::copy);

    py::class_<FunctionExpressionBinaryOperatorImpl>(m, "FunctionExpressionBinaryOperator")  //
        .def("__str__", [](const FunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionBinaryOperatorImpl::get_index, py::return_value_policy::copy)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator, py::return_value_policy::copy)
        .def("get_left_function_expression", &FunctionExpressionBinaryOperatorImpl::get_left_function_expression, py::return_value_policy::reference_internal)
        .def("get_right_function_expression",
             &FunctionExpressionBinaryOperatorImpl::get_right_function_expression,
             py::return_value_policy::reference_internal);

    py::class_<FunctionExpressionMultiOperatorImpl>(m, "FunctionExpressionMultiOperator")  //
        .def("__str__", [](const FunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionMultiOperatorImpl::get_index, py::return_value_policy::copy)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator, py::return_value_policy::copy)
        .def("get_function_expressions", &FunctionExpressionMultiOperatorImpl::get_function_expressions, py::return_value_policy::copy);

    py::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", [](const FunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__repr__", [](const FunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("get_index", &FunctionExpressionMinusImpl::get_index, py::return_value_policy::copy)
        .def("get_function_expression", &FunctionExpressionMinusImpl::get_function_expression, py::return_value_policy::reference_internal);

    auto bind_function_expression_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<FunctionExpressionFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const FunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &FunctionExpressionFunctionImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_function", &FunctionExpressionFunctionImpl<Tag>::get_function, py::return_value_policy::reference_internal);
    };
    bind_function_expression_function("StaticFunctionExpressionFunction", Static {});
    bind_function_expression_function("FluentFunctionExpressionFunction", Fluent {});
    bind_function_expression_function("AuxiliaryFunctionExpressionFunction", Auxiliary {});

    /* NumericEffect */
    auto bind_numeric_effect = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<NumericEffectImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const NumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const NumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &NumericEffectImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_assign_operator", &NumericEffectImpl<Tag>::get_assign_operator, py::return_value_policy::copy)
            .def("get_function", &NumericEffectImpl<Tag>::get_function, py::return_value_policy::reference_internal)
            .def("get_function_expression", &NumericEffectImpl<Tag>::get_function_expression, py::return_value_policy::reference_internal);
    };

    bind_numeric_effect("FluentNumericEffect", Fluent {});
    bind_numeric_effect("AuxiliaryNumericEffect", Auxiliary {});

    /* NumericConstraint */
    py::class_<NumericConstraintImpl>(m, "NumericConstraint")  //
        .def("__str__", [](const NumericConstraintImpl& self) { return to_string(self); })
        .def("__repr__", [](const NumericConstraintImpl& self) { return to_string(self); })
        .def("get_index", &NumericConstraintImpl::get_index, py::return_value_policy::copy)
        .def("get_binary_comparator", &NumericConstraintImpl::get_binary_comparator, py::return_value_policy::copy)
        .def("get_left_function_expression", &NumericConstraintImpl::get_left_function_expression, py::return_value_policy::reference_internal)
        .def("get_right_function_expression", &NumericConstraintImpl::get_right_function_expression, py::return_value_policy::reference_internal);
    py::bind_vector<NumericConstraintList>(m, "NumericConstraintList");

    /* ConjunctiveCondition */
    py::class_<ConjunctiveConditionImpl>(m, "ConjunctiveCondition")  //
        .def("get_parameters", &ConjunctiveConditionImpl::get_parameters, py::return_value_policy::copy)
        .def("get_static_literals", &ConjunctiveConditionImpl::get_literals<Static>, py::return_value_policy::copy)
        .def("get_fluent_literals", &ConjunctiveConditionImpl::get_literals<Fluent>, py::return_value_policy::copy)
        .def("get_derived_literals", &ConjunctiveConditionImpl::get_literals<Derived>, py::return_value_policy::copy)
        .def("get_nullary_ground_static_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<Static>, py::return_value_policy::copy)
        .def("get_nullary_ground_fluent_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<Fluent>, py::return_value_policy::copy)
        .def("get_nullary_ground_derived_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<Derived>, py::return_value_policy::copy)
        .def("get_numeric_constraints", &ConjunctiveConditionImpl::get_numeric_constraints, py::return_value_policy::copy);

    /* ConjunctiveEffectImpl */
    py::class_<ConjunctiveEffectImpl>(m, "ConjunctiveEffect")  //
        .def("__str__", [](const ConjunctiveEffectImpl& self) { return to_string(self); })
        .def("__repr__", [](const ConjunctiveEffectImpl& self) { return to_string(self); })
        .def("get_index", &ConjunctiveEffectImpl::get_index, py::return_value_policy::copy)
        .def("get_parameters", &ConjunctiveEffectImpl::get_parameters, py::return_value_policy::copy)
        .def("get_literals", &ConjunctiveEffectImpl::get_literals, py::return_value_policy::copy)
        .def("get_fluent_numeric_effects", &ConjunctiveEffectImpl::get_fluent_numeric_effects, py::return_value_policy::copy)
        .def("get_auxiliary_numeric_effect", &ConjunctiveEffectImpl::get_auxiliary_numeric_effect, py::return_value_policy::copy);

    /* ConditionalEffect */
    py::class_<ConditionalEffectImpl>(m, "ConditionalEffect")  //
        .def("__str__", [](const ConditionalEffectImpl& self) { return to_string(self); })
        .def("__repr__", [](const ConditionalEffectImpl& self) { return to_string(self); })
        .def("get_index", &ConditionalEffectImpl::get_index, py::return_value_policy::copy)
        .def("get_parameters", &ConditionalEffectImpl::get_conjunctive_condition, py::return_value_policy::reference_internal)
        .def("get_static_conditions", &ConditionalEffectImpl::get_conjunctive_effect, py::return_value_policy::reference_internal);
    py::bind_vector<ConditionalEffectList>(m, "ConditionalEffectList");

    /* Action */
    py::class_<ActionImpl>(m, "Action")  //
        .def("__str__", [](const ActionImpl& self) { return to_string(self); })
        .def("__repr__", [](const ActionImpl& self) { return to_string(self); })
        .def("get_index", &ActionImpl::get_index, py::return_value_policy::copy)
        .def("get_name", &ActionImpl::get_name, py::return_value_policy::copy)
        .def("get_parameters", &ActionImpl::get_parameters, py::return_value_policy::copy)
        .def("get_conjunctive_condition", &ActionImpl::get_conjunctive_condition, py::return_value_policy::reference_internal)
        .def("get_conjunctive_effect", &ActionImpl::get_conjunctive_effect, py::return_value_policy::reference_internal)
        .def("get_conditional_effects", &ActionImpl::get_conditional_effects, py::return_value_policy::copy)
        .def("get_arity", &ActionImpl::get_arity, py::return_value_policy::copy);
    py::bind_vector<ActionList>(m, "ActionList");

    /* Axiom */
    py::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", [](const AxiomImpl& self) { return to_string(self); })
        .def("__repr__", [](const AxiomImpl& self) { return to_string(self); })
        .def("get_index", &AxiomImpl::get_index, py::return_value_policy::copy)
        .def("get_conjunctive_condition", &AxiomImpl::get_conjunctive_condition, py::return_value_policy::reference_internal)
        .def("get_literal", &AxiomImpl::get_literal, py::return_value_policy::reference_internal)
        .def("get_arity", &AxiomImpl::get_arity, py::return_value_policy::copy);
    py::bind_vector<AxiomList>(m, "AxiomList");

    /* GroundFunctionExpression */
    py::class_<GroundFunctionExpressionImpl>(m, "GroundFunctionExpression")  //
        .def("get",
             [](const GroundFunctionExpressionImpl& fexpr) -> py::object { return std::visit([](auto&& arg) { return py::cast(arg); }, fexpr.get_variant()); });
    py::bind_vector<GroundFunctionExpressionList>(m, "GroundFunctionExpressionList");

    py::class_<GroundFunctionExpressionNumberImpl>(m, "GroundFunctionExpressionNumber")  //
        .def("__str__", [](const GroundFunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionNumberImpl::get_index, py::return_value_policy::copy)
        .def("get_number", &GroundFunctionExpressionNumberImpl::get_number, py::return_value_policy::copy);

    py::class_<GroundFunctionExpressionBinaryOperatorImpl>(m, "GroundFunctionExpressionBinaryOperator")  //
        .def("__str__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionBinaryOperatorImpl::get_index, py::return_value_policy::copy)
        .def("get_binary_operator", &GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator, py::return_value_policy::copy)
        .def("get_left_function_expression",
             &GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression,
             py::return_value_policy::reference_internal)
        .def("get_right_function_expression",
             &GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression,
             py::return_value_policy::reference_internal);

    py::class_<GroundFunctionExpressionMultiOperatorImpl>(m, "GroundFunctionExpressionMultiOperator")  //
        .def("__str__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionMultiOperatorImpl::get_index, py::return_value_policy::copy)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator, py::return_value_policy::copy)
        .def("get_function_expressions", &GroundFunctionExpressionMultiOperatorImpl::get_function_expressions, py::return_value_policy::copy);

    py::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", [](const GroundFunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__repr__", [](const GroundFunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("get_index", &GroundFunctionExpressionMinusImpl::get_index, py::return_value_policy::copy)
        .def("get_function_expression", &GroundFunctionExpressionMinusImpl::get_function_expression, py::return_value_policy::reference_internal);

    auto bind_ground_function_expression_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundFunctionExpressionFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundFunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundFunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundFunctionExpressionFunctionImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_function", &GroundFunctionExpressionFunctionImpl<Tag>::get_function, py::return_value_policy::reference_internal);
    };
    bind_ground_function_expression_function("StaticGroundFunctionExpressionFunction", Static {});
    bind_ground_function_expression_function("FluentGroundFunctionExpressionFunction", Fluent {});
    bind_ground_function_expression_function("AuxiliaryGroundFunctionExpressionFunction", Auxiliary {});

    /* OptimizationMetric */
    py::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", [](const OptimizationMetricImpl& self) { return to_string(self); })
        .def("__repr__", [](const OptimizationMetricImpl& self) { return to_string(self); })
        .def("get_index", &OptimizationMetricImpl::get_index, py::return_value_policy::copy)
        .def("get_function_expression", &OptimizationMetricImpl::get_function_expression, py::return_value_policy::reference_internal)
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, py::return_value_policy::reference_internal);

    auto bind_ground_numeric_effect = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<GroundNumericEffectImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundNumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("__repr__", [](const GroundNumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundNumericEffectImpl<Tag>::get_index, py::return_value_policy::copy)
            .def("get_assign_operator", &GroundNumericEffectImpl<Tag>::get_assign_operator, py::return_value_policy::copy)
            .def("get_function", &GroundNumericEffectImpl<Tag>::get_function, py::return_value_policy::reference_internal)
            .def("get_function_expression", &GroundNumericEffectImpl<Tag>::get_function_expression, py::return_value_policy::reference_internal);
    };
    bind_ground_numeric_effect("FluentGroundNumericEffect", Fluent {});
    bind_ground_numeric_effect("AuxiliaryGroundNumericEffect", Auxiliary {});

    /* GroundConjunctiveCondition */
    py::class_<GroundConjunctiveCondition>(m, "GroundConjunctiveCondition")
        .def("get_fluent_positive_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_positive_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_positive_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_positive_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_positive_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_positive_precondition<Derived>, py::const_),
             py::return_value_policy::copy)
        .def("get_fluent_negative_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_negative_precondition<Fluent>, py::const_),
             py::return_value_policy::copy)
        .def("get_static_negative_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_negative_precondition<Static>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_negative_condition",
             py::overload_cast<>(&GroundConjunctiveCondition::get_negative_precondition<Derived>, py::const_),
             py::return_value_policy::copy);

    /* GroundConjunctiveEffect */
    py::class_<GroundConjunctiveEffect>(m, "GroundConjunctiveEffect")
        .def("get_positive_effects", py::overload_cast<>(&GroundConjunctiveEffect::get_positive_effects, py::const_), py::return_value_policy::copy)
        .def("get_negative_effects", py::overload_cast<>(&GroundConjunctiveEffect::get_negative_effects, py::const_), py::return_value_policy::copy)
        .def("get_fluent_numeric_effects", py::overload_cast<>(&GroundConjunctiveEffect::get_fluent_numeric_effects, py::const_), py::return_value_policy::copy)
        .def("get_auxiliary_numeric_effect",
             py::overload_cast<>(&GroundConjunctiveEffect::get_auxiliary_numeric_effect, py::const_),
             py::return_value_policy::copy);

    /* GroundConditionalEffect */
    py::class_<GroundConditionalEffect>(m, "GroundConditionalEffect")
        .def("get_conjunctive_condition", py::overload_cast<>(&GroundConditionalEffect::get_conjunctive_condition, py::const_), py::return_value_policy::copy)
        .def("get_conjunctive_effect", py::overload_cast<>(&GroundConditionalEffect::get_conjunctive_effect, py::const_), py::return_value_policy::copy);

    /* GroundAction */
    py::class_<GroundActionImpl>(m, "GroundAction")  //
        .def(
            "__hash__",
            [](const GroundActionImpl& self) { return self.get_index(); },
            py::return_value_policy::copy)
        .def("__eq__", [](const GroundActionImpl& lhs, const GroundActionImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("to_string",
             [](const GroundActionImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAction(&self), std::cref(problem), FullActionFormatterTag {});
                 return ss.str();
             })
        .def("to_string_for_plan",
             [](const GroundActionImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAction(&self), std::cref(problem), PlanActionFormatterTag {});
                 return ss.str();
             })
        .def("get_index", py::overload_cast<>(&GroundActionImpl::get_index, py::const_), py::return_value_policy::copy)
        .def("get_action_index", py::overload_cast<>(&GroundActionImpl::get_action_index, py::const_), py::return_value_policy::copy)
        .def("get_object_indices", py::overload_cast<>(&GroundActionImpl::get_object_indices, py::const_), py::return_value_policy::copy)
        .def("get_conjunctive_condition", py::overload_cast<>(&GroundActionImpl::get_conjunctive_condition, py::const_), py::return_value_policy::copy)
        .def("get_conjunctive_effect", py::overload_cast<>(&GroundActionImpl::get_conjunctive_effect, py::const_), py::return_value_policy::copy)
        .def("get_conditional_effects", py::overload_cast<>(&GroundActionImpl::get_conditional_effects, py::const_), py::return_value_policy::copy);
    py::bind_vector<GroundActionList>(m, "GroundActionList");

    /* GroundEffectDerivedLiteral */
    py::class_<GroundEffectDerivedLiteral>(m, "GroundEffectDerivedLiteral")
        .def_readonly("is_negated", &GroundEffectDerivedLiteral::is_negated, py::return_value_policy::copy)
        .def_readonly("atom_index", &GroundEffectDerivedLiteral::atom_index, py::return_value_policy::copy);

    /* GroundAxiom */
    py::class_<GroundAxiomImpl>(m, "GroundAxiom")  //
        .def("__hash__", [](const GroundAxiomImpl& self) { return self.get_index(); })
        .def("__eq__", [](const GroundAxiomImpl& lhs, const GroundAxiomImpl& rhs) { return lhs.get_index() == rhs.get_index(); })
        .def("to_string",
             [](const GroundAxiomImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 ss << std::make_tuple(GroundAxiom(&self), std::cref(problem));
                 return ss.str();
             })
        .def("get_index", py::overload_cast<>(&GroundAxiomImpl::get_index, py::const_), py::return_value_policy::copy)
        .def("get_axiom_index", py::overload_cast<>(&GroundAxiomImpl::get_axiom_index, py::const_), py::return_value_policy::copy)
        .def("get_object_indices", py::overload_cast<>(&GroundAxiomImpl::get_object_indices, py::const_), py::return_value_policy::copy)
        .def("get_conjunctive_condition", py::overload_cast<>(&GroundAxiomImpl::get_conjunctive_condition, py::const_), py::return_value_policy::copy)
        .def("get_derived_effect", py::overload_cast<>(&GroundAxiomImpl::get_derived_effect, py::const_), py::return_value_policy::copy);
    py::bind_vector<GroundAxiomList>(m, "GroundAxiomList");

    /* PDDLRepositories*/
    // ATTENTION: cannot provide modifiers since Problem and Domain returns a reference
    py::class_<PDDLRepositories>(m, "PDDLRepositories")  //
        .def(
            "get_static_ground_atoms",
            [](const PDDLRepositories& self) { return GroundAtomList<Static>(self.get_ground_atoms<Static>().begin(), self.get_ground_atoms<Static>().end()); },
            py::return_value_policy::copy)
        .def("get_static_ground_atom", &PDDLRepositories::get_ground_atom<Static>, py::return_value_policy::reference_internal)
        .def(
            "get_fluent_ground_atoms",
            [](const PDDLRepositories& self) { return GroundAtomList<Fluent>(self.get_ground_atoms<Fluent>().begin(), self.get_ground_atoms<Fluent>().end()); },
            py::return_value_policy::copy)
        .def("get_fluent_ground_atom", &PDDLRepositories::get_ground_atom<Fluent>, py::return_value_policy::reference_internal)
        .def(
            "get_derived_ground_atoms",
            [](const PDDLRepositories& self)
            { return GroundAtomList<Derived>(self.get_ground_atoms<Derived>().begin(), self.get_ground_atoms<Derived>().end()); },
            py::return_value_policy::copy)
        .def("get_derived_ground_atom", &PDDLRepositories::get_ground_atom<Derived>, py::return_value_policy::reference_internal)
        .def("get_static_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Static, std::vector<size_t>>, py::const_),
             py::return_value_policy::copy)
        .def("get_fluent_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Fluent, std::vector<size_t>>, py::const_),
             py::return_value_policy::copy)
        .def("get_derived_ground_atoms_from_indices",
             py::overload_cast<const std::vector<size_t>&>(&PDDLRepositories::get_ground_atoms_from_indices<Derived, std::vector<size_t>>, py::const_),
             py::return_value_policy::copy)
        .def("get_object", &PDDLRepositories::get_object, py::return_value_policy::reference_internal);

    /* AssignmentSets */
    auto bind_assignment_set = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<AssignmentSet<Tag>>(m, class_name.c_str())
            .def(py::init<size_t, const PredicateList<Tag>&>(), py::arg("num_objects"), py::arg("predicates"))
            .def("reset", &AssignmentSet<Tag>::reset)
            .def("insert_ground_atoms", &AssignmentSet<Tag>::insert_ground_atoms, py::arg("ground_atoms"))
            .def("insert_ground_atom", &AssignmentSet<Tag>::insert_ground_atoms, py::arg("ground_atom"));
    };
    bind_assignment_set("StaticAssignmentSet", Static {});
    bind_assignment_set("FluentAssignmentSet", Fluent {});
    bind_assignment_set("DerivedAssignmentSet", Derived {});

    /* NumericAssignmentSets */
    auto bind_numeric_assignment_set = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        py::class_<NumericAssignmentSet<Tag>>(m, class_name.c_str())
            .def(py::init<>())
            .def(py::init<size_t, const FunctionSkeletonList<Tag>&>(), py::arg("num_objects"), py::arg("function_skeletons"))
            .def("reset", &NumericAssignmentSet<Tag>::reset)
            .def("insert_ground_function_values",
                 &NumericAssignmentSet<Tag>::insert_ground_function_values,
                 py::arg("ground_functions"),
                 py::arg("numeric_values"));
    };

    bind_numeric_assignment_set("StaticNumericAssignmentSet", Static {});
    bind_numeric_assignment_set("FluentNumericAssignmentSet", Fluent {});

    /* Domain */
    py::class_<DomainImpl, std::shared_ptr<DomainImpl>>(m, "Domain")  //
        .def("__str__", [](const DomainImpl& self) { return to_string(self); })
        .def("__repr__", [](const DomainImpl& self) { return to_string(self); })
        .def("get_repositories", &DomainImpl::get_repositories, py::return_value_policy::reference_internal)
        .def("get_filepath",
             [](const DomainImpl& self)
             { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; })
        .def("get_name", &DomainImpl::get_name, py::return_value_policy::copy)
        .def("get_constants", &DomainImpl::get_constants, py::return_value_policy::copy)
        .def("get_static_predicates", &DomainImpl::get_predicates<Static>, py::return_value_policy::copy)
        .def("get_fluent_predicates", &DomainImpl::get_predicates<Fluent>, py::return_value_policy::copy)
        .def("get_derived_predicates", &DomainImpl::get_predicates<Derived>, py::return_value_policy::copy)
        .def("get_static_functions", &DomainImpl::get_function_skeletons<Static>, py::return_value_policy::copy)
        .def("get_fluent_functions", &DomainImpl::get_function_skeletons<Fluent>, py::return_value_policy::copy)
        .def("get_auxiliary_function", &DomainImpl::get_auxiliary_function_skeleton, py::return_value_policy::copy)
        .def("get_actions", &DomainImpl::get_actions, py::return_value_policy::copy)
        .def("get_requirements", &DomainImpl::get_requirements, py::return_value_policy::copy)
        .def("get_name_to_static_predicate", &DomainImpl::get_name_to_predicate<Static>, py::return_value_policy::copy)
        .def("get_name_to_fluent_predicate", &DomainImpl::get_name_to_predicate<Fluent>, py::return_value_policy::copy)
        .def("get_name_to_derived_predicate", &DomainImpl::get_name_to_predicate<Derived>, py::return_value_policy::copy);

    /* Problem */
    py::class_<ProblemImpl, std::shared_ptr<ProblemImpl>>(m, "Problem")  //
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::string& problem_filepath, const loki::Options& options)
            { return ProblemImpl::create(domain_filepath, problem_filepath, options); },
            py::arg("domain_filepath"),
            py::arg("problem_filepath"),
            py::arg("options") = loki::Options())
        .def("__str__", [](const ProblemImpl& self) { return to_string(self); })
        .def("__repr__", [](const ProblemImpl& self) { return to_string(self); })
        .def("get_index", &ProblemImpl::get_index, py::return_value_policy::copy)
        .def("get_repositories", &ProblemImpl::get_repositories, py::return_value_policy::copy)
        .def("get_filepath",
             [](const ProblemImpl& self)
             { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; })
        .def("get_name", &ProblemImpl::get_name, py::return_value_policy::copy)
        .def("get_domain", &ProblemImpl::get_domain, py::return_value_policy::copy)
        .def("get_requirements", &ProblemImpl::get_requirements, py::return_value_policy::copy)
        .def("get_objects", &ProblemImpl::get_objects, py::return_value_policy::copy)
        .def("get_static_initial_literals", &ProblemImpl::get_initial_literals<Static>, py::return_value_policy::copy)
        .def("get_fluent_initial_literals", &ProblemImpl::get_initial_literals<Fluent>, py::return_value_policy::copy)
        .def("get_static_function_values", &ProblemImpl::get_initial_function_values<Static>, py::return_value_policy::copy)
        .def("get_fluent_function_values", &ProblemImpl::get_initial_function_values<Fluent>, py::return_value_policy::copy)
        .def("get_auxiliary_function_value", &ProblemImpl::get_auxiliary_function_value, py::return_value_policy::copy)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, py::return_value_policy::copy)
        .def("get_static_goal_condition", &ProblemImpl::get_goal_condition<Static>, py::return_value_policy::copy)
        .def("get_fluent_goal_condition", &ProblemImpl::get_goal_condition<Fluent>, py::return_value_policy::copy)
        .def("get_derived_goal_condition", &ProblemImpl::get_goal_condition<Derived>, py::return_value_policy::copy)
        .def("get_static_initial_atoms", &ProblemImpl::get_static_initial_atoms, py::return_value_policy::copy)
        .def("get_fluent_initial_atoms", &ProblemImpl::get_fluent_initial_atoms, py::return_value_policy::copy)
        .def("get_static_assignment_set", &ProblemImpl::get_static_assignment_set, py::return_value_policy::copy);
    py::bind_vector<ProblemList>(m, "ProblemList");

    /* GeneralizedProblem */
    py::class_<GeneralizedProblem>(m, "GeneralizedProblem")
        .def(py::init(
                 [](const std::string& domain_filepath, const std::vector<std::string>& problem_filepaths, const loki::Options& options)
                 {
                     std::vector<fs::path> paths;
                     for (const auto& filepath : problem_filepaths)
                     {
                         paths.emplace_back(filepath);
                     }
                     return GeneralizedProblem(fs::path(domain_filepath), paths, options);
                 }),
             py::arg("domain_filepath"),
             py::arg("problem_filepaths"),
             py::arg("options") = loki::Options())

        .def(py::init([](const std::string& domain_filepath, const std::string& problems_directory, const loki::Options& options)
                      { return GeneralizedProblem(fs::path(domain_filepath), fs::path(problems_directory), options); }),
             py::arg("domain_filepath"),
             py::arg("problems_directory"),
             py::arg("options") = loki::Options())

        .def(py::init<Domain, ProblemList>(), py::arg("domain"), py::arg("problems"))

        .def("get_domain", &GeneralizedProblem::get_domain)
        .def("get_problems", &GeneralizedProblem::get_problems);

    /**
     * Parser
     */

    py::class_<Parser>(m, "Parser")
        .def(py::init([](const std::string& domain_filepath, const loki::Options& options) { return Parser(std::filesystem::path(domain_filepath), options); }),
             py::arg("domain_filepath"),
             py::arg("options") = loki::Options())
        .def(
            "parse_problem",
            [](Parser& self, const std::string& problem_filepath, const loki::Options& options) { return self.parse_problem(problem_filepath, options); },
            py::arg("problem_filepath"),
            py::arg("options") = loki::Options())
        .def("get_domain", &Parser::get_domain);

    /**
     * Translator
     */

    py::class_<DomainTranslationResult>(m, "DomainTranslationResult")
        .def(py::init<Domain, Domain>(), py::arg("original_domain"), py::arg("translated_domain"))
        .def("get_original_domain", &DomainTranslationResult::get_original_domain)
        .def("get_translated_domain", &DomainTranslationResult::get_translated_domain);

    m.def("translate", py::overload_cast<const Domain&>(&translate), py::arg("domain"));
    m.def("translate", py::overload_cast<const Problem&, const DomainTranslationResult&>(&translate), py::arg("problem"), py::arg("domain_translation_result"));
}
