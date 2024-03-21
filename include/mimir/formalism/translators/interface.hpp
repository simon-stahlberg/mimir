#ifndef MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_

#include <loki/pddl/declarations.hpp>
#include <loki/pddl/factories.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Interface class.
 */
template<typename Derived>
class ITranslator
{
private:
    ITranslator() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Collect information.
    void prepare(const loki::pddl::TypeImpl& type) { self().prepare_base(type); }
    void prepare(const loki::pddl::ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const loki::pddl::VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const loki::pddl::TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::TermImpl& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::ParameterImpl& parameter) { self().prepare_base(parameter); }
    void prepare(const loki::pddl::PredicateImpl& predicate) { self().prepare_base(predicate); }
    void prepare(const loki::pddl::AtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const loki::pddl::GroundAtomImpl& atom) { self().prepare_base(atom); }
    void prepare(const loki::pddl::LiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const loki::pddl::NumericFluentImpl& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const loki::pddl::GroundLiteralImpl& literal) { self().prepare_base(literal); }
    void prepare(const loki::pddl::ConditionLiteralImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionAndImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionOrImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionNotImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionImplyImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionExistsImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionForallImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionImpl& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::EffectLiteralImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectAndImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectNumericImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalForallImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalWhenImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const loki::pddl::FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const loki::pddl::ActionImpl& action) { self().prepare_base(action); }
    void prepare(const loki::pddl::DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const loki::pddl::OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const loki::pddl::ProblemImpl& problem) { self().prepare_base(problem); }

    /// @brief Apply problem translation.
    loki::pddl::Type translate(const loki::pddl::TypeImpl& type) { self().translate_base(type); }
    loki::pddl::Object translate(const loki::pddl::ObjectImpl& object) { self().translate_base(object); }
    loki::pddl::Variable translate(const loki::pddl::VariableImpl& variable) { self().translate_base(variable); }
    loki::pddl::Term translate(const loki::pddl::TermObjectImpl& term) { self().translate_base(term); }
    loki::pddl::Term translate(const loki::pddl::TermVariableImpl& term) { self().translate_base(term); }
    loki::pddl::Term translate(const loki::pddl::TermImpl& term) { self().translate_base(term); }
    loki::pddl::Parameter translate(const loki::pddl::ParameterImpl& parameter) { self().translate_base(parameter); }
    loki::pddl::Parameter translate(const loki::pddl::PredicateImpl& predicate) { self().translate_base(predicate); }
    loki::pddl::Atom translate(const loki::pddl::AtomImpl& atom) { self().translate_base(atom); }
    loki::pddl::GroundAtom translate(const loki::pddl::GroundAtomImpl& atom) { self().translate_base(atom); }
    loki::pddl::Literal translate(const loki::pddl::LiteralImpl& literal) { self().translate_base(literal); }
    loki::pddl::GroundLiteral translate(const loki::pddl::GroundLiteralImpl& literal) { self().translate_base(literal); }
    loki::pddl::NumericFluent translate(const loki::pddl::NumericFluentImpl& numeric_fluent) { self().translate_base(numeric_fluent); }
    loki::pddl::Condition translate(const loki::pddl::ConditionLiteralImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionAndImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionOrImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionNotImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionImplyImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionExistsImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionForallImpl& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionImpl& condition) { self().translate_base(condition); }
    loki::pddl::Effect translate(const loki::pddl::EffectLiteralImpl& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectAndImpl& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectNumericImpl& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalForallImpl& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalWhenImpl& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectImpl& effect) { self().translate_base(effect); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionNumberImpl& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionFunctionImpl& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionImpl& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionSkeleton translate(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { self().translate_base(function_skeleton); }
    loki::pddl::Function translate(const loki::pddl::FunctionImpl& function) { self().translate_base(function); }
    loki::pddl::Action translate(const loki::pddl::ActionImpl& action) { self().translate_base(action); }
    loki::pddl::Domain translate(const loki::pddl::DomainImpl& domain) { self().translate_base(domain); }
    loki::pddl::Action translate(const loki::pddl::OptimizationMetricImpl& metric) { self().translate_base(metric); }
    loki::pddl::Problem translate(const loki::pddl::ProblemImpl& problem) { self().translate_base(problem); }

    /// @brief Collect information and apply problem translation.
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run(const loki::pddl::ProblemImpl& problem) { self().run_base(problem); }
};

}

#endif
