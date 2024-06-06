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

#include <sstream>

namespace mimir
{

/**
 * TupleIndexMapper
 */

TupleIndexMapper::TupleIndexMapper(int arity, int num_atoms) : m_arity(arity), m_num_atoms(num_atoms)
{
    if (!(arity > 0 && arity < MAX_ARITY))
    {
        throw std::runtime_error("TupleIndexMapper only works with arity > 0 and arity < " + std::to_string(MAX_ARITY) + ".");
    }
    for (int i = 0; i < m_arity; ++i)
    {
        m_factors[i] = std::pow(m_num_atoms, i);
    }
}

TupleIndex TupleIndexMapper::to_tuple_index(const AtomIndices& atom_indices) const
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

void TupleIndexMapper::to_atom_indices(TupleIndex tuple_index, AtomIndices& out_atom_indices) const
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
    auto atom_indices = AtomIndices {};
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

const int* TupleIndexMapper::get_factors() const { return m_factors; }

int TupleIndexMapper::get_max_tuple_index() const
{
    // x^0 + ... + x^n = (x^{n+1} - 1) / 2
    return ((std::pow(get_num_atoms(), get_arity() + 1) - 1) / 2) - 1;
}

/**
 * SingleStateTupleIndexGenerator
 */

SingleStateTupleIndexGenerator::SingleStateTupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices) :
    m_tuple_index_mapper(&tuple_index_mapper),
    m_atom_indices(&atom_indices)
{
}

SingleStateTupleIndexGenerator::const_iterator::const_iterator() : m_tuple_index_mapper(nullptr), m_atom_indices(nullptr) {}

SingleStateTupleIndexGenerator::const_iterator::const_iterator(const TupleIndexMapper& tuple_index_mapper, const AtomIndices& atom_indices, bool begin) :
    m_tuple_index_mapper(&tuple_index_mapper),
    m_atom_indices(&atom_indices),
    m_cur(begin ? 0 : tuple_index_mapper.get_max_tuple_index() + 1),
    m_indices({})
{
    assert(!atom_indices.empty());
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));

    if (begin)
    {
        // Find the indices and set begin tuple index
        const int* factors = tuple_index_mapper.get_factors();
        for (int i = 0; i < tuple_index_mapper.get_arity(); ++i)
        {
            m_indices[i] = i;
            m_cur += atom_indices[i] * factors[i];
        }
        assert(std::is_sorted(m_indices, m_indices + tuple_index_mapper.get_arity()));
    }
}

void SingleStateTupleIndexGenerator::const_iterator::next_tuple_index()
{
    // Increment indices from right to left until obtaining an
    // array of form {num_atoms,...,num_atoms} indicating the end
    // Update the tuple index on the fly.

    // Fetch data
    const int arity = m_tuple_index_mapper->get_arity();
    const int* factors = m_tuple_index_mapper->get_factors();

    const int num_atoms = m_atom_indices->size();

    // Find the rightmost index to increment, i.e., non place-holder.
    int i = m_tuple_index_mapper->get_arity() - 1;
    while (i >= 0 && (m_indices[i] == num_atoms - 1))
    {
        --i;
    }
    if (i < 0)
    {
        // Exit when all indices have reached their maximum values
        ++m_cur;
        return;
    }

    // Advance and update cur based on the change
    const int index = ++m_indices[i];
    m_cur += factors[i] * ((*m_atom_indices)[index] - (*m_atom_indices)[index - 1]);

    // Update indices right of the incremented rightmost index i.
    for (int j = i + 1; j < arity; ++j)
    {
        int old_index = m_indices[j];
        if (old_index == num_atoms - 1)
        {
            // Exit when index an update would result in an index larger than the place holder.
            return;
        }

        // Update and update cur based on the change
        int new_index = m_indices[j] = m_indices[j - 1] + 1;
        m_cur += factors[j] * ((*m_atom_indices)[new_index] - (*m_atom_indices)[old_index]);
    }
}

SingleStateTupleIndexGenerator::const_iterator::value_type SingleStateTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_tuple_index_mapper && m_atom_indices);
    return m_cur;
}

SingleStateTupleIndexGenerator::const_iterator& SingleStateTupleIndexGenerator::const_iterator::operator++()
{
    next_tuple_index();
    return *this;
}

SingleStateTupleIndexGenerator::const_iterator SingleStateTupleIndexGenerator::const_iterator::operator++(int)
{
    const_iterator tmp = *this;
    ++(*this);
    return tmp;
}

bool SingleStateTupleIndexGenerator::const_iterator::operator==(const const_iterator& other) const { return m_cur == other.m_cur; }

bool SingleStateTupleIndexGenerator::const_iterator::operator!=(const const_iterator& other) const { return !(*this == other); }

SingleStateTupleIndexGenerator::const_iterator SingleStateTupleIndexGenerator::begin() const
{
    return const_iterator(*m_tuple_index_mapper, *m_atom_indices, true);
}

SingleStateTupleIndexGenerator::const_iterator SingleStateTupleIndexGenerator::end() const
{
    return const_iterator(*m_tuple_index_mapper, *m_atom_indices, false);
}

/**
 * StatePairTupleIndexGenerator
 */

StatePairTupleIndexGenerator::StatePairTupleIndexGenerator(const TupleIndexMapper& tuple_index_mapper,
                                                           const AtomIndices& atom_indices,
                                                           const AtomIndices& add_atom_indices) :
    m_tuple_index_mapper(&tuple_index_mapper),
    m_atom_indices(&atom_indices),
    m_add_atom_indices(&add_atom_indices)
{
}

StatePairTupleIndexGenerator::const_iterator::const_iterator() : m_tuple_index_mapper(nullptr), m_a_atom_indices({}) {}

StatePairTupleIndexGenerator::const_iterator::const_iterator(const TupleIndexMapper& tuple_index_mapper,
                                                             const AtomIndices& atom_indices,
                                                             const AtomIndices& add_atom_indices,
                                                             bool begin) :
    m_tuple_index_mapper(&tuple_index_mapper),
    m_a_atom_indices({}),
    m_a_num_atom_indices({}),
    m_end_inner(begin ? false : true),
    m_cur_inner(0),
    m_end_outter(begin ? false : true),
    m_cur_outter(0),
    m_a({}),
    m_a_index_jumper({}),
    m_indices({})
{
    // atom_indices can never be empty, due to adding the place holder.
    assert(!atom_indices.empty());
    // if add_atom_indices is empty, the iterator should not be called.
    assert(!add_atom_indices.empty());
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
    assert(std::is_sorted(add_atom_indices.begin(), add_atom_indices.end()));

    if (begin)
    {
        // Rearrange data.
        m_a_atom_indices[0] = &atom_indices;
        m_a_num_atom_indices[0] = atom_indices.size();
        m_a_atom_indices[1] = &add_atom_indices;
        m_a_num_atom_indices[1] = add_atom_indices.size();

        // Initialize m_a_index_jumper to know the next large element in the opposite atom indices vector when iterating
        initialize_index_jumper();

        // Initialize m_cur_outter and m_cur_inner
        next_outter_begin();
    }
}

void StatePairTupleIndexGenerator::const_iterator::initialize_index_jumper()
{
    for (int i = 0; i < m_a_num_atom_indices[0]; ++i)
    {
        m_a_index_jumper[0][i] = std::numeric_limits<int>::max();
    }
    for (int i = 0; i < m_a_num_atom_indices[1]; ++i)
    {
        m_a_index_jumper[1][i] = std::numeric_limits<int>::max();
    }
    int j = 0;
    int i = 0;
    while (j < m_a_num_atom_indices[0] && i < m_a_num_atom_indices[1])
    {
        if ((*m_a_atom_indices[0])[j] < (*m_a_atom_indices[1])[i])
        {
            m_a_index_jumper[0][j] = i;
            ++j;
        }
        else if ((*m_a_atom_indices[0])[j] > (*m_a_atom_indices[1])[i])
        {
            m_a_index_jumper[1][i] = j;
            ++i;
        }
        else
        {
            m_a_index_jumper[1][i] = j;
            m_a_index_jumper[0][j] = i;
            ++j;
            ++i;
        }
    }
}

bool StatePairTupleIndexGenerator::const_iterator::next_outter_begin()
{
    const int arity = m_tuple_index_mapper->get_arity();
    const int* factors = m_tuple_index_mapper->get_factors();

    /* Advance outter iteration */

    // In the constructor call, advance to 1, meaning that we would like to pick exactly one atom index from add_atom_index
    ++m_cur_outter;

    for (; m_cur_outter < std::pow(2, arity); ++m_cur_outter)
    {
        // Create a binary representation, e,g., arity = 4 and m_cur_outter = 3 => [1,0,1,0] meaning that
        // the first and third indices should be chosen from add_atom_indices, and
        // the second and fourth indices should be chosen from atom_indices
        int tmp_cur_outter = m_cur_outter;
        for (int i = 0; i < arity; ++i)
        {
            m_a[i] = (tmp_cur_outter & 1) > 0;
            tmp_cur_outter >>= 1;
        }

        /* Initialize inner iteration */

        // Create the begin set of indices and its corresponding tuple_index
        // If no such tuple_index exists, continue with next m_cur_outter.
        m_indices[0] = 0;
        m_cur_inner = (*m_a_atom_indices[m_a[0]])[0] * factors[0];
        bool has_valid_tuple_index = true;
        for (int i = 1; i < arity; ++i)
        {
            const int prev_a = m_a[i - 1];
            const int prev_index = m_indices[i - 1];
            const int cur_index = m_a_index_jumper[prev_a][prev_index];
            if (cur_index == std::numeric_limits<int>::max())
            {
                has_valid_tuple_index = false;
                break;
            }
            const int cur_a = m_a[i];
            const int cur_atom_index = (*m_a_atom_indices[cur_a])[cur_index];
            m_indices[i] = cur_index;
            m_cur_inner += cur_atom_index * factors[i];
        }
        if (has_valid_tuple_index)
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

void StatePairTupleIndexGenerator::const_iterator::next_tuple_index()
{
    while (true)
    {
        const int arity = m_tuple_index_mapper->get_arity();
        const int* factors = m_tuple_index_mapper->get_factors();

        /* Advance outter iteration */

        if (m_end_inner)
        {
            next_outter_begin();
            // Either points to next tuple index or m_outter_end and m_inner_end
            return;
        }

        /* Advance inner iteration */

        // Find the rightmost index and increment it
        int i = arity - 1;
        while (i >= 0 && (m_indices[i] >= m_a_num_atom_indices[m_a[i]] - 1))
        {
            --i;
        }
        if (i < 0)
        {
            // Continue to next outter loop when all indices have reached their maximum values
            m_end_inner = true;
            continue;
        }
        int index = ++m_indices[i];
        m_cur_inner += factors[i] * ((*m_a_atom_indices[m_a[i]])[index] - (*m_a_atom_indices[m_a[i]])[index - 1]);

        // Update indices right of the incremented rightmost index i.
        bool exhausted = false;
        for (int j = i + 1; j < arity; ++j)
        {
            int old_index = m_indices[j];
            int new_index = -1;

            if (m_a[j - 1] == m_a[j])
            {
                if (old_index == m_a_num_atom_indices[m_a[j]] - 1)
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = m_indices[j - 1] + 1;
            }
            else
            {
                if (m_a_index_jumper[m_a[j - 1]][m_indices[j - 1]] == std::numeric_limits<int>::max())
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = m_a_index_jumper[m_a[j - 1]][m_indices[j - 1]];
            }
            assert(new_index >= 0);

            m_cur_inner += factors[j] * ((*m_a_atom_indices[m_a[j]])[new_index] - (*m_a_atom_indices[m_a[j]])[old_index]);
        }
        if (exhausted)
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
    assert(m_tuple_index_mapper && &m_a_atom_indices[0] && &m_a_atom_indices[1]);
    // Do not allow interpreting end as position.
    assert(!(m_end_inner && m_end_outter));

    return m_cur_inner;
}

StatePairTupleIndexGenerator::const_iterator& StatePairTupleIndexGenerator::const_iterator::operator++()
{
    next_tuple_index();
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

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::begin() const
{
    return const_iterator(*m_tuple_index_mapper, *m_atom_indices, *m_add_atom_indices, true);
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::end() const
{
    return const_iterator(*m_tuple_index_mapper, *m_atom_indices, *m_add_atom_indices, false);
}

/**
 * DynamicNoveltyTable
 */

DynamicNoveltyTable::DynamicNoveltyTable(int arity, int num_atoms) :
    m_tuple_index_mapper(TupleIndexMapper(arity, num_atoms)),
    m_table(std::vector<bool>(m_tuple_index_mapper.get_max_tuple_index() + 1, false))
{
}

void DynamicNoveltyTable::resize_to_fit(int atom_index)
{
    // Fetch data.
    const auto arity = m_tuple_index_mapper.get_arity();
    const auto old_placeholder = m_tuple_index_mapper.get_num_atoms();

    // Compute size of new table
    int new_size = m_tuple_index_mapper.get_num_atoms();
    while (new_size < atom_index + 1 + 1)  // additional +1 for placeholder
    {
        // Use doubling strategy to get constant amortized cost of resize.
        new_size *= 2;
    }
    const int new_placeholder = new_size;
    auto new_tuple_index_mapper = TupleIndexMapper(arity, new_size);
    auto new_table = std::vector<bool>(new_tuple_index_mapper.get_max_tuple_index() + 1, false);

    // Convert tuple indices that are not novel from old to new table.
    auto atom_indices = AtomIndices(arity);
    for (int tuple_index = 0; tuple_index < static_cast<int>(m_table.size()); ++tuple_index)
    {
        if (m_table[tuple_index])
        {
            m_tuple_index_mapper.to_atom_indices(tuple_index, atom_indices);
            for (int i = 0; i < arity; ++i)
            {
                if (atom_indices[i] == old_placeholder)
                {
                    atom_indices[i] = new_placeholder;
                }
            }
            const int new_tuple_index = new_tuple_index_mapper.to_tuple_index(atom_indices);
            new_table[new_tuple_index] = true;
        }
    }

    // Swap old and new data.
    m_tuple_index_mapper = std::move(new_tuple_index_mapper);
    m_table = std::move(new_table);
}

bool DynamicNoveltyTable::test_novelty_and_update_table(const State state)
{
    m_tmp_atom_indices.clear();
    for (const auto atom_id : state.get_atoms<Fluent>())
    {
        m_tmp_atom_indices.push_back(atom_id);
    }
    assert(std::is_sorted(m_tmp_atom_indices.begin(), m_tmp_atom_indices.end()));

    bool is_novel = false;
    for (const auto tuple_index : SingleStateTupleIndexGenerator(m_tuple_index_mapper, m_tmp_atom_indices))
    {
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
    m_tmp_atom_indices.clear();
    for (const auto atom_id : state.get_atoms<Fluent>())
    {
        m_tmp_atom_indices.push_back(atom_id);
    }
    assert(std::is_sorted(m_tmp_atom_indices.begin(), m_tmp_atom_indices.end()));

    m_tmp_add_atom_indices.clear();
    for (const auto atom_id : succ_state.get_atoms<Fluent>())
    {
        if (!state.get_atoms<Fluent>().get(atom_id))
        {
            m_tmp_add_atom_indices.push_back(atom_id);
        }
    }
    assert(std::is_sorted(m_tmp_add_atom_indices.begin(), m_tmp_add_atom_indices.end()));

    bool is_novel = false;
    for (const auto tuple_index : StatePairTupleIndexGenerator(m_tuple_index_mapper, m_tmp_atom_indices, m_tmp_add_atom_indices))
    {
        if (!is_novel && !m_table[tuple_index])
        {
            is_novel = true;
        }
        m_table[tuple_index] = true;
    }
    return is_novel;
}

/**
 * NoveltyPruning
 */

ArityZeroNoveltyPruning::ArityZeroNoveltyPruning(State initial_state) : m_initial_state(initial_state) {}

bool ArityZeroNoveltyPruning::test_prune_initial_state(const State state) { return false; }

bool ArityZeroNoveltyPruning::test_prune_successor_state(const State state, const State succ_state) { return state != m_initial_state; }

ArityKNoveltyPruning::ArityKNoveltyPruning(int arity, int num_atoms) : m_novelty_table(arity, num_atoms) {}

bool ArityKNoveltyPruning::test_prune_initial_state(const State state) { return m_novelty_table.test_novelty_and_update_table(state); }

bool ArityKNoveltyPruning::test_prune_successor_state(const State state, const State succ_state)
{
    return m_novelty_table.test_novelty_and_update_table(state, succ_state);
}
}