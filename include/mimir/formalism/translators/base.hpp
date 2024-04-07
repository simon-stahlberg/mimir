#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_

#include "mimir/formalism/translators/interface.hpp"

#include <deque>
#include <loki/pddl/visitors.hpp>
#include <unordered_map>

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

    std::unordered_map<Requirements, Requirements> m_translated_requirements;
    std::unordered_map<Type, Type> m_translated_types;
    std::unordered_map<Object, Object> m_translated_objects;
    std::unordered_map<Variable, Variable> m_translated_variables;
    std::unordered_map<Term, Term> m_translated_terms;
    std::unordered_map<Parameter, Parameter> m_translated_parameters;
    std::unordered_map<Predicate, Predicate> m_translated_predicates;
    std::unordered_map<Atom, Atom> m_translated_atoms;
    std::unordered_map<GroundAtom, GroundAtom> m_translated_ground_atoms;
    std::unordered_map<Literal, Literal> m_translated_literals;
    std::unordered_map<GroundLiteral, GroundLiteral> m_translated_ground_literals;
    std::unordered_map<NumericFluent, NumericFluent> m_translated_numeric_fluents;
    std::unordered_map<Condition, Condition> m_translated_conditions;
    std::unordered_map<Effect, Effect> m_translated_effects;
    std::unordered_map<FunctionExpression, FunctionExpression> m_translated_function_expressions;
    std::unordered_map<FunctionSkeleton, FunctionSkeleton> m_translated_function_skeletons;
    std::unordered_map<Function, Function> m_translated_functions;
    std::unordered_map<Action, Action> m_translated_actions;
    std::unordered_map<DerivedPredicate, DerivedPredicate> m_translated_derived_predicates;
    std::unordered_map<Domain, Domain> m_translated_domains;
    std::unordered_map<OptimizationMetric, OptimizationMetric> m_translated_optimization_metrics;
    std::unordered_map<Problem, Problem> m_translated_problems;

protected:
    PDDLFactories& m_pddl_factories;

    explicit BaseTranslator(PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

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
    void prepare_base(const TermImpl& term) { self().prepare_impl(term); }
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
    void prepare_base(const ConditionImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const EffectLiteralImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectAndImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectNumericImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectConditionalForallImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectConditionalWhenImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const EffectImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_base(const FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_base(const FunctionExpressionImpl& function_expression) { self().prepare_impl(function_expression); }
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
    void prepare_impl(const TermImpl& term)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, term);
    }
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
    void prepare_impl(const ConditionImpl& condition)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, condition);
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
    void prepare_impl(const EffectImpl& effect)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, effect);
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
    void prepare_impl(const FunctionExpressionImpl& function_expression)
    {
        std::visit([this](auto&& arg) { return this->prepare(arg); }, function_expression);
    }
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
    Term translate_base(const TermImpl& term) { return self().translate_impl(term); }
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
    Condition translate_base(const ConditionImpl& condition) { return self().translate_impl(condition); }
    Effect translate_base(const EffectLiteralImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectAndImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectNumericImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectConditionalForallImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectConditionalWhenImpl& effect) { return self().translate_impl(effect); }
    Effect translate_base(const EffectImpl& effect) { return self().translate_impl(effect); }
    FunctionExpression translate_base(const FunctionExpressionNumberImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionBinaryOperatorImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionMultiOperatorImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionMinusImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionFunctionImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionExpression translate_base(const FunctionExpressionImpl& function_expression) { return self().translate_impl(function_expression); }
    FunctionSkeleton translate_base(const FunctionSkeletonImpl& function_skeleton) { return self().translate_impl(function_skeleton); }
    Function translate_base(const FunctionImpl& function) { return self().translate_impl(function); }
    Action translate_base(const ActionImpl& action) { return self().translate_impl(action); }
    DerivedPredicate translate_base(const DerivedPredicateImpl& derived_predicate) { return self().translate_impl(derived_predicate); }
    Domain translate_base(const DomainImpl& domain) { return self().translate_impl(domain); }
    OptimizationMetric translate_base(const OptimizationMetricImpl& metric) { return self().translate_impl(metric); }
    Problem translate_base(const ProblemImpl& problem) { return self().translate_impl(problem); }

    /// @brief Retrieve or create cache entry of translation to avoid recomputations.
    template<typename Impl, typename TranslateFunc>
    auto cached_translated_impl(const Impl& impl, std::unordered_map<const Impl*, const Impl*>& cache, const TranslateFunc& translateFunc)
    {
        // Access from cache
        auto it = cache.find(&impl);
        if (it != cache.end())
        {
            return it->second;
        }

        // Translate
        auto translated = translateFunc(impl);

        // Insert into cache
        cache.emplace(&impl, translated);

        return translated;
    }
    Requirements translate_impl(const RequirementsImpl& requirements)
    {
        return cached_translated_impl(requirements,
                                      m_translated_requirements,
                                      [this](const RequirementsImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_requirements(arg.get_requirements()); });
    }
    Type translate_impl(const TypeImpl& type)
    {
        return cached_translated_impl(type,
                                      m_translated_types,
                                      [this](const TypeImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_type(arg.get_name(), this->translate(arg.get_bases())); });
    }
    Object translate_impl(const ObjectImpl& object)
    {
        return cached_translated_impl(object,
                                      m_translated_objects,
                                      [this](const ObjectImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_object(arg.get_name(), this->translate(arg.get_bases())); });
    }
    Variable translate_impl(const VariableImpl& variable)
    {
        return cached_translated_impl(variable,
                                      m_translated_variables,
                                      [this](const VariableImpl& arg) { return this->m_pddl_factories.get_or_create_variable(arg.get_name()); });
    }
    Term translate_impl(const TermObjectImpl& term) { return this->m_pddl_factories.get_or_create_term_object(this->translate(*term.get_object())); }
    Term translate_impl(const TermVariableImpl& term) { return this->m_pddl_factories.get_or_create_term_variable(this->translate(*term.get_variable())); }
    Term translate_impl(const TermImpl& term)
    {
        return cached_translated_impl(term,
                                      m_translated_terms,
                                      [this, &term](const TermImpl& arg) { return std::visit([this](auto&& arg) { return this->translate(arg); }, term); });
    }
    Parameter translate_impl(const ParameterImpl& parameter)
    {
        return cached_translated_impl(
            parameter,
            m_translated_parameters,
            [this](const ParameterImpl& arg)
            { return this->m_pddl_factories.get_or_create_parameter(this->translate(*arg.get_variable()), this->translate(arg.get_bases())); });
    }
    Predicate translate_impl(const PredicateImpl& predicate)
    {
        return cached_translated_impl(predicate,
                                      m_translated_predicates,
                                      [this](const PredicateImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_predicate(arg.get_name(), this->translate(arg.get_parameters())); });
    }
    Atom translate_impl(const AtomImpl& atom)
    {
        return cached_translated_impl(
            atom,
            m_translated_atoms,
            [this](const AtomImpl& arg)
            { return this->m_pddl_factories.get_or_create_atom(this->translate(*arg.get_predicate()), this->translate(arg.get_terms())); });
    }
    GroundAtom translate_impl(const GroundAtomImpl& atom)
    {
        return cached_translated_impl(
            atom,
            m_translated_ground_atoms,
            [this](const GroundAtomImpl& arg)
            { return this->m_pddl_factories.get_or_create_ground_atom(this->translate(*arg.get_predicate()), this->translate(arg.get_objects())); });
    }
    Literal translate_impl(const LiteralImpl& literal)
    {
        return cached_translated_impl(literal,
                                      m_translated_literals,
                                      [this](const LiteralImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_literal(arg.is_negated(), this->translate(*arg.get_atom())); });
    }
    GroundLiteral translate_impl(const GroundLiteralImpl& literal)
    {
        return cached_translated_impl(literal,
                                      m_translated_ground_literals,
                                      [this](const GroundLiteralImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_ground_literal(arg.is_negated(), this->translate(*arg.get_atom())); });
    }
    NumericFluent translate_impl(const NumericFluentImpl& numeric_fluent)
    {
        return cached_translated_impl(numeric_fluent,
                                      m_translated_numeric_fluents,
                                      [this](const NumericFluentImpl& arg)
                                      { return this->m_pddl_factories.get_or_create_numeric_fluent(this->translate(*arg.get_function()), arg.get_number()); });
    }
    Condition translate_impl(const ConditionLiteralImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_literal(this->translate(*condition.get_literal()));
    }
    Condition translate_impl(const ConditionAndImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_and(this->translate(condition.get_conditions()));
    }
    Condition translate_impl(const ConditionOrImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_or(this->translate(condition.get_conditions()));
    }
    Condition translate_impl(const ConditionNotImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_not(this->translate(*condition.get_condition()));
    }
    Condition translate_impl(const ConditionImplyImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_imply(this->translate(*condition.get_condition_left()),
                                                                    this->translate(*condition.get_condition_right()));
    }
    Condition translate_impl(const ConditionExistsImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_exists(this->translate(condition.get_parameters()), this->translate(*condition.get_condition()));
    }
    Condition translate_impl(const ConditionForallImpl& condition)
    {
        return this->m_pddl_factories.get_or_create_condition_forall(this->translate(condition.get_parameters()), this->translate(*condition.get_condition()));
    }
    Condition translate_impl(const ConditionImpl& condition)
    {
        return cached_translated_impl(condition,
                                      m_translated_conditions,
                                      [this, &condition](const ConditionImpl& arg)
                                      { return std::visit([this](auto&& arg) { return this->translate(arg); }, condition); });
    }
    Effect translate_impl(const EffectLiteralImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_literal(this->translate(*effect.get_literal()));
    }
    Effect translate_impl(const EffectAndImpl& effect) { return this->m_pddl_factories.get_or_create_effect_and(this->translate(effect.get_effects())); }
    Effect translate_impl(const EffectNumericImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_numeric(effect.get_assign_operator(),
                                                                   this->translate(*effect.get_function()),
                                                                   this->translate(*effect.get_function_expression()));
    }
    Effect translate_impl(const EffectConditionalForallImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_forall(this->translate(effect.get_parameters()), this->translate(*effect.get_effect()));
    }
    Effect translate_impl(const EffectConditionalWhenImpl& effect)
    {
        return this->m_pddl_factories.get_or_create_effect_conditional_when(this->translate(*effect.get_condition()), this->translate(*effect.get_effect()));
    }
    Effect translate_impl(const EffectImpl& effect)
    {
        return cached_translated_impl(effect,
                                      m_translated_effects,
                                      [this, &effect](const EffectImpl& arg)
                                      { return std::visit([this](auto&& arg) { return this->translate(arg); }, effect); });
    }
    FunctionExpression translate_impl(const FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_number(function_expression.get_number());
    }
    FunctionExpression translate_impl(const FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_binary_operator(function_expression.get_binary_operator(),
                                                                                        this->translate(*function_expression.get_left_function_expression()),
                                                                                        this->translate(*function_expression.get_right_function_expression()));
    }
    FunctionExpression translate_impl(const FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_multi_operator(function_expression.get_multi_operator(),
                                                                                       this->translate(function_expression.get_function_expressions()));
    }
    FunctionExpression translate_impl(const FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_minus(this->translate(*function_expression.get_function_expression()));
    }
    FunctionExpression translate_impl(const FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.get_or_create_function_expression_function(this->translate(*function_expression.get_function()));
    }
    FunctionExpression translate_impl(const FunctionExpressionImpl& function_expression)
    {
        return cached_translated_impl(function_expression,
                                      m_translated_function_expressions,
                                      [this, &function_expression](const FunctionExpressionImpl& arg)
                                      { return std::visit([this](auto&& arg) { return this->translate(arg); }, function_expression); });
    }
    FunctionSkeleton translate_impl(const FunctionSkeletonImpl& function_skeleton)
    {
        return cached_translated_impl(function_skeleton,
                                      m_translated_function_skeletons,
                                      [this](const FunctionSkeletonImpl& arg) {
                                          return this->m_pddl_factories.get_or_create_function_skeleton(arg.get_name(),
                                                                                                        this->translate(arg.get_parameters()),
                                                                                                        this->translate(*arg.get_type()));
                                      });
    }
    Function translate_impl(const FunctionImpl& function)
    {
        return cached_translated_impl(
            function,
            m_translated_functions,
            [this](const FunctionImpl& arg)
            { return this->m_pddl_factories.get_or_create_function(this->translate(*arg.get_function_skeleton()), this->translate(arg.get_terms())); });
    }
    Action translate_impl(const ActionImpl& action)
    {
        return cached_translated_impl(
            action,
            m_translated_actions,
            [this](const ActionImpl& arg)
            {
                return this->m_pddl_factories.get_or_create_action(
                    arg.get_name(),
                    this->translate(arg.get_parameters()),
                    (arg.get_condition().has_value() ? std::optional<Condition>(this->translate(*arg.get_condition().value())) : std::nullopt),
                    (arg.get_effect().has_value() ? std::optional<Effect>(this->translate(*arg.get_effect().value())) : std::nullopt));
            });
    }
    DerivedPredicate translate_impl(const DerivedPredicateImpl& derived_predicate)
    {
        return cached_translated_impl(
            derived_predicate,
            m_translated_derived_predicates,
            [this](const DerivedPredicateImpl& arg)
            { return this->m_pddl_factories.get_or_create_derived_predicate(this->translate(*arg.get_predicate()), this->translate(*arg.get_condition())); });
    }
    Domain translate_impl(const DomainImpl& domain)
    {
        return cached_translated_impl(domain,
                                      m_translated_domains,
                                      [this](const DomainImpl& arg)
                                      {
                                          return this->m_pddl_factories.get_or_create_domain(arg.get_name(),
                                                                                             this->translate(*arg.get_requirements()),
                                                                                             this->translate(arg.get_types()),
                                                                                             this->translate(arg.get_constants()),
                                                                                             this->translate(arg.get_predicates()),
                                                                                             this->translate(arg.get_functions()),
                                                                                             this->translate(arg.get_actions()),
                                                                                             this->translate(arg.get_derived_predicates()));
                                      });
    }
    OptimizationMetric translate_impl(const OptimizationMetricImpl& metric)
    {
        return cached_translated_impl(metric,
                                      m_translated_optimization_metrics,
                                      [this](const OptimizationMetricImpl& arg) {
                                          return this->m_pddl_factories.get_or_create_optimization_metric(arg.get_optimization_metric(),
                                                                                                          this->translate(*arg.get_function_expression()));
                                      });
    }

    Problem translate_impl(const ProblemImpl& problem)
    {
        return cached_translated_impl(problem,
                                      m_translated_problems,
                                      [this](const ProblemImpl& arg)
                                      {
                                          return this->m_pddl_factories.get_or_create_problem(
                                              this->translate(*arg.get_domain()),
                                              arg.get_name(),
                                              this->translate(*arg.get_requirements()),
                                              this->translate(arg.get_objects()),
                                              this->translate(arg.get_initial_literals()),
                                              this->translate(arg.get_numeric_fluents()),
                                              this->translate(*arg.get_goal_condition()),
                                              (arg.get_optimization_metric().has_value() ?
                                                   std::optional<OptimizationMetric>(this->translate(*arg.get_optimization_metric().value())) :
                                                   std::nullopt));
                                      });
    }

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    Problem run_base(const ProblemImpl& problem) { return self().run_impl(problem); }

    Problem run_impl(const ProblemImpl& problem)
    {
        self().prepare(problem);
        return self().translate(problem);
    }
};

}

#endif
