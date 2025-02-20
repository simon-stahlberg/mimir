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

#include "mimir/languages/description_logics/cnf_grammar.hpp"

#include "parser.hpp"

namespace mimir::dl::cnf_grammar
{

Grammar::Grammar(const grammar::Grammar& grammar)
{
    /* 1. Substitute primitives in complicated rules. */

    /* 2. Decompose complicated rules into simple rules. */
}

template<ConceptOrRole D>
bool Grammar::test_match(dl::Constructor<D> constructor) const
{
    const auto& start_symbol = get_start_symbol<D>();

    if (!start_symbol)
    {
        return false;  ///< sentence is not part of language.
    }

    // Check whether constructor matches primitive rules.
    const auto& primitive_rules = get_rules<D, Primitive>(start_symbol.value());

    if (std::any_of(primitive_rules.begin(), primitive_rules.end(), [&, constructor](auto&& rule) { return rule->test_match(constructor, *this); }))
    {
        return true;
    }

    // Check whether constructor matches composite rules.
    const auto& composite_rules = get_rules<D, Composite>(start_symbol.value());

    if (std::any_of(composite_rules.begin(), composite_rules.end(), [&, constructor](auto&& rule) { return rule->test_match(constructor, *this); }))
    {
        return true;
    }

    return false;
}

template bool Grammar::test_match(dl::Constructor<Concept> constructor) const;
template bool Grammar::test_match(dl::Constructor<Role> constructor) const;

}
