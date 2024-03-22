#ifndef MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_INTERFACE_HPP_

#include <boost/container/small_vector.hpp>
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

    struct PrepareVisitor
    {
        ITranslator& m_translator;

        explicit PrepareVisitor(ITranslator& translator) : m_translator(translator) {}

        template<typename T>
        void operator()(const T& element) const
        {
            m_translator.prepare(element);
        }
    };

    struct TranslateVisitor
    {
        ITranslator& m_translator;

        explicit TranslateVisitor(ITranslator& translator) : m_translator(translator) {}

        template<typename T>
        auto operator()(const T& element) const
        {
            return m_translator.translate(element);
        }
    };

public:
    /// @brief Collect information.
    template<typename T>
    void prepare(const std::vector<const T*>& vec)
    {
        for (const auto ptr : vec)
        {
            self().prepare(*ptr);
        }
    }
    template<typename T, size_t N>
    void prepare(const boost::container::small_vector<const T*, N>& vec)
    {
        for (const auto ptr : vec)
        {
            self().prepare(*ptr);
        }
    }
    void prepare(const loki::pddl::RequirementsImpl& requirements) { self().prepare_base(requirements); }
    void prepare(const loki::pddl::TypeImpl& type) { self().prepare_base(type); }
    void prepare(const loki::pddl::ObjectImpl& object) { self().prepare_base(object); }
    void prepare(const loki::pddl::VariableImpl& variable) { self().prepare_base(variable); }
    void prepare(const loki::pddl::TermObjectImpl& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::TermVariableImpl& term) { self().prepare_base(term); }
    void prepare(const loki::pddl::TermImpl& term) { std::visit(PrepareVisitor(*this), term); }
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
    void prepare(const loki::pddl::ConditionImpl& condition) { std::visit(PrepareVisitor(*this), condition); }
    void prepare(const loki::pddl::EffectLiteralImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectAndImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectNumericImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalForallImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectConditionalWhenImpl& effect) { self().prepare_base(effect); }
    void prepare(const loki::pddl::EffectImpl& effect) { std::visit(PrepareVisitor(*this), effect); }
    void prepare(const loki::pddl::FunctionExpressionNumberImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) { self().prepare_base(function_expression); }
    void prepare(const loki::pddl::FunctionExpressionImpl& function_expression) { std::visit(PrepareVisitor(*this), function_expression); }
    void prepare(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { self().prepare_base(function_skeleton); }
    void prepare(const loki::pddl::FunctionImpl& function) { self().prepare_base(function); }
    void prepare(const loki::pddl::ActionImpl& action) { self().prepare_base(action); }
    void prepare(const loki::pddl::DomainImpl& domain) { self().prepare_base(domain); }
    void prepare(const loki::pddl::OptimizationMetricImpl& metric) { self().prepare_base(metric); }
    void prepare(const loki::pddl::ProblemImpl& problem) { self().prepare_base(problem); }

    /// @brief Apply problem translation.
    template<typename T>
    auto translate(const std::vector<const T*>& vec)
    {
        std::vector<const T*> result_vec;
        result_vec.reserve(vec.size());
        for (const auto ptr : vec)
        {
            result_vec.push_back(self().translate(*ptr));
        }
        return result_vec;
    }
    template<typename T, size_t N>
    auto translate(const boost::container::small_vector<const T*, N>& vec)
    {
        boost::container::small_vector<const T*, N> result_vec;
        result_vec.reserve(vec.size());
        for (const auto ptr : vec)
        {
            result_vec.push_back(self().translate(*ptr));
        }
        return result_vec;
    }
    loki::pddl::Requirements translate(const loki::pddl::RequirementsImpl& requirements) { return self().translate_base(requirements); }
    loki::pddl::Type translate(const loki::pddl::TypeImpl& type) { return self().translate_base(type); }
    loki::pddl::Object translate(const loki::pddl::ObjectImpl& object) { return self().translate_base(object); }
    loki::pddl::Variable translate(const loki::pddl::VariableImpl& variable) { return self().translate_base(variable); }
    loki::pddl::Term translate(const loki::pddl::TermObjectImpl& term) { return self().translate_base(term); }
    loki::pddl::Term translate(const loki::pddl::TermVariableImpl& term) { return self().translate_base(term); }
    loki::pddl::Term translate(const loki::pddl::TermImpl& term) { return std::visit(TranslateVisitor(*this), term); }
    loki::pddl::Parameter translate(const loki::pddl::ParameterImpl& parameter) { return self().translate_base(parameter); }
    loki::pddl::Parameter translate(const loki::pddl::PredicateImpl& predicate) { return self().translate_base(predicate); }
    loki::pddl::Atom translate(const loki::pddl::AtomImpl& atom) { return self().translate_base(atom); }
    loki::pddl::GroundAtom translate(const loki::pddl::GroundAtomImpl& atom) { return self().translate_base(atom); }
    loki::pddl::Literal translate(const loki::pddl::LiteralImpl& literal) { return self().translate_base(literal); }
    loki::pddl::GroundLiteral translate(const loki::pddl::GroundLiteralImpl& literal) { return self().translate_base(literal); }
    loki::pddl::NumericFluent translate(const loki::pddl::NumericFluentImpl& numeric_fluent) { return self().translate_base(numeric_fluent); }
    loki::pddl::Condition translate(const loki::pddl::ConditionLiteralImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionAndImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionOrImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionNotImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionImplyImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionExistsImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionForallImpl& condition) { return self().translate_base(condition); }
    loki::pddl::Condition translate(const loki::pddl::ConditionImpl& condition) { return std::visit(TranslateVisitor(*this), condition); }
    loki::pddl::Effect translate(const loki::pddl::EffectLiteralImpl& effect) { return self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectAndImpl& effect) { return self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectNumericImpl& effect) { return self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalForallImpl& effect) { return self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectConditionalWhenImpl& effect) { return self().translate_base(effect); }
    loki::pddl::Effect translate(const loki::pddl::EffectImpl& effect) { return std::visit(TranslateVisitor(*this), effect); }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionNumberImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionMinusImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionFunctionImpl& function_expression)
    {
        return self().translate_base(function_expression);
    }
    loki::pddl::FunctionExpression translate(const loki::pddl::FunctionExpressionImpl& function_expression)
    {
        return std::visit(TranslateVisitor(*this), function_expression);
    }
    loki::pddl::FunctionSkeleton translate(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { return self().translate_base(function_skeleton); }
    loki::pddl::Function translate(const loki::pddl::FunctionImpl& function) { return self().translate_base(function); }
    loki::pddl::Action translate(const loki::pddl::ActionImpl& action) { return self().translate_base(action); }
    loki::pddl::Domain translate(const loki::pddl::DomainImpl& domain) { return self().translate_base(domain); }
    loki::pddl::Action translate(const loki::pddl::OptimizationMetricImpl& metric) { return self().translate_base(metric); }
    loki::pddl::Problem translate(const loki::pddl::ProblemImpl& problem) { return self().translate_base(problem); }

    /// @brief Collect information and apply problem translation.
    loki::pddl::Problem run(const loki::pddl::ProblemImpl& problem, loki::PDDLFactories& ref_pddl_factories)
    {
        return self().run_base(problem, ref_pddl_factories);
    }
};

}

#endif
