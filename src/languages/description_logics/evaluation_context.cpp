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

#include "mimir/languages/description_logics/evaluation_context.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{
EvaluationContext::EvaluationContext(search::State state, Problem problem, DenotationRepositories& ref_repositories) :
    m_state(state),
    m_problem(problem),
    m_builders(),
    m_scratch_builders(),
    m_repositories(ref_repositories)
{
}

void EvaluationContext::set_state(search::State state) { m_state = state; }

void EvaluationContext::set_problem(formalism::Problem problem) { m_problem = problem; }

search::State EvaluationContext::get_state() const { return m_state; }

const Problem& EvaluationContext::get_problem() const { return m_problem; }

Denotations& EvaluationContext::get_builders() { return m_builders; }

Denotations& EvaluationContext::get_scratch_builders() { return m_scratch_builders; }

DenotationRepositories& EvaluationContext::get_repositories() { return m_repositories; }

}