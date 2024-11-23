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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_TRANSLATORS_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_TRANSLATORS_INTERFACE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <concepts>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir::dl
{

/**
 * Interface class.
 */
template<typename Derived_>
class ITranslator
{
private:
    ITranslator() = default;
    friend Derived_;

    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    /// @brief Prepare all elements in a container.
    template<std::ranges::forward_range Range>
    void prepare(const Range& input)
    {
        std::ranges::for_each(input, [this](auto&& arg) { this->prepare(std::forward<decltype(arg)>(arg)); });
    }
    template<typename T>
    void prepare(T constructor)
    {
        self().prepare_base(constructor);
    }
    /// @brief Translate a container of elements into a container of elements.
    template<std::ranges::forward_range Range>
    auto translate(const Range& input)
    {
        std::remove_cvref_t<Range> output;

        if constexpr (requires { output.reserve(std::ranges::size(input)); })
        {
            output.reserve(std::ranges::size(input));
        }

        std::ranges::transform(input, std::back_inserter(output), [this](const auto& arg) { return this->translate(arg); });

        return output;
    }
    DerivationRule<Concept> translate(DerivationRule<Concept> constructor) { return self().translate_base(constructor); }
    NonTerminal<Concept> translate(NonTerminal<Concept> constructor) { return self().translate_base(constructor); }
    Choice<Concept> translate(Choice<Concept> constructor) { return self().translate_base(constructor); }
    ConceptBot translate(ConceptBot constructor) { return self().translate_base(constructor); }
    ConceptTop translate(ConceptTop constructor) { return self().translate_base(constructor); }
    ConceptAtomicState<Static> translate(ConceptAtomicState<Static> constructor) { return self().translate_base(constructor); }
    ConceptAtomicState<Fluent> translate(ConceptAtomicState<Fluent> constructor) { return self().translate_base(constructor); }
    ConceptAtomicState<Derived> translate(ConceptAtomicState<Derived> constructor) { return self().translate_base(constructor); }
    ConceptAtomicGoal<Static> translate(ConceptAtomicGoal<Static> constructor) { return self().translate_base(constructor); }
    ConceptAtomicGoal<Fluent> translate(ConceptAtomicGoal<Fluent> constructor) { return self().translate_base(constructor); }
    ConceptAtomicGoal<Derived> translate(ConceptAtomicGoal<Derived> constructor) { return self().translate_base(constructor); }
    ConceptIntersection translate(ConceptIntersection constructor) { return self().translate_base(constructor); }
    ConceptUnion translate(ConceptUnion constructor) { return self().translate_base(constructor); }
    ConceptNegation translate(ConceptNegation constructor) { return self().translate_base(constructor); }
    ConceptValueRestriction translate(ConceptValueRestriction constructor) { return self().translate_base(constructor); }
    ConceptExistentialQuantification translate(ConceptExistentialQuantification constructor) { return self().translate_base(constructor); }
    ConceptRoleValueMapContainment translate(ConceptRoleValueMapContainment constructor) { return self().translate_base(constructor); }
    ConceptRoleValueMapEquality translate(ConceptRoleValueMapEquality constructor) { return self().translate_base(constructor); }
    ConceptNominal translate(ConceptNominal constructor) { return self().translate_base(constructor); }

    DerivationRule<Role> translate(DerivationRule<Role> constructor) { return self().translate_base(constructor); }
    DerivationRule<Role> translate(DerivationRule<Role> constructor) { return self().translate_base(constructor); }
    NonTerminal<Role> translate(NonTerminal<Role> constructor) { return self().translate_base(constructor); }
    Choice<Role> translate(Choice<Role> constructor) { return self().translate_base(constructor); }
    RoleUniversal translate(RoleUniversal constructor) { return self().translate_base(constructor); }
    RoleAtomicState<Static> translate(RoleAtomicState<Static> constructor) { return self().translate_base(constructor); }
    RoleAtomicState<Fluent> translate(RoleAtomicState<Fluent> constructor) { return self().translate_base(constructor); }
    RoleAtomicState<Derived> translate(RoleAtomicState<Derived> constructor) { return self().translate_base(constructor); }
    RoleAtomicGoal<Static> translate(RoleAtomicGoal<Static> constructor) { return self().translate_base(constructor); }
    RoleAtomicGoal<Fluent> translate(RoleAtomicGoal<Fluent> constructor) { return self().translate_base(constructor); }
    RoleAtomicGoal<Derived> translate(RoleAtomicGoal<Derived> constructor) { return self().translate_base(constructor); }
    RoleIntersection translate(RoleIntersection constructor) { return self().translate_base(constructor); }
    RoleUnion translate(RoleUnion constructor) { return self().translate_base(constructor); }
    RoleComplement translate(RoleComplement constructor) { return self().translate_base(constructor); }
    RoleInverse translate(RoleInverse constructor) { return self().translate_base(constructor); }
    RoleComposition translate(RoleComposition constructor) { return self().translate_base(constructor); }
    RoleTransitiveClosure translate(RoleTransitiveClosure constructor) { return self().translate_base(constructor); }
    RoleReflexiveTransitiveClosure translate(RoleReflexiveTransitiveClosure constructor) { return self().translate_base(constructor); }
    RoleRestriction translate(RoleRestriction constructor) { return self().translate_base(constructor); }
    RoleIdentity translate(RoleIdentity constructor) { return self().translate_base(constructor); }

    /// @brief Collect information and apply problem translation.
    ConstructorTagToDerivationRuleList run(const ConstructorTagToDerivationRuleList& derivation_rules) { return self().run_base(derivation_rules); }
};

}

#endif
