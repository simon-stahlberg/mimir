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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EVALUATION_CONTEXT_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_EVALUATION_CONTEXT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/denotation_repositories.hpp"
#include "mimir/search/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::languages::dl
{

class EvaluationContext
{
private:
    /* Memory */
    search::State m_state;
    formalism::Problem m_problem;
    Denotations m_builders;
    DenotationRepositories& m_repositories;

public:
    EvaluationContext(search::State state, formalism::Problem problem, DenotationRepositories& ref_repositories);

    /**
     * Getters
     */
    search::State get_state() const;

    const formalism::Problem& get_problem() const;

    Denotations& get_builders();

    DenotationRepositories& get_repositories();
};
}

#endif
