#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_

#include "mimir/formalism/translators/interface.hpp"

#include <loki/pddl/visitors.hpp>
#include <unordered_set>

namespace mimir
{

/**
 * Base implementation.
 */
template<typename Derived>
class BaseTranslator : public ITranslator<BaseTranslator<Derived>>
{
private:
    BaseTranslator() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    struct PrepareVisitor
    {
        BaseTranslator& m_translator;

        explicit PrepareVisitor(BaseTranslator& translator) : m_translator(translator) {}

        template<typename T>
        void operator()(const T& element) const
        {
            m_translator.prepare(element);
        }
    };

    struct TranslateVisitor
    {
        BaseTranslator& m_translator;

        explicit TranslateVisitor(BaseTranslator& translator) : m_translator(translator) {}

        template<typename T>
        auto operator()(const T& element) const
        {
            return m_translator.translate(element);
        }
    };

protected:
    PDDLFactories& m_pddl_factories;

    explicit BaseTranslator(PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

    Condition translate_flatten_condition_and(const ConditionAndImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto and_condition = std::get_if<ConditionAndImpl>(nested_condition))
            {
                auto translated_nested_and_conditions = this->translate(and_condition->get_conditions());
                translated_nested_conditions.insert(translated_nested_conditions.end(),
                                                    translated_nested_and_conditions.begin(),
                                                    translated_nested_and_conditions.end());
            }
            else
            {
                translated_nested_conditions.push_back(this->translate(*nested_condition));
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(translated_nested_conditions);
    }

    Condition translate_flatten_condition_or(const ConditionOrImpl& condition)
    {
        auto translated_nested_conditions = ConditionList {};
        for (const auto& nested_condition : condition.get_conditions())
        {
            if (const auto or_condition = std::get_if<ConditionOrImpl>(nested_condition))
            {
                auto translated_nested_and_conditions = this->translate(or_condition->get_conditions());
                translated_nested_conditions.insert(translated_nested_conditions.end(),
                                                    translated_nested_and_conditions.begin(),
                                                    translated_nested_and_conditions.end());
            }
            else
            {
                translated_nested_conditions.push_back(this->translate(*nested_condition));
            }
        }
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(translated_nested_conditions);
    }

protected:
    /* Implement ITranslator interface */
    friend class ITranslator<BaseTranslator<Derived>>;

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    void prepare_base(const RequirementsImpl& requirements) { self().prepare_impl(requirements); }
    void prepare_base(const TypeImpl& type) { self().prepare_impl(type); }
    void prepare_base(const ObjectImpl& object) { self().prepare_impl(object); }
    void prepare_base(const VariableImpl& variable) { self().prepare_impl(variable); }
    void prepare_base(const TermObjectImpl& term) { self().prepare_impl(term); }
    void prepare_base(const TermVariableImpl& term) { self().prepare_impl(term); }
    void prepare_base(const TermImpl& term) { std::visit(PrepareVisitor(*this), term); }
    void prepare_base(const ParameterImpl& parameter) { self().prepare_impl(parameter); }
    void prepare_base(const PredicateImpl& predicate) { self().prepare_impl(predicate); }
    void prepare_base(const AtomImpl& atom) { self().prepare_impl(atom); }
    void prepare_base(const GroundAtomImpl& atom) { self().prepare_impl(atom); }
    void prepare_base(const LiteralImpl& literal) { self().prepare_impl(literal); }
    void prepare_base(const NumericFluentImpl& numeric_fluent) { self().prepare_impl(numeric_fluent); }
    void prepare_base(const GroundLiteralImpl& literal) { self().prepare_impl(literal); }
    void prepare_base(const ConditionLiteralImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionAndImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionOrImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionNotImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionImplyImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionExistsImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionForallImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const ConditionImpl& condition) { std::visit(PrepareVisitor(*this), condition); }
    void prepare_base(const EffectLiteralImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectAndImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectNumericImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectConditionalForallImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectConditionalWhenImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectImpl& effect) { std::visit(PrepareVisitor(*this), effect); }
    void prepare_base(const FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_base(const FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_base(const FunctionExpressionImpl& function_expression) { std::visit(PrepareVisitor(*this), function_expression); }
    void prepare_base(const FunctionSkeletonImpl& function_skeleton) { self().prepare_impl(function_skeleton); }
    void prepare_base(const FunctionImpl& function) { self().prepare_impl(function); }
    void prepare_base(const ActionImpl& action) { self().prepare_impl(action); }
    void prepare_base(const DerivedPredicateImpl& derived_predicate) { self().prepare_impl(derived_predicate); }
    void prepare_base(const DomainImpl& domain) { self().prepare_impl(domain); }
    void prepare_base(const OptimizationMetricImpl& metric) { self().prepare_impl(metric); }
    void prepare_base(const ProblemImpl& problem) { self().prepare_impl(problem); }

    void prepare_impl(const RequirementsImpl& requirements) {}
    void prepare_impl(const TypeImpl& type) { this->prepare(type.get_bases()); }
    void prepare_impl(const ObjectImpl& object) {}
    void prepare_impl(const VariableImpl& variable) {}
    void prepare_impl(const TermObjectImpl& term) { this->prepare(*term.get_object()); }
    void prepare_impl(const TermVariableImpl& term) { this->prepare(*term.get_variable()); }
    void prepare_impl(const ParameterImpl& parameter) { this->prepare(*parameter.get_variable()); }
    void prepare_impl(const PredicateImpl& predicate) {}
    void prepare_impl(const AtomImpl& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_terms());
    }
    void prepare_impl(const GroundAtomImpl& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_objects());
    }
    void prepare_impl(const LiteralImpl& literal) { this->prepare(*literal.get_atom()); }
    void prepare_impl(const NumericFluentImpl& numeric_fluent) { this->prepare(*numeric_fluent.get_function()); }
    void prepare_impl(const GroundLiteralImpl& literal) { this->prepare(*literal.get_atom()); }
    void prepare_impl(const ConditionLiteralImpl& condition) { this->prepare(*condition.get_literal()); }
    void prepare_impl(const ConditionAndImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const ConditionOrImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const ConditionNotImpl& condition) { this->prepare(*condition.get_condition()); }
    void prepare_impl(const ConditionImplyImpl& condition)
    {
        this->prepare(*condition.get_condition_left());
        this->prepare(*condition.get_condition_right());
    }
    void prepare_impl(const ConditionExistsImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const ConditionForallImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const EffectLiteralImpl& effect) { this->prepare(*effect.get_literal()); }
    void prepare_impl(const EffectAndImpl& effect) { this->prepare(effect.get_effects()); }
    void prepare_impl(const EffectNumericImpl& effect)
    {
        this->prepare(*effect.get_function());
        this->prepare(*effect.get_function_expression());
    }
    void prepare_impl(const EffectConditionalForallImpl& effect)
    {
        this->prepare(effect.get_parameters());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const EffectConditionalWhenImpl& effect)
    {
        this->prepare(*effect.get_condition());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const FunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        this->prepare(*function_expression.get_left_function_expression());
        this->prepare(*function_expression.get_right_function_expression());
    }
    void prepare_impl(const FunctionExpressionMultiOperatorImpl& function_expression) { this->prepare(function_expression.get_function_expressions()); }
    void prepare_impl(const FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_impl(const FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_impl(const FunctionSkeletonImpl& function_skeleton)
    {
        this->prepare(function_skeleton.get_parameters());
        this->prepare(*function_skeleton.get_type());
    }
    void prepare_impl(const FunctionImpl& function)
    {
        this->prepare(*function.get_function_skeleton());
        this->prepare(function.get_terms());
    }
    void prepare_impl(const ActionImpl& action)
    {
        this->prepare(action.get_parameters());
        if (action.get_condition().has_value())
        {
            this->prepare(*action.get_condition().value());
        }
        if (action.get_effect().has_value())
        {
            this->prepare(*action.get_effect().value());
        }
    }
    void prepare_impl(const DerivedPredicateImpl& derived_predicate)
    {
        this->prepare(derived_predicate.get_predicate());
        this->prepare(derived_predicate.get_condition());
    }
    void prepare_impl(const DomainImpl& domain)
    {
        this->prepare(*domain.get_requirements());
        this->prepare(domain.get_types());
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
    }
    void prepare_impl(const OptimizationMetricImpl& metric) { this->prepare(*metric.get_function_expression()); }
    void prepare_impl(const ProblemImpl& problem)
    {
        this->prepare(*problem.get_domain());
        this->prepare(*problem.get_requirements());
        this->prepare(problem.get_objects());
        this->prepare(problem.get_initial_literals());
        this->prepare(problem.get_numeric_fluents());
        this->prepare(*problem.get_goal_condition());
        if (problem.get_optimization_metric().has_value())
        {
            this->prepare(*problem.get_optimization_metric().value());
        }
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    Requirements translate_base(const RequirementsImpl& requirements) { return self().translate_impl(requirements); }
    Type translate_base(const TypeImpl& type) { return self().translate_impl(type); }
    Object translate_base(const ObjectImpl& object) { return self().translate_impl(object); }
    Variable translate_base(const VariableImpl& variable) { return self().translate_impl(variable); }
    Term translate_base(const TermObjectImpl& term) { return self().translate_impl(term); }
    Term translate_base(const TermVariableImpl& term) { return self().translate_impl(term); }
    Term translate_base(const TermImpl& term) { return std::visit(TranslateVisitor(*this), term); }
    Parameter translate_base(const ParameterImpl& parameter) { return self().translate_impl(parameter); }
    Predicate translate_base(const PredicateImpl& predicate) { return self().translate_impl(predicate); }
    Atom translate_base(const AtomImpl& atom) { return self().translate_impl(atom); }
    GroundAtom translate_base(const GroundAtomImpl& atom) { return self().translate_impl(atom); }
    Literal translate_base(const LiteralImpl& literal) { return self().translate_impl(literal); }
    GroundLiteral translate_base(const GroundLiteralImpl& literal) { return self().translate_impl(literal); }
    NumericFluent translate_base(const NumericFluentImpl& numeric_fluent) { return self().translate_impl(numeric_fluent); }
    Condition translate_base(const ConditionLiteralImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionAndImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionOrImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionNotImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionImplyImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionExistsImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionForallImpl& condition) { return self().translate_impl(condition); }
    Condition translate_base(const ConditionImpl& condition) { return std::visit(TranslateVisitor(*this), condition); }
    Effect translate_base(const EffectLiteralImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectAndImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectNumericImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectConditionalForallImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectConditionalWhenImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectImpl& effect) { return std::visit(TranslateVisitor(*this), effect); }
    FunctionExpression translate_base(const FunctionExpressionNumberImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionMinusImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionFunctionImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionImpl& function_expression) { return std::visit(TranslateVisitor(*this), function_expression); }
    FunctionSkeleton translate_base(const FunctionSkeletonImpl& function_skeleton) { return self().translate_impl(function_skeleton); }
    Function translate_base(const FunctionImpl& function) { return self().translate_impl(function); }
    Action translate_base(const ActionImpl& action) { return self().translate_impl(action); }
    DerivedPredicate translate_base(const DerivedPredicateImpl& derived_predicate) { return self().translate_impl(derived_predicate); }
    Domain translate_base(const DomainImpl& domain) { return self().translate_impl(domain); }
    OptimizationMetric translate_base(const OptimizationMetricImpl& metric) { return self().translate_impl(metric); }
    Problem translate_base(const ProblemImpl& problem) { return self().translate_impl(problem); }

    Requirements translate_impl(const RequirementsImpl& requirements)
    {
        return this->m_pddl_factories.requirements.template get_or_create<RequirementsImpl>(requirements.get_requirements());
    }
    Type translate_impl(const TypeImpl& type)
    {
        return this->m_pddl_factories.types.template get_or_create<TypeImpl>(type.get_name(), this->translate(type.get_bases()));
    }
    Object translate_impl(const ObjectImpl& object)
    {
        return this->m_pddl_factories.objects.template get_or_create<ObjectImpl>(object.get_name(), this->translate(object.get_bases()));
    }
    Variable translate_impl(const VariableImpl& variable) { return this->m_pddl_factories.variables.template get_or_create<VariableImpl>(variable.get_name()); }
    Term translate_impl(const TermObjectImpl& term)
    {
        return this->m_pddl_factories.terms.template get_or_create<TermObjectImpl>(this->translate(*term.get_object()));
    }
    Term translate_impl(const TermVariableImpl& term)
    {
        return this->m_pddl_factories.terms.template get_or_create<TermVariableImpl>(this->translate(*term.get_variable()));
    }
    Parameter translate_impl(const ParameterImpl& parameter)
    {
        return this->m_pddl_factories.parameters.template get_or_create<ParameterImpl>(this->translate(*parameter.get_variable()),
                                                                                       this->translate(parameter.get_bases()));
    }
    Predicate translate_impl(const PredicateImpl& predicate)
    {
        return this->m_pddl_factories.predicates.template get_or_create<PredicateImpl>(predicate.get_name(), this->translate(predicate.get_parameters()));
    }
    Atom translate_impl(const AtomImpl& atom)
    {
        return this->m_pddl_factories.atoms.template get_or_create<AtomImpl>(this->translate(*atom.get_predicate()), this->translate(atom.get_terms()));
    }
    GroundAtom translate_impl(const GroundAtomImpl& atom)
    {
        return this->m_pddl_factories.ground_atoms.template get_or_create<GroundAtomImpl>(this->translate(*atom.get_predicate()),
                                                                                          this->translate(atom.get_objects()));
    }
    Literal translate_impl(const LiteralImpl& literal)
    {
        return this->m_pddl_factories.literals.template get_or_create<LiteralImpl>(literal.is_negated(), this->translate(*literal.get_atom()));
    }
    GroundLiteral translate_impl(const GroundLiteralImpl& literal)
    {
        return this->m_pddl_factories.ground_literals.template get_or_create<GroundLiteralImpl>(literal.is_negated(), this->translate(*literal.get_atom()));
    }
    NumericFluent translate_impl(const NumericFluentImpl& numeric_fluent)
    {
        return this->m_pddl_factories.numeric_fluents.template get_or_create<NumericFluentImpl>(this->translate(*numeric_fluent.get_function()),
                                                                                                numeric_fluent.get_number());
    }
    Condition translate_impl(const ConditionLiteralImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionLiteralImpl>(this->translate(*condition.get_literal()));
    }
    Condition translate_impl(const ConditionAndImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionAndImpl>(this->translate(condition.get_conditions()));
    }
    Condition translate_impl(const ConditionOrImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionOrImpl>(this->translate(condition.get_conditions()));
    }
    Condition translate_impl(const ConditionNotImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionNotImpl>(this->translate(*condition.get_condition()));
    }
    Condition translate_impl(const ConditionImplyImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionImplyImpl>(this->translate(*condition.get_condition_left()),
                                                                                            this->translate(*condition.get_condition_right()));
    }
    Condition translate_impl(const ConditionExistsImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionExistsImpl>(this->translate(condition.get_parameters()),
                                                                                             this->translate(*condition.get_condition()));
    }
    Condition translate_impl(const ConditionForallImpl& condition)
    {
        return this->m_pddl_factories.conditions.template get_or_create<ConditionForallImpl>(this->translate(condition.get_parameters()),
                                                                                             this->translate(*condition.get_condition()));
    }
    Effect translate_impl(const EffectLiteralImpl& effect)
    {
        return this->m_pddl_factories.effects.template get_or_create<EffectLiteralImpl>(this->translate(*effect.get_literal()));
    }
    Effect translate_impl(const EffectAndImpl& effect)
    {
        return this->m_pddl_factories.effects.template get_or_create<EffectAndImpl>(this->translate(effect.get_effects()));
    }
    Effect translate_impl(const EffectNumericImpl& effect)
    {
        return this->m_pddl_factories.effects.template get_or_create<EffectNumericImpl>(effect.get_assign_operator(),
                                                                                        this->translate(*effect.get_function()),
                                                                                        this->translate(*effect.get_function_expression()));
    }
    Effect translate_impl(const EffectConditionalForallImpl& effect)
    {
        return this->m_pddl_factories.effects.template get_or_create<EffectConditionalForallImpl>(this->translate(effect.get_parameters()),
                                                                                                  this->translate(*effect.get_effect()));
    }
    Effect translate_impl(const EffectConditionalWhenImpl& effect)
    {
        return this->m_pddl_factories.effects.template get_or_create<EffectConditionalWhenImpl>(this->translate(*effect.get_condition()),
                                                                                                this->translate(*effect.get_effect()));
    }
    FunctionExpression translate_impl(const FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.template get_or_create<FunctionExpressionNumberImpl>(function_expression.get_number());
    }
    FunctionExpression translate_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.template get_or_create<FunctionExpressionBinaryOperatorImpl>(
            function_expression.get_binary_operator(),
            this->translate(*function_expression.get_left_function_expression()),
            this->translate(*function_expression.get_right_function_expression()));
    }
    FunctionExpression translate_impl(const FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.template get_or_create<FunctionExpressionMultiOperatorImpl>(
            function_expression.get_multi_operator(),
            this->translate(function_expression.get_function_expressions()));
    }
    FunctionExpression translate_impl(const FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.template get_or_create<FunctionExpressionMinusImpl>(
            this->translate(*function_expression.get_function_expression()));
    }
    FunctionExpression translate_impl(const FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.template get_or_create<FunctionExpressionFunctionImpl>(
            this->translate(*function_expression.get_function()));
    }
    FunctionSkeleton translate_impl(const FunctionSkeletonImpl& function_skeleton)
    {
        return this->m_pddl_factories.function_skeletons.template get_or_create<FunctionSkeletonImpl>(function_skeleton.get_name(),
                                                                                                      this->translate(function_skeleton.get_parameters()),
                                                                                                      this->translate(*function_skeleton.get_type()));
    }
    Function translate_impl(const FunctionImpl& function)
    {
        return this->m_pddl_factories.functions.template get_or_create<FunctionImpl>(this->translate(*function.get_function_skeleton()),
                                                                                     this->translate(function.get_terms()));
    }
    Action translate_impl(const ActionImpl& action)
    {
        return this->m_pddl_factories.actions.template get_or_create<ActionImpl>(
            action.get_name(),
            this->translate(action.get_parameters()),
            (action.get_condition().has_value() ? std::optional<Condition>(this->translate(*action.get_condition().value())) : std::nullopt),
            (action.get_effect().has_value() ? std::optional<Effect>(this->translate(*action.get_effect().value())) : std::nullopt));
    }
    DerivedPredicate translate_impl(const DerivedPredicateImpl& derived_predicate)
    {
        return this->m_pddl_factories.derived_predicates.template get_or_create<DerivedPredicateImpl>(this->translate(*derived_predicate.get_predicate()),
                                                                                                      this->translate(*derived_predicate.get_condition()));
    }
    Domain translate_impl(const DomainImpl& domain)
    {
        return this->m_pddl_factories.domains.template get_or_create<DomainImpl>(domain.get_name(),
                                                                                 this->translate(*domain.get_requirements()),
                                                                                 this->translate(domain.get_types()),
                                                                                 this->translate(domain.get_constants()),
                                                                                 this->translate(domain.get_predicates()),
                                                                                 this->translate(domain.get_functions()),
                                                                                 this->translate(domain.get_actions()),
                                                                                 this->translate(domain.get_derived_predicates()));
    }
    OptimizationMetric translate_impl(const OptimizationMetricImpl& metric)
    {
        return this->m_pddl_factories.optimization_metrics.template get_or_create<OptimizationMetricImpl>(metric.get_optimization_metric(),
                                                                                                          this->translate(*metric.get_function_expression()));
    }

    Problem translate_impl(const ProblemImpl& problem)
    {
        return this->m_pddl_factories.problems.template get_or_create<ProblemImpl>(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            this->translate(*problem.get_requirements()),
            this->translate(problem.get_objects()),
            this->translate(problem.get_initial_literals()),
            this->translate(problem.get_numeric_fluents()),
            this->translate(*problem.get_goal_condition()),
            (problem.get_optimization_metric().has_value() ? std::optional<OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                                                             std::nullopt));
    }

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    Problem run_base(const ProblemImpl& problem) { return self().run_impl(problem); }

    Problem run_impl(const ProblemImpl& problem)
    {
        self().prepare(problem);
        return self().translate(problem);
    }

public:
    PDDLFactories& get_pddl_factories() { return m_pddl_factories; }
};

}

#endif
