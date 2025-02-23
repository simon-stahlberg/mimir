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

#include "mimir/formalism/translator.hpp"

#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/domain_builder.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/problem_builder.hpp"

namespace mimir
{

DomainTranslationResult::DomainTranslationResult(Domain original_domain, Domain translated_domain) :
    original_domain(std::move(original_domain)),
    translated_domain(std::move(translated_domain))
{
}

DomainTranslationResult translate(const Domain& domain)
{
    std::cout << "Given domain" << std::endl;
    std::cout << *domain << std::endl;

    return DomainTranslationResult(domain, domain);
}

const Domain& DomainTranslationResult::get_original_domain() const { return original_domain; }

const Domain& DomainTranslationResult::get_translated_domain() const { return translated_domain; }

Problem translate(const Problem& problem, const DomainTranslationResult& result)
{
    if (result.get_original_domain() != problem->get_domain())
    {
        throw std::runtime_error("translate(problem, result): domain in problem must match original domain in DomainTranslationResult.");
    }

    return problem;
}

}
