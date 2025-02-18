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
    GrammarConstructorRepositories m_constructor_repositories;

    explicit BaseRecurseTranslator(GrammarConstructorRepositories& constructor_repositories) : m_constructor_repositories(constructor_repositories) {}

protected:
    /* Implement ITranslator interface */
    friend class ITranslator<BaseRecurseTranslator<Derived_>>;

    /**
     * Prepare step.
     */
    template<std::ranges::forward_range Range>
    void prepare_base(const Range& range)
    {
        self().prepare_impl(range);
    }
    template<std::ranges::forward_range Range>
    void prepare_impl(const Range& range)
    {
        std::ranges::for_each(range, [this](auto&& arg) { this->prepare(arg); });
    }
    template<typename T>
    void prepare_base(const T& element)
    {
        self().prepare_impl(element);
    }

    /**
     * Translate step.
     */
    template<IsBackInsertibleRange Range>
    auto translate_base(const Range& input)
    {
        return self().translate_impl(input);
    }
    template<IsBackInsertibleRange Range>
    auto translate_impl(const Range& input)
    {
        std::remove_cvref_t<Range> output;

        if constexpr (requires { output.reserve(std::ranges::size(input)); })
        {
            output.reserve(std::ranges::size(input));
        }

        std::ranges::transform(input, std::back_inserter(output), [this](auto&& arg) { return this->translate(arg); });

        return output;
    }
    template<typename T>
    auto translate_base(const T& element)
    {
        return self().translate_impl(element);
    }

    template<ConceptOrRole D>
    DerivationRule<D> translate_impl(DerivationRule<D> constructor)
    {
        return boost::hana::at_key(m_constructor_repositories, boost::hana::type<DerivationRuleImpl<D>> {})
            .get_or_create(this->translate(constructor->get_non_terminal()), this->translate(constructor->get_constructor_or_non_terminals()));
    }
    template<ConceptOrRole D>
    NonTerminal<D> translate_impl(NonTerminal<D> constructor)
    {
        return boost::hana::at_key(m_constructor_repositories, boost::hana::type<NonTerminalImpl<D>> {}).get_or_create(constructor->get_name(), nullptr);
    }
    template<ConceptOrRole D>
    ConstructorOrNonTerminal<D> translate_impl(ConstructorOrNonTerminal<D> constructor)
    {
        return boost::hana::at_key(m_constructor_repositories, boost::hana::type<ConstructorOrNonTerminal<D>> {})
            .get_or_create(std::visit([](auto&& arg) { return this->translate(arg); }, constructor->get_constructor_or_non_terminal));
    }

    ConceptBot translate_impl(ConceptBot constructor) {}
    ConceptTop translate_impl(ConceptTop constructor) {}
    template<StaticOrFluentOrDerived P>
    ConceptAtomicState<P> translate_impl(ConceptAtomicState<P> constructor)
    {
    }
    template<StaticOrFluentOrDerived P>
    ConceptAtomicState<P> translate_impl(ConceptAtomicGoal<P> constructor)
    {
    }
    ConceptIntersection translate_impl(ConceptIntersection constructor) {}
    ConceptUnion translate_impl(ConceptUnion constructor) {}
    ConceptNegation translate_impl(ConceptNegation constructor) {}
    ConceptValueRestriction translate_impl(ConceptValueRestriction constructor) {}
    ConceptExistentialQuantification translate_impl(ConceptExistentialQuantification constructor) {}
    ConceptRoleValueMapContainment translate_impl(ConceptRoleValueMapContainment constructor) {}
    ConceptRoleValueMapEquality translate_impl(ConceptRoleValueMapEquality constructor) {}
    ConceptNominal translate_impl(ConceptNominal constructor) {}

    /* Roles */
    RoleUniversal translate_impl(RoleUniversal constructor) {}
    template<StaticOrFluentOrDerived P>
    RoleAtomicState<P> translate_impl(RoleAtomicState<P> constructor)
    {
    }
    template<StaticOrFluentOrDerived P>
    RoleAtomicGoal<P> translate_impl(RoleAtomicGoal<P> constructor)
    {
    }
    RoleIntersection translate_impl(RoleIntersection constructor) {}
    RoleUnion translate_impl(RoleUnion constructor) {}
    RoleComplement translate_impl(RoleComplement constructor) {}
    RoleInverse translate_impl(RoleInverse constructor) {}
    RoleComposition translate_impl(RoleComposition constructor) {}
    RoleTransitiveClosure translate_impl(RoleTransitiveClosure constructor) {}
    RoleReflexiveTransitiveClosure translate_impl(RoleReflexiveTransitiveClosure constructor) {}
    RoleRestriction translate_impl(RoleRestriction constructor) {}
    RoleIdentity translate_impl(RoleIdentity constructor) {}
};

}

#endif