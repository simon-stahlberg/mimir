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

namespace mimir
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

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const Constructor<D>*>
{
    bool operator()(const Constructor<D>* l, const Constructor<D>* r) const;
};

/* Concepts */

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateState<P>&>
{
    bool operator()(const ConceptPredicateState<P>& l, const ConceptPredicateState<P>& r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateGoal<P>&>
{
    bool operator()(const ConceptPredicateGoal<P>& l, const ConceptPredicateGoal<P>& r) const;
};

template<>
struct UniqueDLEqualTo<const ConceptPredicateAnd&>
{
    bool operator()(const ConceptPredicateAnd& l, const ConceptPredicateAnd& r) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateState<P>&>
{
    bool operator()(const RolePredicateState<P>& l, const RolePredicateState<P>& r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateGoal<P>&>
{
    bool operator()(const RolePredicateGoal<P>& l, const RolePredicateGoal<P>& r) const;
};

template<>
struct UniqueDLEqualTo<const RolePredicateAnd&>
{
    bool operator()(const RolePredicateAnd& l, const RolePredicateAnd& r) const;
};

/**
 * DL grammar constructors
 */

namespace grammar
{

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const Constructor<D>*>
{
    bool operator()(const Constructor<D>* l, const Constructor<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const DerivationRule<D>*>
{
    bool operator()(const DerivationRule<D>* l, const DerivationRule<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const NonTerminal<D>*>
{
    bool operator()(const NonTerminal<D>* l, const NonTerminal<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const ConstructorOrNonTerminalChoice<D>*>
{
    bool operator()(const ConstructorOrNonTerminalChoice<D>* l, const ConstructorOrNonTerminalChoice<D>* r) const;
};

template<IsConceptOrRole D>
struct UniqueDLEqualTo<const Choice<D>*>
{
    bool operator()(const Choice<D>* l, const Choice<D>* r) const;
};

/* Concepts */

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateState<P>&>
{
    bool operator()(const ConceptPredicateState<P>& l, const ConceptPredicateState<P>& r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const ConceptPredicateGoal<P>&>
{
    bool operator()(const ConceptPredicateGoal<P>& l, const ConceptPredicateGoal<P>& r) const;
};

template<>
struct UniqueDLEqualTo<const ConceptPredicateAnd&>
{
    bool operator()(const ConceptPredicateAnd& l, const ConceptPredicateAnd& r) const;
};

/* Roles */

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateState<P>&>
{
    bool operator()(const RolePredicateState<P>& l, const RolePredicateState<P>& r) const;
};

template<PredicateCategory P>
struct UniqueDLEqualTo<const RolePredicateGoal<P>&>
{
    bool operator()(const RolePredicateGoal<P>& l, const RolePredicateGoal<P>& r) const;
};

template<>
struct UniqueDLEqualTo<const RolePredicateAnd&>
{
    bool operator()(const RolePredicateAnd& l, const RolePredicateAnd& r) const;
};
}

}

#endif
