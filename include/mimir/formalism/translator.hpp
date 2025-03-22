/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_TRANSLATOR_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism
{

class Translator
{
public:
    /// @brief Constructs a translator that translates a domain into an equivalent representation.
    explicit Translator(const Domain& domain);

    /// @brief Translates a problem using the already translated domain.
    /// @param problem The problem to be translated.
    /// @return The translated problem.
    Problem translate(const Problem& problem) const;

    /// @brief Get the original domain.
    /// @return A reference to the original domain.
    const Domain& get_original_domain() const;

    /// @brief Get the translated domain.
    /// @return A reference to the translated domain.
    const Domain& get_translated_domain() const;

private:
    Domain m_original_domain;
    Domain m_translated_domain;
};

}

#endif
