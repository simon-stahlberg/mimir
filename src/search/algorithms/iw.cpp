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
 * StateTupleIndexGenerator
 */

StateTupleIndexGenerator::IteratorData::IteratorData(int arity, std::shared_ptr<TupleIndexMapper> tuple_index_mapper_) :
    tuple_index_mapper(std::move(tuple_index_mapper_)),
    atom_indices(),
    indices(arity)
{
    assert(tuple_index_mapper);
}

StateTupleIndexGenerator::StateTupleIndexGenerator(IteratorData& data) : m_data(data) {}

StateTupleIndexGenerator::const_iterator::const_iterator() : m_data(nullptr) {}

StateTupleIndexGenerator::const_iterator::const_iterator(IteratorData* data, bool begin) : m_data(data), m_end(!begin), m_cur(begin ? 0 : -1)
{
    if (begin)
    {
        // Fetch data
        const auto& tuple_index_mapper = *m_data->tuple_index_mapper;
        const int arity = tuple_index_mapper.get_arity();
        const int* factors = tuple_index_mapper.get_factors();
        const auto& atom_indices = m_data->atom_indices;
        auto& indices = m_data->indices;

        assert(!atom_indices.empty());
        assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
        assert(static_cast<int>(indices.size()) == arity);

        // Find the indices and set begin tuple index
        for (int i = 0; i < arity; ++i)
        {
            indices[i] = i;
            m_cur += atom_indices[i] * factors[i];
        }
        assert(std::is_sorted(indices.begin(), indices.end()));
    }
    else
    {
        assert(!data);
    }
}

void StateTupleIndexGenerator::const_iterator::advance()
{
    // Increment indices from right to left until obtaining an
    // array of form {num_atoms,...,num_atoms} indicating the end
    // Update the tuple index on the fly.

    // Fetch data
    const auto& tuple_index_mapper = *m_data->tuple_index_mapper;
    const int arity = tuple_index_mapper.get_arity();
    const int* factors = tuple_index_mapper.get_factors();
    const auto& atom_indices = m_data->atom_indices;
    const int num_atoms = atom_indices.size();
    auto& indices = m_data->indices;

    // Find the rightmost index to increment, i.e., non place-holder.
    int i = arity - 1;
    while (i >= 0 && (m_data->indices[i] == num_atoms - 1))
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
    const int index = ++indices[i];
    m_cur += factors[i] * (atom_indices[index] - atom_indices[index - 1]);

    // Update indices right of the incremented rightmost index i.
    for (int j = i + 1; j < arity; ++j)
    {
        if (indices[j - 1] < indices[j])
        {
            // Legal tuple state reached.
            return;
        }

        int old_index = indices[j];
        if (old_index == num_atoms - 1)
        {
            // Exit when index an update would result in an index larger than the place holder.
            m_end = true;
            return;
        }

        // Update and update cur based on the change
        int new_index = indices[j] = indices[j - 1] + 1;
        m_cur += factors[j] * (atom_indices[new_index] - atom_indices[old_index]);
    }
}

StateTupleIndexGenerator::const_iterator::value_type StateTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_data->tuple_index_mapper && m_data);
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

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::begin() const { return const_iterator(&m_data, true); }

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * StatePairTupleIndexGenerator
 */

StatePairTupleIndexGenerator::IteratorData::IteratorData(int arity, std::shared_ptr<TupleIndexMapper> tuple_index_mapper_) :
    tuple_index_mapper(std::move(tuple_index_mapper_)),
    a(arity),
    a_index_jumper(),
    a_atom_indices(),
    a_num_atom_indices(),
    indices(arity)
{
    assert(tuple_index_mapper);
}

StatePairTupleIndexGenerator::StatePairTupleIndexGenerator(IteratorData& data) : m_data(data) {}

StatePairTupleIndexGenerator::const_iterator::const_iterator() : m_data(nullptr) {}

StatePairTupleIndexGenerator::const_iterator::const_iterator(IteratorData* data, bool begin) :
    m_data(data),
    m_end_inner(begin ? false : true),
    m_cur_inner(begin ? 0 : -1),
    m_end_outter(begin ? false : true),
    m_cur_outter(begin ? 0 : -1)
{
    if (begin)
    {
        // Fetch data
        const auto& atom_indices = m_data->a_atom_indices[0];
        const auto& add_atom_indices = m_data->a_atom_indices[1];
        const auto& num_atom_indices = m_data->a_num_atom_indices[0];
        const auto& num_add_atom_indices = m_data->a_num_atom_indices[1];
        auto& indices = m_data->indices;
        const auto arity = m_data->tuple_index_mapper->get_arity();
        const auto& a = m_data->a;

        // atom_indices can never be empty, due to adding the place holder.
        assert(!atom_indices.empty());
        // if add_atom_indices is empty, the iterator should not be called.
        // If this assertion triggers, there must be a in the tuple index generation.
        assert(!add_atom_indices.empty());
        assert(static_cast<int>(atom_indices.size()) == num_atom_indices);
        assert(static_cast<int>(add_atom_indices.size()) == num_add_atom_indices);
        assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
        assert(std::is_sorted(add_atom_indices.begin(), add_atom_indices.end()));
        assert(static_cast<int>(a.size()) == arity);
        assert(static_cast<int>(indices.size()) == arity);

        // Initialize m_a_index_jumper to know the next large element in the opposite atom indices vector when iterating
        initialize_index_jumper();

        // Initialize m_cur_outter and m_cur_inner
        advance_outter();
    }
    else
    {
        assert(!m_data);
    }
}

void StatePairTupleIndexGenerator::const_iterator::initialize_index_jumper()
{
    // Fetch data
    const auto& a_atom_indices = m_data->a_atom_indices;
    const auto& a_num_atom_indices = m_data->a_num_atom_indices;
    auto& a_index_jumper = m_data->a_index_jumper;

    a_index_jumper[0].clear();
    a_index_jumper[1].clear();
    a_index_jumper[0].resize(a_num_atom_indices[0], std::numeric_limits<int>::max());
    a_index_jumper[1].resize(a_num_atom_indices[1], std::numeric_limits<int>::max());

    int j = 0;
    int i = 0;
    while (j < a_num_atom_indices[0] && i < a_num_atom_indices[1])
    {
        if (a_atom_indices[0][j] < a_atom_indices[1][i])
        {
            a_index_jumper[0][j] = i;
            ++j;
        }
        else if (a_atom_indices[0][j] > a_atom_indices[1][i])
        {
            a_index_jumper[1][i] = j;
            ++i;
        }
        else
        {
            a_index_jumper[1][i] = j;
            a_index_jumper[0][j] = i;
            ++j;
            ++i;
        }
    }
}

bool StatePairTupleIndexGenerator::const_iterator::advance_outter()
{
    // Fetch data
    const auto& tuple_index_mapper = *m_data->tuple_index_mapper;
    const int arity = tuple_index_mapper.get_arity();
    const int* factors = tuple_index_mapper.get_factors();
    auto& a = m_data->a;
    auto& indices = m_data->indices;
    const auto& a_atom_indices = m_data->a_atom_indices;
    const auto& a_num_atom_indices = m_data->a_num_atom_indices;
    const auto& a_index_jumper = m_data->a_index_jumper;

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
            a[i] = (tmp_cur_outter & 1) > 0;
            tmp_cur_outter >>= 1;
        }

        /* Initialize inner iteration */

        // Create the begin set of indices and its corresponding tuple_index
        // If no such tuple_index exists, continue with next m_cur_outter.
        indices[0] = 0;
        m_cur_inner = a_atom_indices[a[0]][0] * factors[0];
        bool exhausted = false;
        for (int i = 1; i < arity; ++i)
        {
            const int prev_index = indices[i - 1];
            const int prev_a = a[i - 1];
            const int cur_a = a[i];

            int new_index = -1;
            if (prev_a == cur_a)
            {
                if (prev_index == a_num_atom_indices[cur_a] - 1)
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = prev_index + 1;
            }
            else
            {
                if (a_index_jumper[prev_a][prev_index] == std::numeric_limits<int>::max())
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = a_index_jumper[prev_a][prev_index];
            }
            assert(new_index >= 0);

            indices[i] = new_index;
            m_cur_inner += factors[i] * a_atom_indices[cur_a][new_index];
        }
        if (!exhausted)
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
    while (true)
    {
        // Fetch data
        const auto& tuple_index_mapper = *m_data->tuple_index_mapper;
        const int arity = tuple_index_mapper.get_arity();
        const int* factors = tuple_index_mapper.get_factors();

        /* Advance outter iteration */

        if (m_end_inner)
        {
            advance_outter();
            // Either points to next tuple index or m_outter_end and m_inner_end
            return;
        }

        /* Advance inner iteration */

        // Fetch data
        auto& indices = m_data->indices;
        const auto& a_num_atom_indices = m_data->a_num_atom_indices;
        const auto& a = m_data->a;

        // Find the rightmost index and increment it
        int i = arity - 1;
        while (i >= 0 && (indices[i] == a_num_atom_indices[a[i]] - 1))
        {
            --i;
        }
        if (i < 0)
        {
            // Continue to next outter loop when all indices have reached their maximum values
            m_end_inner = true;
            continue;
        }

        // Fetch data
        const auto& a_atom_indices = m_data->a_atom_indices;
        const auto& a_index_jumper = m_data->a_index_jumper;

        int index = ++indices[i];
        m_cur_inner += factors[i] * (a_atom_indices[a[i]][index] - a_atom_indices[a[i]][index - 1]);

        // Update indices right of the incremented rightmost index i.
        bool exhausted = false;
        for (int j = i + 1; j < arity; ++j)
        {
            const int prev_index = indices[j - 1];
            const int cur_index = indices[j];
            int prev_a = a[j - 1];
            int cur_a = a[j];

            if (a_atom_indices[prev_a][prev_index] < a_atom_indices[cur_a][cur_index])
            {
                // Legal tuple state reached.
                return;
            }

            int old_index = indices[j];

            int new_index = -1;
            if (prev_a == cur_a)
            {
                if (old_index == a_num_atom_indices[cur_a] - 1)
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = prev_index + 1;
            }
            else
            {
                if (a_index_jumper[prev_a][prev_index] == std::numeric_limits<int>::max())
                {
                    // Cannot increment index
                    exhausted = true;
                    break;
                }
                new_index = a_index_jumper[prev_a][prev_index];
            }
            assert(new_index >= 0);

            indices[j] = new_index;
            m_cur_inner += factors[j] * (a_atom_indices[cur_a][new_index] - a_atom_indices[cur_a][old_index]);
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
    assert(m_data);
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

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::begin() const { return const_iterator(&m_data, true); }

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * FluentAndDerivedMapper
 */

FluentAndDerivedMapper::FluentAndDerivedMapper() : m_fluent_remap(), m_derived_remap(), m_num_atoms(0) {}

const int FluentAndDerivedMapper::UNDEFINED = -1;

void FluentAndDerivedMapper::remap_and_combine_and_sort(const State state, AtomIndices& out_atoms)
{
    out_atoms.clear();

    // Simultaneously remap and combine
    for (const auto& atom_id : state.get_atoms<Fluent>())
    {
        if (atom_id >= m_fluent_remap.size())
        {
            m_fluent_remap.resize(atom_id + 1, UNDEFINED);
            m_fluent_remap[atom_id] = m_num_atoms++;
        }
        else
        {
            if (m_fluent_remap[atom_id] == UNDEFINED)
            {
                m_fluent_remap[atom_id] = m_num_atoms++;
            }
        }
        out_atoms.push_back(m_fluent_remap[atom_id]);
    }

    for (const auto& atom_id : state.get_atoms<Derived>())
    {
        if (atom_id >= m_derived_remap.size())
        {
            m_derived_remap.resize(atom_id + 1, UNDEFINED);
            m_derived_remap[atom_id] = m_num_atoms++;
        }
        else
        {
            if (m_derived_remap[atom_id] == UNDEFINED)
            {
                m_derived_remap[atom_id] = m_num_atoms++;
            }
        }
        out_atoms.push_back(m_derived_remap[atom_id]);
    }

    // Sort
    std::sort(out_atoms.begin(), out_atoms.end());

    assert(out_atoms.size() == state.get_atoms<Fluent>().count() + state.get_atoms<Derived>().count());
}

void FluentAndDerivedMapper::remap_and_combine_and_sort(const State state, const State succ_state, AtomIndices& out_atoms, AtomIndices& out_add_atoms)
{
    out_atoms.clear();
    out_add_atoms.clear();

    // Simultaneously remap and combine
    for (const auto& atom_id : succ_state.get_atoms<Fluent>())
    {
        if (atom_id >= m_fluent_remap.size())
        {
            m_fluent_remap.resize(atom_id + 1, UNDEFINED);
            m_fluent_remap[atom_id] = m_num_atoms++;
        }
        else
        {
            if (m_fluent_remap[atom_id] == UNDEFINED)
            {
                m_fluent_remap[atom_id] = m_num_atoms++;
            }
        }
        if (state.get_atoms<Fluent>().get(atom_id))
        {
            out_atoms.push_back(m_fluent_remap[atom_id]);
        }
        else
        {
            out_add_atoms.push_back(m_fluent_remap[atom_id]);
        }
    }

    for (const auto& atom_id : succ_state.get_atoms<Derived>())
    {
        if (atom_id >= m_derived_remap.size())
        {
            m_derived_remap.resize(atom_id + 1, UNDEFINED);
            m_derived_remap[atom_id] = m_num_atoms++;
        }
        else
        {
            if (m_derived_remap[atom_id] == UNDEFINED)
            {
                m_derived_remap[atom_id] = m_num_atoms++;
            }
        }
        if (state.get_atoms<Derived>().get(atom_id))
        {
            out_atoms.push_back(m_derived_remap[atom_id]);
        }
        else
        {
            out_add_atoms.push_back(m_derived_remap[atom_id]);
        }
    }

    // Sort
    std::sort(out_atoms.begin(), out_atoms.end());
    std::sort(out_add_atoms.begin(), out_add_atoms.end());

    assert(out_atoms.size() + out_add_atoms.size() == succ_state.get_atoms<Fluent>().count() + succ_state.get_atoms<Derived>().count());
}

/**
 * DynamicNoveltyTable
 */

DynamicNoveltyTable::DynamicNoveltyTable(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper) :
    m_atom_index_mapper(std::move(atom_index_mapper)),
    m_tuple_index_mapper(std::make_shared<TupleIndexMapper>(arity, num_atoms)),
    m_table(std::vector<bool>(m_tuple_index_mapper->get_max_tuple_index() + 1, false)),
    m_single_state_iterator_data(arity, m_tuple_index_mapper),
    m_pair_state_iterator_data(arity, m_tuple_index_mapper)
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
    auto atom_indices = AtomIndices(arity);
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

bool DynamicNoveltyTable::test_novelty_and_update_table(const State state)
{
    m_atom_index_mapper->remap_and_combine_and_sort(state, m_single_state_iterator_data.atom_indices);
    assert(std::is_sorted(m_single_state_iterator_data.atom_indices.begin(), m_single_state_iterator_data.atom_indices.end()));

    bool is_novel = false;
    for (const auto tuple_index : StateTupleIndexGenerator(m_single_state_iterator_data))
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
    auto& atom_indices = m_pair_state_iterator_data.a_atom_indices[0];
    auto& add_atom_indices = m_pair_state_iterator_data.a_atom_indices[1];
    auto& num_atom_indices = m_pair_state_iterator_data.a_num_atom_indices[0];
    auto& num_add_atom_indices = m_pair_state_iterator_data.a_num_atom_indices[1];

    m_atom_index_mapper->remap_and_combine_and_sort(state, succ_state, atom_indices, add_atom_indices);
    num_atom_indices = atom_indices.size();
    num_add_atom_indices = add_atom_indices.size();
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
    assert(std::is_sorted(add_atom_indices.begin(), add_atom_indices.end()));

    bool is_novel = false;
    for (const auto tuple_index : StatePairTupleIndexGenerator(m_pair_state_iterator_data))
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

bool ArityZeroNoveltyPruning::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) { return state != m_initial_state; }

ArityKNoveltyPruning::ArityKNoveltyPruning(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper) :
    m_novelty_table(arity, num_atoms, std::move(atom_index_mapper))
{
}

bool ArityKNoveltyPruning::test_prune_initial_state(const State state)
{
    if (m_generated_states.count(state.get_id()))
    {
        return true;
    }
    m_generated_states.insert(state.get_id());

    return !m_novelty_table.test_novelty_and_update_table(state);
}

bool ArityKNoveltyPruning::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ)
{
    if (m_generated_states.count(succ_state.get_id()))
    {
        return true;
    }
    m_generated_states.insert(succ_state.get_id());

    return !m_novelty_table.test_novelty_and_update_table(state, succ_state);
}
}