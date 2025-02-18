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

#include "mimir/common/concepts.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <algorithm>
#include <boost/container/small_vector.hpp>
#include <concepts>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir::dl::grammar
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
    template<typename T>
    void prepare(const T& element)
    {
        self().prepare_base(element);
    }

    template<typename T>
    auto translate(const T& element)
    {
        return self().translate_base(element);
    }

    grammar::Grammar run(const grammar::Grammar& grammar) { return self().run_base(grammar); }
};

}

#endif
