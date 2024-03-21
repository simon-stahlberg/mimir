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

public:
    /// @brief Collect information.
    ///        Default behavior recursively calls prepare.
    void prepare_base(const loki::pddl::TypeImpl& type) { self().prepare_impl(type); }
    void prepare_base(const loki::pddl::ObjectImpl& object) { self().prepare_impl(object); }
    void prepare_base(const loki::pddl::VariableImpl& variable) { self().prepare_impl(variable); }
    void prepare_base(const loki::pddl::TermObjectImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::pddl::TermVariableImpl& term) { self().prepare_impl(term); }
    void prepare_base(const loki::pddl::TermImpl& term) { self().prepare_impl(term); }
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
    void prepare_base(const loki::pddl::ConditionImpl& condition) { self().prepare_impl(condition); }
    void prepare_base(const loki::pddl::EffectLiteralImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectAndImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectNumericImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectConditionalForallImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectConditionalWhenImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::EffectImpl& effect) { self().prepare_impl(effect); }
    void prepare_base(const loki::pddl::FunctionExpressionNumberImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionMinusImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionExpressionImpl& function_expression) { self().prepare_impl(function_expression); }
    void prepare_base(const loki::pddl::FunctionSkeletonImpl& function_skeleton) { self().prepare_impl(function_skeleton); }
    void prepare_base(const loki::pddl::FunctionImpl& function) { self().prepare_impl(function); }
    void prepare_base(const loki::pddl::ActionImpl& action) { self().prepare_impl(action); }
    void prepare_base(const loki::pddl::DomainImpl& domain) { self().prepare_impl(domain); }
    void prepare_base(const loki::pddl::OptimizationMetricImpl& metric) { self().prepare_impl(metric); }
    void prepare_base(const loki::pddl::ProblemImpl& problem) { self().prepare_impl(problem); }

    template<typename T>
    void prepare_impl(const std::vector<T>& vec)
    {
        for (const auto& element : vec)
        {
            this->prepare(element);
        }
    }
    void prepare_impl(const loki::pddl::TypeImpl& type) {}
    void prepare_impl(const loki::pddl::ObjectImpl& object) {}
    void prepare_impl(const loki::pddl::VariableImpl& variable) {}
    void prepare_impl(const loki::pddl::TermObjectImpl& term) {}
    void prepare_impl(const loki::pddl::TermVariableImpl& term) {}
    void prepare_impl(const loki::pddl::TermImpl& term) {}
    void prepare_impl(const loki::pddl::ParameterImpl& parameter) {}
    void prepare_impl(const loki::pddl::PredicateImpl& predicate) {}
    void prepare_impl(const loki::pddl::AtomImpl& atom) {}
    void prepare_impl(const loki::pddl::GroundAtomImpl& atom) {}
    void prepare_impl(const loki::pddl::LiteralImpl& literal) {}
    void prepare_impl(const loki::pddl::NumericFluentImpl& numeric_fluent) {}
    void prepare_impl(const loki::pddl::GroundLiteralImpl& literal) {}
    void prepare_impl(const loki::pddl::ConditionLiteralImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionAndImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionOrImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionNotImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionImplyImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionExistsImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionForallImpl& condition) {}
    void prepare_impl(const loki::pddl::ConditionImpl& condition) {}
    void prepare_impl(const loki::pddl::EffectLiteralImpl& effect) {}
    void prepare_impl(const loki::pddl::EffectAndImpl& effect) {}
    void prepare_impl(const loki::pddl::EffectNumericImpl& effect) {}
    void prepare_impl(const loki::pddl::EffectConditionalForallImpl& effect) {}
    void prepare_impl(const loki::pddl::EffectConditionalWhenImpl& effect) {}
    void prepare_impl(const loki::pddl::EffectImpl& effect) {}
    void prepare_impl(const loki::pddl::FunctionExpressionNumberImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionBinaryOperatorImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionMultiOperatorImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionMinusImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionFunctionImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionExpressionImpl& function_expression) {}
    void prepare_impl(const loki::pddl::FunctionSkeletonImpl& function_skeleton) {}
    void prepare_impl(const loki::pddl::FunctionImpl& function) {}
    void prepare_impl(const loki::pddl::ActionImpl& action)
    {
        this->prepare(action.get_parameters());
        this->prepare(action.get_condition());
        this->prepare(action.get_effect());
    }
    void prepare_impl(const loki::pddl::DomainImpl& domain)
    {
        this->prepare(domain.get_requirements());
        this->prepare(domain.get_types());
        this->prepare(domain.get_constants());
        this->prepare(domain.get_predicates());
        this->prepare(domain.get_functions());
        this->prepare(domain.get_actions());
    }
    void prepare_impl(const loki::pddl::OptimizationMetricImpl& metric) { this->prepare(metric.get_function_expression()); }
    void prepare_impl(const loki::pddl::ProblemImpl& problem)
    {
        this->prepare(problem.get_domain());
        this->prepare(problem.get_requirements());
        this->prepare(problem.get_objects());
        this->prepare(problem.get_initial_literals());
        this->prepare(problem.get_numeric_fluents());
        this->prepare(problem.get_goal_condition());
        this->prepare(problem.get_optimization_metric());
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    loki::pddl::Domain translate_base(const loki::pddl::DomainImpl& domain) { self().translate_impl(domain); }
    loki::pddl::Action translate_base(const loki::pddl::ActionImpl& action) { self().translate_impl(action); }
    loki::pddl::Problem translate_base(const loki::pddl::ProblemImpl& problem) { self().translate_impl(problem); }

    loki::pddl::Action translate_impl(const loki::pddl::ActionImpl& action) {}
    loki::pddl::Domain translate_impl(const loki::pddl::DomainImpl& domain) {}
    loki::pddl::Problem translate_impl(const loki::pddl::ProblemImpl& problem) {}

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_base(const loki::pddl::ProblemImpl& problem) { self().run_impl(problem); }
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_impl(const loki::pddl::ProblemImpl& problem)
    {
        auto pddl_factories = loki::PDDLFactories();
        self().prepare(problem);
        auto translated_problem = self().translate(problem);
        return std::make_tuple(translated_problem, std::move(pddl_factories));
    }
};

}

#endif
