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

#include "mimir/formalism/ground_conjunctive_condition.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/hash.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/problem.hpp"

#include <ostream>
#include <tuple>

namespace mimir::formalism
{

/* GroundConjunctiveCondition */

GroundConjunctiveConditionImpl::GroundConjunctiveConditionImpl(Index index,
                                                               const FlatIndexList* positive_static_atoms,
                                                               const FlatIndexList* negative_static_atoms,
                                                               const FlatIndexList* positive_fluent_atoms,
                                                               const FlatIndexList* negative_fluent_atoms,
                                                               const FlatIndexList* positive_derived_atoms,
                                                               const FlatIndexList* negative_derived_atoms,
                                                               GroundNumericConstraintList numeric_constraints) :
    m_index(index),
    m_positive_static_atoms(positive_static_atoms),
    m_negative_static_atoms(negative_static_atoms),
    m_positive_fluent_atoms(positive_fluent_atoms),
    m_negative_fluent_atoms(negative_fluent_atoms),
    m_positive_derived_atoms(positive_derived_atoms),
    m_negative_derived_atoms(negative_derived_atoms),
    m_numeric_constraints(std::move(numeric_constraints))
{
    assert(m_positive_static_atoms->is_compressed());
    assert(m_negative_static_atoms->is_compressed());
    assert(m_positive_fluent_atoms->is_compressed());
    assert(m_negative_fluent_atoms->is_compressed());
    assert(m_positive_derived_atoms->is_compressed());
    assert(m_negative_derived_atoms->is_compressed());

    assert(std::is_sorted(m_positive_static_atoms->compressed_begin(), m_positive_static_atoms->compressed_end()));
    assert(std::is_sorted(m_negative_static_atoms->compressed_begin(), m_negative_static_atoms->compressed_end()));
    assert(std::is_sorted(m_positive_fluent_atoms->compressed_begin(), m_positive_fluent_atoms->compressed_end()));
    assert(std::is_sorted(m_negative_fluent_atoms->compressed_begin(), m_negative_fluent_atoms->compressed_end()));
    assert(std::is_sorted(m_positive_derived_atoms->compressed_begin(), m_positive_derived_atoms->compressed_end()));
    assert(std::is_sorted(m_negative_derived_atoms->compressed_begin(), m_negative_derived_atoms->compressed_end()));
}

Index GroundConjunctiveConditionImpl::get_index() const { return m_index; }

const GroundNumericConstraintList& GroundConjunctiveConditionImpl::get_numeric_constraints() const { return m_numeric_constraints; }

}

namespace mimir
{

/**
 * Pretty printing
 */

template<>
std::ostream& operator<<(std::ostream& os, const std::tuple<formalism::GroundConjunctiveCondition, const formalism::ProblemImpl&>& data)
{
    const auto& [conjunctive_condition, problem] = data;

    const auto positive_static_precondition_indices = conjunctive_condition->get_positive_precondition<formalism::StaticTag>();
    const auto negative_static_precondition_indices = conjunctive_condition->get_negative_precondition<formalism::StaticTag>();
    const auto positive_fluent_precondition_indices = conjunctive_condition->get_positive_precondition<formalism::FluentTag>();
    const auto negative_fluent_precondition_indices = conjunctive_condition->get_negative_precondition<formalism::FluentTag>();
    const auto positive_derived_precondition_indices = conjunctive_condition->get_positive_precondition<formalism::DerivedTag>();
    const auto negative_derived_precondition_indices = conjunctive_condition->get_negative_precondition<formalism::DerivedTag>();

    auto positive_static_precondition = formalism::GroundAtomList<formalism::StaticTag> {};
    auto negative_static_precondition = formalism::GroundAtomList<formalism::StaticTag> {};
    auto positive_fluent_precondition = formalism::GroundAtomList<formalism::FluentTag> {};
    auto negative_fluent_precondition = formalism::GroundAtomList<formalism::FluentTag> {};
    auto positive_derived_precondition = formalism::GroundAtomList<formalism::DerivedTag> {};
    auto negative_derived_precondition = formalism::GroundAtomList<formalism::DerivedTag> {};
    const auto& ground_numeric_constraints = conjunctive_condition->get_numeric_constraints();

    problem.get_repositories().get_ground_atoms_from_indices<formalism::StaticTag>(positive_static_precondition_indices, positive_static_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::StaticTag>(negative_static_precondition_indices, negative_static_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::FluentTag>(positive_fluent_precondition_indices, positive_fluent_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::FluentTag>(negative_fluent_precondition_indices, negative_fluent_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::DerivedTag>(positive_derived_precondition_indices, positive_derived_precondition);
    problem.get_repositories().get_ground_atoms_from_indices<formalism::DerivedTag>(negative_derived_precondition_indices, negative_derived_precondition);

    os << "positive static precondition=";
    mimir::operator<<(os, positive_static_precondition);
    os << ", " << "negative static precondition=";
    mimir::operator<<(os, negative_static_precondition);
    os << ", " << "positive fluent precondition=";
    mimir::operator<<(os, positive_fluent_precondition);
    os << ", " << "negative fluent precondition=";
    mimir::operator<<(os, negative_fluent_precondition);
    os << ", " << "positive derived precondition=";
    mimir::operator<<(os, positive_derived_precondition);
    os << ", " << "negative derived precondition=";
    mimir::operator<<(os, negative_derived_precondition);
    os << ", " << "numeric constraints=";
    mimir::operator<<(os, ground_numeric_constraints);

    return os;
}
}
