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
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/languages/description_logics/constructor_properties.hpp"
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
    using CategoryToConstructorsByComplexity = boost::hana::map<boost::hana::pair<boost::hana::type<Concept>, std::vector<ConstructorList<Concept>>>,
                                                                boost::hana::pair<boost::hana::type<Role>, std::vector<ConstructorList<Role>>>>;

    CategoryToConstructorsByComplexity constructors_by_complexity = CategoryToConstructorsByComplexity();
};

/**
 * Refinement of constructors that accept predicates.
 */

template<PredicateCategory P>
static bool refine_concept_atomic_state(Problem problem,
                                        const grammar::Grammar& grammar,
                                        const Options& options,
                                        VariadicConstructorFactory& ref_constructor_repos,
                                        RefinementPruningFunction& ref_pruning_function,
                                        SearchSpace& ref_search_space)
{
    for (const auto& predicate : problem->get_domain()->get_predicates<P>())
    {
        if (predicate->get_arity() != 1)
            continue;

        const auto concept_ = ref_constructor_repos.template get<ConceptAtomicStateFactory<P>>().template get_or_create<ConceptAtomicStateImpl<P>>(predicate);

        if (grammar.test_match(concept_) && !ref_pruning_function.should_prune(concept_))
        {
            if (boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<Concept> {}).size() < 2)
                boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<Concept> {}).resize(2);

            boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<Concept> {})[1].push_back(concept_);
        }
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

template<IsConceptOrRole... Ds>
class ConstructorArgumentConstIterator
{
private:
    /* Inputs */
    std::tuple<std::vector<ConstructorList<Ds>>*...> m_constructors_by_complexity;
    std::array<size_t, sizeof...(Ds)> m_sizes;

    /* Internal state */
    SumPartitionConstIterator<sizeof...(Ds)> m_complexity_distribution_iter;
    SumPartitionConstIterator<sizeof...(Ds)> m_complexity_distribution_end;
    std::array<size_t, sizeof...(Ds) + 1> m_indices;
    std::tuple<Constructor<Ds>...> m_values;
    size_t m_pos;

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
        ++m_complexity_distribution_iter;
        m_indices.fill(0);

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
        m_sizes(),
        m_complexity_distribution_iter(SumPartitionConstIterator<sizeof...(Ds)>(std::numeric_limits<size_t>::max(), true)),
        m_complexity_distribution_end(SumPartitionConstIterator<sizeof...(Ds)>(std::numeric_limits<size_t>::max(), false)),
        m_indices(),
        m_values(),
        m_pos(0)
    {
    }
    ConstructorArgumentConstIterator(std::tuple<std::reference_wrapper<std::vector<ConstructorList<Ds>>>...>& constructors_by_complexity,
                                     size_t complexity,
                                     bool begin) :
        m_constructors_by_complexity(std::apply([](auto&... refs) { return std::make_tuple(&refs.get()...); }, constructors_by_complexity)),
        m_sizes(std::apply([](auto&... refs) { return std::array<size_t, sizeof...(Ds)> { refs.get().size()... }; }, constructors_by_complexity)),
        m_complexity_distribution_iter(SumPartitionConstIterator<sizeof...(Ds)>(complexity, true)),
        m_complexity_distribution_end(SumPartitionConstIterator<sizeof...(Ds)>(complexity, false)),
        m_indices(),
        m_values(),
        m_pos(0)
    {
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
                                         VariadicConstructorFactory& ref_constructor_repos,
                                         RefinementPruningFunction& ref_pruning_function,
                                         SearchSpace& ref_search_space,
                                         Statistics& ref_statistics,
                                         size_t complexity)
{
    /* Extract properties using ConstructorProperties */
    using ConstructorType = typename ConstructorProperties<ConstructorImplType>::ConstructorType;
    using FactoryType = typename ConstructorProperties<ConstructorImplType>::FactoryType;
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
                return ref_constructor_repos.template get<FactoryType>().template get_or_create<ConstructorImplType>(
                    std::forward<decltype(unpacked_args)>(unpacked_args)...);
            },
            args);

        /* Test rejection by grammar. */

        if (!grammar.test_match(constructor))
        {
            ++boost::hana::at_key(ref_statistics.num_rejected_by_grammar, boost::hana::type<ConstructorType> {});

            continue;
        }

        /* Test pruning. */

        if (!ref_pruning_function.should_prune(constructor))
        {
            ++boost::hana::at_key(ref_statistics.num_generated, boost::hana::type<ConstructorType> {});

            if (boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<ConstructorType> {}).size() < complexity)
                boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<ConstructorType> {}).resize(complexity + 2);

            boost::hana::at_key(ref_search_space.constructors_by_complexity, boost::hana::type<ConstructorType> {})[complexity + 1].push_back(constructor);
        }
        else
        {
            ++boost::hana::at_key(ref_statistics.num_pruned, boost::hana::type<ConstructorType> {});
        }

        /* Check whether resource limits were reached; TODO: time and memory limit. */

        if (boost::hana::at_key(ref_statistics.num_generated, boost::hana::type<ConstructorType> {})
            >= boost::hana::at_key(options.max_constructors, boost::hana::type<ConstructorType> {}))
        {
            return true;
        }
    }

    return false;
}

static bool refine_primitives(Problem problem,
                              const grammar::Grammar& grammar,
                              const Options& options,
                              VariadicConstructorFactory& ref_constructor_repos,
                              RefinementPruningFunction& ref_pruning_function,
                              SearchSpace& ref_search_space,
                              Statistics& ref_statistics)
{
    return refine_concept_atomic_state<Static>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space)
           || refine_concept_atomic_state<Fluent>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space)
           || refine_concept_atomic_state<Derived>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space);
}

static bool refine_composites(Problem problem,
                              const grammar::Grammar& grammar,
                              const Options& options,
                              VariadicConstructorFactory& ref_constructor_repos,
                              RefinementPruningFunction& ref_pruning_function,
                              SearchSpace& ref_search_space,
                              Statistics& ref_statistics)
{
    for (size_t complexity = 1; complexity < options.max_complexity; ++complexity)
    {
        if (complexity >= 1)  // unary composites need at least complexity 1.
        {
            // TODO: add refinements later
        }
        if (complexity >= 2)  // binary composites need at least complexity 2.
        {
            if (refine_composite_constructor<ConceptIntersectionImpl>(problem,
                                                                      grammar,
                                                                      options,
                                                                      ref_constructor_repos,
                                                                      ref_pruning_function,
                                                                      ref_search_space,
                                                                      ref_statistics,
                                                                      complexity))
                return true;

            if (refine_composite_constructor<
                    ConceptUnionImpl>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space, ref_statistics, complexity))
                return true;

            // TODO: add more refinements later
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
                   VariadicConstructorFactory& ref_constructor_repos,
                   RefinementPruningFunction& ref_pruning_function,
                   SearchSpace& ref_search_space,
                   Statistics& ref_statistics)
{
    if (options.max_complexity > 0)
    {
        if (refine_primitives(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space, ref_statistics))
            return;

        if (refine_composites(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space, ref_statistics))
            return;
    }
}

Result refine(Problem problem,
              const grammar::Grammar& grammar,
              const Options& options,
              VariadicConstructorFactory& ref_constructor_repos,
              RefinementPruningFunction& ref_pruning_function)
{
    auto result = Result();
    auto search_space = SearchSpace();

    refine_helper(problem, grammar, options, ref_constructor_repos, ref_pruning_function, search_space, result.statistics);

    fetch_results(search_space, result);

    return result;
}

}
