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
    template<std::ranges::forward_range Range>
    void prepare(const Range& input)
    {
        std::ranges::for_each(input, [this](auto&& arg) { this->prepare(*arg); });
    }
    template<typename T>
    void prepare(const T& element)
    {
        self().prepare_base(element);
    }

    /// @brief Translate a container of elements into a container of elements.
    template<IsBackInsertibleRange Range>
    auto translate(const Range& input)
    {
        std::remove_cvref_t<Range> output;

        if constexpr (requires { output.reserve(std::ranges::size(input)); })
        {
            output.reserve(std::ranges::size(input));
        }

        std::ranges::transform(input, std::back_inserter(output), [this](auto&& arg) { return this->translate(*arg); });

        return output;
    }
    template<typename T>
    auto translate(const T& element)
    {
        return self().translate_base(element);
    }

    /// @brief Collect information and apply problem translation.
    ConstructorTagToDerivationRuleList run(const ConstructorTagToDerivationRuleList& derivation_rules) { return self().run_base(derivation_rules); }
};

}

#endif
