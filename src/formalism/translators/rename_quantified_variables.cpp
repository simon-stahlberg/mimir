#include "mimir/formalism/translators/rename_quantified_variables.hpp"

namespace mimir
{

void RenameQuantifiedVariablesTranslator::prepare_impl(const loki::VariableImpl& variable) { m_variables.insert(&variable); }

loki::Variable RenameQuantifiedVariablesTranslator::translate_impl(const loki::VariableImpl& variable) { return m_scopes.get().get_renaming().at(&variable); }

loki::Action RenameQuantifiedVariablesTranslator::translate_impl(const loki::ActionImpl& action)
{
    m_scopes.open_scope(action.get_parameters(), this->m_num_quantifications, this->m_pddl_factories);

    const auto translated_parameters = this->translate(action.get_parameters());
    const auto translated_conditions =
        (action.get_condition().has_value() ? std::optional<loki::Condition>(this->translate(*action.get_condition().value())) : std::nullopt);
    const auto translated_effect =
        (action.get_effect().has_value() ? std::optional<loki::Effect>(this->translate(*action.get_effect().value())) : std::nullopt);

    m_scopes.close_scope();

    return this->m_pddl_factories.get_or_create_action(action.get_name(), translated_parameters, translated_conditions, translated_effect);
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    m_scopes.open_scope(condition.get_parameters(), this->m_num_quantifications, this->m_pddl_factories);

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    m_scopes.close_scope();

    return this->m_pddl_factories.get_or_create_condition_exists(translated_parameters, translated_nested_condition);
}

loki::Condition RenameQuantifiedVariablesTranslator::translate_impl(const loki::ConditionForallImpl& condition)
{
    m_scopes.open_scope(condition.get_parameters(), this->m_num_quantifications, this->m_pddl_factories);

    const auto translated_parameters = this->translate(condition.get_parameters());
    const auto translated_nested_condition = this->translate(*condition.get_condition());

    m_scopes.close_scope();

    return this->m_pddl_factories.get_or_create_condition_forall(translated_parameters, translated_nested_condition);
}

loki::Effect RenameQuantifiedVariablesTranslator::translate_impl(const loki::EffectConditionalForallImpl& effect)
{
    m_scopes.open_scope(effect.get_parameters(), this->m_num_quantifications, this->m_pddl_factories);

    const auto translated_parameters = this->translate(effect.get_parameters());
    const auto translated_nested_effect = this->translate(*effect.get_effect());

    m_scopes.close_scope();

    return this->m_pddl_factories.get_or_create_effect_conditional_forall(translated_parameters, translated_nested_effect);
}

loki::Problem RenameQuantifiedVariablesTranslator::run_impl(const loki::ProblemImpl& problem)
{
    this->prepare(problem);

    m_scopes.open_scope(loki::VariableList(m_variables.begin(), m_variables.end()), this->m_num_quantifications, this->m_pddl_factories);

    auto translated_problem = this->translate(problem);

    m_scopes.close_scope();

    return translated_problem;
}

RenameQuantifiedVariablesTranslator::RenameQuantifiedVariablesTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}
