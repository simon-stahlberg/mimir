#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_

#include "mimir/formalism/translators/interface.hpp"

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

protected:
    loki::PDDLFactories& m_pddl_factories;

public:
    explicit BaseTranslator(loki::PDDLFactories& pddl_factories) : m_pddl_factories(pddl_factories) {}

    /// @brief Collect information.
    ///        Default implementation recursively calls prepare.
    void prepare_base(const loki::pddl::RequirementsImpl& requirements) { self().prepare_impl(requirements); }
    void prepare_base(const loki::pddl::TypeImpl& type) { self().prepare_impl(type); }
    void prepare_base(const loki::pddl::ObjectImpl& object) { self().prepare_impl(object); }
    void prepare_base(const loki::pddl::VariableImpl& variable) { self().prepare_impl(variable); }
    void prepare_base(const loki::pddl::TermObjectImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::pddl::TermVariableImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::pddl::ParameterImpl& parameter) { self().prepare_impl(parameter); }
    void prepare_base(const loki::pddl::PredicateImpl& predicate) { self().prepare_impl(predicate); }
    void prepare_base(const loki::pddl::AtomImpl& atom) { self().prepare_impl(atom); }
    void prepare_base(const loki::pddl::GroundAtomImpl& atom) { self().prepare_impl(atom); }
    void prepare_base(const loki::pddl::LiteralImpl& literal) { self().prepare_impl(literal); }
    void prepare_base(const loki::pddl::NumericFluentImpl& numeric_fluent) { self().prepare_impl(numeric_fluent); }
    void prepare_base(const loki::pddl::GroundLiteralImpl& literal) { self().prepare_impl(literal); }
    void prepare_base(const loki::pddl::ConditionLiteralImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionAndImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionOrImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionNotImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionImplyImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionExistsImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::ConditionForallImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::EffectLiteralImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectAndImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectNumericImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectConditionalForallImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectConditionalWhenImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_base(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_base(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { self().prepare_impl(function_skeleton); }
    void prepare_base(const loki::pddl::FunctionImpl& function) { self().prepare_impl(function); }
    void prepare_base(const loki::pddl::ActionImpl& action) { self().prepare_impl(action); }
    void prepare_base(const loki::pddl::DomainImpl& domain) { self().prepare_impl(domain); }
    void prepare_base(const loki::pddl::OptimizationMetricImpl& metric) { self().prepare_impl(metric); }
    void prepare_base(const loki::pddl::ProblemImpl& problem) { self().prepare_impl(problem); }

    void prepare_impl(const loki::pddl::RequirementsImpl& requirements) {}
    void prepare_impl(const loki::pddl::TypeImpl& type) { this->prepare(type.get_bases()); }
    void prepare_impl(const loki::pddl::ObjectImpl& object) {}
    void prepare_impl(const loki::pddl::VariableImpl& variable) {}
    void prepare_impl(const loki::pddl::TermObjectImpl& term) { this->prepare(*term.get_object()); }
    void prepare_impl(const loki::pddl::TermVariableImpl& term) { this->prepare(*term.get_variable()); }
    void prepare_impl(const loki::pddl::ParameterImpl& parameter) { this->prepare(*parameter.get_variable()); }
    void prepare_impl(const loki::pddl::PredicateImpl& predicate) {}
    void prepare_impl(const loki::pddl::AtomImpl& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_terms());
    }
    void prepare_impl(const loki::pddl::GroundAtomImpl& atom)
    {
        this->prepare(*atom.get_predicate());
        this->prepare(atom.get_objects());
    }
    void prepare_impl(const loki::pddl::LiteralImpl& literal) { this->prepare(*literal.get_atom()); }
    void prepare_impl(const loki::pddl::NumericFluentImpl& numeric_fluent) { this->prepare(*numeric_fluent.get_function()); }
    void prepare_impl(const loki::pddl::GroundLiteralImpl& literal) { this->prepare(*literal.get_atom()); }
    void prepare_impl(const loki::pddl::ConditionLiteralImpl& condition) { this->prepare(*condition.get_literal()); }
    void prepare_impl(const loki::pddl::ConditionAndImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const loki::pddl::ConditionOrImpl& condition) { this->prepare(condition.get_conditions()); }
    void prepare_impl(const loki::pddl::ConditionNotImpl& condition) { this->prepare(*condition.get_condition()); }
    void prepare_impl(const loki::pddl::ConditionImplyImpl& condition)
    {
        this->prepare(*condition.get_condition_left());
        this->prepare(*condition.get_condition_right());
    }
    void prepare_impl(const loki::pddl::ConditionExistsImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const loki::pddl::ConditionForallImpl& condition)
    {
        this->prepare(condition.get_parameters());
        this->prepare(*condition.get_condition());
    }
    void prepare_impl(const loki::pddl::EffectLiteralImpl& effect) { this->prepare(*effect.get_literal()); }
    void prepare_impl(const loki::pddl::EffectAndImpl& effect) { this->prepare(effect.get_effects()); }
    void prepare_impl(const loki::pddl::EffectNumericImpl& effect)
    {
        this->prepare(*effect.get_function());
        this->prepare(*effect.get_function_expression());
    }
    void prepare_impl(const loki::pddl::EffectConditionalForallImpl& effect)
    {
        this->prepare(effect.get_parameters());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const loki::pddl::EffectConditionalWhenImpl& effect)
    {
        this->prepare(*effect.get_condition());
        this->prepare(*effect.get_effect());
    }
    void prepare_impl(const loki::pddl::FunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        this->prepare(*function_expression.get_left_function_expression());
        this->prepare(*function_expression.get_right_function_expression());
    }
    void prepare_impl(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        this->prepare(function_expression.get_function_expressions());
    }
    void prepare_impl(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { this->prepare(*function_expression.get_function_expression()); }
    void prepare_impl(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) { this->prepare(*function_expression.get_function()); }
    void prepare_impl(const loki::pddl::FunctionSkeletonImpl& function_skeleton)
    {
        this->prepare(function_skeleton.get_parameters());
        this->prepare(*function_skeleton.get_type());
    }
    void prepare_impl(const loki::pddl::FunctionImpl& function)
    {
        this->prepare(*function.get_function_skeleton());
        this->prepare(function.get_terms());
    }
    void prepare_impl(const loki::pddl::ActionImpl& action)
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
    void prepare_impl(const loki::pddl::DomainImpl& domain)
    {
        this->prepare(*domain.get_requirements());
        this->prepare(domain.get_types());
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
    }
    void prepare_impl(const loki::pddl::OptimizationMetricImpl& metric) { this->prepare(*metric.get_function_expression()); }
    void prepare_impl(const loki::pddl::ProblemImpl& problem)
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
    loki::pddl::Requirements translate_base(const loki::pddl::RequirementsImpl& requirements) { return self().translate_impl(requirements); }
    loki::pddl::Type translate_base(const loki::pddl::TypeImpl& type) { return self().translate_impl(type); }
    loki::pddl::Object translate_base(const loki::pddl::ObjectImpl& object) { return self().translate_impl(object); }
    loki::pddl::Variable translate_base(const loki::pddl::VariableImpl& variable) { return self().translate_impl(variable); }
    loki::pddl::Term translate_base(const loki::pddl::TermObjectImpl& term) { return self().translate_impl(term); }
    loki::pddl::Term translate_base(const loki::pddl::TermVariableImpl& term) { return self().translate_impl(term); }
    loki::pddl::Parameter translate_base(const loki::pddl::ParameterImpl& parameter) { return self().translate_impl(parameter); }
    loki::pddl::Predicate translate_base(const loki::pddl::PredicateImpl& predicate) { return self().translate_impl(predicate); }
    loki::pddl::Atom translate_base(const loki::pddl::AtomImpl& atom) { return self().translate_impl(atom); }
    loki::pddl::GroundAtom translate_base(const loki::pddl::GroundAtomImpl& atom) { return self().translate_impl(atom); }
    loki::pddl::Literal translate_base(const loki::pddl::LiteralImpl& literal) { return self().translate_impl(literal); }
    loki::pddl::GroundLiteral translate_base(const loki::pddl::GroundLiteralImpl& literal) { return self().translate_impl(literal); }
    loki::pddl::NumericFluent translate_base(const loki::pddl::NumericFluentImpl& numeric_fluent) { return self().translate_impl(numeric_fluent); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionLiteralImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionAndImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionOrImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionNotImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionImplyImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionExistsImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Condition translate_base(const loki::pddl::ConditionForallImpl& condition) { return self().translate_impl(condition); }
    loki::pddl::Effect translate_base(const loki::pddl::EffectLiteralImpl& effect) { return self().translate_impl(effect); }
    loki::pddl::Effect translate_base(const loki::pddl::EffectAndImpl& effect) { return self().translate_impl(effect); }
    loki::pddl::Effect translate_base(const loki::pddl::EffectNumericImpl& effect) { return self().translate_impl(effect); }
    loki::pddl::Effect translate_base(const loki::pddl::EffectConditionalForallImpl& effect) { return self().translate_impl(effect); }
    loki::pddl::Effect translate_base(const loki::pddl::EffectConditionalWhenImpl& effect) { return self().translate_impl(effect); }
    loki::pddl::FunctionExpression translate_base(const loki::pddl::FunctionExpressionNumberImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::pddl::FunctionExpression translate_base(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::pddl::FunctionExpression translate_base(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::pddl::FunctionExpression translate_base(const loki::pddl::FunctionExpressionMinusImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::pddl::FunctionExpression translate_base(const loki::pddl::FunctionExpressionFunctionImpl& function_expression)
    {
        return self().translate_impl(function_expression);
    }
    loki::pddl::FunctionSkeleton translate_base(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { return self().translate_impl(function_skeleton); }
    loki::pddl::Function translate_base(const loki::pddl::FunctionImpl& function) { return self().translate_impl(function); }
    loki::pddl::Action translate_base(const loki::pddl::ActionImpl& action) { return self().translate_impl(action); }
    loki::pddl::Domain translate_base(const loki::pddl::DomainImpl& domain) { return self().translate_impl(domain); }
    loki::pddl::OptimizationMetric translate_base(const loki::pddl::OptimizationMetricImpl& metric) { return self().translate_impl(metric); }
    loki::pddl::Problem translate_base(const loki::pddl::ProblemImpl& problem) { return self().translate_impl(problem); }

    loki::pddl::Requirements translate_impl(const loki::pddl::RequirementsImpl& requirements)
    {
        return this->m_pddl_factories.requirements.get_or_create<loki::pddl::RequirementsImpl>(requirements.get_requirements());
    }
    loki::pddl::Type translate_impl(const loki::pddl::TypeImpl& type)
    {
        return this->m_pddl_factories.types.get_or_create<loki::pddl::TypeImpl>(type.get_name(), this->translate(type.get_bases()));
    }
    loki::pddl::Object translate_impl(const loki::pddl::ObjectImpl& object)
    {
        return this->m_pddl_factories.objects.get_or_create<loki::pddl::ObjectImpl>(object.get_name(), this->translate(object.get_bases()));
    }
    loki::pddl::Variable translate_impl(const loki::pddl::VariableImpl& variable)
    {
        return this->m_pddl_factories.variables.get_or_create<loki::pddl::VariableImpl>(variable.get_name());
    }
    loki::pddl::Term translate_impl(const loki::pddl::TermObjectImpl& term)
    {
        return this->m_pddl_factories.terms.get_or_create<loki::pddl::TermObjectImpl>(this->translate(*term.get_object()));
    }
    loki::pddl::Term translate_impl(const loki::pddl::TermVariableImpl& term)
    {
        return this->m_pddl_factories.terms.get_or_create<loki::pddl::TermVariableImpl>(this->translate(*term.get_variable()));
    }
    loki::pddl::Parameter translate_impl(const loki::pddl::ParameterImpl& parameter)
    {
        return this->m_pddl_factories.parameters.get_or_create<loki::pddl::ParameterImpl>(this->translate(*parameter.get_variable()),
                                                                                          this->translate(parameter.get_bases()));
    }
    loki::pddl::Predicate translate_impl(const loki::pddl::PredicateImpl& predicate)
    {
        return this->m_pddl_factories.predicates.get_or_create<loki::pddl::PredicateImpl>(predicate.get_name(), this->translate(predicate.get_parameters()));
    }
    loki::pddl::Atom translate_impl(const loki::pddl::AtomImpl& atom)
    {
        return this->m_pddl_factories.atoms.get_or_create<loki::pddl::AtomImpl>(this->translate(*atom.get_predicate()), this->translate(atom.get_terms()));
    }
    loki::pddl::GroundAtom translate_impl(const loki::pddl::GroundAtomImpl& atom)
    {
        return this->m_pddl_factories.ground_atoms.get_or_create<loki::pddl::GroundAtomImpl>(this->translate(*atom.get_predicate()),
                                                                                             this->translate(atom.get_objects()));
    }
    loki::pddl::Literal translate_impl(const loki::pddl::LiteralImpl& literal)
    {
        return this->m_pddl_factories.literals.get_or_create<loki::pddl::LiteralImpl>(literal.is_negated(), this->translate(*literal.get_atom()));
    }
    loki::pddl::GroundLiteral translate_impl(const loki::pddl::GroundLiteralImpl& literal)
    {
        return this->m_pddl_factories.ground_literals.get_or_create<loki::pddl::GroundLiteralImpl>(literal.is_negated(), this->translate(*literal.get_atom()));
    }
    loki::pddl::NumericFluent translate_impl(const loki::pddl::NumericFluentImpl& numeric_fluent)
    {
        return this->m_pddl_factories.numeric_fluents.get_or_create<loki::pddl::NumericFluentImpl>(this->translate(*numeric_fluent.get_function()),
                                                                                                   numeric_fluent.get_number());
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionLiteralImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionLiteralImpl>(this->translate(*condition.get_literal()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionAndImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionAndImpl>(this->translate(condition.get_conditions()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionOrImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionOrImpl>(this->translate(condition.get_conditions()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionNotImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionNotImpl>(this->translate(*condition.get_condition()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionImplyImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionImplyImpl>(this->translate(*condition.get_condition_left()),
                                                                                               this->translate(*condition.get_condition_right()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionExistsImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionExistsImpl>(this->translate(condition.get_parameters()),
                                                                                                this->translate(*condition.get_condition()));
    }
    loki::pddl::Condition translate_impl(const loki::pddl::ConditionForallImpl& condition)
    {
        return this->m_pddl_factories.conditions.get_or_create<loki::pddl::ConditionForallImpl>(this->translate(condition.get_parameters()),
                                                                                                this->translate(*condition.get_condition()));
    }
    loki::pddl::Effect translate_impl(const loki::pddl::EffectLiteralImpl& effect)
    {
        return this->m_pddl_factories.effects.get_or_create<loki::pddl::EffectLiteralImpl>(this->translate(*effect.get_literal()));
    }
    loki::pddl::Effect translate_impl(const loki::pddl::EffectAndImpl& effect)
    {
        return this->m_pddl_factories.effects.get_or_create<loki::pddl::EffectAndImpl>(this->translate(effect.get_effects()));
    }
    loki::pddl::Effect translate_impl(const loki::pddl::EffectNumericImpl& effect)
    {
        return this->m_pddl_factories.effects.get_or_create<loki::pddl::EffectNumericImpl>(effect.get_assign_operator(),
                                                                                           this->translate(*effect.get_function()),
                                                                                           this->translate(*effect.get_function_expression()));
    }
    loki::pddl::Effect translate_impl(const loki::pddl::EffectConditionalForallImpl& effect)
    {
        return this->m_pddl_factories.effects.get_or_create<loki::pddl::EffectConditionalForallImpl>(this->translate(effect.get_parameters()),
                                                                                                     this->translate(*effect.get_effect()));
    }
    loki::pddl::Effect translate_impl(const loki::pddl::EffectConditionalWhenImpl& effect)
    {
        return this->m_pddl_factories.effects.get_or_create<loki::pddl::EffectConditionalWhenImpl>(this->translate(*effect.get_condition()),
                                                                                                   this->translate(*effect.get_effect()));
    }
    loki::pddl::FunctionExpression translate_impl(const loki::pddl::FunctionExpressionNumberImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.get_or_create<loki::pddl::FunctionExpressionNumberImpl>(function_expression.get_number());
    }
    loki::pddl::FunctionExpression translate_impl(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.get_or_create<loki::pddl::FunctionExpressionBinaryOperatorImpl>(
            function_expression.get_binary_operator(),
            this->translate(*function_expression.get_left_function_expression()),
            this->translate(*function_expression.get_right_function_expression()));
    }
    loki::pddl::FunctionExpression translate_impl(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.get_or_create<loki::pddl::FunctionExpressionMultiOperatorImpl>(
            function_expression.get_multi_operator(),
            this->translate(function_expression.get_function_expressions()));
    }
    loki::pddl::FunctionExpression translate_impl(const loki::pddl::FunctionExpressionMinusImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.get_or_create<loki::pddl::FunctionExpressionMinusImpl>(
            this->translate(*function_expression.get_function_expression()));
    }
    loki::pddl::FunctionExpression translate_impl(const loki::pddl::FunctionExpressionFunctionImpl& function_expression)
    {
        return this->m_pddl_factories.function_expressions.get_or_create<loki::pddl::FunctionExpressionFunctionImpl>(
            this->translate(*function_expression.get_function()));
    }
    loki::pddl::FunctionSkeleton translate_impl(const loki::pddl::FunctionSkeletonImpl& function_skeleton)
    {
        return this->m_pddl_factories.function_skeletons.get_or_create<loki::pddl::FunctionSkeletonImpl>(function_skeleton.get_name(),
                                                                                                         this->translate(function_skeleton.get_parameters()),
                                                                                                         this->translate(*function_skeleton.get_type()));
    }
    loki::pddl::Function translate_impl(const loki::pddl::FunctionImpl& function)
    {
        return this->m_pddl_factories.functions.get_or_create<loki::pddl::FunctionImpl>(this->translate(*function.get_function_skeleton()),
                                                                                        this->translate(function.get_terms()));
    }
    loki::pddl::Action translate_impl(const loki::pddl::ActionImpl& action)
    {
        return this->m_pddl_factories.actions.get_or_create<loki::pddl::ActionImpl>(
            action.get_name(),
            this->translate(action.get_parameters()),
            (action.get_condition().has_value() ? std::optional<loki::pddl::Condition>(this->translate(*action.get_condition().value())) : std::nullopt),
            (action.get_effect().has_value() ? std::optional<loki::pddl::Effect>(this->translate(*action.get_effect().value())) : std::nullopt));
    }
    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain)
    {
        return this->m_pddl_factories.domains.get_or_create<loki::pddl::DomainImpl>(domain.get_name(),
                                                                                    this->translate(*domain.get_requirements()),
                                                                                    this->translate(domain.get_types()),
                                                                                    this->translate(domain.get_constants()),
                                                                                    this->translate(domain.get_predicates()),
                                                                                    this->translate(domain.get_functions()),
                                                                                    this->translate(domain.get_actions()));
    }
    loki::pddl::OptimizationMetric translate_impl(const loki::pddl::OptimizationMetricImpl& metric)
    {
        return this->m_pddl_factories.optimization_metrics.get_or_create<loki::pddl::OptimizationMetricImpl>(
            metric.get_optimization_metric(),
            this->translate(*metric.get_function_expression()));
    }

    loki::pddl::Problem translate_impl(const loki::pddl::ProblemImpl& problem)
    {
        std::cout << "BaseTranslator translate problem" << std::endl;
        return this->m_pddl_factories.problems.get_or_create<loki::pddl::ProblemImpl>(
            this->translate(*problem.get_domain()),
            problem.get_name(),
            this->translate(*problem.get_requirements()),
            this->translate(problem.get_objects()),
            this->translate(problem.get_initial_literals()),
            this->translate(problem.get_numeric_fluents()),
            this->translate(*problem.get_goal_condition()),
            (problem.get_optimization_metric().has_value() ?
                 std::optional<loki::pddl::OptimizationMetric>(this->translate(*problem.get_optimization_metric().value())) :
                 std::nullopt));
    }

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    loki::pddl::Problem run_base(const loki::pddl::ProblemImpl& problem) { return self().run_impl(problem); }

    loki::pddl::Problem run_impl(const loki::pddl::ProblemImpl& problem)
    {
        self().prepare(problem);
        return self().translate(problem);
    }
};

}

#endif
