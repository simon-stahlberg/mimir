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

#include "mimir/search/state.hpp"

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"

#include <ostream>
#include <tuple>

using namespace mimir::formalism;

namespace mimir::search
{

/* InternalState */

InternalStateImpl::InternalStateImpl() : m_self_allocated(true), m_data(new uint8_t[s_capacity]) {}

InternalStateImpl::~InternalStateImpl()
{
    if (m_self_allocated && m_data)
    {
        delete[] m_data;
    }
}

void InternalStateImpl::set_data(const UnpackedData& data)
{
    assert(m_self_allocated && m_data);

    std::memset(m_data, uint8_t(0), s_capacity);

    size_t start_bit = 36;  // 6 * 6 bits

    size_t num_bits_index = std::bit_width(data.index);
    buffering::write_uint<Index>(m_data, 0, 6, num_bits_index);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_index, data.index);
    start_bit += num_bits_index;

    const auto [fluent_index, fluent_size] = valla::read_slot(data.fluent_atoms);

    size_t num_bits_fluent_index = std::bit_width(fluent_index);
    buffering::write_uint<Index>(m_data, 6, 12, num_bits_fluent_index);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_fluent_index, fluent_index);
    start_bit += num_bits_fluent_index;

    size_t num_bits_fluent_size = std::bit_width(valla::read_pos(fluent_size, 1));
    buffering::write_uint<Index>(m_data, 12, 18, num_bits_fluent_size);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_fluent_size, fluent_size);
    start_bit += num_bits_fluent_size;

    const auto [derived_index, derived_size] = valla::read_slot(data.derived_atoms);

    size_t num_bits_derived_index = std::bit_width(derived_index);
    buffering::write_uint<Index>(m_data, 18, 24, num_bits_derived_index);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_derived_index, derived_index);
    start_bit += num_bits_derived_index;

    size_t num_bits_derived_size = std::bit_width(derived_size);
    buffering::write_uint<Index>(m_data, 24, 30, num_bits_derived_size);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_derived_size, derived_size);
    start_bit += num_bits_derived_size;

    size_t num_bits_numeric = std::bit_width(data.numeric_variables);
    buffering::write_uint<Index>(m_data, 30, 36, num_bits_numeric);
    buffering::write_uint<Index>(m_data, start_bit, start_bit + num_bits_numeric, data.numeric_variables);
    start_bit += num_bits_numeric;
}

InternalStateImpl::UnpackedData InternalStateImpl::get_data() const
{
    auto result = UnpackedData {};
    size_t start_bit = 36;  // 6 * 6 bits

    size_t num_bits_index = buffering::read_uint<size_t>(m_data, 0, 6);

    result.index = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_index);
    start_bit += num_bits_index;

    size_t num_bits_fluent_index = buffering::read_uint<size_t>(m_data, 6, 12);
    Index fluent_index = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_fluent_index);
    start_bit += num_bits_fluent_index;

    size_t num_bits_fluent_size = buffering::read_uint<size_t>(m_data, 12, 18);
    Index fluent_size = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_fluent_size);
    start_bit += num_bits_fluent_size;

    result.fluent_atoms = valla::make_slot(fluent_index, fluent_size);

    size_t num_bits_derived_index = buffering::read_uint<size_t>(m_data, 18, 24);
    Index derived_index = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_derived_index);
    start_bit += num_bits_derived_index;

    size_t num_bits_derived_size = buffering::read_uint<size_t>(m_data, 24, 30);
    Index derived_size = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_derived_size);
    start_bit += num_bits_derived_size;

    result.derived_atoms = valla::make_slot(derived_index, derived_size);

    size_t num_bits_numeric_size = buffering::read_uint<size_t>(m_data, 30, 36);
    result.numeric_variables = buffering::read_uint<Index>(m_data, start_bit, start_bit + num_bits_numeric_size);
    start_bit += num_bits_numeric_size;

    return result;
}

size_t InternalStateImpl::buffer_size() const
{
    size_t start_bit = 36;  // Header: 6 fields * 6 bits

    size_t total_bits = start_bit;

    total_bits += buffering::read_uint<size_t>(m_data, 0, 6);    // index
    total_bits += buffering::read_uint<size_t>(m_data, 6, 12);   // fluent index
    total_bits += buffering::read_uint<size_t>(m_data, 12, 18);  // fluent size
    total_bits += buffering::read_uint<size_t>(m_data, 18, 24);  // derived index
    total_bits += buffering::read_uint<size_t>(m_data, 24, 30);  // derived size
    total_bits += buffering::read_uint<size_t>(m_data, 30, 36);  // numeric vars

    // ceil(total_bits / 8)
    return (total_bits + 7) / 8;
}

/**
 * State
 */

State::State(const InternalStateImpl& internal, const formalism::ProblemImpl& problem) : m_internal(&internal), m_problem(&problem)
{
    assert(std::is_sorted(v::begin(m_internal->template get_atoms<FluentTag>(), m_problem->get_tree_table()), v::end()));
    assert(std::is_sorted(v::begin(m_internal->template get_atoms<DerivedTag>(), m_problem->get_tree_table()), v::end()));
}

bool State::operator==(const State& other) const noexcept { return loki::EqualTo<State> {}(*this, other); }

bool State::operator!=(const State& other) const noexcept { return !(*this == other); }

template<IsFluentOrDerivedTag P>
bool State::literal_holds(GroundLiteral<P> literal) const
{
    return (std::find(get_atoms<P>().begin(), get_atoms<P>().end(), literal->get_index()) != get_atoms<P>().end()) == literal->get_polarity();
}

template bool State::literal_holds(GroundLiteral<FluentTag> literal) const;
template bool State::literal_holds(GroundLiteral<DerivedTag> literal) const;

template<IsFluentOrDerivedTag P>
bool State::literals_hold(const GroundLiteralList<P>& literals) const
{
    return std::all_of(literals.begin(), literals.end(), [this](auto&& arg) { return this->literal_holds(arg); });
}

template bool State::literals_hold(const GroundLiteralList<FluentTag>& literals) const;
template bool State::literals_hold(const GroundLiteralList<DerivedTag>& literals) const;

bool State::numeric_constraint_holds(GroundNumericConstraint numeric_constraint, const FlatDoubleList& static_numeric_variables) const
{
    return evaluate(numeric_constraint, static_numeric_variables, get_numeric_variables());
}

bool State::numeric_constraints_hold(const GroundNumericConstraintList& numeric_constraints, const FlatDoubleList& static_numeric_variables) const
{
    return std::all_of(numeric_constraints.begin(),
                       numeric_constraints.end(),
                       [this, &static_numeric_variables](auto&& arg) { return this->numeric_constraint_holds(arg, static_numeric_variables); });
}

const InternalStateImpl& State::get_internal_state() const { return *m_internal; }

const formalism::ProblemImpl& State::get_problem() const { return *m_problem; }

Index State::get_index() const { return m_internal->get_index(); }

const FlatDoubleList& State::get_numeric_variables() const { return *m_problem->get_double_list(m_internal->get_numeric_variables()); }

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& os, const search::State& state)
{
    auto fluent_ground_atoms = GroundAtomList<FluentTag> {};
    auto static_ground_atoms = GroundAtomList<StaticTag> {};
    auto derived_ground_atoms = GroundAtomList<DerivedTag> {};
    auto fluent_function_values = std::vector<std::pair<GroundFunction<FluentTag>, ContinuousCost>> {};

    state.get_problem().get_repositories().get_ground_atoms_from_indices(state.get_atoms<FluentTag>(), fluent_ground_atoms);
    state.get_problem().get_repositories().get_ground_atoms_from_indices(state.get_problem().get_static_initial_positive_atoms_bitset(), static_ground_atoms);
    state.get_problem().get_repositories().get_ground_atoms_from_indices(state.get_atoms<DerivedTag>(), derived_ground_atoms);
    state.get_problem().get_repositories().get_ground_function_values(state.get_numeric_variables(), fluent_function_values);

    // Sort by name for easier comparison
    std::sort(fluent_ground_atoms.begin(), fluent_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(static_ground_atoms.begin(), static_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });
    std::sort(derived_ground_atoms.begin(), derived_ground_atoms.end(), [](const auto& lhs, const auto& rhs) { return to_string(*lhs) < to_string(*rhs); });

    os << "State(" << "index=" << state.get_index() << ", " << "fluent atoms=";
    mimir::operator<<(os, fluent_ground_atoms);
    os << ", " << "static atoms=";
    mimir::operator<<(os, static_ground_atoms);
    os << ", " << "derived atoms=";
    mimir::operator<<(os, derived_ground_atoms);
    os << ", " << "fluent numerics=";
    mimir::operator<<(os, fluent_function_values);
    os << ")";

    return os;
}
}
