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

#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_properties.hpp"
#include "mimir/languages/description_logics/constructor_visitors_formatter.hpp"
#include "mimir/languages/description_logics/refinement.hpp"
#include "mimir/search/state.hpp"

#include <ranges>

namespace mimir::dl::refinement_brfs
{

/**
 * SearchSpace
 */

struct SearchSpace
{
    using ConstructorTagToConstructorsByComplexity = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::vector<ConstructorList<Concept>>>,
                                                                      boost::hana::pair<boost::hana::type<Role>, std::vector<ConstructorList<Role>>>>;

    ConstructorTagToConstructorsByComplexity constructors_by_complexity = ConstructorTagToConstructorsByComplexity();
};

/**
 * Post construction refinement
 */

template<ConstructorTag D>
static bool try_insert_into_search_space(Problem problem,
                                         const grammar::Grammar& grammar,
                                         const Options& options,
                                         Constructor<D> constructor,
                                         size_t complexity,
                                         RefinementPruningFunction& ref_pruning_function,
                                         SearchSpace& ref_search_space,
                                         Statistics& ref_statistics)
{
    /* Test rejection by grammar. */

    if (!grammar.test_match(constructor))
    {
        ++boost::hana::at_key(ref_statistics.num_rejected_by_grammar, boost::hana::type<D> {});

        if (options.verbosity >= 2)
            std::cout << "rejected constructor: " << std::make_tuple(Constructor<D>(constructor), BNFFormatterVisitorTag {}) << std::endl;

        return false;
    }

    /* Test pruning. */

    if (!ref_pruning_function.should_prune(constructor))
    {
        ++boost::hana::at_key(ref_statistics.num_generated, boost::hana::type<D> {});

        boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<D> {}).at(complexity).push_back(constructor);

        if (options.verbosity >= 2)
            std::cout << "generated constructor: " << std::make_tuple(Constructor<D>(constructor), BNFFormatterVisitorTag {}) << std::endl;
    }
    else
    {
        ++boost::hana::at_key(ref_statistics.num_pruned, boost::hana::type<D> {});

        if (options.verbosity >= 2)
            std::cout << "pruned constructor: " << std::make_tuple(Constructor<D>(constructor), BNFFormatterVisitorTag {}) << std::endl;

        return false;
    }

    /* Check whether resource limits were reached; TODO: time and memory limit. */

    if (boost::hana::at_key(ref_statistics.num_generated, boost::hana::type<D> {}) >= boost::hana::at_key(options.max_constructors, boost::hana::type<D> {}))
    {
        if (options.verbosity >= 2)
            std::cout << "Reached resource limits during refinement!" << std::endl;

        return true;
    }

    return false;
}

/**
 * Refinement of constructors that accept concepts and/or roles.
 */

/// @brief `SumPartitionConstIterator` generates partitions of a number into `N` elements.
/// Each element is at least `1`, and the sum of the elements is equal to `sum`.
/// The iterator traverses all unique partitions in lexicographic order.
///
/// Example usage:
/// @code
/// SumPartitionConstIterator<3> it(5, true);
/// for (; it != SumPartitionConstIterator<3>(5, false); ++it) {
///     for (auto value : *it) {
///         std::cout << value << " ";
///     }
///     std::cout << std::endl;
/// }
/// // Output:
/// // 3 1 1
/// // 2 2 1
/// // 2 1 2
/// // 1 3 1
/// // 1 2 2
/// // 1 1 3
/// @endcode
/// @tparam N is the size of each partition.
template<size_t N>
class SumPartitionConstIterator
{
private:
    /* Inputs */
    size_t m_sum;

    /* Internal state */
    std::array<size_t, N> m_partition;
    size_t m_pos;

    bool increment_distribution()
    {
        for (size_t i = N - 1; i > 0; --i)
        {
            if (m_partition[i] > 1)
            {
                m_partition[i]--;
                m_partition[i - 1]++;
                return true;
            }
        }

        return false;
    }

    void advance()
    {
        ++m_pos;

        if (increment_distribution())
            return;

        // If we reach here, we've iterated through all combinations
        m_pos = std::numeric_limits<size_t>::max();
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::array<size_t, N>;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    SumPartitionConstIterator() : m_sum(0), m_partition(), m_pos(0) {}
    SumPartitionConstIterator(size_t sum, bool begin) : m_sum(sum), m_partition(), m_pos(begin ? 0 : std::numeric_limits<size_t>::max())
    {
        if (sum < N)
            throw std::invalid_argument("Complexity must be greater than or equal to the number of elements (N).");

        if (begin)
        {
            // min value is 1.
            m_partition.fill(1);
            m_partition[0] = m_sum - (N - 1);
        }
    }
    value_type operator*() const { return m_partition; }
    SumPartitionConstIterator& operator++()
    {
        advance();
        return *this;
    }
    SumPartitionConstIterator operator++(int)
    {
        SumPartitionConstIterator it = *this;
        advance();
        return it;
    }
    bool operator==(const SumPartitionConstIterator& other) const { return m_pos == other.m_pos; }
    bool operator!=(const SumPartitionConstIterator& other) const { return !(*this == other); }
};

template<ConstructorTag... Ds>
class ConstructorArgumentConstIterator
{
private:
    /* Inputs */
    std::tuple<std::vector<ConstructorList<Ds>>*...> m_constructors_by_complexity;

    /* Internal state */
    SumPartitionConstIterator<sizeof...(Ds)> m_complexity_distribution_iter;
    SumPartitionConstIterator<sizeof...(Ds)> m_complexity_distribution_end;
    std::array<size_t, sizeof...(Ds)> m_sizes;
    std::array<size_t, sizeof...(Ds)> m_indices;
    std::tuple<Constructor<Ds>...> m_values;
    size_t m_pos;

    bool try_initialize_sizes()
    {
        bool success = true;
        constexpr std::size_t num_elements = sizeof...(Ds);

        [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
        {
            ((
                 [&]
                 {
                     m_sizes[Indices] = std::get<Indices>(m_constructors_by_complexity)->at((*m_complexity_distribution_iter)[Indices]).size();
                     if (m_sizes[Indices] == 0)
                     {
                         success = false;
                     }
                 }()),
             ...);
        }
        (std::make_index_sequence<num_elements> {});

        return success;
    }

    void initialize()
    {
        while (m_complexity_distribution_iter != m_complexity_distribution_end)
        {
            if (try_initialize_sizes())
            {
                return;
            }

            ++m_complexity_distribution_iter;

            m_indices.fill(0);
        }

        if (m_complexity_distribution_iter == m_complexity_distribution_end)
        {
            m_pos = std::numeric_limits<size_t>::max();
        }
    }

    bool increment_indices()
    {
        for (size_t i = 0; i < sizeof...(Ds); ++i)
        {
            if (++m_indices[i] < m_sizes[i])
                return true;

            m_indices[i] = 0;
        }

        return false;
    }

    bool increment_distribution()
    {
        while (m_complexity_distribution_iter != m_complexity_distribution_end)
        {
            ++m_complexity_distribution_iter;

            if (try_initialize_sizes())
            {
                return true;
            }

            m_indices.fill(0);
        }

        return (m_complexity_distribution_iter != m_complexity_distribution_end);
    }

    void advance()
    {
        ++m_pos;

        if (increment_indices())
            return;

        if (increment_distribution())
            return;

        // If we reach here, we've iterated through all combinations
        m_pos = std::numeric_limits<size_t>::max();
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<Constructor<Ds>...>;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    ConstructorArgumentConstIterator() :
        m_constructors_by_complexity(),
        m_complexity_distribution_iter(SumPartitionConstIterator<sizeof...(Ds)>(std::numeric_limits<size_t>::max(), true)),
        m_complexity_distribution_end(SumPartitionConstIterator<sizeof...(Ds)>(std::numeric_limits<size_t>::max(), false)),
        m_sizes(),
        m_indices(),
        m_values(),
        m_pos(0)
    {
    }
    ConstructorArgumentConstIterator(std::tuple<std::reference_wrapper<std::vector<ConstructorList<Ds>>>...>& constructors_by_complexity,
                                     size_t complexity,
                                     bool begin) :
        m_constructors_by_complexity(std::apply([](auto&... refs) { return std::make_tuple(&refs.get()...); }, constructors_by_complexity)),
        m_complexity_distribution_iter(SumPartitionConstIterator<sizeof...(Ds)>(complexity, true)),
        m_complexity_distribution_end(SumPartitionConstIterator<sizeof...(Ds)>(complexity, false)),
        m_sizes(),
        m_indices(),
        m_values(),
        m_pos(begin ? 0 : std::numeric_limits<size_t>::max())
    {
        if (begin)
        {
            initialize();
        }
    }
    value_type operator*()
    {
        constexpr std::size_t num_elements = sizeof...(Ds);

        // Use fold expression with explicit index expansion
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
        {
            ((std::get<Indices>(m_values) =
                  std::get<Indices>(m_constructors_by_complexity)->at((*m_complexity_distribution_iter)[Indices]).at(m_indices[Indices])),
             ...);
        }
        (std::make_index_sequence<num_elements> {});

        return m_values;
    }
    ConstructorArgumentConstIterator& operator++()
    {
        advance();
        return *this;
    }
    ConstructorArgumentConstIterator operator++(int)
    {
        ConstructorArgumentConstIterator it = *this;
        advance();
        return it;
    }
    bool operator==(const ConstructorArgumentConstIterator& other) const { return m_pos == other.m_pos; }
    bool operator!=(const ConstructorArgumentConstIterator& other) const { return !(*this == other); }
};

template<typename... Ds>
static auto make_constructors_by_complexity(SearchSpace& search_space)
{
    return std::tuple(std::ref(boost::hana::at_key(search_space.constructors_by_complexity, boost::hana::type<Ds> {}))...);
}

template<typename ConstructorImplType>
static bool refine_composite_constructor(Problem problem,
                                         const grammar::Grammar& grammar,
                                         const Options& options,
                                         ConstructorTagToRepository& ref_constructor_repositories,
                                         RefinementPruningFunction& ref_pruning_function,
                                         SearchSpace& ref_search_space,
                                         Statistics& ref_statistics,
                                         size_t complexity)
{
    /* Extract properties using ConstructorProperties */
    using ArgumentTypes = typename ConstructorProperties<ConstructorImplType>::ArgumentTypes;

    /* Create the tuple of constructors by complexity based on argument types */
    auto constructors_by_complexity =
        std::apply([&](auto&&... args) { return make_constructors_by_complexity<std::decay_t<decltype(args)>...>(ref_search_space); }, ArgumentTypes {});

    for (const auto args : std::ranges::subrange(ConstructorArgumentConstIterator(constructors_by_complexity, complexity, true),
                                                 ConstructorArgumentConstIterator(constructors_by_complexity, complexity, false)))
    {
        /* Create constructor. */

        const auto constructor = std::apply(
            [&](auto&&... unpacked_args)
            {
                return boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConstructorImplType> {})
                    .get_or_create(std::forward<decltype(unpacked_args)>(unpacked_args)...);
            },
            args);

        /* Check grammar, prune, insert. */

        try_insert_into_search_space(problem, grammar, options, constructor, complexity + 1, ref_pruning_function, ref_search_space, ref_statistics);
    }

    return false;
}

/**
 * Refinement of concept bot.
 */

static bool refine_primitive_concept_bot(Problem problem,
                                         const grammar::Grammar& grammar,
                                         const Options& options,
                                         ConstructorTagToRepository& ref_constructor_repositories,
                                         RefinementPruningFunction& ref_pruning_function,
                                         SearchSpace& ref_search_space,
                                         Statistics& ref_statistics)
{
    const auto concept_bot = boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptBotImpl> {}).get_or_create();

    return try_insert_into_search_space(problem, grammar, options, concept_bot, 1, ref_pruning_function, ref_search_space, ref_statistics);
}

/**
 * Refinement of concept top.
 */

static bool refine_primitive_concept_top(Problem problem,
                                         const grammar::Grammar& grammar,
                                         const Options& options,
                                         ConstructorTagToRepository& ref_constructor_repositories,
                                         RefinementPruningFunction& ref_pruning_function,
                                         SearchSpace& ref_search_space,
                                         Statistics& ref_statistics)
{
    const auto concept_top = boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptTopImpl> {}).get_or_create();

    return try_insert_into_search_space(problem, grammar, options, concept_top, 1, ref_pruning_function, ref_search_space, ref_statistics);
}

/**
 * Refinement of role universal.
 */

static bool refine_primitive_role_universal(Problem problem,
                                            const grammar::Grammar& grammar,
                                            const Options& options,
                                            ConstructorTagToRepository& ref_constructor_repositories,
                                            RefinementPruningFunction& ref_pruning_function,
                                            SearchSpace& ref_search_space,
                                            Statistics& ref_statistics)
{
    const auto role_universal = boost::hana::at_key(ref_constructor_repositories, boost::hana::type<RoleUniversalImpl> {}).get_or_create();

    return try_insert_into_search_space(problem, grammar, options, role_universal, 1, ref_pruning_function, ref_search_space, ref_statistics);
}

/**
 * Refinement of concept nominal.
 */

static bool refine_primitive_concept_nominal(Problem problem,
                                             const grammar::Grammar& grammar,
                                             const Options& options,
                                             ConstructorTagToRepository& ref_constructor_repositories,
                                             RefinementPruningFunction& ref_pruning_function,
                                             SearchSpace& ref_search_space,
                                             Statistics& ref_statistics)
{
    for (const auto& object : problem->get_domain()->get_constants())
    {
        const auto concept_nominal = boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptNominalImpl> {}).get_or_create(object);

        return try_insert_into_search_space(problem, grammar, options, concept_nominal, 1, ref_pruning_function, ref_search_space, ref_statistics);
    }

    return false;
}

/**
 * Refinement of atomic constructors that accept predicates.
 */

template<PredicateTag P>
static bool refine_primitive_constructor_atomic(Problem problem,
                                                const grammar::Grammar& grammar,
                                                const Options& options,
                                                ConstructorTagToRepository& ref_constructor_repositories,
                                                RefinementPruningFunction& ref_pruning_function,
                                                SearchSpace& ref_search_space,
                                                Statistics& ref_statistics)
{
    for (const auto& predicate : problem->get_domain()->get_predicates<P>())
    {
        if (predicate->get_arity() == 1)
        {
            const auto concept_state =
                boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptAtomicStateImpl<P>> {}).get_or_create(predicate);

            if (try_insert_into_search_space(problem, grammar, options, concept_state, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;

            const auto concept_goal_true =
                boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, true);

            if (try_insert_into_search_space(problem, grammar, options, concept_goal_true, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;

            const auto concept_goal_false =
                boost::hana::at_key(ref_constructor_repositories, boost::hana::type<ConceptAtomicGoalImpl<P>> {}).get_or_create(predicate, false);

            if (try_insert_into_search_space(problem, grammar, options, concept_goal_false, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;
        }
        else if (predicate->get_arity() == 2)
        {
            const auto role_state = boost::hana::at_key(ref_constructor_repositories, boost::hana::type<RoleAtomicStateImpl<P>> {}).get_or_create(predicate);

            if (try_insert_into_search_space(problem, grammar, options, role_state, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;

            const auto role_goal_true =
                boost::hana::at_key(ref_constructor_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, true);

            if (try_insert_into_search_space(problem, grammar, options, role_goal_true, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;

            const auto role_goal_false =
                boost::hana::at_key(ref_constructor_repositories, boost::hana::type<RoleAtomicGoalImpl<P>> {}).get_or_create(predicate, false);

            if (try_insert_into_search_space(problem, grammar, options, role_goal_false, 1, ref_pruning_function, ref_search_space, ref_statistics))
                return true;
        }
    }

    return false;
}

static bool refine_primitives(Problem problem,
                              const grammar::Grammar& grammar,
                              const Options& options,
                              ConstructorTagToRepository& ref_constructor_repositories,
                              RefinementPruningFunction& ref_pruning_function,
                              SearchSpace& ref_search_space,
                              Statistics& ref_statistics)
{
    if (options.verbosity >= 1)
        std::cout << "Refine primitive constructors." << std::endl;

    return refine_primitive_concept_bot(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics)
           || refine_primitive_concept_top(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics)
           || refine_primitive_role_universal(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics)
           || refine_primitive_concept_nominal(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics)
           || refine_primitive_constructor_atomic<Static>(problem,
                                                          grammar,
                                                          options,
                                                          ref_constructor_repositories,
                                                          ref_pruning_function,
                                                          ref_search_space,
                                                          ref_statistics)
           || refine_primitive_constructor_atomic<Fluent>(problem,
                                                          grammar,
                                                          options,
                                                          ref_constructor_repositories,
                                                          ref_pruning_function,
                                                          ref_search_space,
                                                          ref_statistics)
           || refine_primitive_constructor_atomic<Derived>(problem,
                                                           grammar,
                                                           options,
                                                           ref_constructor_repositories,
                                                           ref_pruning_function,
                                                           ref_search_space,
                                                           ref_statistics);
}

static bool refine_composites(Problem problem,
                              const grammar::Grammar& grammar,
                              const Options& options,
                              ConstructorTagToRepository& ref_constructor_repositories,
                              RefinementPruningFunction& ref_pruning_function,
                              SearchSpace& ref_search_space,
                              Statistics& ref_statistics)
{
    for (size_t complexity = 1; complexity < options.max_complexity; ++complexity)
    {
        if (options.verbosity >= 1)
            std::cout << "Refine composite constructors with complexity " << complexity << "." << std::endl;

        if (complexity >= 1)  // unary composites need at least complexity 1.
        {
            if (refine_composite_constructor<ConceptNegationImpl>(problem,
                                                                  grammar,
                                                                  options,
                                                                  ref_constructor_repositories,
                                                                  ref_pruning_function,
                                                                  ref_search_space,
                                                                  ref_statistics,
                                                                  complexity))
                return true;

            if (refine_composite_constructor<RoleComplementImpl>(problem,
                                                                 grammar,
                                                                 options,
                                                                 ref_constructor_repositories,
                                                                 ref_pruning_function,
                                                                 ref_search_space,
                                                                 ref_statistics,
                                                                 complexity))
                return true;

            if (refine_composite_constructor<RoleInverseImpl>(problem,
                                                              grammar,
                                                              options,
                                                              ref_constructor_repositories,
                                                              ref_pruning_function,
                                                              ref_search_space,
                                                              ref_statistics,
                                                              complexity))
                return true;

            if (refine_composite_constructor<RoleTransitiveClosureImpl>(problem,
                                                                        grammar,
                                                                        options,
                                                                        ref_constructor_repositories,
                                                                        ref_pruning_function,
                                                                        ref_search_space,
                                                                        ref_statistics,
                                                                        complexity))
                return true;

            if (refine_composite_constructor<RoleReflexiveTransitiveClosureImpl>(problem,
                                                                                 grammar,
                                                                                 options,
                                                                                 ref_constructor_repositories,
                                                                                 ref_pruning_function,
                                                                                 ref_search_space,
                                                                                 ref_statistics,
                                                                                 complexity))
                return true;

            if (refine_composite_constructor<RoleIdentityImpl>(problem,
                                                               grammar,
                                                               options,
                                                               ref_constructor_repositories,
                                                               ref_pruning_function,
                                                               ref_search_space,
                                                               ref_statistics,
                                                               complexity))
                return true;
        }
        if (complexity >= 2)  // binary composites need at least complexity 2.
        {
            if (refine_composite_constructor<ConceptIntersectionImpl>(problem,
                                                                      grammar,
                                                                      options,
                                                                      ref_constructor_repositories,
                                                                      ref_pruning_function,
                                                                      ref_search_space,
                                                                      ref_statistics,
                                                                      complexity))
                return true;

            if (refine_composite_constructor<ConceptUnionImpl>(problem,
                                                               grammar,
                                                               options,
                                                               ref_constructor_repositories,
                                                               ref_pruning_function,
                                                               ref_search_space,
                                                               ref_statistics,
                                                               complexity))
                return true;

            if (refine_composite_constructor<ConceptValueRestrictionImpl>(problem,
                                                                          grammar,
                                                                          options,
                                                                          ref_constructor_repositories,
                                                                          ref_pruning_function,
                                                                          ref_search_space,
                                                                          ref_statistics,
                                                                          complexity))
                return true;

            if (refine_composite_constructor<ConceptExistentialQuantificationImpl>(problem,
                                                                                   grammar,
                                                                                   options,
                                                                                   ref_constructor_repositories,
                                                                                   ref_pruning_function,
                                                                                   ref_search_space,
                                                                                   ref_statistics,
                                                                                   complexity))
                return true;

            if (refine_composite_constructor<ConceptRoleValueMapContainmentImpl>(problem,
                                                                                 grammar,
                                                                                 options,
                                                                                 ref_constructor_repositories,
                                                                                 ref_pruning_function,
                                                                                 ref_search_space,
                                                                                 ref_statistics,
                                                                                 complexity))
                return true;

            if (refine_composite_constructor<ConceptRoleValueMapEqualityImpl>(problem,
                                                                              grammar,
                                                                              options,
                                                                              ref_constructor_repositories,
                                                                              ref_pruning_function,
                                                                              ref_search_space,
                                                                              ref_statistics,
                                                                              complexity))
                return true;

            if (refine_composite_constructor<RoleIntersectionImpl>(problem,
                                                                   grammar,
                                                                   options,
                                                                   ref_constructor_repositories,
                                                                   ref_pruning_function,
                                                                   ref_search_space,
                                                                   ref_statistics,
                                                                   complexity))
                return true;

            if (refine_composite_constructor<
                    RoleUnionImpl>(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics, complexity))
                return true;

            if (refine_composite_constructor<RoleCompositionImpl>(problem,
                                                                  grammar,
                                                                  options,
                                                                  ref_constructor_repositories,
                                                                  ref_pruning_function,
                                                                  ref_search_space,
                                                                  ref_statistics,
                                                                  complexity))
                return true;

            if (refine_composite_constructor<RoleRestrictionImpl>(problem,
                                                                  grammar,
                                                                  options,
                                                                  ref_constructor_repositories,
                                                                  ref_pruning_function,
                                                                  ref_search_space,
                                                                  ref_statistics,
                                                                  complexity))
                return true;
        }
        if (complexity >= 3)  // ternary composites need at least complexity 3.
        {
            // TODO: add refinements later
        }
    }

    return false;
}

static void fetch_results(const SearchSpace& search_space, Result& result)
{
    boost::hana::for_each(search_space.constructors_by_complexity,
                          [&](auto pair)
                          {
                              // Access the key and value using hana::first and hana::second
                              const auto& key = boost::hana::first(pair);                          // Access the key (e.g., Concept or Role)
                              const auto& constructors_by_complexity = boost::hana::second(pair);  // Access the associated constructors

                              // Append the constructors to the corresponding result entry
                              auto& result_constructors = boost::hana::at_key(result.constructors, key);
                              for (const auto& constructors : constructors_by_complexity)
                              {
                                  result_constructors.insert(result_constructors.end(), constructors.begin(), constructors.end());
                              }
                          });
}

void refine_helper(Problem problem,
                   const grammar::Grammar& grammar,
                   const Options& options,
                   ConstructorTagToRepository& ref_constructor_repositories,
                   RefinementPruningFunction& ref_pruning_function,
                   SearchSpace& ref_search_space,
                   Statistics& ref_statistics)
{
    if (options.max_complexity > 0)
    {
        if (refine_primitives(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics))
            return;

        if (refine_composites(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, ref_search_space, ref_statistics))
            return;
    }
}

Result refine(Problem problem,
              const grammar::Grammar& grammar,
              const Options& options,
              ConstructorTagToRepository& ref_constructor_repositories,
              RefinementPruningFunction& ref_pruning_function)
{
    auto result = Result();
    auto search_space = SearchSpace();

    boost::hana::at_key(search_space.constructors_by_complexity, boost::hana::type<Concept> {}).resize(options.max_complexity + 1);
    boost::hana::at_key(search_space.constructors_by_complexity, boost::hana::type<Role> {}).resize(options.max_complexity + 1);

    refine_helper(problem, grammar, options, ref_constructor_repositories, ref_pruning_function, search_space, result.statistics);

    fetch_results(search_space, result);

    return result;
}

}
