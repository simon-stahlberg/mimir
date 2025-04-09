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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/brfs/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers.hpp"
#include "mimir/search/algorithms/iw/event_handlers/interface.hpp"
#include "mimir/search/algorithms/iw/novelty_table.hpp"
#include "mimir/search/algorithms/iw/pruning_strategy.hpp"
#include "mimir/search/algorithms/iw/tuple_index_generators.hpp"
#include "mimir/search/algorithms/iw/tuple_index_mapper.hpp"
#include "mimir/search/algorithms/iw/types.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/utils.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <sstream>

using namespace mimir::formalism;

namespace mimir::search::iw
{

/**
 * TupleIndexMapper
 */

TupleIndexMapper::TupleIndexMapper(size_t arity) : TupleIndexMapper(arity, 0) {}

TupleIndexMapper::TupleIndexMapper(size_t arity, size_t num_atoms) : m_arity(arity), m_num_atoms(num_atoms), m_empty_tuple_index(0)
{
    initialize(arity, num_atoms);
}

void TupleIndexMapper::initialize(size_t arity, size_t num_atoms)
{
    m_arity = arity;
    m_num_atoms = num_atoms;
    m_empty_tuple_index = 0;

    if (!(arity >= 0 && arity < MAX_ARITY))
    {
        throw std::runtime_error("TupleIndexMapper only works with 0 <= arity < " + std::to_string(MAX_ARITY) + ".");
    }

    for (size_t i = 0; i < m_arity; ++i)
    {
        m_factors[i] = std::pow((m_num_atoms + 1), i);  ///< +1 to account for placeholder.
    }
}

TupleIndex TupleIndexMapper::to_tuple_index(const AtomIndexList& atom_indices) const
{
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));
    assert(atom_indices.size() <= m_arity);

    TupleIndex result = 0;
    // aggregate atom indices.
    for (size_t i = 0; i < atom_indices.size(); ++i)
    {
        result += m_factors[i] * atom_indices[i];
    }
    // aggregate placeholders
    for (size_t i = atom_indices.size(); i < m_arity; ++i)
    {
        result += m_factors[i] * m_num_atoms;
    }
    return result;
}

void TupleIndexMapper::to_atom_indices(TupleIndex tuple_index, AtomIndexList& out_atom_indices) const
{
    out_atom_indices.clear();

    for (int i = m_arity - 1; i >= 0; --i)
    {
        const auto atom_index = tuple_index / m_factors[i];

        if (atom_index != m_num_atoms)  // filter placeholder
        {
            out_atom_indices.push_back(atom_index);
        }

        tuple_index -= atom_index * m_factors[i];
    }
    std::reverse(out_atom_indices.begin(), out_atom_indices.end());
}

AtomIndexList TupleIndexMapper::to_atom_indices(TupleIndex tuple_index) const
{
    auto atom_indices = AtomIndexList {};
    to_atom_indices(tuple_index, atom_indices);
    return atom_indices;
}

std::string TupleIndexMapper::tuple_index_to_string(TupleIndex tuple_index) const
{
    auto atom_indices = to_atom_indices(tuple_index);
    std::stringstream ss;
    ss << "(";
    for (const auto atom_index : atom_indices)
    {
        ss << atom_index << ",";
    }
    ss << ")";
    return ss.str();
}

size_t TupleIndexMapper::get_num_atoms() const { return m_num_atoms; }

size_t TupleIndexMapper::get_arity() const { return m_arity; }

const std::array<size_t, MAX_ARITY>& TupleIndexMapper::get_factors() const { return m_factors; }

TupleIndex TupleIndexMapper::get_max_tuple_index() const { return get_empty_tuple_index(); }

TupleIndex TupleIndexMapper::get_empty_tuple_index() const { return (std::pow(get_num_atoms() + 1, get_arity()) - 1); }

/**
 * StateTupleIndexGenerator
 */

StateTupleIndexGenerator::StateTupleIndexGenerator(const TupleIndexMapper* tuple_index_mapper_) : tuple_index_mapper(tuple_index_mapper_), atom_indices() {}

StateTupleIndexGenerator::const_iterator::const_iterator() : m_tuple_index_mapper(nullptr), m_atoms(nullptr) {}

StateTupleIndexGenerator::const_iterator::const_iterator(StateTupleIndexGenerator* stig, bool begin) :
    m_tuple_index_mapper(begin ? stig->tuple_index_mapper : nullptr),
    m_atoms(begin ? &stig->atom_indices : nullptr),
    m_end(!begin),
    m_cur(begin ? 0 : -1)
{
    if (begin)
    {
        // Fetch data
        const auto arity = m_tuple_index_mapper->get_arity();
        const auto& factors = m_tuple_index_mapper->get_factors();

        assert(!m_atoms->empty());
        assert(std::is_sorted(m_atoms->begin(), m_atoms->end()));

        // Find the indices and set begin tuple index
        for (size_t i = 0; i < arity; ++i)
        {
            m_indices[i] = i;
            m_cur += (*m_atoms)[i] * factors[i];
        }
    }
    else
    {
        assert(!stig);
    }
}

std::optional<size_t> StateTupleIndexGenerator::const_iterator::find_rightmost_incrementable_index()
{
    const auto arity = get_tuple_index_mapper().get_arity();

    int i = static_cast<int>(arity) - 1;
    while (i >= 0 && (m_indices[i] == get_atoms().size() - 1))
    {
        --i;
    }
    return (i < 0) ? std::nullopt : std::optional<size_t>(i);
}

void StateTupleIndexGenerator::const_iterator::advance()
{
    /* Fetch data */
    const auto arity = get_tuple_index_mapper().get_arity();
    const auto& factors = get_tuple_index_mapper().get_factors();
    const auto num_atoms = get_atoms().size();

    /* Find the rightmost index to increment, i.e., non place-holder. */
    const auto rightmost_incrementable_index = find_rightmost_incrementable_index();
    if (!rightmost_incrementable_index)
    {
        // Exit when all indices have reached their maximum values
        m_end = true;
        ++m_cur;
        return;
    }
    const auto i = rightmost_incrementable_index.value();

    /* Advance and update cur based on the change. */
    const auto index = ++m_indices[i];
    const auto diff_i = get_atoms()[index] - get_atoms()[index - 1];
    m_cur += diff_i * factors[i];

    // Update indices right of the incremented rightmost index i.
    for (size_t j = i + 1; j < arity; ++j)
    {
        // Backup old_index for difference update
        size_t old_index = m_indices[j];

        // Cap at placeholder index
        size_t new_index = m_indices[j] = std::min(num_atoms - 1, m_indices[j - 1] + 1);

        // Update and update cur based on the change
        const auto diff_j = get_atoms()[new_index] - get_atoms()[old_index];
        m_cur += diff_j * factors[j];
    }
}

const TupleIndexMapper& StateTupleIndexGenerator::const_iterator::get_tuple_index_mapper() const
{
    assert(m_tuple_index_mapper);
    return *m_tuple_index_mapper;
}

const AtomIndexList& StateTupleIndexGenerator::const_iterator::get_atoms() const
{
    assert(m_atoms);
    return *m_atoms;
}

StateTupleIndexGenerator::const_iterator::value_type StateTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_tuple_index_mapper && m_atoms);
    assert(m_cur >= 0);
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
    atom_indices.clear();

    const auto& fluent_atoms = state->get_atoms<FluentTag>();
    atom_indices.insert(atom_indices.end(), fluent_atoms.begin(), fluent_atoms.end());
    // Add place holder to generate tuples of size < arity
    atom_indices.push_back(tuple_index_mapper->get_num_atoms());
    assert(std::is_sorted(atom_indices.begin(), atom_indices.end()));

    return const_iterator(this, true);
}

StateTupleIndexGenerator::const_iterator StateTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * StatePairTupleIndexGenerator
 */

StatePairTupleIndexGenerator::StatePairTupleIndexGenerator(const TupleIndexMapper* tuple_index_mapper_) : tuple_index_mapper(tuple_index_mapper_) {}

StatePairTupleIndexGenerator::const_iterator::const_iterator() :
    m_tuple_index_mapper(nullptr),
    m_a_atoms(nullptr),
    m_a_jumpers(nullptr),
    m_indices(),
    m_a(),
    m_cur_outter(-1),
    m_cur_inner(-1),
    m_end_outter(false),
    m_end_inner(false)
{
}

StatePairTupleIndexGenerator::const_iterator::const_iterator(StatePairTupleIndexGenerator* sptig, bool begin) :
    m_tuple_index_mapper(begin ? sptig->tuple_index_mapper : nullptr),
    m_a_atoms(begin ? &sptig->a_atom_indices : nullptr),
    m_a_jumpers(begin ? &sptig->a_index_jumper : nullptr),
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
        assert(!get_atoms()[0].empty());
        // Ensure that add_atom_indices is nonempty due to correct generation of tuples in previous states.
        assert(!get_atoms()[1].empty());
        // Ensure that atom indices are sorted
        assert(std::is_sorted(get_atoms()[0].begin(), get_atoms()[0].end()));
        assert(std::is_sorted(get_atoms()[1].begin(), get_atoms()[1].end()));

        // Initialize m_a_jumpers to know the next large element in the opposite atom indices vector when iterating
        initialize_jumpers();

        // Initialize m_cur_outter and m_cur_inner
        advance_outter();
    }
    else
    {
        assert(!sptig);
    }
}

void StatePairTupleIndexGenerator::const_iterator::initialize_jumpers()
{
    get_jumpers()[0].clear();
    get_jumpers()[1].clear();
    get_jumpers()[0].resize(get_atoms()[0].size(), std::numeric_limits<size_t>::max());
    get_jumpers()[1].resize(get_atoms()[1].size(), std::numeric_limits<size_t>::max());

    size_t j = 0;
    size_t i = 0;
    while (j < get_atoms()[0].size() && i < get_atoms()[1].size())
    {
        if (get_atoms()[0][j] < get_atoms()[1][i])
        {
            get_jumpers()[0][j] = i;
            ++j;
        }
        else if (get_atoms()[0][j] > get_atoms()[1][i])
        {
            get_jumpers()[1][i] = j;
            ++i;
        }
        else
        {
            get_jumpers()[1][i] = j;
            get_jumpers()[0][j] = i;
            ++j;
            ++i;
        }
    }
}

static void compute_binary_representation(size_t value, size_t arity, std::array<bool, MAX_ARITY>& output)
{
    for (size_t i = 0; i < arity; ++i)
    {
        output[i] = (value & (1 << i)) != 0;
    }
}

std::optional<size_t> StatePairTupleIndexGenerator::const_iterator::find_rightmost_incrementable_index()
{
    const int arity = get_tuple_index_mapper().get_arity();
    int i = arity - 1;

    // Rightmost check: new index must be within bounds
    if (m_indices[i] < get_atoms()[m_a[i]].size() - 1)
    {
        return i;
    }
    --i;

    // Inner check: new index must be within bounds and its increased value must be smaller than the value right to it.
    while (i >= 0 && ((m_indices[i] == get_atoms()[m_a[i]].size() - 1) || (get_atoms()[m_a[i]][m_indices[i] + 1] >= get_atoms()[m_a[i + 1]][m_indices[i + 1]])))
    {
        --i;
    }
    return (i < 0) ? std::nullopt : std::optional<size_t>(i);
}

std::optional<size_t> StatePairTupleIndexGenerator::const_iterator::find_next_index(size_t i)
{
    if (m_a[i - 1] == m_a[i])  // Jump between same set of atoms, i.e., atom_indices or add_atom_indices
    {
        if (!m_a[i])
        {
            // Same update as in the else case but simply capped at placeholder.
            return std::min(get_atoms()[m_a[i]].size() - 1, m_indices[i - 1] + 1);
        }
        else
        {
            if (m_indices[i - 1] == get_atoms()[m_a[i]].size() - 1)
            {
                // Cannot increment index
                return std::nullopt;
            }
            else
            {
                // Pick next larger atom from same set.
                return m_indices[i - 1] + 1;
            }
        }
    }
    else  // Jump across different set of atoms, i.e., atom_indices to add_atom_indices or vice versa.
    {
        if (!m_a[i])
        {
            // Same update as in the else case but simply capped at placeholder.
            return std::min(get_atoms()[m_a[i]].size() - 1, get_jumpers()[m_a[i - 1]][m_indices[i - 1]]);
        }
        else
        {
            if (get_jumpers()[m_a[i - 1]][m_indices[i - 1]] == std::numeric_limits<size_t>::max())
            {
                // Cannot increment index
                return std::nullopt;
            }
            else
            {
                // Jump over to next set of atoms.
                return get_jumpers()[m_a[i - 1]][m_indices[i - 1]];
            }
        }
    }
    return std::nullopt;
}

bool StatePairTupleIndexGenerator::const_iterator::advance_outter()
{
    // Fetch data
    const auto arity = get_tuple_index_mapper().get_arity();

    /* Advance outter iteration */

    // In the constructor call, advances to 1, meaning that we would like to pick exactly one atom index from add_atom_index
    ++m_cur_outter;

    // 1 << arity == 2^arity
    for (; m_cur_outter < (1 << arity); ++m_cur_outter)
    {
        // Create a binary representation, e,g., arity = 4 and m_cur_outter = 3 => [1,1,0,0] meaning that
        // the first and second indices should be chosen from add_atom_indices, and
        // the third and fourth indices should be chosen from atom_indices
        compute_binary_representation(m_cur_outter, arity, m_a);

        /* Initialize inner iteration */

        // Create the begin set of indices and its corresponding tuple_index
        // If no such tuple_index exists, continue with next m_cur_outter.
        if (try_create_first_inner_tuple())
        {
            m_end_inner = false;
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
        /* 1. Advance outter iteration */

        if (m_end_inner)
        {
            advance_outter();
            return;  // Exit because iterator either points to next tuple index or m_outter_end and m_inner_end
        }

        /* 2. Advance inner iteration */

        // 2.1. Find the rightmost index and increment it
        const auto right_most_incrementable_index = find_rightmost_incrementable_index();

        if (!right_most_incrementable_index)
        {
            m_end_inner = true;
            continue;  // Continue to next outter loop when all indices have reached their maximum values
        }
        const auto i = right_most_incrementable_index.value();

        if (!try_create_next_inner_tuple(i))
        {
            m_end_inner = true;
            continue;  // Continue to next outter loop when no tuple index can be constructed anymore
        }

        return;  // Exit because construction of next tuple index was successful
    }
}

bool StatePairTupleIndexGenerator::const_iterator::try_create_first_inner_tuple()
{
    // Fetch data
    const auto arity = get_tuple_index_mapper().get_arity();
    const auto& factors = get_tuple_index_mapper().get_factors();

    m_indices[0] = 0;
    const auto atom_0 = get_atoms()[m_a[0]][0];
    m_cur_inner = atom_0 * factors[0];

    for (size_t j = 1; j < arity; ++j)
    {
        const auto next_index = find_next_index(j);

        if (!next_index)
        {
            return false;  // Exit because construction of tuple index failed.
        }
        const auto new_index = next_index.value();

        m_indices[j] = new_index;
        const auto atom_j = get_atoms()[m_a[j]][new_index];
        m_cur_inner += atom_j * factors[j];
    }

    return true;
}

bool StatePairTupleIndexGenerator::const_iterator::try_create_next_inner_tuple(size_t i)
{
    // Fetch data
    const auto arity = get_tuple_index_mapper().get_arity();
    const auto& factors = get_tuple_index_mapper().get_factors();

    const auto index = ++m_indices[i];

    // 2.2. Difference update
    const auto diff_i = (get_atoms()[m_a[i]][index] - get_atoms()[m_a[i]][index - 1]);
    m_cur_inner += diff_i * factors[i];

    // 2.3. Update indices j right of the incremented rightmost index i.
    for (size_t j = i + 1; j < arity; ++j)
    {
        // Backup old_index for difference update
        const auto old_index = m_indices[j];

        const auto next_index = find_next_index(j);
        if (!next_index)
        {
            return false;  // Exit because construction of tuple index failed.
        }
        const auto new_index = next_index.value();

        m_indices[j] = new_index;
        // Difference update
        const auto diff_j = (get_atoms()[m_a[j]][new_index] - get_atoms()[m_a[j]][old_index]);
        m_cur_inner += diff_j * factors[j];
    }

    return true;
}

const TupleIndexMapper& StatePairTupleIndexGenerator::const_iterator::get_tuple_index_mapper() const
{
    assert(m_tuple_index_mapper);
    return *m_tuple_index_mapper;
}

const std::array<AtomIndexList, 2>& StatePairTupleIndexGenerator::const_iterator::get_atoms() const
{
    assert(m_a_atoms);
    return *m_a_atoms;
}

std::array<std::vector<size_t>, 2>& StatePairTupleIndexGenerator::const_iterator::get_jumpers() const
{
    assert(m_a_jumpers);
    return *m_a_jumpers;
}

StatePairTupleIndexGenerator::const_iterator::value_type StatePairTupleIndexGenerator::const_iterator::operator*() const
{
    assert(m_tuple_index_mapper && m_a_atoms && m_a_jumpers);
    assert(m_cur_inner >= 0);
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
    a_atom_indices[0].clear();
    a_atom_indices[1].clear();
    const auto& state_fluent_atoms = state->get_atoms<FluentTag>();
    const auto& succ_state_fluent_atoms = succ_state->get_atoms<FluentTag>();

    auto it1 = succ_state_fluent_atoms.begin();
    auto it2 = state_fluent_atoms.begin();

    while (it1 != succ_state_fluent_atoms.end() && it2 != state_fluent_atoms.end())
    {
        if (*it1 < *it2)
        {
            // Fluent only occurs in succ_state_fluent_atoms
            a_atom_indices[1].push_back(*it1);
            ++it1;
        }
        else if (*it2 < *it1)
        {
            ++it2;
        }
        else
        {
            // Common fluent found
            a_atom_indices[0].push_back(*it1);
            ++it1;
            ++it2;
        }
    }
    // Add the remaining atoms
    for (; it1 != succ_state_fluent_atoms.end(); ++it1)
    {
        a_atom_indices[1].push_back(*it1);
    }

    // Add place holder to generate tuples of size < arity
    a_atom_indices[0].push_back(tuple_index_mapper->get_num_atoms());

    assert(std::is_sorted(a_atom_indices[0].begin(), a_atom_indices[0].end()));
    assert(std::is_sorted(a_atom_indices[1].begin(), a_atom_indices[1].end()));

    /* Return end iterator immediately if the successors contains no added atoms. */
    return a_atom_indices[1].empty() ? const_iterator(nullptr, false) : const_iterator(this, true);
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::begin(const AtomIndexList& atom_indices, const AtomIndexList& add_atom_indices)
{
    a_atom_indices[0] = atom_indices;
    a_atom_indices[1] = add_atom_indices;
    assert(std::is_sorted(a_atom_indices[0].begin(), a_atom_indices[0].end()));
    assert(std::is_sorted(a_atom_indices[1].begin(), a_atom_indices[1].end()));

    /* Return end iterator immediately if the successors contains no added atoms. */
    return a_atom_indices[1].empty() ? const_iterator(nullptr, false) : const_iterator(this, true);
}

StatePairTupleIndexGenerator::const_iterator StatePairTupleIndexGenerator::end() const { return const_iterator(nullptr, false); }

/**
 * DynamicNoveltyTable
 */

DynamicNoveltyTable::DynamicNoveltyTable(size_t arity) :
    m_tuple_index_mapper(arity),
    m_table(std::vector<bool>(m_tuple_index_mapper.get_max_tuple_index() + 1, false)),
    m_state_tuple_index_generator(&m_tuple_index_mapper),
    m_state_pair_tuple_index_generator(&m_tuple_index_mapper)
{
}

DynamicNoveltyTable::DynamicNoveltyTable(size_t arity, size_t num_atoms) :
    m_tuple_index_mapper(TupleIndexMapper(arity, num_atoms)),
    m_table(std::vector<bool>(m_tuple_index_mapper.get_max_tuple_index() + 1, false)),
    m_state_tuple_index_generator(&m_tuple_index_mapper),
    m_state_pair_tuple_index_generator(&m_tuple_index_mapper)
{
}

void DynamicNoveltyTable::resize_to_fit(AtomIndex atom_index)
{
    if (atom_index < m_tuple_index_mapper.get_num_atoms())
    {
        return;  // atom fits.
    }

    // Fetch data.
    const auto arity = m_tuple_index_mapper.get_arity();

    // Compute size of new table
    auto new_size = std::max(size_t(1), m_tuple_index_mapper.get_num_atoms());
    while (new_size < atom_index + 1 + 1)  // additional +1 for placeholder
    {
        new_size *= 2;  ///< Use doubling strategy
    }
    const auto new_placeholder = new_size;

    const auto old_tuple_index_mapper = m_tuple_index_mapper;  ///< backup old tuple index mapper for remapping

    m_tuple_index_mapper.initialize(arity, new_size);  ///< resize to fit all tuples

    auto new_table = std::vector<bool>(m_tuple_index_mapper.get_max_tuple_index() + 1, false);

    // Convert tuple indices that are not novel from old to new table.
    auto atom_indices = AtomIndexList(arity);

    for (TupleIndex tuple_index = 0; tuple_index < m_table.size(); ++tuple_index)
    {
        if (m_table[tuple_index])
        {
            old_tuple_index_mapper.to_atom_indices(tuple_index, atom_indices);

            for (size_t i = atom_indices.size(); i < arity; ++i)
            {
                atom_indices.push_back(new_placeholder);
            }

            const auto new_tuple_index = m_tuple_index_mapper.to_tuple_index(atom_indices);

            new_table[new_tuple_index] = true;
        }
    }

    // Swap old and new data.
    m_table = std::move(new_table);
}

void DynamicNoveltyTable::resize_to_fit(State state)
{
    const auto& fluent_atoms = state->get_atoms<FluentTag>();

    const auto it = std::max_element(fluent_atoms.begin(), fluent_atoms.end());

    if (it == fluent_atoms.end())
    {
        return;  ///< The state is empty. No atom must fit.
    }

    resize_to_fit(*it);
}

void DynamicNoveltyTable::compute_novel_tuples(const State state, std::vector<AtomIndexList>& out_novel_tuples)
{
    out_novel_tuples.clear();

    resize_to_fit(state);

    for (auto it = m_state_tuple_index_generator.begin(state); it != m_state_tuple_index_generator.end(); ++it)
    {
        const auto tuple_index = *it;

        assert(tuple_index < m_table.size());

        if (!m_table[tuple_index])
        {
            out_novel_tuples.push_back(m_tuple_index_mapper.to_atom_indices(tuple_index));
        }
    }
}

void DynamicNoveltyTable::insert_tuples(const std::vector<AtomIndexList>& tuples)
{
    for (const auto& tuple : tuples)
    {
        const auto tuple_index = m_tuple_index_mapper.to_tuple_index(tuple);

        assert(tuple_index < m_table.size());

        m_table[tuple_index] = true;
    }
}

bool DynamicNoveltyTable::test_novelty_and_update_table(const State state)
{
    resize_to_fit(state);

    bool is_novel = false;
    for (auto it = m_state_tuple_index_generator.begin(state); it != m_state_tuple_index_generator.end(); ++it)
    {
        const auto tuple_index = *it;
        // std::cout << tuple_index << " " << m_tuple_index_mapper.tuple_index_to_string(tuple_index) << std::endl;

        assert(tuple_index < m_table.size());

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
    resize_to_fit(state);
    resize_to_fit(succ_state);

    bool is_novel = false;
    for (auto it = m_state_pair_tuple_index_generator.begin(state, succ_state); it != m_state_pair_tuple_index_generator.end(); ++it)
    {
        const auto tuple_index = *it;
        // std::cout << tuple_index << " " << m_tuple_index_mapper.tuple_index_to_string(tuple_index) << std::endl;

        assert(tuple_index < m_table.size());

        if (!is_novel && !m_table[tuple_index])
        {
            is_novel = true;
        }
        m_table[tuple_index] = true;
    }
    return is_novel;
}

void DynamicNoveltyTable::reset() { std::fill(m_table.begin(), m_table.end(), false); }
const TupleIndexMapper& DynamicNoveltyTable::get_tuple_index_mapper() const { return m_tuple_index_mapper; }

/**
 * NoveltyPruning
 */

ArityZeroNoveltyPruningStrategy::ArityZeroNoveltyPruningStrategy(State initial_state) : m_initial_state(initial_state) {}

bool ArityZeroNoveltyPruningStrategy::test_prune_initial_state(const State state) { return false; }

bool ArityZeroNoveltyPruningStrategy::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ)
{
    return state != m_initial_state || state == succ_state;
}

ArityKNoveltyPruningStrategy::ArityKNoveltyPruningStrategy(size_t arity, size_t num_atoms) : m_novelty_table(arity) {}

bool ArityKNoveltyPruningStrategy::test_prune_initial_state(const State state)
{
    if (m_generated_states.count(state->get_index()))
    {
        assert(!m_novelty_table.test_novelty_and_update_table(state));
        return true;
    }
    m_generated_states.insert(state->get_index());

    return !m_novelty_table.test_novelty_and_update_table(state);
}

bool ArityKNoveltyPruningStrategy::test_prune_successor_state(const State state, const State succ_state, bool is_new_succ)
{
    if (state == succ_state)
    {
        return true;
    }

    if (m_generated_states.count(succ_state->get_index()))
    {
        assert(!m_novelty_table.test_novelty_and_update_table(state, succ_state));
        return true;
    }
    m_generated_states.insert(succ_state->get_index());

    return !m_novelty_table.test_novelty_and_update_table(state, succ_state);
}

/* IterativeWidthAlgorithm */

SearchResult find_solution(const SearchContext& context,
                           State start_state_,
                           size_t max_arity_,
                           EventHandler iw_event_handler_,
                           brfs::EventHandler brfs_event_handler_,
                           GoalStrategy goal_strategy_)
{
    auto& problem = *context->get_problem();
    auto& applicable_action_generator = *context->get_applicable_action_generator();
    auto& state_repository = *context->get_state_repository();

    const auto max_arity = max_arity_;
    const auto [start_state, start_g_value] =
        (start_state_) ? std::make_pair(start_state_, compute_state_metric_value(start_state_, problem)) : state_repository.get_or_create_initial_state();
    const auto iw_event_handler = (iw_event_handler_) ? iw_event_handler_ : DefaultEventHandlerImpl::create(context->get_problem());
    const auto brfs_event_handler = (brfs_event_handler_) ? brfs_event_handler_ : brfs::DefaultEventHandlerImpl::create(context->get_problem());
    const auto goal_strategy = (goal_strategy_) ? goal_strategy_ : ProblemGoalStrategyImpl::create(context->get_problem());

    if (max_arity >= MAX_ARITY)
    {
        throw std::runtime_error("iw::find_solution(...): max_arity (" + std::to_string(max_arity) + ") cannot be greater than or equal to MAX_ARITY ("
                                 + std::to_string(MAX_ARITY) + ") compile time constant.");
    }

    iw_event_handler->on_start_search(start_state);

    size_t cur_arity = 0;
    while (cur_arity <= max_arity)
    {
        iw_event_handler->on_start_arity_search(start_state, cur_arity);

        const auto result =
            (cur_arity > 0) ?
                find_solution(context,
                              start_state,
                              brfs_event_handler,
                              goal_strategy,
                              std::make_shared<ArityKNoveltyPruningStrategy>(cur_arity, INITIAL_TABLE_ATOMS)) :
                find_solution(context, start_state, brfs_event_handler, goal_strategy, std::make_shared<ArityZeroNoveltyPruningStrategy>(start_state));

        iw_event_handler->on_end_arity_search(brfs_event_handler->get_statistics());

        if (result.status == SearchStatus::SOLVED)
        {
            iw_event_handler->on_end_search();
            if (!iw_event_handler->is_quiet())
            {
                applicable_action_generator.on_end_search();
                state_repository.get_axiom_evaluator()->on_end_search();
            }
            iw_event_handler->on_solved(result.plan.value());

            return result;
        }
        else if (result.status == SearchStatus::UNSOLVABLE)
        {
            iw_event_handler->on_unsolvable();

            return result;
        }

        ++cur_arity;
    }

    auto result = SearchResult();
    result.status = SearchStatus::FAILED;
    return result;
}
}