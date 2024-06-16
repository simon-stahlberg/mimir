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

#include "mimir/search/algorithms/iw.hpp"

// Keep all implementations in the same translation unit
#include "mimir/search/algorithms/iw/dynamic_novelty_table.hpp"
#include "mimir/search/algorithms/iw/index_mappers.hpp"
#include "mimir/search/algorithms/iw/pruning_strategy.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"

#include <sstream>

namespace mimir
{

/**
 * TupleIndexMapper
 */

TupleIndexMapper::TupleIndexMapper(int arity, int num_atoms) : m_arity(arity), m_num_atoms(num_atoms), m_empty_tuple_index(0)
{
    if (!(arity >= 0 && arity < MAX_ARITY))
    {
        throw std::runtime_error("TupleIndexMapper only works with 0 <= arity < " + std::to_string(MAX_ARITY) + ".");
    }
    //  Initialize factors
    for (int i = 0; i < m_arity; ++i)
    {
        m_factors[i] = std::pow(m_num_atoms, i);
    }
    // Initialize empty tuple index
    for (int i = 0; i < m_arity; ++i)
    {
        m_empty_tuple_index += m_num_atoms * m_factors[i];
    }
}

TupleIndex TupleIndexMapper::to_tuple_index(const AtomIndexList& atom_indices) const
{
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
    assert(static_cast<int>(atom_indices.size()) == m_arity);

    TupleIndex result = 0;
    for (int i = 0; i < m_arity; ++i)
    {
        result += m_factors[i] * atom_indices[i];
    }
    return result;
}

void TupleIndexMapper::to_atom_indices(TupleIndex tuple_index, AtomIndexList& out_atom_indices) const
{
    out_atom_indices.clear();

    for (int i = m_arity - 1; i >= 0; --i)
    {
        // We need to use min to decode (m_num_atoms,...,m_num_atoms) correctly
        int atom_index = std::min(m_num_atoms, tuple_index / m_factors[i]);
        if (atom_index != m_num_atoms)
        {
            out_atom_indices.push_back(atom_index);
        }
        tuple_index -= atom_index * m_factors[i];
    }
    std::reverse(out_atom_indices.begin(), out_atom_indices.end());
}

std::string TupleIndexMapper::tuple_index_to_string(TupleIndex tuple_index) const
{
    auto atom_indices = AtomIndexList {};
    to_atom_indices(tuple_index, atom_indices);
    std::stringstream ss;
    ss << "(";
    for (const auto atom_index : atom_indices)
    {
        ss << atom_index << ",";
    }
    ss << ")";
    return ss.str();
}

int TupleIndexMapper::get_num_atoms() const { return m_num_atoms; }

int TupleIndexMapper::get_arity() const { return m_arity; }

const std::array<int, MAX_ARITY>& TupleIndexMapper::get_factors() const { return m_factors; }

int TupleIndexMapper::get_max_tuple_index() const
{
    // x^0 + ... + x^n = (x^{n+1} - 1) / 2
    return ((std::pow(get_num_atoms(), get_arity() + 1) - 1) / 2) - 1;
}

TupleIndex TupleIndexMapper::get_empty_tuple_index() const { return m_empty_tuple_index; }

/**
 * FluentAndDerivedMapper
 */

FluentAndDerivedMapper::FluentAndDerivedMapper() : m_fluent_remap(), m_derived_remap(), m_is_remapped_fluent(), m_inverse_remap(), m_num_atoms(0) {}

FluentAndDerivedMapper::FluentAndDerivedMapper(const FluentGroundAtomFactory& fluent_atoms, const DerivedGroundAtomFactory& derived_atoms) :
    m_fluent_remap(fluent_atoms.size(), -1),
    m_derived_remap(derived_atoms.size(), -1),
    m_is_remapped_fluent(fluent_atoms.size() + derived_atoms.size(), false),
    m_inverse_remap(fluent_atoms.size() + derived_atoms.size(), UNDEFINED),
    m_num_atoms(0)
{
    for (const auto& atom : fluent_atoms)
    {
        const auto atom_id = atom.get_identifier();
        const auto remapped_atom_id = m_num_atoms++;
        m_fluent_remap.at(atom_id) = remapped_atom_id;
        m_is_remapped_fluent.at(atom_id) = true;
        m_inverse_remap.at(remapped_atom_id) = atom_id;
    }
    for (const auto& atom : derived_atoms)
    {
        const auto atom_id = atom.get_identifier();
        const auto remapped_atom_id = m_num_atoms++;
        m_derived_remap.at(atom_id) = remapped_atom_id;
        m_inverse_remap.at(remapped_atom_id) = atom_id;
    }
    assert(m_num_atoms == static_cast<int>(fluent_atoms.size()) + static_cast<int>(derived_atoms.size()));
}

const int FluentAndDerivedMapper::UNDEFINED = -1;

void FluentAndDerivedMapper::remap_atoms(const State state)
{
    for (const auto& atom_id : state.get_atoms<Fluent>())
    {
        if (atom_id >= m_fluent_remap.size())
        {
            m_fluent_remap.resize(atom_id + 1, UNDEFINED);
            m_inverse_remap.resize(atom_id + 1, UNDEFINED);
            m_is_remapped_fluent.resize(atom_id + 1, false);
        }
        if (m_fluent_remap.at(atom_id) != UNDEFINED)
        {
            continue;
        }

        const auto remapped_atom_id = m_num_atoms++;
        m_fluent_remap.at(atom_id) = remapped_atom_id;
        m_is_remapped_fluent.at(atom_id) = true;
        m_inverse_remap.at(remapped_atom_id) = atom_id;
    }

    for (const auto& atom_id : state.get_atoms<Derived>())
    {
        if (atom_id >= m_derived_remap.size())
        {
            m_derived_remap.resize(atom_id + 1, UNDEFINED);
            m_inverse_remap.resize(atom_id + 1, UNDEFINED);
            m_is_remapped_fluent.resize(atom_id + 1, false);
        }
        if (m_derived_remap.at(atom_id) != UNDEFINED)
        {
            continue;
        }

        const auto remapped_atom_id = m_num_atoms++;
        m_derived_remap.at(atom_id) = remapped_atom_id;
        m_inverse_remap.at(remapped_atom_id) = atom_id;
    }
}

void FluentAndDerivedMapper::remap_and_combine_and_sort(const State state, AtomIndexList& out_atoms)
{
    // Remap
    remap_atoms(state);

    // Combine and sort
    combine_and_sort(state, out_atoms);
}

void FluentAndDerivedMapper::remap_and_combine_and_sort(const State state, const State succ_state, AtomIndexList& out_atoms, AtomIndexList& out_add_atoms)
{
    // Remap
    remap_atoms(state);
    remap_atoms(succ_state);

    // Combine and sort
    combine_and_sort(state, succ_state, out_atoms, out_add_atoms);
}

void FluentAndDerivedMapper::combine_and_sort(const State state, AtomIndexList& out_atoms)
{
    // Combine
    out_atoms.clear();
    for (const auto& atom_id : state.get_atoms<Fluent>())
    {
        assert(m_fluent_remap.at(atom_id) != UNDEFINED);

        out_atoms.push_back(m_fluent_remap.at(atom_id));
    }

    for (const auto& atom_id : state.get_atoms<Derived>())
    {
        assert(m_derived_remap.at(atom_id) != UNDEFINED);

        out_atoms.push_back(m_derived_remap.at(atom_id));
    }

    // Sort
    std::sort(out_atoms.begin(), out_atoms.end());

    assert(out_atoms.size() == state.get_atoms<Fluent>().count() + state.get_atoms<Derived>().count());
}

void FluentAndDerivedMapper::combine_and_sort(const State state, const State succ_state, AtomIndexList& out_atoms, AtomIndexList& out_add_atoms)
{
    // Combine
    out_atoms.clear();
    out_add_atoms.clear();
    for (const auto& atom_id : succ_state.get_atoms<Fluent>())
    {
        assert(m_fluent_remap.at(atom_id) != UNDEFINED);

        state.get_atoms<Fluent>().get(atom_id) ? out_atoms.push_back(m_fluent_remap.at(atom_id)) : out_add_atoms.push_back(m_fluent_remap.at(atom_id));
    }

    for (const auto& atom_id : succ_state.get_atoms<Derived>())
    {
        assert(m_derived_remap.at(atom_id) != UNDEFINED);

        state.get_atoms<Derived>().get(atom_id) ? out_atoms.push_back(m_derived_remap.at(atom_id)) : out_add_atoms.push_back(m_derived_remap.at(atom_id));
    }

    // Sort
    std::sort(out_atoms.begin(), out_atoms.end());
    std::sort(out_add_atoms.begin(), out_add_atoms.end());

    assert(out_atoms.size() + out_add_atoms.size() == succ_state.get_atoms<Fluent>().count() + succ_state.get_atoms<Derived>().count());
}

const std::vector<int>& FluentAndDerivedMapper::get_fluent_remap() const { return m_fluent_remap; }

const std::vector<int>& FluentAndDerivedMapper::get_derived_remap() const { return m_derived_remap; }

void FluentAndDerivedMapper::inverse_remap_and_separate(const AtomIndexList& combined_atoms, AtomIndexList& out_fluent_atoms, AtomIndexList& out_derived_atoms)
{
    out_fluent_atoms.clear();
    out_derived_atoms.clear();

    for (const auto atom_id : combined_atoms)
    {
        // Ensure that we are not computing the inverse of an index
        // that has never been mapped in the forward direction before.
        assert(m_inverse_remap.at(atom_id) != UNDEFINED);

        if (m_is_remapped_fluent.at(atom_id))
        {
            out_fluent_atoms.push_back(m_inverse_remap.at(atom_id));
        }
        else
        {
            out_derived_atoms.push_back(m_inverse_remap.at(atom_id));
        }
    }
}

const std::vector<bool>& FluentAndDerivedMapper::get_is_remapped_fluent() const { return m_is_remapped_fluent; }

const std::vector<int>& FluentAndDerivedMapper::get_inverse_remap() const { return m_inverse_remap; }

/**
 * StateTupleIndexGenerator
 */

StateTupleIndexGenerator::StateTupleIndexGenerator(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper_,
                                                   std::shared_ptr<TupleIndexMapper> tuple_index_mapper_) :
    atom_index_mapper(std::move(atom_index_mapper_)),
    tuple_index_mapper(std::move(tuple_index_mapper_)),
    atom_indices()
{
    assert(tuple_index_mapper);
}

StateTupleIndexGenerator::const_iterator::const_iterator() : m_tuple_index_mapper(nullptr), m_atom_indices(nullptr) {}

StateTupleIndexGenerator::const_iterator::const_iterator(StateTupleIndexGenerator* stig, bool begin) :
    m_tuple_index_mapper(begin ? stig->tuple_index_mapper.get() : nullptr),
    m_atom_indices(begin ? &stig->atom_indices : nullptr),
    m_end(!begin),
    m_cur(begin ? 0 : -1)
{
    if (begin)
    {
        // Fetch data
        const int arity = m_tuple_index_mapper->get_arity();
        const auto& factors = m_tuple_index_mapper->get_factors();

        assert(!m_atom_indices->empty());
        assert(std::is_sorted(m_atom_indices->begin(), m_atom_indices->end()));

        // Find the indices and set begin tuple index
        for (int i = 0; i < arity; ++i)
        {
            m_indices[i] = i;
            m_cur += (*m_atom_indices)[i] * factors[i];
        }
    }
    else
    {
        assert(!stig);
    }
}

void StateTupleIndexGenerator::const_iterator::advance()
{
    // Increment indices from right to left until obtaining an
    // array of form {num_atoms,...,num_atoms} indicating the end
    // Update the tuple index on the fly.

    // Fetch data
    const int arity = m_tuple_index_mapper->get_arity();
    const auto& factors = m_tuple_index_mapper->get_factors();
    const int num_atoms = m_atom_indices->size();

    // Find the rightmost index to increment, i.e., non place-holder.
    int i = arity - 1;
    while (i >= 0 && (m_indices[i] == num_atoms - 1))
    {
        --i;
    }
    if (i < 0)
    {
        // Exit when all indices have reached their maximum values
        m_end = true;
        ++m_cur;
        return;
    }

    // Advance and update cur based on the change
    const int index = ++m_indices[i];
    m_cur += factors[i] * ((*m_atom_indices)[index] - (*m_atom_indices)[index - 1]);

    // Update indices right of the incremented rightmost index i.
    for (int j = i + 1; j < arity; ++j)
    {
        // Backup old_index for difference update
        int old_index = m_indices[j];

        // Cap at placeholder index
        int new_index = m_indices[j] = std::min(num_atoms - 1, m_indices[j - 1] + 1);

        // Update and update cur based on the change
        m_cur += factors[j] * ((*m_atom_indices)[new_index] - (*m_atom_indices)[old_index]);
    }
}

StateTupleIndexGenerator::const_iterator::value_type StateTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_tuple_index_mapper && m_atom_indices);
    return m_cur;
}

StateTupleIndexGenerator::const_iterator& StateTupleIndexGenerator::const_iterator::operator++()
{
    advance();
    return *this;
}

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool StateTupleIndexGenerator::const_iterator::operator==(const const_iterator& other) const
{
    // Compare against end iterator
    if (m_end == other.m_end)
    {
        return true;
    }

    // Compare against cur position
    return m_cur == other.m_cur;
}

bool StateTupleIndexGenerator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::begin(const AtomIndexList& atom_indices_)
{
    atom_indices = atom_indices_;
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));

    return const_iterator(this, true);
}

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::begin(const State state)
{
    atom_index_mapper->remap_and_combine_and_sort(state, atom_indices);
    // Add place holder to generate tuples if size < arity
    atom_indices.push_back(tuple_index_mapper->get_num_atoms());

    // std::cout << "atom_indices: " << atom_indices << std::endl;

    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));

    return const_iterator(this, true);
}

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * StatePairTupleIndexGenerator
 */

StatePairTupleIndexGenerator::StatePairTupleIndexGenerator(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper_,
                                                           std::shared_ptr<TupleIndexMapper> tuple_index_mapper_) :
    atom_index_mapper(std::move(atom_index_mapper_)),
    tuple_index_mapper(std::move(tuple_index_mapper_))
{
}

StatePairTupleIndexGenerator::const_iterator::const_iterator() :
    m_tuple_index_mapper(nullptr),
    m_a_atom_indices(nullptr),
    m_a_index_jumper(nullptr),
    m_indices(),
    m_a(),
    m_cur_outter(-1),
    m_cur_inner(-1),
    m_end_outter(false),
    m_end_inner(false)
{
}

StatePairTupleIndexGenerator::const_iterator::const_iterator(StatePairTupleIndexGenerator* sptig, bool begin) :
    m_tuple_index_mapper(begin ? sptig->tuple_index_mapper.get() : nullptr),
    m_a_atom_indices(begin ? &sptig->a_atom_indices : nullptr),
    m_a_index_jumper(begin ? &sptig->a_index_jumper : nullptr),
    m_indices(),
    m_a(),
    m_cur_outter(begin ? 0 : -1),
    m_cur_inner(begin ? 0 : -1),
    m_end_outter(begin ? false : true),
    m_end_inner(begin ? false : true)
{
    if (begin)
    {
        // Ensure that atom_indices is nonempty due to adding the place holder.
        assert(!(*m_a_atom_indices)[0].empty());
        // Ensure that add_atom_indices is nonempty due to correct generation of tuples in previous states.
        assert(!(*m_a_atom_indices)[1].empty());
        // Ensure that atom indices are sorted
        assert(std::is_sorted((*m_a_atom_indices)[0].begin(), (*m_a_atom_indices)[0].end()));
        assert(std::is_sorted((*m_a_atom_indices)[1].begin(), (*m_a_atom_indices)[1].end()));

        // Initialize m_a_index_jumper to know the next large element in the opposite atom indices vector when iterating
        initialize_index_jumper();

        // Initialize m_cur_outter and m_cur_inner
        advance_outter();
    }
    else
    {
        assert(!sptig);
    }
}

const int StatePairTupleIndexGenerator::const_iterator::UNDEFINED = -1;

void StatePairTupleIndexGenerator::const_iterator::initialize_index_jumper()
{
    (*m_a_index_jumper)[0].clear();
    (*m_a_index_jumper)[1].clear();
    (*m_a_index_jumper)[0].resize((*m_a_atom_indices)[0].size(), UNDEFINED);
    (*m_a_index_jumper)[1].resize((*m_a_atom_indices)[1].size(), UNDEFINED);

    int j = 0;
    int i = 0;
    while (j < static_cast<int>((*m_a_atom_indices)[0].size()) && i < static_cast<int>((*m_a_atom_indices)[1].size()))
    {
        if ((*m_a_atom_indices)[0][j] < (*m_a_atom_indices)[1][i])
        {
            (*m_a_index_jumper)[0][j] = i;
            ++j;
        }
        else if ((*m_a_atom_indices)[0][j] > (*m_a_atom_indices)[1][i])
        {
            (*m_a_index_jumper)[1][i] = j;
            ++i;
        }
        else
        {
            (*m_a_index_jumper)[1][i] = j;
            (*m_a_index_jumper)[0][j] = i;
            ++j;
            ++i;
        }
    }
}

static void compute_binary_representation(int value, int arity, std::array<bool, MAX_ARITY>& output)
{
    // std::cout << "m_a [";
    for (int i = 0; i < arity; ++i)
    {
        output[i] = (value & (1 << i)) != 0;
        // std::cout << output[i] << ",";
    }
    // std::cout << "]" << std::endl;
}

int StatePairTupleIndexGenerator::const_iterator::find_rightmost_incrementable_index()
{
    const int arity = m_tuple_index_mapper->get_arity();
    int i = arity - 1;

    // Rightmost check: new index must be within bounds
    if (m_indices[i] < static_cast<int>((*m_a_atom_indices)[m_a[i]].size()) - 1)
    {
        return i;
    }
    --i;

    // Inner check: new index must be within bounds and its increased value must be smaller than the value right to it.
    while (i >= 0
           && ((m_indices[i] == static_cast<int>((*m_a_atom_indices)[m_a[i]].size()) - 1)
               || ((*m_a_atom_indices)[m_a[i]][m_indices[i] + 1] >= (*m_a_atom_indices)[m_a[i + 1]][m_indices[i + 1]])))
    {
        --i;
    }
    return i;
}

int StatePairTupleIndexGenerator::const_iterator::find_new_index(int i)
{
    if (m_a[i - 1] == m_a[i])
    {
        if (m_a[i] == 0)
        {
            // Cap at placeholder index
            return std::min(static_cast<int>((*m_a_atom_indices)[m_a[i]].size()) - 1, m_indices[i - 1] + 1);
        }
        else
        {
            if (m_indices[i - 1] == static_cast<int>((*m_a_atom_indices)[m_a[i]].size()) - 1)
            {
                // Cannot increment index
                return UNDEFINED;
            }
            else
            {
                return m_indices[i - 1] + 1;
            }
        }
    }
    else
    {
        if (m_a[i] == 0)
        {
            // Cap at placeholder index
            return std::min(static_cast<int>((*m_a_atom_indices)[m_a[i]].size()) - 1, (*m_a_index_jumper)[m_a[i - 1]][m_indices[i - 1]]);
        }
        else
        {
            if ((*m_a_index_jumper)[m_a[i - 1]][m_indices[i - 1]] == UNDEFINED)
            {
                // Cannot increment index
                return UNDEFINED;
            }
            else
            {
                return (*m_a_index_jumper)[m_a[i - 1]][m_indices[i - 1]];
            }
        }
    }
    return UNDEFINED;
}

bool StatePairTupleIndexGenerator::const_iterator::advance_outter()
{
    // Fetch data
    const int arity = m_tuple_index_mapper->get_arity();
    const auto& factors = m_tuple_index_mapper->get_factors();

    /* Advance outter iteration */

    // In the constructor call, advances to 1, meaning that we would like to pick exactly one atom index from add_atom_index
    ++m_cur_outter;

    // 1 << arity == 2^arity
    for (; m_cur_outter < (1 << arity); ++m_cur_outter)
    {
        // Create a binary representation, e,g., arity = 4 and m_cur_outter = 3 => [1,0,1,0] meaning that
        // the first and third indices should be chosen from add_atom_indices, and
        // the second and fourth indices should be chosen from atom_indices
        compute_binary_representation(m_cur_outter, arity, m_a);

        /* Initialize inner iteration */

        // Create the begin set of indices and its corresponding tuple_index
        // If no such tuple_index exists, continue with next m_cur_outter.
        m_indices[0] = 0;
        m_cur_inner = (*m_a_atom_indices)[m_a[0]][0] * factors[0];

        bool failed = false;
        for (int j = 1; j < arity; ++j)
        {
            int new_index = find_new_index(j);

            // std::cout << "j: " << j << " new_index: " << new_index << std::endl;

            if (new_index == UNDEFINED)
            {
                failed = true;
                break;
            }
            else
            {
                m_indices[j] = new_index;
                // Initial update
                m_cur_inner += factors[j] * (*m_a_atom_indices)[m_a[j]][new_index];
            }
        }
        if (!failed)
        {
            m_end_inner = false;
            m_end_outter = false;
            return true;
        }
    }

    // No valid tuple was found => set to end
    m_end_inner = true;
    m_end_outter = true;

    return false;
}

void StatePairTupleIndexGenerator::const_iterator::advance_inner()
{
    // Fetch data
    const int arity = m_tuple_index_mapper->get_arity();
    const auto& factors = m_tuple_index_mapper->get_factors();

    while (true)
    {
        /* 1. Advance outter iteration */

        if (m_end_inner)
        {
            advance_outter();
            // Either points to next tuple index or m_outter_end and m_inner_end
            return;
        }

        /* 2. Advance inner iteration */

        // 2.1. Find the rightmost index and increment it
        int i = find_rightmost_incrementable_index();
        // std::cout << "i: " << i << std::endl;
        if (i < 0)
        {
            // Continue to next outter loop when all indices have reached their maximum values
            m_end_inner = true;
            continue;
        }
        int index = ++m_indices[i];

        // 2.2. Difference update
        m_cur_inner += factors[i] * ((*m_a_atom_indices)[m_a[i]][index] - (*m_a_atom_indices)[m_a[i]][index - 1]);

        // 2.3. Update indices right of the incremented rightmost index i.
        bool failed = false;
        for (int j = i + 1; j < arity; ++j)
        {
            // Backup old_index for difference update
            int old_index = m_indices[j];

            int new_index = find_new_index(j);
            if (new_index == UNDEFINED)
            {
                failed = true;
                break;
            }
            else
            {
                m_indices[j] = new_index;
                // Difference update
                m_cur_inner += factors[j] * ((*m_a_atom_indices)[m_a[j]][new_index] - (*m_a_atom_indices)[m_a[j]][old_index]);
            }
        }
        if (failed)
        {
            // Continue to next outter loop when no tuple index can be constructed anymore
            m_end_inner = true;
            continue;
        }

        // Exit because construction of next tuple index was successful
        return;
    }
}

StatePairTupleIndexGenerator::const_iterator::value_type StatePairTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_tuple_index_mapper && m_a_atom_indices && m_a_index_jumper);
    // Do not allow interpreting end as position.
    assert(!(m_end_inner && m_end_outter));

    return m_cur_inner;
}

StatePairTupleIndexGenerator::const_iterator& StatePairTupleIndexGenerator::const_iterator::operator++()
{
    advance_inner();
    return *this;
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool StatePairTupleIndexGenerator::const_iterator::operator==(const const_iterator& other) const
{
    // Compare against end iterator
    if (m_end_inner == other.m_end_inner && m_end_outter == other.m_end_outter)
    {
        return true;
    }
    // Compare outter and inner iterator positions
    return m_cur_outter == other.m_cur_outter && m_cur_inner == other.m_cur_inner;
}

bool StatePairTupleIndexGenerator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::begin(const State state, const State succ_state)
{
    atom_index_mapper->remap_and_combine_and_sort(state, succ_state, a_atom_indices[0], a_atom_indices[1]);
    // Add place holder to generate tuples if size < arity
    a_atom_indices[0].push_back(tuple_index_mapper->get_num_atoms());

    // std::cout << "atom_indices: " << a_atom_indices[0] << std::endl;
    // std::cout << "add_atom_indices: " << a_atom_indices[1] << std::endl;

    assert(std::is_sorted(a_atom_indices[0].begin(), a_atom_indices[0].end()));
    assert(std::is_sorted(a_atom_indices[1].begin(), a_atom_indices[1].end()));

    return const_iterator(this, true);
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::begin(const AtomIndexList& atom_indices, const AtomIndexList& add_atom_indices)
{
    a_atom_indices[0] = atom_indices;
    a_atom_indices[1] = add_atom_indices;
    assert(std::is_sorted(a_atom_indices[0].begin(), a_atom_indices[0].end()));
    assert(std::is_sorted(a_atom_indices[1].begin(), a_atom_indices[1].end()));

    return const_iterator(this, true);
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * DynamicNoveltyTable
 */

DynamicNoveltyTable::DynamicNoveltyTable(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper, std::shared_ptr<TupleIndexMapper> tuple_index_mapper) :
    m_atom_index_mapper(std::move(atom_index_mapper)),
    m_tuple_index_mapper(std::move(tuple_index_mapper)),
    m_table(std::vector<bool>(m_tuple_index_mapper->get_max_tuple_index() + 1, false)),
    m_state_tuple_index_generator(m_atom_index_mapper, m_tuple_index_mapper),
    m_state_pair_tuple_index_generator(m_atom_index_mapper, m_tuple_index_mapper)
{
}

void DynamicNoveltyTable::resize_to_fit(int atom_index)
{
    // Fetch data.
    const auto arity = m_tuple_index_mapper->get_arity();
    const auto old_placeholder = m_tuple_index_mapper->get_num_atoms();

    // Compute size of new table
    int new_size = m_tuple_index_mapper->get_num_atoms();
    while (new_size < atom_index + 1 + 1)  // additional +1 for placeholder
    {
        // Use doubling strategy to get amortized cost O(arity) for resize.
        new_size *= 2;
    }
    const int new_placeholder = new_size;
    auto new_tuple_index_mapper = std::make_shared<TupleIndexMapper>(arity, new_size);
    auto new_table = std::vector<bool>(new_tuple_index_mapper->get_max_tuple_index() + 1, false);

    // Convert tuple indices that are not novel from old to new table.
    auto atom_indices = AtomIndexList(arity);
    for (int tuple_index = 0; tuple_index < static_cast<int>(m_table.size()); ++tuple_index)
    {
        if (m_table[tuple_index])
        {
            m_tuple_index_mapper->to_atom_indices(tuple_index, atom_indices);
            for (int i = 0; i < arity; ++i)
            {
                if (atom_indices[i] == old_placeholder)
                {
                    atom_indices[i] = new_placeholder;
                }
            }
            const int new_tuple_index = new_tuple_index_mapper->to_tuple_index(atom_indices);
            new_table[new_tuple_index] = true;
        }
    }

    // Swap old and new data.
    m_tuple_index_mapper = std::move(new_tuple_index_mapper);
    m_table = std::move(new_table);
}

void DynamicNoveltyTable::compute_novel_tuple_indices(const State state, TupleIndexList& out_novel_tuple_indices)
{
    out_novel_tuple_indices.clear();

    for (auto it = m_state_tuple_index_generator.begin(state); it != m_state_tuple_index_generator.end(); ++it)
    {
        const int tuple_index = *it;

        assert(tuple_index < static_cast<int>(m_table.size()));

        if (!m_table[tuple_index])
        {
            out_novel_tuple_indices.push_back(tuple_index);
        }
    }
}

void DynamicNoveltyTable::insert_tuple_indices(const TupleIndexList& tuple_indices)
{
    for (const auto& tuple_index : tuple_indices)
    {
        assert(tuple_index < static_cast<int>(m_table.size()));

        m_table[tuple_index] = true;
    }
}

bool DynamicNoveltyTable::test_novelty_and_update_table(const State state)
{
    bool is_novel = false;
    for (auto it = m_state_tuple_index_generator.begin(state); it != m_state_tuple_index_generator.end(); ++it)
    {
        const int tuple_index = *it;

        // std::cout << tuple_index << " " << m_tuple_index_mapper->tuple_index_to_string(tuple_index) << std::endl;

        assert(tuple_index < static_cast<int>(m_table.size()));

        if (!is_novel && !m_table[tuple_index])
        {
            is_novel = true;
        }
        m_table[tuple_index] = true;
    }
    return is_novel;
}

bool DynamicNoveltyTable::test_novelty_and_update_table(const State state, const State succ_state)
{
    bool is_novel = false;
    for (auto it = m_state_pair_tuple_index_generator.begin(state, succ_state); it != m_state_pair_tuple_index_generator.end(); ++it)
    {
        const int tuple_index = *it;

        // std::cout << tuple_index << " " << m_tuple_index_mapper->tuple_index_to_string(tuple_index) << std::endl;

        assert(tuple_index < static_cast<int>(m_table.size()));

        if (!is_novel && !m_table[tuple_index])
        {
            is_novel = true;
        }
        m_table[tuple_index] = true;
    }
    return is_novel;
}

void DynamicNoveltyTable::reset() { std::fill(m_table.begin(), m_table.end(), false); }

/**
 * NoveltyPruning
 */

ArityZeroNoveltyPruning::ArityZeroNoveltyPruning(State initial_state) : m_initial_state(initial_state) {}

bool ArityZeroNoveltyPruning::test_prune_initial_state(const State state) { return false; }

bool ArityZeroNoveltyPruning::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) { return state != m_initial_state; }

ArityKNoveltyPruning::ArityKNoveltyPruning(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper) :
    m_novelty_table(std::move(atom_index_mapper), std::make_shared<TupleIndexMapper>(arity, num_atoms))
{
}

bool ArityKNoveltyPruning::test_prune_initial_state(const State state)
{
    if (m_generated_states.count(state.get_id()))
    {
        assert(!m_novelty_table.test_novelty_and_update_table(state));
        return true;
    }
    m_generated_states.insert(state.get_id());

    return !m_novelty_table.test_novelty_and_update_table(state);
}

bool ArityKNoveltyPruning::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ)
{
    if (m_generated_states.count(succ_state.get_id()))
    {
        assert(!m_novelty_table.test_novelty_and_update_table(state, succ_state));
        return true;
    }
    m_generated_states.insert(succ_state.get_id());

    return !m_novelty_table.test_novelty_and_update_table(state, succ_state);
}
}