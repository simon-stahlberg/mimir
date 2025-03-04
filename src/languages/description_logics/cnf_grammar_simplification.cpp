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

#include "cnf_grammar_simplification.hpp"

namespace mimir::dl::cnf_grammar
{

Grammar eliminate_rules_with_identical_body(const Grammar& grammar)
{
    auto repositories = cnf_grammar::ConstructorRepositories();
    auto start_symbols = cnf_grammar::StartSymbolsContainer();
    auto derivation_rules = cnf_grammar::DerivationRulesContainer();
    auto substitution_rules = cnf_grammar::SubstitutionRulesContainer();

    using HanaInverseDerivationRules =
        boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::unordered_map<Constructor<Concept>, NonTerminalList<Concept>>>,
                         boost::hana::pair<boost::hana::type<Role>, std::unordered_map<Constructor<Role>, NonTerminalList<Role>>>>;

    auto inverse_derivation_rules = HanaInverseDerivationRules();
}

Grammar simplify(const Grammar& grammar)
{
    /* Step 1 Identify rules with same body */

    /* Step 2 resubstitute substitution rules using fixed point */

    // If right handside of substitution rule has a single rule

    /* Step 3 order substitution rules in order of evaluation through topological sorting */

    /* Step 4 remove rules of unreachable non-terminals */
}
}
