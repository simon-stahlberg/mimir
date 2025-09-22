/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/formalism/translator/collect_statistics.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/numeric_constraint.hpp"
#include "mimir/formalism/predicate.hpp"

namespace mimir::formalism
{

void CollectStatisticsTranslator::prepare_level_2(NumericConstraint condition)
{
    const auto arity = condition->get_terms().size();
    (arity > m_max_arity) ? ++m_num_constraints_by_oob_arity : ++m_num_constraints_by_arity[arity];

    this->prepare_level_0(condition->get_left_function_expression());
    this->prepare_level_0(condition->get_right_function_expression());
    this->prepare_level_0(condition->get_terms());
}

void CollectStatisticsTranslator::prepare_level_2(Action action)
{
    const auto arity = action->get_conjunctive_condition()->get_arity();
    (arity > m_max_arity) ? ++m_num_actions_by_oob_arity : ++m_num_actions_by_arity[arity];

    this->prepare_level_0(action->get_conjunctive_condition());
    this->prepare_level_0(action->get_conditional_effects());
}

void CollectStatisticsTranslator::prepare_level_2(Axiom axiom)
{
    const auto arity = axiom->get_conjunctive_condition()->get_arity();
    (arity > m_max_arity) ? ++m_num_axioms_by_oob_arity : ++m_num_axioms_by_arity[arity];

    this->prepare_level_0(axiom->get_conjunctive_condition());
    this->prepare_level_0(axiom->get_literal());
}

void CollectStatisticsTranslator::prepare_level_2(const DomainImpl* domain)
{
    boost::hana::for_each(domain->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              for (const auto& predicate : boost::hana::second(pair))
                              {
                                  const auto arity = predicate->get_arity();
                                  (arity > m_max_arity) ? ++m_num_predicates_by_oob_arity : ++m_num_predicates_by_arity[arity];
                              }
                          });

    boost::hana::for_each(domain->get_hana_function_skeletons(),
                          [&](auto&& pair)
                          {
                              for (const auto& function : boost::hana::second(pair))
                              {
                                  const auto arity = function->get_arity();
                                  (arity > m_max_arity) ? m_num_functions_by_oob_arity : ++m_num_functions_by_arity[arity];
                              }
                          });

    this->prepare_level_0(domain->get_requirements());
    this->prepare_level_0(domain->get_constants());
    this->prepare_level_0(domain->get_predicates<StaticTag>());
    this->prepare_level_0(domain->get_predicates<FluentTag>());
    this->prepare_level_0(domain->get_predicates<DerivedTag>());
    this->prepare_level_0(domain->get_function_skeletons<StaticTag>());
    this->prepare_level_0(domain->get_function_skeletons<FluentTag>());
    this->prepare_level_0(domain->get_auxiliary_function_skeleton());
    this->prepare_level_0(domain->get_actions());
    this->prepare_level_0(domain->get_axioms());
}

CollectStatisticsTranslator::CollectStatisticsTranslator() :
    m_num_predicates_by_arity(m_max_arity + 1, 0),
    m_num_predicates_by_oob_arity(0),
    m_num_functions_by_arity(m_max_arity + 1, 0),
    m_num_functions_by_oob_arity(0),
    m_num_constraints_by_arity(m_max_arity + 1, 0),
    m_num_constraints_by_oob_arity(0),
    m_num_actions_by_arity(m_max_arity + 1, 0),
    m_num_actions_by_oob_arity(0),
    m_num_axioms_by_arity(m_max_arity + 1, 0),
    m_num_axioms_by_oob_arity(0)
{
}

extern std::ostream& operator<<(std::ostream& out, const CollectStatisticsTranslator& translator)
{
    std::cout << "Num predicates by arity: ";
    mimir::operator<<(std::cout, translator.m_num_predicates_by_arity);
    std::cout << std::endl;
    std::cout << "Num predicates by oob arity: " << translator.m_num_predicates_by_oob_arity << std::endl;

    std::cout << "Num functions by arity: ";
    mimir::operator<<(std::cout, translator.m_num_functions_by_arity);
    std::cout << std::endl;
    std::cout << "Num functions by oob arity: " << translator.m_num_functions_by_oob_arity << std::endl;

    std::cout << "Num constraints by arity: ";
    mimir::operator<<(std::cout, translator.m_num_constraints_by_arity);
    std::cout << std::endl;
    std::cout << "Num constraints by oob arity: " << translator.m_num_constraints_by_oob_arity << std::endl;

    std::cout << "Num actions by arity: ";
    mimir::operator<<(std::cout, translator.m_num_actions_by_arity);
    std::cout << std::endl;
    std::cout << "Num actions by oob arity: " << translator.m_num_actions_by_oob_arity << std::endl;

    std::cout << "Num axioms by arity: ";
    mimir::operator<<(std::cout, translator.m_num_axioms_by_arity);
    std::cout << std::endl;
    std::cout << "Num axioms by oob arity: " << translator.m_num_axioms_by_oob_arity << std::endl;

    return out;
}

}
