#include "../init_declarations.hpp"

namespace mimir::formalism
{

void bind_module_definitions(nb::module_& m)
{
    nb::enum_<loki::RequirementEnum>(m, "RequirementEnum")
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

    nb::enum_<loki::AssignOperatorEnum>(m, "AssignOperatorEnum")
        .value("ASSIGN", loki::AssignOperatorEnum::ASSIGN)
        .value("SCALE_UP", loki::AssignOperatorEnum::SCALE_UP)
        .value("SCALE_DOWN", loki::AssignOperatorEnum::SCALE_DOWN)
        .value("INCREASE", loki::AssignOperatorEnum::INCREASE)
        .value("DECREASE", loki::AssignOperatorEnum::DECREASE)
        .export_values();

    nb::enum_<loki::BinaryOperatorEnum>(m, "BinaryOperatorEnum")
        .value("MUL", loki::BinaryOperatorEnum::MUL)
        .value("PLUS", loki::BinaryOperatorEnum::PLUS)
        .value("MINUS", loki::BinaryOperatorEnum::MINUS)
        .value("DIV", loki::BinaryOperatorEnum::DIV)
        .export_values();

    nb::enum_<loki::MultiOperatorEnum>(m, "MultiOperatorEnum")
        .value("MUL", loki::MultiOperatorEnum::MUL)
        .value("PLUS", loki::MultiOperatorEnum::PLUS)
        .export_values();

    nb::enum_<loki::BinaryComparatorEnum>(m, "BinaryComparatorEnum")
        .value("EQUAL", loki::BinaryComparatorEnum::EQUAL)
        .value("GREATER", loki::BinaryComparatorEnum::GREATER)
        .value("GREATER_EQUAL", loki::BinaryComparatorEnum::GREATER_EQUAL)
        .value("LESS", loki::BinaryComparatorEnum::LESS)
        .value("LESS_EQUAL", loki::BinaryComparatorEnum::LESS_EQUAL)
        .export_values();

    nb::enum_<loki::OptimizationMetricEnum>(m, "OptimizationMetricEnum")
        .value("MINIMIZE", loki::OptimizationMetricEnum::MINIMIZE)
        .value("MAXIMIZE", loki::OptimizationMetricEnum::MAXIMIZE)
        .export_values();

    nb::class_<loki::Options>(m, "ParserOptions")
        .def(nb::init<>())
        .def_rw("strict", &loki::Options::strict, "Enable strict mode")
        .def_rw("quiet", &loki::Options::quiet, "Suppress output");

    nb::class_<RequirementsImpl>(m, "Requirements")  //
        .def("__str__", [](const RequirementsImpl& self) { return to_string(self); })
        .def("__eq__", [](const RequirementsImpl& lhs, const RequirementsImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const RequirementsImpl& lhs, const RequirementsImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const RequirementsImpl& self) { return std::hash<Requirements> {}(&self); })
        .def("get_index", &RequirementsImpl::get_index, nb::rv_policy::copy)
        .def("get_requirements", &RequirementsImpl::get_requirements, nb::rv_policy::copy);

    nb::class_<ObjectImpl>(m, "Object")  //
        .def("__str__", [](const ObjectImpl& self) { return to_string(self); })
        .def("__eq__", [](const ObjectImpl& lhs, const ObjectImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ObjectImpl& lhs, const ObjectImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ObjectImpl& self) { return std::hash<Object> {}(&self); })
        .def("get_index", &ObjectImpl::get_index, nb::rv_policy::copy)
        .def("get_name", &ObjectImpl::get_name, nb::rv_policy::copy);
    nb::bind_vector<ObjectList>(m, "ObjectList");

    nb::class_<VariableImpl>(m, "Variable")  //
        .def("__str__", [](const VariableImpl& self) { return to_string(self); })
        .def("__eq__", [](const VariableImpl& lhs, const VariableImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const VariableImpl& lhs, const VariableImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const VariableImpl& self) { return std::hash<Variable> {}(&self); })
        .def("get_index", &VariableImpl::get_index, nb::rv_policy::copy)
        .def("get_name", &VariableImpl::get_name, nb::rv_policy::copy);
    nb::bind_vector<VariableList>(m, "VariableList");

    nb::class_<TermImpl>(m, "Term")  //
        .def("__str__", [](const TermImpl& self) { return to_string(self); })
        .def("__eq__", [](const TermImpl& lhs, const TermImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const TermImpl& lhs, const TermImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const TermImpl& self) { return std::hash<Term> {}(&self); })
        .def(
            "get",
            [](const TermImpl& term) -> nb::object { return std::visit([](auto&& arg) { return nb::cast(arg); }, term.get_variant()); },
            nb::keep_alive<0, 1>());
    nb::bind_vector<TermList>(m, "TermList");

    auto bind_predicate = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<PredicateImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const PredicateImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const PredicateImpl<Tag>& lhs, const PredicateImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const PredicateImpl<Tag>& lhs, const PredicateImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const PredicateImpl<Tag>& self) { return std::hash<Predicate<Tag>> {}(&self); })
            .def("get_index", &PredicateImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_name", &PredicateImpl<Tag>::get_name, nb::rv_policy::copy)
            .def("get_parameters", &PredicateImpl<Tag>::get_parameters, nb::rv_policy::copy)
            .def("get_arity", &PredicateImpl<Tag>::get_arity, nb::rv_policy::copy);
        nb::bind_vector<PredicateList<Tag>>(m, (class_name + "List").c_str());
        nb::bind_map<ToPredicateMap<std::string, Tag>>(m, (class_name + "Map").c_str());
    };
    bind_predicate("StaticPredicate", StaticTag {});
    bind_predicate("FluentPredicate", FluentTag {});
    bind_predicate("DerivedPredicate", DerivedTag {});

    auto bind_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<AtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const AtomImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const AtomImpl<Tag>& lhs, const AtomImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const AtomImpl<Tag>& lhs, const AtomImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const AtomImpl<Tag>& self) { return std::hash<Atom<Tag>> {}(&self); })
            .def("get_index", &AtomImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_predicate", &AtomImpl<Tag>::get_predicate, nb::rv_policy::reference_internal)
            .def("get_terms", &AtomImpl<Tag>::get_terms, nb::rv_policy::copy)
            .def("get_variables", &AtomImpl<Tag>::get_variables, nb::rv_policy::reference_internal);
        nb::bind_vector<AtomList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_atom("StaticAtom", StaticTag {});
    bind_atom("FluentAtom", FluentTag {});
    bind_atom("DerivedAtom", DerivedTag {});

    auto bind_function_skeleton = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<FunctionSkeletonImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionSkeletonImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const FunctionSkeletonImpl<Tag>& lhs, const FunctionSkeletonImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const FunctionSkeletonImpl<Tag>& lhs, const FunctionSkeletonImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const FunctionSkeletonImpl<Tag>& self) { return std::hash<FunctionSkeleton<Tag>> {}(&self); })
            .def("get_index", &FunctionSkeletonImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_name", &FunctionSkeletonImpl<Tag>::get_name, nb::rv_policy::copy)
            .def("get_parameters", &FunctionSkeletonImpl<Tag>::get_parameters, nb::rv_policy::copy);
        nb::bind_vector<FunctionSkeletonList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_function_skeleton("StaticFunctionSkeleton", StaticTag {});
    bind_function_skeleton("FluentFunctionSkeleton", FluentTag {});
    bind_function_skeleton("AuxiliaryFunctionSkeleton", AuxiliaryTag {});

    auto bind_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<FunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const FunctionImpl<Tag>& lhs, const FunctionImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const FunctionImpl<Tag>& lhs, const FunctionImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const FunctionImpl<Tag>& self) { return std::hash<Function<Tag>> {}(&self); })
            .def("get_index", &FunctionImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_function_skeleton", &FunctionImpl<Tag>::get_function_skeleton, nb::rv_policy::reference_internal)
            .def("get_terms", &FunctionImpl<Tag>::get_terms, nb::rv_policy::copy);
        nb::bind_vector<FunctionList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_function("StaticFunction", StaticTag {});
    bind_function("FluentFunction", FluentTag {});
    bind_function("AuxiliaryFunction", AuxiliaryTag {});

    auto bind_ground_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__hash__", [](const GroundFunctionImpl<Tag>& self) { return std::hash<GroundFunction<Tag>> {}(&self); })
            .def("__eq__", [](const GroundFunctionImpl<Tag>& lhs, const GroundFunctionImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const GroundFunctionImpl<Tag>& lhs, const GroundFunctionImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__str__", [](const GroundFunctionImpl<Tag>& self) { return to_string(self); })
            .def("get_index", &GroundFunctionImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_function_skeleton", &GroundFunctionImpl<Tag>::get_function_skeleton, nb::rv_policy::reference_internal)
            .def("get_objects", &GroundFunctionImpl<Tag>::get_objects, nb::rv_policy::copy);
        nb::bind_vector<GroundFunctionList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_ground_function("StaticGroundFunction", StaticTag {});
    bind_ground_function("FluentGroundFunction", FluentTag {});
    bind_ground_function("AuxiliaryGroundFunction", AuxiliaryTag {});

    auto bind_ground_atom = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundAtomImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundAtomImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const GroundAtomImpl<Tag>& lhs, const GroundAtomImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const GroundAtomImpl<Tag>& lhs, const GroundAtomImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const GroundAtomImpl<Tag>& self) { return std::hash<GroundAtom<Tag>> {}(&self); })
            .def("get_index", &GroundAtomImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_arity", &GroundAtomImpl<Tag>::get_arity, nb::rv_policy::copy)
            .def("get_predicate", &GroundAtomImpl<Tag>::get_predicate, nb::rv_policy::reference_internal)
            .def("get_objects", &GroundAtomImpl<Tag>::get_objects, nb::rv_policy::copy);
        nb::bind_vector<GroundAtomList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_ground_atom("StaticGroundAtom", StaticTag {});
    bind_ground_atom("FluentGroundAtom", FluentTag {});
    bind_ground_atom("DerivedGroundAtom", DerivedTag {});

    auto bind_ground_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundLiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundLiteralImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const GroundLiteralImpl<Tag>& lhs, const GroundLiteralImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const GroundLiteralImpl<Tag>& lhs, const GroundLiteralImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const GroundLiteralImpl<Tag>& self) { return std::hash<GroundLiteral<Tag>> {}(&self); })
            .def("get_index", &GroundLiteralImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_atom", &GroundLiteralImpl<Tag>::get_atom, nb::rv_policy::reference_internal)
            .def("get_polarity", &GroundLiteralImpl<Tag>::get_polarity, nb::rv_policy::copy);
        nb::bind_vector<GroundLiteralList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_ground_literal("StaticGroundLiteral", StaticTag {});
    bind_ground_literal("FluentGroundLiteral", FluentTag {});
    bind_ground_literal("DerivedGroundLiteral", DerivedTag {});

    auto bind_literal = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<LiteralImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const LiteralImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const LiteralImpl<Tag>& lhs, const LiteralImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const LiteralImpl<Tag>& lhs, const LiteralImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const LiteralImpl<Tag>& self) { return std::hash<Literal<Tag>> {}(&self); })
            .def("get_index", &LiteralImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_atom", &LiteralImpl<Tag>::get_atom, nb::rv_policy::reference_internal)
            .def("get_polarity", &LiteralImpl<Tag>::get_polarity, nb::rv_policy::copy);
        nb::bind_vector<LiteralList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_literal("StaticLiteral", StaticTag {});
    bind_literal("FluentLiteral", FluentTag {});
    bind_literal("DerivedLiteral", DerivedTag {});

    auto bind_ground_function_value = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundFunctionValueImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundFunctionValueImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const GroundFunctionValueImpl<Tag>& lhs, const GroundFunctionValueImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const GroundFunctionValueImpl<Tag>& lhs, const GroundFunctionValueImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const GroundFunctionValueImpl<Tag>& self) { return std::hash<GroundFunctionValue<Tag>> {}(&self); })
            .def("get_index", &GroundFunctionValueImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_function", &GroundFunctionValueImpl<Tag>::get_function, nb::rv_policy::reference_internal)
            .def("get_number", &GroundFunctionValueImpl<Tag>::get_number, nb::rv_policy::copy);
        nb::bind_vector<GroundFunctionValueList<Tag>>(m, (class_name + "List").c_str());
    };
    bind_ground_function_value("StaticGroundFunctionValue", StaticTag {});
    bind_ground_function_value("FluentGroundFunctionValue", FluentTag {});
    bind_ground_function_value("AuxiliaryGroundFunctionValue", AuxiliaryTag {});

    nb::class_<FunctionExpressionImpl>(m, "FunctionExpression")  //
        .def("__str__", [](const FunctionExpressionImpl& self) { return to_string(self); })
        .def("__eq__", [](const FunctionExpressionImpl& lhs, const FunctionExpressionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const FunctionExpressionImpl& lhs, const FunctionExpressionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const FunctionExpressionImpl& self) { return std::hash<FunctionExpression> {}(&self); })
        .def("get",
             [](const FunctionExpressionImpl& fexpr) -> nb::object { return std::visit([](auto&& arg) { return nb::cast(arg); }, fexpr.get_variant()); });
    nb::bind_vector<FunctionExpressionList>(m, "FunctionExpressionList");

    nb::class_<FunctionExpressionNumberImpl>(m, "FunctionExpressionNumber")  //
        .def("__str__", [](const FunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__eq__", [](const FunctionExpressionNumberImpl& lhs, const FunctionExpressionNumberImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const FunctionExpressionNumberImpl& lhs, const FunctionExpressionNumberImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const FunctionExpressionNumberImpl& self) { return std::hash<FunctionExpressionNumber> {}(&self); })
        .def("get_index", &FunctionExpressionNumberImpl::get_index, nb::rv_policy::copy)
        .def("get_number", &FunctionExpressionNumberImpl::get_number, nb::rv_policy::copy);

    nb::class_<FunctionExpressionBinaryOperatorImpl>(m, "FunctionExpressionBinaryOperator")  //
        .def("__str__", [](const FunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__eq__", [](const FunctionExpressionBinaryOperatorImpl& lhs, const FunctionExpressionBinaryOperatorImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const FunctionExpressionBinaryOperatorImpl& lhs, const FunctionExpressionBinaryOperatorImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const FunctionExpressionBinaryOperatorImpl& self) { return std::hash<FunctionExpressionBinaryOperator> {}(&self); })
        .def("get_index", &FunctionExpressionBinaryOperatorImpl::get_index, nb::rv_policy::copy)
        .def("get_binary_operator", &FunctionExpressionBinaryOperatorImpl::get_binary_operator, nb::rv_policy::copy)
        .def("get_left_function_expression", &FunctionExpressionBinaryOperatorImpl::get_left_function_expression, nb::rv_policy::reference_internal)
        .def("get_right_function_expression", &FunctionExpressionBinaryOperatorImpl::get_right_function_expression, nb::rv_policy::reference_internal);

    nb::class_<FunctionExpressionMultiOperatorImpl>(m, "FunctionExpressionMultiOperator")  //
        .def("__str__", [](const FunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__eq__", [](const FunctionExpressionMultiOperatorImpl& lhs, const FunctionExpressionMultiOperatorImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const FunctionExpressionMultiOperatorImpl& lhs, const FunctionExpressionMultiOperatorImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const FunctionExpressionMultiOperatorImpl& self) { return std::hash<FunctionExpressionMultiOperator> {}(&self); })
        .def("get_index", &FunctionExpressionMultiOperatorImpl::get_index, nb::rv_policy::copy)
        .def("get_multi_operator", &FunctionExpressionMultiOperatorImpl::get_multi_operator, nb::rv_policy::copy)
        .def("get_function_expressions", &FunctionExpressionMultiOperatorImpl::get_function_expressions, nb::rv_policy::copy);

    nb::class_<FunctionExpressionMinusImpl>(m, "FunctionExpressionMinus")  //
        .def("__str__", [](const FunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__eq__", [](const FunctionExpressionMinusImpl& lhs, const FunctionExpressionMinusImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const FunctionExpressionMinusImpl& lhs, const FunctionExpressionMinusImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const FunctionExpressionMinusImpl& self) { return std::hash<FunctionExpressionMinus> {}(&self); })
        .def("get_index", &FunctionExpressionMinusImpl::get_index, nb::rv_policy::copy)
        .def("get_function_expression", &FunctionExpressionMinusImpl::get_function_expression, nb::rv_policy::reference_internal);

    auto bind_function_expression_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<FunctionExpressionFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const FunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const FunctionExpressionFunctionImpl<Tag>& lhs, const FunctionExpressionFunctionImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const FunctionExpressionFunctionImpl<Tag>& lhs, const FunctionExpressionFunctionImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const FunctionExpressionFunctionImpl<Tag>& self) { return std::hash<FunctionExpressionFunction<Tag>> {}(&self); })
            .def("get_index", &FunctionExpressionFunctionImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_function", &FunctionExpressionFunctionImpl<Tag>::get_function, nb::rv_policy::reference_internal);
    };
    bind_function_expression_function("StaticFunctionExpressionFunction", StaticTag {});
    bind_function_expression_function("FluentFunctionExpressionFunction", FluentTag {});
    bind_function_expression_function("AuxiliaryFunctionExpressionFunction", AuxiliaryTag {});

    /* NumericEffect */
    auto bind_numeric_effect = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<NumericEffectImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const NumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const NumericEffectImpl<Tag>& lhs, const NumericEffectImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const NumericEffectImpl<Tag>& lhs, const NumericEffectImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const NumericEffectImpl<Tag>& self) { return std::hash<NumericEffect<Tag>> {}(&self); })
            .def("get_index", &NumericEffectImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_assign_operator", &NumericEffectImpl<Tag>::get_assign_operator, nb::rv_policy::copy)
            .def("get_function", &NumericEffectImpl<Tag>::get_function, nb::rv_policy::reference_internal)
            .def("get_function_expression", &NumericEffectImpl<Tag>::get_function_expression, nb::rv_policy::reference_internal);
    };

    bind_numeric_effect("FluentNumericEffect", FluentTag {});
    bind_numeric_effect("AuxiliaryNumericEffect", AuxiliaryTag {});

    /* NumericConstraint */
    nb::class_<NumericConstraintImpl>(m, "NumericConstraint")  //
        .def("__str__", [](const NumericConstraintImpl& self) { return to_string(self); })
        .def("__eq__", [](const NumericConstraintImpl& lhs, const NumericConstraintImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const NumericConstraintImpl& lhs, const NumericConstraintImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const NumericConstraintImpl& self) { return std::hash<NumericConstraint> {}(&self); })
        .def("get_index", &NumericConstraintImpl::get_index, nb::rv_policy::copy)
        .def("get_binary_comparator", &NumericConstraintImpl::get_binary_comparator, nb::rv_policy::copy)
        .def("get_left_function_expression", &NumericConstraintImpl::get_left_function_expression, nb::rv_policy::reference_internal)
        .def("get_right_function_expression", &NumericConstraintImpl::get_right_function_expression, nb::rv_policy::reference_internal);
    nb::bind_vector<NumericConstraintList>(m, "NumericConstraintList");

    /* ConjunctiveCondition */
    nb::class_<ConjunctiveConditionImpl>(m, "ConjunctiveCondition")  //
        .def("__str__", [](const ConjunctiveConditionImpl& self) { return to_string(self); })
        .def("__eq__", [](const ConjunctiveConditionImpl& lhs, const ConjunctiveConditionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ConjunctiveConditionImpl& lhs, const ConjunctiveConditionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ConjunctiveConditionImpl& self) { return std::hash<ConjunctiveCondition> {}(&self); })
        .def("get_index", &ConjunctiveConditionImpl::get_index, nb::rv_policy::copy)
        .def("get_parameters", &ConjunctiveConditionImpl::get_parameters, nb::rv_policy::copy)
        .def("get_static_literals", &ConjunctiveConditionImpl::get_literals<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_literals", &ConjunctiveConditionImpl::get_literals<FluentTag>, nb::rv_policy::copy)
        .def("get_derived_literals", &ConjunctiveConditionImpl::get_literals<DerivedTag>, nb::rv_policy::copy)
        .def("get_nullary_ground_static_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<StaticTag>, nb::rv_policy::copy)
        .def("get_nullary_ground_fluent_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<FluentTag>, nb::rv_policy::copy)
        .def("get_nullary_ground_derived_literals", &ConjunctiveConditionImpl::get_nullary_ground_literals<DerivedTag>, nb::rv_policy::copy)
        .def("get_numeric_constraints", &ConjunctiveConditionImpl::get_numeric_constraints, nb::rv_policy::copy);

    /* ConjunctiveEffectImpl */
    nb::class_<ConjunctiveEffectImpl>(m, "ConjunctiveEffect")  //
        .def("__str__", [](const ConjunctiveEffectImpl& self) { return to_string(self); })
        .def("__eq__", [](const ConjunctiveEffectImpl& lhs, const ConjunctiveEffectImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ConjunctiveEffectImpl& lhs, const ConjunctiveEffectImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ConjunctiveEffectImpl& self) { return std::hash<ConjunctiveEffect> {}(&self); })
        .def("get_index", &ConjunctiveEffectImpl::get_index, nb::rv_policy::copy)
        .def("get_parameters", &ConjunctiveEffectImpl::get_parameters, nb::rv_policy::copy)
        .def("get_literals", &ConjunctiveEffectImpl::get_literals, nb::rv_policy::copy)
        .def("get_fluent_numeric_effects", &ConjunctiveEffectImpl::get_fluent_numeric_effects, nb::rv_policy::copy)
        .def("get_auxiliary_numeric_effect", &ConjunctiveEffectImpl::get_auxiliary_numeric_effect, nb::rv_policy::copy);

    /* ConditionalEffect */
    nb::class_<ConditionalEffectImpl>(m, "ConditionalEffect")  //
        .def("__str__", [](const ConditionalEffectImpl& self) { return to_string(self); })
        .def("__eq__", [](const ConditionalEffectImpl& lhs, const ConditionalEffectImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ConditionalEffectImpl& lhs, const ConditionalEffectImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ConditionalEffectImpl& self) { return std::hash<ConditionalEffect> {}(&self); })
        .def("get_index", &ConditionalEffectImpl::get_index, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &ConditionalEffectImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_conjunctive_effect", &ConditionalEffectImpl::get_conjunctive_effect, nb::rv_policy::reference_internal);
    nb::bind_vector<ConditionalEffectList>(m, "ConditionalEffectList");

    /* Action */
    nb::class_<ActionImpl>(m, "Action")  //
        .def("__str__", [](const ActionImpl& self) { return to_string(self); })
        .def("__eq__", [](const ActionImpl& lhs, const ActionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const ActionImpl& lhs, const ActionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const ActionImpl& self) { return std::hash<Action> {}(&self); })
        .def("get_index", &ActionImpl::get_index, nb::rv_policy::copy)
        .def("get_name", &ActionImpl::get_name, nb::rv_policy::copy)
        .def("get_parameters", &ActionImpl::get_parameters, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &ActionImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_conjunctive_effect", &ActionImpl::get_conjunctive_effect, nb::rv_policy::reference_internal)
        .def("get_conditional_effects", &ActionImpl::get_conditional_effects, nb::rv_policy::copy)
        .def("get_arity", &ActionImpl::get_arity, nb::rv_policy::copy);
    nb::bind_vector<ActionList>(m, "ActionList");

    /* Axiom */
    nb::class_<AxiomImpl>(m, "Axiom")  //
        .def("__str__", [](const AxiomImpl& self) { return to_string(self); })
        .def("__eq__", [](const AxiomImpl& lhs, const AxiomImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const AxiomImpl& lhs, const AxiomImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const AxiomImpl& self) { return std::hash<Axiom> {}(&self); })
        .def("get_index", &AxiomImpl::get_index, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &AxiomImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_literal", &AxiomImpl::get_literal, nb::rv_policy::reference_internal)
        .def("get_arity", &AxiomImpl::get_arity, nb::rv_policy::copy);
    nb::bind_vector<AxiomList>(m, "AxiomList");

    /* GroundFunctionExpression */
    nb::class_<GroundFunctionExpressionImpl>(m, "GroundFunctionExpression")  //
        .def("__str__", [](const GroundFunctionExpressionImpl& self) { return to_string(self); })
        .def("__eq__", [](const GroundFunctionExpressionImpl& lhs, const GroundFunctionExpressionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundFunctionExpressionImpl& lhs, const GroundFunctionExpressionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundFunctionExpressionImpl& self) { return std::hash<GroundFunctionExpression> {}(&self); })
        .def("get",
             [](const GroundFunctionExpressionImpl& fexpr) -> nb::object { return std::visit([](auto&& arg) { return nb::cast(arg); }, fexpr.get_variant()); });
    nb::bind_vector<GroundFunctionExpressionList>(m, "GroundFunctionExpressionList");

    nb::class_<GroundFunctionExpressionNumberImpl>(m, "GroundFunctionExpressionNumber")  //
        .def("__str__", [](const GroundFunctionExpressionNumberImpl& self) { return to_string(self); })
        .def("__eq__", [](const GroundFunctionExpressionNumberImpl& lhs, const GroundFunctionExpressionNumberImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundFunctionExpressionNumberImpl& lhs, const GroundFunctionExpressionNumberImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundFunctionExpressionNumberImpl& self) { return std::hash<GroundFunctionExpressionNumber> {}(&self); })
        .def("get_index", &GroundFunctionExpressionNumberImpl::get_index, nb::rv_policy::copy)
        .def("get_number", &GroundFunctionExpressionNumberImpl::get_number, nb::rv_policy::copy);

    nb::class_<GroundFunctionExpressionBinaryOperatorImpl>(m, "GroundFunctionExpressionBinaryOperator")  //
        .def("__str__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return to_string(self); })
        .def("__eq__",
             [](const GroundFunctionExpressionBinaryOperatorImpl& lhs, const GroundFunctionExpressionBinaryOperatorImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__",
             [](const GroundFunctionExpressionBinaryOperatorImpl& lhs, const GroundFunctionExpressionBinaryOperatorImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundFunctionExpressionBinaryOperatorImpl& self) { return std::hash<GroundFunctionExpressionBinaryOperator> {}(&self); })
        .def("get_index", &GroundFunctionExpressionBinaryOperatorImpl::get_index, nb::rv_policy::copy)
        .def("get_binary_operator", &GroundFunctionExpressionBinaryOperatorImpl::get_binary_operator, nb::rv_policy::copy)
        .def("get_left_function_expression", &GroundFunctionExpressionBinaryOperatorImpl::get_left_function_expression, nb::rv_policy::reference_internal)
        .def("get_right_function_expression", &GroundFunctionExpressionBinaryOperatorImpl::get_right_function_expression, nb::rv_policy::reference_internal);

    nb::class_<GroundFunctionExpressionMultiOperatorImpl>(m, "GroundFunctionExpressionMultiOperator")  //
        .def("__str__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return to_string(self); })
        .def("__eq__", [](const GroundFunctionExpressionMultiOperatorImpl& lhs, const GroundFunctionExpressionMultiOperatorImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundFunctionExpressionMultiOperatorImpl& lhs, const GroundFunctionExpressionMultiOperatorImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundFunctionExpressionMultiOperatorImpl& self) { return std::hash<GroundFunctionExpressionMultiOperator> {}(&self); })
        .def("get_index", &GroundFunctionExpressionMultiOperatorImpl::get_index, nb::rv_policy::copy)
        .def("get_multi_operator", &GroundFunctionExpressionMultiOperatorImpl::get_multi_operator, nb::rv_policy::copy)
        .def("get_function_expressions", &GroundFunctionExpressionMultiOperatorImpl::get_function_expressions, nb::rv_policy::copy);

    nb::class_<GroundFunctionExpressionMinusImpl>(m, "GroundFunctionExpressionMinus")  //
        .def("__str__", [](const GroundFunctionExpressionMinusImpl& self) { return to_string(self); })
        .def("__eq__", [](const GroundFunctionExpressionMinusImpl& lhs, const GroundFunctionExpressionMinusImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundFunctionExpressionMinusImpl& lhs, const GroundFunctionExpressionMinusImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundFunctionExpressionMinusImpl& self) { return std::hash<GroundFunctionExpressionMinus> {}(&self); })
        .def("get_index", &GroundFunctionExpressionMinusImpl::get_index, nb::rv_policy::copy)
        .def("get_function_expression", &GroundFunctionExpressionMinusImpl::get_function_expression, nb::rv_policy::reference_internal);

    auto bind_ground_function_expression_function = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundFunctionExpressionFunctionImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundFunctionExpressionFunctionImpl<Tag>& self) { return to_string(self); })
            .def("__eq__",
                 [](const GroundFunctionExpressionFunctionImpl<Tag>& lhs, const GroundFunctionExpressionFunctionImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__",
                 [](const GroundFunctionExpressionFunctionImpl<Tag>& lhs, const GroundFunctionExpressionFunctionImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const GroundFunctionExpressionFunctionImpl<Tag>& self) { return std::hash<GroundFunctionExpressionFunction<Tag>> {}(&self); })
            .def("get_index", &GroundFunctionExpressionFunctionImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_function", &GroundFunctionExpressionFunctionImpl<Tag>::get_function, nb::rv_policy::reference_internal);
    };
    bind_ground_function_expression_function("StaticGroundFunctionExpressionFunction", StaticTag {});
    bind_ground_function_expression_function("FluentGroundFunctionExpressionFunction", FluentTag {});
    bind_ground_function_expression_function("AuxiliaryGroundFunctionExpressionFunction", AuxiliaryTag {});

    /* OptimizationMetric */
    nb::class_<OptimizationMetricImpl>(m, "OptimizationMetric")  //
        .def("__str__", [](const OptimizationMetricImpl& self) { return to_string(self); })
        .def("__eq__", [](const OptimizationMetricImpl& lhs, const OptimizationMetricImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const OptimizationMetricImpl& lhs, const OptimizationMetricImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const OptimizationMetricImpl& self) { return std::hash<OptimizationMetric> {}(&self); })
        .def("get_index", &OptimizationMetricImpl::get_index, nb::rv_policy::copy)
        .def("get_function_expression", &OptimizationMetricImpl::get_function_expression, nb::rv_policy::reference_internal)
        .def("get_optimization_metric", &OptimizationMetricImpl::get_optimization_metric, nb::rv_policy::reference_internal);

    auto bind_ground_numeric_effect = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<GroundNumericEffectImpl<Tag>>(m, class_name.c_str())
            .def("__str__", [](const GroundNumericEffectImpl<Tag>& self) { return to_string(self); })
            .def("__eq__", [](const GroundNumericEffectImpl<Tag>& lhs, const GroundNumericEffectImpl<Tag>& rhs) { return &lhs == &rhs; })
            .def("__ne__", [](const GroundNumericEffectImpl<Tag>& lhs, const GroundNumericEffectImpl<Tag>& rhs) { return &lhs != &rhs; })
            .def("__hash__", [](const GroundNumericEffectImpl<Tag>& self) { return std::hash<GroundNumericEffect<Tag>> {}(&self); })
            .def("get_index", &GroundNumericEffectImpl<Tag>::get_index, nb::rv_policy::copy)
            .def("get_assign_operator", &GroundNumericEffectImpl<Tag>::get_assign_operator, nb::rv_policy::copy)
            .def("get_function", &GroundNumericEffectImpl<Tag>::get_function, nb::rv_policy::reference_internal)
            .def("get_function_expression", &GroundNumericEffectImpl<Tag>::get_function_expression, nb::rv_policy::reference_internal);
    };
    bind_ground_numeric_effect("FluentGroundNumericEffect", FluentTag {});
    bind_ground_numeric_effect("AuxiliaryGroundNumericEffect", AuxiliaryTag {});

    /* GroundNumericConstraint */
    nb::class_<GroundNumericConstraintImpl>(m, "GroundNumericConstraint")  //
        .def("__str__", [](const GroundNumericConstraintImpl& self) { return to_string(self); })
        .def("__eq__", [](const GroundNumericConstraintImpl& lhs, const GroundNumericConstraintImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundNumericConstraintImpl& lhs, const GroundNumericConstraintImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundNumericConstraintImpl& self) { return std::hash<GroundNumericConstraint> {}(&self); })
        .def("get_index", &GroundNumericConstraintImpl::get_index, nb::rv_policy::copy)
        .def("get_binary_comparator", &GroundNumericConstraintImpl::get_binary_comparator, nb::rv_policy::copy)
        .def("get_left_function_expression", &GroundNumericConstraintImpl::get_left_function_expression, nb::rv_policy::reference_internal)
        .def("get_right_function_expression", &GroundNumericConstraintImpl::get_right_function_expression, nb::rv_policy::reference_internal);
    nb::bind_vector<GroundNumericConstraintList>(m, "GroundNumericConstraintList");

    /* GroundConjunctiveCondition */
    nb::class_<GroundConjunctiveConditionImpl>(m, "GroundConjunctiveCondition")
        .def("to_string",
             [](const GroundConjunctiveConditionImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundConjunctiveCondition(&self), std::cref(problem)));
                 return ss.str();
             })
        .def("__eq__", [](const GroundConjunctiveConditionImpl& lhs, const GroundConjunctiveConditionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundConjunctiveConditionImpl& lhs, const GroundConjunctiveConditionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundConjunctiveConditionImpl& self) { return std::hash<GroundConjunctiveCondition> {}(&self); })
        .def("get_index", &GroundConjunctiveConditionImpl::get_index, nb::rv_policy::copy)
        .def(
            "get_static_positive_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over positive static atom indices.",
                                         self.get_positive_precondition<StaticTag>().begin(),
                                         self.get_positive_precondition<StaticTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_fluent_positive_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over positive fluent atom indices.",
                                         self.get_positive_precondition<FluentTag>().begin(),
                                         self.get_positive_precondition<FluentTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_derived_positive_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over positive derived atom indices.",
                                         self.get_positive_precondition<DerivedTag>().begin(),
                                         self.get_positive_precondition<DerivedTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_static_negative_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over negative static atom indices.",
                                         self.get_negative_precondition<StaticTag>().begin(),
                                         self.get_negative_precondition<StaticTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_fluent_negative_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over negative fluent atom indices.",
                                         self.get_negative_precondition<FluentTag>().begin(),
                                         self.get_negative_precondition<FluentTag>().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_derived_negative_condition",
            [](const GroundConjunctiveConditionImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over negative derived atom indices.",
                                         self.get_negative_precondition<DerivedTag>().begin(),
                                         self.get_negative_precondition<DerivedTag>().end());
            },
            nb::keep_alive<0, 1>());

    /* GroundConjunctiveEffect */
    nb::class_<GroundConjunctiveEffectImpl>(m, "GroundConjunctiveEffect")
        .def("to_string",
             [](const GroundConjunctiveEffectImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundConjunctiveEffect(&self), std::cref(problem)));
                 return ss.str();
             })
        .def("__eq__", [](const GroundConjunctiveEffectImpl& lhs, const GroundConjunctiveEffectImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundConjunctiveEffectImpl& lhs, const GroundConjunctiveEffectImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundConjunctiveEffectImpl& self) { return std::hash<GroundConjunctiveEffect> {}(&self); })
        .def("get_index", &GroundConjunctiveEffectImpl::get_index, nb::rv_policy::copy)
        .def(
            "get_positive_effects",
            [](const GroundConjunctiveEffectImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over positive fluent atom indices.",
                                         self.get_positive_effects().begin(),
                                         self.get_positive_effects().end());
            },
            nb::keep_alive<0, 1>())
        .def(
            "get_negative_effects",
            [](const GroundConjunctiveEffectImpl& self)
            {
                return nb::make_iterator(nb::type<nb::iterator>(),
                                         "Iterator over negative fluent atom indices.",
                                         self.get_negative_effects().begin(),
                                         self.get_negative_effects().end());
            },
            nb::keep_alive<0, 1>())
        .def("get_fluent_numeric_effects", nb::overload_cast<>(&GroundConjunctiveEffectImpl::get_fluent_numeric_effects, nb::const_), nb::rv_policy::copy)
        .def("get_auxiliary_numeric_effect", nb::overload_cast<>(&GroundConjunctiveEffectImpl::get_auxiliary_numeric_effect, nb::const_), nb::rv_policy::copy);

    /* GroundConditionalEffect */
    nb::class_<GroundConditionalEffectImpl>(m, "GroundConditionalEffect")
        .def("to_string",
             [](const GroundConditionalEffectImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundConditionalEffect(&self), std::cref(problem)));
                 return ss.str();
             })
        .def("__eq__", [](const GroundConditionalEffectImpl& lhs, const GroundConditionalEffectImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundConditionalEffectImpl& lhs, const GroundConditionalEffectImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundConditionalEffectImpl& self) { return std::hash<GroundConditionalEffect> {}(&self); })
        .def("get_index", &GroundConditionalEffectImpl::get_index, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &GroundConditionalEffectImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_conjunctive_effect", &GroundConditionalEffectImpl::get_conjunctive_effect, nb::rv_policy::reference_internal);
    nb::bind_vector<GroundConditionalEffectList>(m, "GroundConditionalEffectList");

    /* GroundAction */
    nb::class_<GroundActionImpl>(m, "GroundAction")  //
        .def("to_string",
             [](const GroundActionImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundAction(&self), std::cref(problem), GroundActionImpl::FullFormatterTag {}));
                 return ss.str();
             })
        .def("__eq__", [](const GroundActionImpl& lhs, const GroundActionImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundActionImpl& lhs, const GroundActionImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundActionImpl& self) { return &self; })
        .def("to_string_for_plan",
             [](const GroundActionImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundAction(&self), std::cref(problem), GroundActionImpl::PlanFormatterTag {}));
                 return ss.str();
             })
        .def("get_index", &GroundActionImpl::get_index, nb::rv_policy::copy)
        .def("get_action", &GroundActionImpl::get_action, nb::rv_policy::reference_internal)
        .def("get_objects", &GroundActionImpl::get_objects, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &GroundActionImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_conjunctive_effect", &GroundActionImpl::get_conjunctive_effect, nb::rv_policy::reference_internal)
        .def("get_conditional_effects", &GroundActionImpl::get_conditional_effects, nb::rv_policy::reference_internal);
    nb::bind_vector<GroundActionList>(m, "GroundActionList");

    /* GroundAxiom */
    nb::class_<GroundAxiomImpl>(m, "GroundAxiom")  //
        .def("to_string",
             [](const GroundAxiomImpl& self, const ProblemImpl& problem)
             {
                 std::stringstream ss;
                 mimir::operator<<(ss, std::make_tuple(GroundAxiom(&self), std::cref(problem)));
                 return ss.str();
             })
        .def("__eq__", [](const GroundAxiomImpl& lhs, const GroundAxiomImpl& rhs) { return &lhs == &rhs; })
        .def("__ne__", [](const GroundAxiomImpl& lhs, const GroundAxiomImpl& rhs) { return &lhs != &rhs; })
        .def("__hash__", [](const GroundAxiomImpl& self) { return std::hash<GroundAxiom> {}(&self); })
        .def("get_index", &GroundAxiomImpl::get_index, nb::rv_policy::copy)
        .def("get_axiom", &GroundAxiomImpl::get_axiom, nb::rv_policy::reference_internal)
        .def("get_objects", &GroundAxiomImpl::get_objects, nb::rv_policy::copy)
        .def("get_conjunctive_condition", &GroundAxiomImpl::get_conjunctive_condition, nb::rv_policy::reference_internal)
        .def("get_literal", &GroundAxiomImpl::get_literal, nb::rv_policy::reference_internal);
    nb::bind_vector<GroundAxiomList>(m, "GroundAxiomList");

    /* Repositories*/
    // ATTENTION: cannot provide modifiers since Problem and Domain returns a reference
    nb::class_<Repositories>(m, "Repositories")  //
        .def(
            "get_static_ground_atoms",
            [](const Repositories& self)
            { return GroundAtomList<StaticTag>(self.get_ground_atoms<StaticTag>().begin(), self.get_ground_atoms<StaticTag>().end()); },
            nb::rv_policy::copy)
        .def("get_static_ground_atom", &Repositories::get_ground_atom<StaticTag>, nb::rv_policy::reference_internal)
        .def(
            "get_fluent_ground_atoms",
            [](const Repositories& self)
            { return GroundAtomList<FluentTag>(self.get_ground_atoms<FluentTag>().begin(), self.get_ground_atoms<FluentTag>().end()); },
            nb::rv_policy::copy)
        .def("get_fluent_ground_atom", &Repositories::get_ground_atom<FluentTag>, nb::rv_policy::reference_internal)
        .def(
            "get_derived_ground_atoms",
            [](const Repositories& self)
            { return GroundAtomList<DerivedTag>(self.get_ground_atoms<DerivedTag>().begin(), self.get_ground_atoms<DerivedTag>().end()); },
            nb::rv_policy::copy)
        .def("get_derived_ground_atom", &Repositories::get_ground_atom<DerivedTag>, nb::rv_policy::reference_internal)
        .def("get_static_ground_atoms_from_indices",
             nb::overload_cast<const std::vector<size_t>&>(&Repositories::get_ground_atoms_from_indices<StaticTag, std::vector<size_t>>, nb::const_),
             nb::rv_policy::copy)
        .def("get_fluent_ground_atoms_from_indices",
             nb::overload_cast<const std::vector<size_t>&>(&Repositories::get_ground_atoms_from_indices<FluentTag, std::vector<size_t>>, nb::const_),
             nb::rv_policy::copy)
        .def("get_derived_ground_atoms_from_indices",
             nb::overload_cast<const std::vector<size_t>&>(&Repositories::get_ground_atoms_from_indices<DerivedTag, std::vector<size_t>>, nb::const_),
             nb::rv_policy::copy)
        .def("get_object", &Repositories::get_object, nb::rv_policy::reference_internal);

    /* AssignmentSets */
    auto bind_assignment_set = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<AssignmentSet<Tag>>(m, class_name.c_str())
            .def(nb::init<size_t, const PredicateList<Tag>&>(), "num_objects"_a, "predicates"_a)
            .def("reset", &AssignmentSet<Tag>::reset)
            .def("insert_ground_atoms", &AssignmentSet<Tag>::insert_ground_atoms, "ground_atoms"_a)
            .def("insert_ground_atom", &AssignmentSet<Tag>::insert_ground_atoms, "ground_atom"_a);
    };
    bind_assignment_set("StaticAssignmentSet", StaticTag {});
    bind_assignment_set("FluentAssignmentSet", FluentTag {});
    bind_assignment_set("DerivedAssignmentSet", DerivedTag {});

    /* NumericAssignmentSets */
    auto bind_numeric_assignment_set = [&]<typename Tag>(const std::string& class_name, Tag)
    {
        nb::class_<NumericAssignmentSet<Tag>>(m, class_name.c_str())
            .def(nb::init<>())
            .def(nb::init<size_t, const FunctionSkeletonList<Tag>&>(), "num_objects"_a, "function_skeletons"_a)
            .def("reset", &NumericAssignmentSet<Tag>::reset)
            .def("insert_ground_function_values", &NumericAssignmentSet<Tag>::insert_ground_function_values, "ground_functions"_a, "numeric_values"_a);
    };

    bind_numeric_assignment_set("StaticNumericAssignmentSet", StaticTag {});
    bind_numeric_assignment_set("FluentNumericAssignmentSet", FluentTag {});

    /* Domain */
    nb::class_<DomainImpl>(m, "Domain")  //
        .def("__str__", [](const DomainImpl& self) { return to_string(self); })
        .def("get_repositories", &DomainImpl::get_repositories, nb::rv_policy::reference_internal)
        .def("get_filepath",
             [](const DomainImpl& self)
             { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; })
        .def("get_name", &DomainImpl::get_name, nb::rv_policy::copy)
        .def("get_constants", &DomainImpl::get_constants, nb::rv_policy::copy)
        .def("get_static_predicates", &DomainImpl::get_predicates<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_predicates", &DomainImpl::get_predicates<FluentTag>, nb::rv_policy::copy)
        .def("get_derived_predicates", &DomainImpl::get_predicates<DerivedTag>, nb::rv_policy::copy)
        .def("get_static_functions", &DomainImpl::get_function_skeletons<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_functions", &DomainImpl::get_function_skeletons<FluentTag>, nb::rv_policy::copy)
        .def("get_auxiliary_function", &DomainImpl::get_auxiliary_function_skeleton, nb::rv_policy::copy)
        .def("get_actions", &DomainImpl::get_actions, nb::rv_policy::copy)
        .def("get_requirements", &DomainImpl::get_requirements, nb::rv_policy::reference_internal)
        .def("get_name_to_static_predicate", &DomainImpl::get_name_to_predicate<StaticTag>, nb::rv_policy::copy)
        .def("get_name_to_fluent_predicate", &DomainImpl::get_name_to_predicate<FluentTag>, nb::rv_policy::copy)
        .def("get_name_to_derived_predicate", &DomainImpl::get_name_to_predicate<DerivedTag>, nb::rv_policy::copy);

    /* Problem */
    nb::class_<ProblemImpl>(m, "Problem")  //
        .def_static(
            "create",
            [](const std::string& domain_filepath, const std::string& problem_filepath, const loki::Options& options)
            { return ProblemImpl::create(domain_filepath, problem_filepath, options); },
            "domain_filepath"_a,
            "problem_filepath"_a,
            "options"_a = loki::Options())
        .def("__str__", [](const ProblemImpl& self) { return to_string(self); })
        .def("get_index", &ProblemImpl::get_index, nb::rv_policy::copy)
        .def("get_repositories", &ProblemImpl::get_repositories, nb::rv_policy::reference_internal)
        .def("get_filepath",
             [](const ProblemImpl& self)
             { return (self.get_filepath().has_value()) ? std::optional<std::string>(self.get_filepath()->string()) : std::nullopt; })
        .def("get_name", &ProblemImpl::get_name, nb::rv_policy::copy)
        .def("get_domain", &ProblemImpl::get_domain, nb::rv_policy::copy)
        .def("get_requirements", &ProblemImpl::get_requirements, nb::rv_policy::reference_internal)
        .def("get_objects", &ProblemImpl::get_objects, nb::rv_policy::copy)
        .def("get_problem_and_domain_objects", &ProblemImpl::get_problem_and_domain_objects, nb::rv_policy::copy)
        .def("get_problem_and_domain_derived_predicates", &ProblemImpl::get_problem_and_domain_derived_predicates, nb::rv_policy::copy)
        .def("get_static_initial_literals", &ProblemImpl::get_initial_literals<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_initial_literals", &ProblemImpl::get_initial_literals<FluentTag>, nb::rv_policy::copy)
        .def("get_static_function_values", &ProblemImpl::get_initial_function_values<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_function_values", &ProblemImpl::get_initial_function_values<FluentTag>, nb::rv_policy::copy)
        .def("get_auxiliary_function_value", &ProblemImpl::get_auxiliary_function_value, nb::rv_policy::copy)
        .def("get_optimization_metric", &ProblemImpl::get_optimization_metric, nb::rv_policy::copy)
        .def("get_static_goal_condition", &ProblemImpl::get_goal_condition<StaticTag>, nb::rv_policy::copy)
        .def("get_fluent_goal_condition", &ProblemImpl::get_goal_condition<FluentTag>, nb::rv_policy::copy)
        .def("get_derived_goal_condition", &ProblemImpl::get_goal_condition<DerivedTag>, nb::rv_policy::copy)
        .def("get_static_initial_atoms", &ProblemImpl::get_static_initial_atoms, nb::rv_policy::copy)
        .def("get_fluent_initial_atoms", &ProblemImpl::get_fluent_initial_atoms, nb::rv_policy::copy)
        .def("get_static_assignment_set", &ProblemImpl::get_static_assignment_set, nb::rv_policy::copy)
        .def(
            "ground",
            [](ProblemImpl& self, Action lifted, const ObjectList& binding) -> GroundAction { return self.ground(lifted, binding); },
            "action"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, FunctionExpression lifted, const ObjectList& binding) -> GroundFunctionExpression { return self.ground(lifted, binding); },
            "function_expression"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, NumericConstraint lifted, const ObjectList& binding) -> GroundNumericConstraint { return self.ground(lifted, binding); },
            "numeric_constraint"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def("get_or_create_ground_atom", &ProblemImpl::get_or_create_ground_atom<StaticTag>, "predicate"_a, "binding"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_ground_atom", &ProblemImpl::get_or_create_ground_atom<FluentTag>, "predicate"_a, "binding"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_ground_atom", &ProblemImpl::get_or_create_ground_atom<DerivedTag>, "predicate"_a, "binding"_a, nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Literal<StaticTag> lifted, const ObjectList& binding) -> GroundLiteral<StaticTag> { return self.ground(lifted, binding); },
            "literal"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Literal<FluentTag> lifted, const ObjectList& binding) -> GroundLiteral<FluentTag> { return self.ground(lifted, binding); },
            "literal"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Literal<DerivedTag> lifted, const ObjectList& binding) -> GroundLiteral<DerivedTag> { return self.ground(lifted, binding); },
            "literal"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, NumericEffect<FluentTag> lifted, const ObjectList& binding) -> GroundNumericEffect<FluentTag>
            { return self.ground(lifted, binding); },
            "numeric_effect"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, NumericEffect<AuxiliaryTag> lifted, const ObjectList& binding) -> GroundNumericEffect<AuxiliaryTag>
            { return self.ground(lifted, binding); },
            "numeric_effect"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Function<StaticTag> lifted, const ObjectList& binding) -> GroundFunction<StaticTag> { return self.ground(lifted, binding); },
            "function"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Function<FluentTag> lifted, const ObjectList& binding) -> GroundFunction<FluentTag> { return self.ground(lifted, binding); },
            "function"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def(
            "ground",
            [](ProblemImpl& self, Function<AuxiliaryTag> lifted, const ObjectList& binding) -> GroundFunction<AuxiliaryTag>
            { return self.ground(lifted, binding); },
            "function"_a,
            "binding"_a,
            nb::rv_policy::reference_internal)
        .def("get_or_create_variable", &ProblemImpl::get_or_create_variable, "name"_a, "parameter_index"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_term", nb::overload_cast<Variable>(&ProblemImpl::get_or_create_term), "variable"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_term", nb::overload_cast<Object>(&ProblemImpl::get_or_create_term), "object"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_atom", &ProblemImpl::get_or_create_atom<StaticTag>, "predicate"_a, "terms"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_atom", &ProblemImpl::get_or_create_atom<FluentTag>, "predicate"_a, "terms"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_atom", &ProblemImpl::get_or_create_atom<DerivedTag>, "predicate"_a, "terms"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_literal", &ProblemImpl::get_or_create_literal<StaticTag>, "polarity"_a, "atom"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_literal", &ProblemImpl::get_or_create_literal<FluentTag>, "polarity"_a, "atom"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_literal", &ProblemImpl::get_or_create_literal<DerivedTag>, "polarity"_a, "atom"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_function",
             &ProblemImpl::get_or_create_function<StaticTag>,
             "function_skeleton"_a,
             "terms"_a,
             "mapping"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function",
             &ProblemImpl::get_or_create_function<FluentTag>,
             "function_skeleton"_a,
             "terms"_a,
             "mapping"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function",
             &ProblemImpl::get_or_create_function<AuxiliaryTag>,
             "function_skeleton"_a,
             "terms"_a,
             "mapping"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_number", &ProblemImpl::get_or_create_function_expression_number, "number"_a, nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_binary_operator",
             &ProblemImpl::get_or_create_function_expression_binary_operator,
             "binary_operator"_a,
             "left"_a,
             "right"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_multi_operator",
             &ProblemImpl::get_or_create_function_expression_multi_operator,
             "multi_operator"_a,
             "function_expressions"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_minus",
             &ProblemImpl::get_or_create_function_expression_minus,
             "function_expression"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_function",
             &ProblemImpl::get_or_create_function_expression_function<StaticTag>,
             "function"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression_function",
             &ProblemImpl::get_or_create_function_expression_function<FluentTag>,
             "function"_a,
             nb::rv_policy::reference_internal)
        .def(
            "get_or_create_function_expression",
            [](ProblemImpl& self, FunctionExpressionNumber fexpr) { return self.get_or_create_function_expression(fexpr); },
            "function_expression"_a,
            nb::rv_policy::reference_internal)
        .def(
            "get_or_create_function_expression",
            [](ProblemImpl& self, FunctionExpressionBinaryOperator fexpr) { return self.get_or_create_function_expression(fexpr); },
            "function_expression"_a,
            nb::rv_policy::reference_internal)
        .def(
            "get_or_create_function_expression",
            [](ProblemImpl& self, FunctionExpressionMultiOperator fexpr) { return self.get_or_create_function_expression(fexpr); },
            "function_expression"_a,
            nb::rv_policy::reference_internal)
        .def(
            "get_or_create_function_expression",
            [](ProblemImpl& self, FunctionExpressionMinus fexpr) { return self.get_or_create_function_expression(fexpr); },
            "function_expression"_a,
            nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression",
             nb::overload_cast<FunctionExpressionFunction<StaticTag>>(&ProblemImpl::get_or_create_function_expression<StaticTag>),
             "function_expression"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_function_expression",
             nb::overload_cast<FunctionExpressionFunction<FluentTag>>(&ProblemImpl::get_or_create_function_expression<FluentTag>),
             "function_expression"_a,
             nb::rv_policy::reference_internal)
        .def("get_or_create_numeric_constraint",
             &ProblemImpl::get_or_create_numeric_constraint,
             "comparator"_a,
             "left_expression"_a,
             "right_expression"_a,
             "terms"_a,
             nb::rv_policy::reference_internal)
        .def(
            "get_or_create_conjunctive_condition",
            [](ProblemImpl& self,
               VariableList parameters,
               LiteralList<StaticTag> static_literals,
               LiteralList<FluentTag> fluent_literals,
               LiteralList<DerivedTag> derived_literals,
               NumericConstraintList numeric_constraints)
            {
                return self.get_or_create_conjunctive_condition(
                    std::move(parameters),
                    boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<StaticTag>, static_literals),
                                          boost::hana::make_pair(boost::hana::type_c<FluentTag>, fluent_literals),
                                          boost::hana::make_pair(boost::hana::type_c<DerivedTag>, derived_literals)),
                    std::move(numeric_constraints));
            },
            "parameters"_a,
            "static_literals"_a,
            "fluent_literals"_a,
            "derived_literals"_a,
            "numeric_constraints"_a,
            nb::rv_policy::reference_internal);
    nb::bind_vector<ProblemList>(m, "ProblemList");

    /* GeneralizedProblem */
    nb::class_<GeneralizedProblemImpl>(m, "GeneralizedProblem")
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const std::vector<fs::path>& problem_filepaths, const loki::Options& options)
            { return GeneralizedProblemImpl::create(domain_filepath, problem_filepaths, options); },
            "domain_filepath"_a,
            "problem_filepaths"_a,
            "options"_a = loki::Options())
        .def_static(
            "create",
            [](const fs::path& domain_filepath, const fs::path& problems_directory, const loki::Options& options)
            { return GeneralizedProblemImpl::create(domain_filepath, problems_directory, options); },
            "domain_filepath"_a,
            "problems_directory"_a,
            "options"_a = loki::Options())

        .def_static(
            "create",
            [](Domain domain, ProblemList problems) { return GeneralizedProblemImpl::create(std::move(domain), std::move(problems)); },
            "domain"_a,
            "problems"_a)
        .def("get_domain", &GeneralizedProblemImpl::get_domain)
        .def("get_problems", &GeneralizedProblemImpl::get_problems);

    /**
     * Parser
     */

    nb::class_<Parser>(m, "Parser")
        .def(
            "__init__",
            [](Parser* self, std::string domain_filepath, loki::Options options)
            { new (self) Parser(std::filesystem::path(std::move(domain_filepath)), std::move(options)); },
            "domain_filepath"_a,
            "options"_a = loki::Options())
        .def(
            "parse_problem",
            [](Parser& self, const std::string& problem_filepath, const loki::Options& options) { return self.parse_problem(problem_filepath, options); },
            "problem_filepath"_a,
            "options"_a = loki::Options())
        .def("get_domain", &Parser::get_domain);

    /**
     * Translator
     */

    nb::class_<Translator>(m, "Translator")
        .def(nb::init<const Domain&>(), "domain"_a)
        .def("translate", &Translator::translate, "problem"_a)
        .def("get_original_domain", &Translator::get_original_domain, nb::rv_policy::reference_internal)
        .def("get_translated_domain", &Translator::get_translated_domain, nb::rv_policy::reference_internal);
}

}