#include "mimir/formalism/translators/move_existential_quantifiers.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

loki::Condition MoveExistentialQuantifiersTranslator::translate_impl(const loki::ConditionAndImpl& condition)
{
    const auto& translated_nested_conditions = this->translate(condition.get_conditions());

    auto parameters = loki::ParameterList {};
    auto parts = loki::ConditionList {};
    for (const auto translated_nested_condition : translated_nested_conditions)
    {
        if (const auto translated_nested_condition_exists = std::get_if<loki::ConditionExistsImpl>(translated_nested_condition))
        {
            parameters.insert(parameters.end(),
                              translated_nested_condition_exists->get_parameters().begin(),
                              translated_nested_condition_exists->get_parameters().end());

            parts.push_back(translated_nested_condition_exists->get_condition());
        }
        else
        {
            parts.push_back(translated_nested_condition);
        }
    }

    const auto parts_conjunction = this->m_pddl_factories.get_or_create_condition_and(parts);

    if (parameters.empty())
    {
        return parts_conjunction;
    }

    return this->translate(*this->m_pddl_factories.get_or_create_condition_exists(parameters, parts_conjunction));
}

loki::Condition MoveExistentialQuantifiersTranslator::translate_impl(const loki::ConditionExistsImpl& condition)
{
    return flatten(std::get<loki::ConditionExistsImpl>(*this->m_pddl_factories.get_or_create_condition_exists(this->translate(condition.get_parameters()),
                                                                                                              this->translate(*condition.get_condition()))),
                   this->m_pddl_factories);
}

loki::Problem MoveExistentialQuantifiersTranslator::run_impl(const loki::ProblemImpl& problem) { return this->translate(problem); }

MoveExistentialQuantifiersTranslator::MoveExistentialQuantifiersTranslator(loki::PDDLFactories& pddl_factories) : BaseTranslator(pddl_factories) {}

}