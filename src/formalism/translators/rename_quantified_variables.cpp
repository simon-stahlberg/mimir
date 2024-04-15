#include "mimir/formalism/translators/rename_quantified_variables.hpp"

using namespace std::string_literals;

namespace mimir
{

RenameQuantifiedVariablesTranslator::Scope::Scope(std::unordered_map<loki::Variable, loki::Variable> renaming, const Scope* parent_scope) :
    m_renaming(std::move(renaming)),
    m_parent_scope(parent_scope)
{
}

const std::unordered_map<loki::Variable, loki::Variable>& RenameQuantifiedVariablesTranslator::Scope::get_renaming() const { return m_renaming; }

static loki::Variable create_renamed_variable(const loki::Variable& variable, size_t num_quantification, loki::PDDLFactories& pddl_factories)
{
    return pddl_factories.get_or_create_variable(variable->get_name() + "_" + std::to_string(variable->get_identifier()) + "_"
                                                 + std::to_string(num_quantification));
}

const RenameQuantifiedVariablesTranslator::Scope&
RenameQuantifiedVariablesTranslator::ScopeStack::open_scope(const loki::VariableList& variables,
                                                            std::unordered_map<loki::Variable, size_t>& num_quantifications,
                                                            loki::PDDLFactories& pddl_factories)
{
    assert(m_stack.empty());

    std::unordered_map<loki::Variable, loki::Variable> renamings;
    for (const auto& variable : variables)
    {
        num_quantifications.emplace(variable, 0);

        const auto renamed_variable = create_renamed_variable(variable, 0, pddl_factories);

        renamings.emplace(variable, renamed_variable);
    }

    m_stack.push_back(std::make_unique<Scope>(std::move(renamings)));

    return get();
}

const RenameQuantifiedVariablesTranslator::Scope&
RenameQuantifiedVariablesTranslator::ScopeStack::open_scope(const loki::ParameterList& parameters,
                                                            std::unordered_map<loki::Variable, size_t>& num_quantifications,
                                                            loki::PDDLFactories& pddl_factories)
{
    assert(!m_stack.empty());

    const auto& parent_renamings = get().get_renaming();

    std::unordered_map<loki::Variable, loki::Variable> renamings = parent_renamings;

    for (const auto& parameter : parameters)
    {
        // Increment number of quantifications of the variable.
        const auto renamed_variable = create_renamed_variable(parameter->get_variable(), ++num_quantifications.at(parameter->get_variable()), pddl_factories);

        renamings[parameter->get_variable()] = renamed_variable;
    }

    m_stack.push_back(std::make_unique<Scope>(std::move(renamings), &get()));

    return get();
}

void RenameQuantifiedVariablesTranslator::ScopeStack::close_scope()
{
    assert(!m_stack.empty());
    m_stack.pop_back();
}

const RenameQuantifiedVariablesTranslator::Scope& RenameQuantifiedVariablesTranslator::ScopeStack::get() const { return *m_stack.back(); }

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
