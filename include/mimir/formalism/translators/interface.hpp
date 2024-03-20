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
    void prepare(const loki::pddl::Type& type) { self().prepare_base(type); }
    void prepare(const loki::pddl::Object& object) { self().prepare_base(object); }
    void prepare(const loki::pddl::Variable& variable) { self().prepare_base(variable); }
    void prepare(const loki::pddl::TermObject& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::TermVariable& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::Term& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::Parameter& parameter) { self().prepare_base(parameter); }
    void prepare(const loki::pddl::Predicate& predicate) { self().prepare_base(predicate); }
    void prepare(const loki::pddl::Atom& atom) { self().prepare_base(atom); }
    void prepare(const loki::pddl::GroundAtom& atom) { self().prepare_base(atom); }
    void prepare(const loki::pddl::Literal& literal) { self().prepare_base(literal); }
    void prepare(const loki::pddl::NumericFluent& numeric_fluent) { self().prepare_base(numeric_fluent); }
    void prepare(const loki::pddl::GroundLiteral& literal) { self().prepare_base(literal); }
    void prepare(const loki::pddl::ConditionLiteral& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionAnd& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionOr& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionNot& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionImply& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionExists& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::ConditionForall& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::Condition& condition) { self().prepare_base(condition); }
    void prepare(const loki::pddl::EffectLiteral& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectAnd& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectNumeric& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalForall& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalWhen& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::Effect& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::FunctionExpressionNumber& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionBinaryOperator& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMultiOperator& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMinus& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionFunction& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpression& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionSkeleton& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const loki::pddl::Function& function) { self().prepare_base(function); }
    void prepare(const loki::pddl::Action& action) { self().prepare_base(action); }
    void prepare(const loki::pddl::Domain& domain) { self().prepare_base(domain); }
    void prepare(const loki::pddl::OptimizationMetric& metric) { self().prepare_base(metric); }
    void prepare(const loki::pddl::Problem& problem) { self().prepare_base(problem); }

    /// @brief Apply problem translation.
    loki::pddl::Type translate(const loki::pddl::Type& type) { self().translate_base(type); }
    loki::pddl::Object translate(const loki::pddl::Object& object) { self().translate_base(object); }
    loki::pddl::Variable translate(const loki::pddl::Variable& variable) { self().translate_base(variable); }
    loki::pddl::TermObject translate(const loki::pddl::TermObject& term) { self().translate_base(term); }
    loki::pddl::TermVariable translate(const loki::pddl::TermVariable& term) { self().translate_base(term); }
    loki::pddl::Term translate(const loki::pddl::Term& term) { self().translate_base(term); }
    loki::pddl::Parameter translate(const loki::pddl::Parameter& parameter) { self().translate_base(parameter); }
    loki::pddl::Parameter translate(const loki::pddl::Predicate& predicate) { self().translate_base(predicate); }
    loki::pddl::Atom translate(const loki::pddl::Atom& atom) { self().translate_base(atom); }
    loki::pddl::GroundAtom translate(const loki::pddl::GroundAtom& atom) { self().translate_base(atom); }
    loki::pddl::Literal translate(const loki::pddl::Literal& literal) { self().translate_base(literal); }
    loki::pddl::GroundLiteral translate(const loki::pddl::GroundLiteral& literal) { self().translate_base(literal); }
    loki::pddl::NumericFluent translate(const loki::pddl::NumericFluent& numeric_fluent) { self().translate_base(numeric_fluent); }
    loki::pddl::Condition translate(const loki::pddl::ConditionLiteral& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionAnd& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionOr& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionNot& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionImply& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionExists& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionForall& condition) { self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::Condition& condition) { self().translate_base(condition); }
    loki::pddl::Effect translate(const loki::pddl::EffectLiteral& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectAnd& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectNumeric& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalForall& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalWhen& effect) { self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::Effect& effect) { self().translate_base(effect); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionNumber& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionBinaryOperator& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMultiOperator& function_expression)
    {
        self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMinus& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionFunction& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpression& function_expression) { self().translate_base(function_expression); }
    loki::pddl::FunctionSkeleton translate(const loki::pddl::FunctionSkeleton& function_skeleton) { self().translate_base(function_skeleton); }
    loki::pddl::Function translate(const loki::pddl::Function& function) { self().translate_base(function); }
    loki::pddl::Action translate(const loki::pddl::Action& action) { self().translate_base(action); }
    loki::pddl::Domain translate(const loki::pddl::Domain& domain) { self().translate_base(domain); }
    loki::pddl::Action translate(const loki::pddl::OptimizationMetric& metric) { self().translate_base(metric); }
    loki::pddl::Problem translate(const loki::pddl::Problem& problem) { self().translate_base(problem); }

    /// @brief Collect information and apply problem translation.
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run(const loki::pddl::Problem& problem) { self().run_base(problem); }
};

}

#endif
