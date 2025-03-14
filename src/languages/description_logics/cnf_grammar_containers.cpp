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

#include "mimir/languages/description_logics/cnf_grammar_containers.hpp"

namespace mimir::languages::dl::cnf_grammar
{
const DerivationRuleLists<Concept, Role, Boolean, Numerical> DerivationRulesContainer::empty_lists =
    boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<Concept>, DerivationRuleList<Concept> {}),
                          boost::hana::make_pair(boost::hana::type_c<Role>, DerivationRuleList<Role> {}),
                          boost::hana::make_pair(boost::hana::type_c<Boolean>, DerivationRuleList<Boolean> {}),
                          boost::hana::make_pair(boost::hana::type_c<Numerical>, DerivationRuleList<Numerical> {}));

const SubstitutionRuleLists<Concept, Role, Boolean, Numerical> SubstitutionRulesContainer::empty_lists =
    boost::hana::make_map(boost::hana::make_pair(boost::hana::type_c<Concept>, SubstitutionRuleList<Concept> {}),
                          boost::hana::make_pair(boost::hana::type_c<Role>, SubstitutionRuleList<Role> {}),
                          boost::hana::make_pair(boost::hana::type_c<Boolean>, SubstitutionRuleList<Boolean> {}),
                          boost::hana::make_pair(boost::hana::type_c<Numerical>, SubstitutionRuleList<Numerical> {}));
}
