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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EQUAL_TO_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EQUAL_TO_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

#include <functional>
#include <variant>

namespace mimir::dl
{

/// @brief `UniqueDLEqualTo` is used to compare newly created dl constructors for uniqueness.
/// Since the children are unique, it suffices to compare nested pointers.
template<typename T>
struct UniqueDLEqualTo
{
    bool operator()(const T& l, const T& r) const { return std::equal_to<T>()(l, r); }
};

/// Spezialization for std::variant.
template<typename... Ts>
struct UniqueDLEqualTo<std::variant<Ts...>>
{
    bool operator()(const std::variant<Ts...>& l, const std::variant<Ts...>& r) const
    {
        if (l.index() != r.index())
        {
            return false;  // Different types held
        }
        // Compare the held values, but only if they are of the same type
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
                {
                    return UniqueDLEqualTo<decltype(lhs)>()(lhs, rhs);
                }
                else
                {
                    return false;  // Different types can't be equal
                }
            },
            l,
            r);
    }
};

/**
 * DL constructors
 */

/* Concepts */

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateStateImpl<P>*>
{
    bool operator()(const ConceptPredicateStateImpl<P>* l, const ConceptPredicateStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateGoalImpl<P>*>
{
    bool operator()(const ConceptPredicateGoalImpl<P>* l, const ConceptPredicateGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const ConceptAndImpl*>
{
    bool operator()(const ConceptAndImpl* l, const ConceptAndImpl* r) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateStateImpl<P>*>
{
    bool operator()(const RolePredicateStateImpl<P>* l, const RolePredicateStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateGoalImpl<P>*>
{
    bool operator()(const RolePredicateGoalImpl<P>* l, const RolePredicateGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const RoleAndImpl*>
{
    bool operator()(const RoleAndImpl* l, const RoleAndImpl* r) const;
};

/**
 * DL grammar constructors
 */

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::DerivationRuleImpl<D>*>
{
    bool operator()(const grammar::DerivationRuleImpl<D>* l, const grammar::DerivationRuleImpl<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::NonTerminalImpl<D>*>
{
    bool operator()(const grammar::NonTerminalImpl<D>* l, const grammar::NonTerminalImpl<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::ConstructorOrNonTerminalChoice<D>*>
{
    bool operator()(const grammar::ConstructorOrNonTerminalChoice<D>* l, const grammar::ConstructorOrNonTerminalChoice<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const grammar::ChoiceImpl<D>*>
{
    bool operator()(const grammar::ChoiceImpl<D>* l, const grammar::ChoiceImpl<D>* r) const;
};

/* Concepts */

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::ConceptPredicateStateImpl<P>*>
{
    bool operator()(const grammar::ConceptPredicateStateImpl<P>* l, const grammar::ConceptPredicateStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::ConceptPredicateGoalImpl<P>*>
{
    bool operator()(const grammar::ConceptPredicateGoalImpl<P>* l, const grammar::ConceptPredicateGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::ConceptAndImpl*>
{
    bool operator()(const grammar::ConceptAndImpl* l, const grammar::ConceptAndImpl* r) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::RolePredicateStateImpl<P>*>
{
    bool operator()(const grammar::RolePredicateStateImpl<P>* l, const grammar::RolePredicateStateImpl<P>* r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const grammar::RolePredicateGoalImpl<P>*>
{
    bool operator()(const grammar::RolePredicateGoalImpl<P>* l, const grammar::RolePredicateGoalImpl<P>* r) const;
};

template<>
struct UniqueDLEqualTo<const grammar::RoleAndImpl*>
{
    bool operator()(const grammar::RoleAndImpl* l, const grammar::RoleAndImpl* r) const;
};

}

#endif
