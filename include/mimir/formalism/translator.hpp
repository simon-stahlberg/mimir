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

class DomainTranslationResult
{
private:
    Domain original_domain;  ///< used to check that a problem can be translated with this result.

    Domain translated_domain;

public:
    DomainTranslationResult(Domain original_domain, Domain translated_domain);

    const Domain& get_original_domain() const;
    const Domain& get_translated_domain() const;
};

extern DomainTranslationResult translate(const Domain& domain);

extern Problem translate(const Problem& problem, const DomainTranslationResult& result);

}

#endif
