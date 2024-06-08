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

#ifndef MIMIR_SEARCH_ALGORITHMS_IW_HPP_
#define MIMIR_SEARCH_ALGORITHMS_IW_HPP_

#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/algorithms/interface.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/successor_state_generators.hpp"

namespace mimir
{

/**
 * Large k for IW(k) is generally infeasible.
 * Hence, we use a small constant values within feasible range
 * allowing us to use stack allocated arrays.
 */

const int MAX_ARITY = 10;

/**
 * Type aliases for readability
 */

using AtomIndex = int;
using AtomIndices = std::vector<AtomIndex>;

using TupleIndex = int;
using TupleIndices = std::vector<TupleIndex>;

/// @brief TupleIndexMapper defines a perfect hash function mapping tuples of atom indices to a tuple index.
class TupleIndexMapper
{
private:
    int m_arity;
    int m_num_atoms;

    std::array<int, MAX_ARITY> m_factors;

public:
    TupleIndexMapper(int arity, int num_atoms);

    TupleIndex to_tuple_index(const AtomIndices& atom_indices) const;

    void to_atom_indices(TupleIndex tuple_index, AtomIndices& out_atom_indices) const;

    std::string tuple_index_to_string(TupleIndex tuple_index) const;

    /**
     * Getters
     */
    int get_num_atoms() const;
    int get_arity() const;
    const std::array<int, MAX_ARITY>& get_factors() const;
    int get_max_tuple_index() const;
};

/// @brief FluentAndDerivedMapper encapsulates logic to combine
/// reached fluent and derived atom indices into a common indexing scheme 0,1,...
///
/// This is needed to consider both fluent and derived atoms in the novelty test.
/// Fluent atoms have their own indexing scheme 0,1,...
/// Derived atoms have their own indexing scheme 0,1,...
class FluentAndDerivedMapper
{
private:
    std::vector<int> m_fluent_remap;
    std::vector<int> m_derived_remap;
    int m_num_atoms;

    static const int UNDEFINED;

public:
    FluentAndDerivedMapper();

    void remap_and_combine_and_sort(const State state, AtomIndices& out_atoms);
    void remap_and_combine_and_sort(const State state, const State succ_state, AtomIndices& out_atoms, AtomIndices& out_add_atoms);
};

/// @brief StateTupleIndexGenerator encapsulates iterator logic to generate
/// all combinations of tuple indices of size at most arity.
///
/// The underlying algorithm is an adaption of std::next_permutation
/// with constant amortized cost to compute the next tuple index.
class StateTupleIndexGenerator
{
private:
    std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper;
    std::shared_ptr<TupleIndexMapper> tuple_index_mapper;

    // Preallocated memory for reuse
    AtomIndices atom_indices;

    friend class const_iterator;

public:
    StateTupleIndexGenerator(std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper, std::shared_ptr<TupleIndexMapper> tuple_index_mapper);

    class const_iterator
    {
    private:
        /* Data */
        TupleIndexMapper* m_tuple_index_mapper;
        const AtomIndices* m_atom_indices;

        /* Iterator state */
        std::array<int, MAX_ARITY> m_indices;
        bool m_end;
        int m_cur;

        void advance();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(StateTupleIndexGenerator* data, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin(const State state);
    const_iterator begin(const AtomIndices& atom_indices);
    const_iterator end() const;
};

/// @brief StatePairTupleIndexGenerator encapsulates iterator logic to generate
/// all combinations of tuple indices of size at most the arity
/// such that at least one atom index is coming from add_atom_indices
/// and the others from atom_indices.
///
/// The underlying algorithm is an adaption of std::next_permutation
/// with constant amortized cost to compute the next tuple index.
class StatePairTupleIndexGenerator
{
public:
    /// @brief IteratorData encapsulates containers for memory reuse.
    struct IteratorData
    {
        std::shared_ptr<TupleIndexMapper> tuple_index_mapper;
        int indices[MAX_ARITY];
        // a[i] = 0 => pick from atom_indices, a[i] = 1 => pick from add_atom_indices
        int a[MAX_ARITY];
        std::array<std::vector<int>, 2> a_index_jumper;
        std::array<AtomIndices, 2> a_atom_indices;
        std::array<int, 2> a_num_atom_indices;

        explicit IteratorData(std::shared_ptr<TupleIndexMapper> tuple_index_mapper_);
    };

private:
    IteratorData& m_data;

public:
    StatePairTupleIndexGenerator(IteratorData& data);

    class const_iterator
    {
    private:
        /* Data */
        IteratorData* m_data;

        /* Iterator positions implict representation */
        bool m_end_inner;
        int m_cur_inner;
        bool m_end_outter;
        int m_cur_outter;

        void initialize_index_jumper();

        bool advance_outter();

        void advance_inner();

    public:
        using difference_type = int;
        using value_type = TupleIndex;
        using pointer = value_type*;
        using reference = value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator();
        const_iterator(IteratorData* data, bool begin);
        [[nodiscard]] value_type operator*() const;
        const_iterator& operator++();
        const_iterator operator++(int);
        [[nodiscard]] bool operator==(const const_iterator& other) const;
        [[nodiscard]] bool operator!=(const const_iterator& other) const;
    };

    const_iterator begin() const;
    const_iterator end() const;
};

/// @brief DynamicNoveltyTable encapsulates a table to test novelty of tuples of atoms of size at most arity.
/// It automatically resizes when the atoms do not fit into the table anymore.
class DynamicNoveltyTable
{
private:
    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;

    std::shared_ptr<TupleIndexMapper> m_tuple_index_mapper;

    std::vector<bool> m_table;

    void resize_to_fit(int atom_index);

    // Preallocated memory that will be modified and reused.
    // The constructors give a deeper understanding into
    // what data must be fed into the iterators.
    // There are also unit tests that illustrate correct usage.
    StateTupleIndexGenerator m_state_tuple_index_generator;
    StatePairTupleIndexGenerator::IteratorData m_pair_state_iterator_data;

public:
    DynamicNoveltyTable(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper);

    bool test_novelty_and_update_table(const State state);

    bool test_novelty_and_update_table(const State state, const State succ_state);
};

class ArityZeroNoveltyPruning : public IPruningStrategy
{
private:
    State m_initial_state;

public:
    explicit ArityZeroNoveltyPruning(State initial_state);

    bool test_prune_initial_state(const State state) override;
    bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) override;
};

class ArityKNoveltyPruning : public IPruningStrategy
{
private:
    DynamicNoveltyTable m_novelty_table;

    std::unordered_set<int> m_generated_states;

public:
    explicit ArityKNoveltyPruning(int arity, int num_atoms, std::shared_ptr<FluentAndDerivedMapper> atom_index_mapper);

    bool test_prune_initial_state(const State state) override;
    bool test_prune_successor_state(const State state, const State succ_state, bool is_new_succ) override;
};

class IterativeWidthAlgorithm : public IAlgorithm
{
private:
    std::shared_ptr<IDynamicAAG> m_aag;
    std::shared_ptr<IDynamicSSG> m_ssg;
    std::shared_ptr<IAlgorithmEventHandler> m_event_handler;
    int m_max_arity;

    std::shared_ptr<FluentAndDerivedMapper> m_atom_index_mapper;

    State m_initial_state;
    int m_cur_arity;
    BrFsAlgorithm m_brfs;

public:
    /// @brief Simplest construction
    IterativeWidthAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator, size_t max_arity) :
        IterativeWidthAlgorithm(applicable_action_generator,
                                std::make_shared<SuccessorStateGenerator>(applicable_action_generator),
                                std::make_shared<DebugAlgorithmEventHandler>(),
                                max_arity)
    {
    }

    /// @brief Complete construction
    IterativeWidthAlgorithm(std::shared_ptr<IDynamicAAG> applicable_action_generator,
                            std::shared_ptr<IDynamicSSG> successor_state_generator,
                            std::shared_ptr<IAlgorithmEventHandler> event_handler,
                            int max_arity) :
        m_aag(applicable_action_generator),
        m_ssg(successor_state_generator),
        m_event_handler(event_handler),
        m_max_arity(max_arity),
        m_atom_index_mapper(std::make_shared<FluentAndDerivedMapper>()),
        m_initial_state(m_ssg->get_or_create_initial_state()),
        m_cur_arity(0),
        m_brfs(applicable_action_generator, successor_state_generator, event_handler)
    {
        if (max_arity < 0)
        {
            throw std::runtime_error("Arity must be greater of equal than 0.");
        }
    }

    SearchStatus find_solution(GroundActionList& out_plan) override { return find_solution(m_initial_state, out_plan); }

    SearchStatus find_solution(const State start_state, GroundActionList& out_plan) override
    {
        while (m_cur_arity <= m_max_arity)
        {
            std::cout << "[IterativeWidth] Run IW(" << m_cur_arity << ")" << std::endl;

            auto search_status = (m_cur_arity > 0) ?
                                     m_brfs.find_solution(start_state, std::make_unique<ArityKNoveltyPruning>(m_cur_arity, 64, m_atom_index_mapper), out_plan) :
                                     m_brfs.find_solution(start_state, std::make_unique<ArityZeroNoveltyPruning>(start_state), out_plan);

            if (search_status == SearchStatus::SOLVED)
            {
                return SearchStatus::SOLVED;
            }

            ++m_cur_arity;
        }
        return SearchStatus::FAILED;
    }
};
}

#endif