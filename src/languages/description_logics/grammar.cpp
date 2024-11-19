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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/languages/description_logics/grammar.hpp"

#include "parser.hpp"

namespace mimir::dl::grammar
{

Grammar::Grammar(std::string bnf_description, Domain domain) :
    m_grammar_constructor_repos(create_default_constructor_type_to_repository()),
    m_primitive_production_rules(),
    m_composite_production_rules(),
    m_alternative_rules()
{
    const auto rules = parse(bnf_description, domain, m_grammar_constructor_repos);
    // m_concept_rules = std::move(concept_rules);
    // m_role_rules = std::move(role_rules);
}

template<ConstructorTag D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    return std::any_of(get_primitive_production_rules<D>().begin(),
                       get_primitive_production_rules<D>().end(),
                       [&constructor](const auto& rule) { return rule->test_match(constructor); })
           || std::any_of(get_composite_production_rules<D>().begin(),
                          get_composite_production_rules<D>().end(),
                          [&constructor](const auto& rule) { return rule->test_match(constructor); })
           || std::any_of(get_alternative_rules<D>().begin(),
                          get_alternative_rules<D>().end(),
                          [&constructor](const auto& rule) { return rule->test_match(constructor); });
}

template bool Grammar::test_match(dl::Constructor<Concept> constructor) const;
template bool Grammar::test_match(dl::Constructor<Role> constructor) const;

template<ConstructorTag D>
const DerivationRuleList<D>& Grammar::get_primitive_production_rules() const
{
    return boost::hana::at_key(m_primitive_production_rules, boost::hana::type<D> {});
}

template const DerivationRuleList<Concept>& Grammar::get_primitive_production_rules() const;
template const DerivationRuleList<Role>& Grammar::get_primitive_production_rules() const;

template<ConstructorTag D>
const DerivationRuleList<D>& Grammar::get_composite_production_rules() const
{
    return boost::hana::at_key(m_composite_production_rules, boost::hana::type<D> {});
}

template const DerivationRuleList<Concept>& Grammar::get_composite_production_rules() const;
template const DerivationRuleList<Role>& Grammar::get_composite_production_rules() const;

template<ConstructorTag D>
const DerivationRuleList<D>& Grammar::get_alternative_rules() const
{
    return boost::hana::at_key(m_alternative_rules, boost::hana::type<D> {});
}

template const DerivationRuleList<Concept>& Grammar::get_alternative_rules() const;
template const DerivationRuleList<Role>& Grammar::get_alternative_rules() const;

}
