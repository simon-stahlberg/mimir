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

#ifndef MIMIR_FORMALISM_TRANSLATOR_INTERFACE_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/declarations.hpp"

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir::formalism
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

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    ///////////////////////////////////////////////////////
    /// Prepare
    ///////////////////////////////////////////////////////

    template<typename T>
    auto prepare_level_0(const T& element)
    {
        self().prepare_base(element);
    }

    ///////////////////////////////////////////////////////
    /// Translate
    ///////////////////////////////////////////////////////

    template<typename T>
    auto translate_level_0(const T& element, Repositories& repositories)
    {
        return self().translate_level_1(element, repositories);
    }

    /**
     * For domain and problem we provide specialized functions since they are treated fundamentally different,
     * i.e., there is no entry in the HanaRepositories and we need to pass additional information to create default behaviors.
     */

    auto translate_level_0(const Domain& domain, DomainBuilder& builder) { return self().translate_level_1(domain, builder); }

    auto translate_level_0(const Problem& problem, ProblemBuilder& builder) { return self().translate_level_1(problem, builder); }
};
}

#endif
