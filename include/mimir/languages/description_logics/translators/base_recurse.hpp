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

#ifndef MIMIR_LANGUAGE_DESCRIPTION_LOGICS_TRANSLATORS_BASE_RECURSE_HPP_
#define MIMIR_LANGUAGE_DESCRIPTION_LOGICS_TRANSLATORS_BASE_RECURSE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_repositories.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/translators/interface.hpp"

#include <deque>
#include <unordered_map>

namespace mimir::dl::grammar
{

/**
 * Base implementation recursively calls translate.
 */
template<typename Derived_>
class BaseRecurseTranslator : public ITranslator<BaseRecurseTranslator<Derived_>>
{
private:
    BaseRecurseTranslator() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    ConstructorTagToRepository m_constructor_repositories;

    explicit BaseRecurseTranslator(ConstructorTagToRepository& constructor_repositories) : m_constructor_repositories(constructor_repositories) {}

protected:
    /* Implement ITranslator interface */
    friend class ITranslator<BaseRecurseTranslator<Derived_>>;

    /// @brief Apply grammar translation.
    ///        Default behavior reparses it into the constructor_repositories.
    template<typename T>
    auto translate_base(const T& element)
    {
        return self().translate_impl(element);
    }

    template<ConstructorTag D>
    DerivationRule<D> translate_impl(const DerivationRuleImpl<D>& constructor)
    {
        constructor.
    }
    template<ConstructorTag D>
    NonTerminal<D> translate_impl(const NonTerminalImpl<D>& constructor)
    {
    }
    template<ConstructorTag D>
    Choice<D> translate_impl(const ChoiceImpl<D>& constructor) = 0;

    ConceptBot translate_impl(ConceptBot constructor) = 0;
    ConceptTop translate_impl(ConceptTop constructor) = 0;
    template<PredicateTag P>
    ConceptAtomicState<P> tanslate_impl(const ConceptAtomicStateImpl<P>& constructor);
    template<PredicateTag P>
    ConceptAtomicState<P> translate_impl(const ConceptAtomicGoalImpl<P>& constructor);
    ConceptIntersection translate_impl(const ConceptIntersectionImpl& constructor) = 0;
    ConceptUnion translate_impl(const ConceptUnionImpl& constructor) = 0;
    ConceptNegation translate_impl(const ConceptNegationImpl& constructor) = 0;
    ConceptValueRestriction translate_impl(const ConceptValueRestrictionImpl& constructor) = 0;
    ConceptExistentialQuantification translate_impl(const ConceptExistentialQuantificationImpl& constructor) = 0;
    ConceptRoleValueMapContainment translate_impl(const ConceptRoleValueMapContainmentImpl& constructor) = 0;
    ConceptRoleValueMapEquality translate_impl(const ConceptRoleValueMapEqualityImpl& constructor) = 0;
    ConceptNominal translate_impl(const ConceptNominalImpl& constructor) = 0;

    /* Roles */
    RoleUniversal translate_impl(const RoleUniversalImpl& constructor) = 0;
    template<PredicateTag P>
    RoleAtomicState<P> translate_impl(const RoleAtomicStateImpl<P>& constructor) = 0;
    template<PredicateTag P>
    RoleAtomicGoal<P> translate_impl(const RoleAtomicGoalImpl<P>& constructor) = 0;
    RoleIntersection translate_impl(const RoleIntersectionImpl& constructor) = 0;
    RoleUnion translate_impl(const RoleUnionImpl& constructor) = 0;
    RoleComplement translate_impl(const RoleComplementImpl& constructor) = 0;
    RoleInverse translate_impl(const RoleInverseImpl& constructor) = 0;
    RoleComposition translate_impl(const RoleCompositionImpl& constructor) = 0;
    RoleTransitiveClosure translate_impl(const RoleTransitiveClosureImpl& constructor) = 0;
    RoleReflexiveTransitiveClosure translate_impl(const RoleReflexiveTransitiveClosureImpl& constructor) = 0;
    RoleRestriction translate_impl(const RoleRestrictionImpl& constructor) = 0;
    RoleIdentity translate_impl(const RoleIdentityImpl& constructor) = 0;
};

}

#endif