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
    std::vector<ConstructorList<Concept>> concepts_by_complexity;
    std::vector<ConstructorList<Role>> roles_by_complexity;
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
            if (ref_search_space.concepts_by_complexity.size() < 1)
                ref_search_space.concepts_by_complexity.resize(1);

            ref_search_space.concepts_by_complexity[0].push_back(concept_);
        }
    }

    return false;
}

/**
 * Refinement of constructors that accept concepts and/or roles.
 */

template<size_t N>
class ComplexityDistributionConstIterator
{
private:
    size_t m_complexity;
    std::array<size_t, N> m_distribution;
    size_t m_pos;

public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::array<size_t, N>;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    ComplexityDistributionConstIterator() : m_complexity(0), m_distribution() {}
    ComplexityDistributionConstIterator(size_t complexity, bool begin) : m_complexity(complexity), m_distribution() {}
    value_type operator*() const { return m_distribution; }
    ComplexityDistributionConstIterator& operator++();
    ComplexityDistributionConstIterator operator++(int);
    bool operator==(const ComplexityDistributionConstIterator& other) const { return m_pos == other.m_pos; }
    bool operator!=(const ComplexityDistributionConstIterator& other) const { return !(*this == other); }
};

template<IsConceptOrRole... Ds>
class ConstructorArgumentConstIterator
{
private:
    std::tuple<std::vector<ConstructorList<Ds>>*...> m_constructors_by_complexity;
    std::array<size_t, sizeof...(Ds)> m_sizes;
    size_t m_complexity;

    ComplexityDistributionConstIterator<sizeof...(Ds)> m_complexity_distribution_iterator;
    std::array<size_t, sizeof...(Ds)> m_distribution;

    std::array<size_t, sizeof...(Ds) + 1> m_indices;
    size_t m_i;
    std::tuple<Constructor<Ds>...> m_values;
    size_t m_pos;

    void advance()
    {
        ++m_pos;

        for (size_t i = 0; i < sizeof...(Ds); ++i)
        {
            // Increment the current dimension
            ++m_indices[i];

            // Check if it's within bounds
            if (m_indices[i] < m_sizes[i])
            {
                return;  // No carry-over, exit
            }

            // Reset current index and carry over to the next dimension
            m_indices[i] = 0;
            if (i + 1 < sizeof...(Ds))
            {
                ++m_indices[i + 1];
                if (m_indices[i + 1] < m_sizes[i + 1])
                {
                    return;
                }
            }
        }

        // If we reach here, we've iterated through all combinations
        m_pos = std::numeric_limits<size_t>::max();  // Mark as finished
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<Constructor<Ds>...>;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    ConstructorArgumentConstIterator() :
        m_constructors_by_complexity(),
        m_complexity(0),
        m_complexity_distribution_iterator(ComplexityDistributionConstIterator<sizeof...(Ds)>(0, false)),
        m_indices(),
        m_i(0),
        m_values(),
        m_pos(0)
    {
    }
    ConstructorArgumentConstIterator(std::tuple<std::reference_wrapper<std::vector<ConstructorList<Ds>>>...>& constructors_by_complexity,
                                     size_t complexity,
                                     bool begin) :
        m_constructors_by_complexity(std::apply([](auto&... refs) { return std::make_tuple(&refs.get()...); }, constructors_by_complexity)),
        m_complexity(complexity),
        m_complexity_distribution_iterator(ComplexityDistributionConstIterator<sizeof...(Ds)>(m_complexity, begin))
    {
    }
    value_type operator*()
    {
        constexpr std::size_t num_elements = sizeof...(Ds);

        // Use fold expression with explicit index expansion
        [&]<std::size_t... Indices>(std::index_sequence<Indices...>)
        {
            ((std::get<Indices>(m_values) = std::get<Indices>(m_constructors_by_complexity)->at(m_distribution[Indices]).at(m_indices[Indices])), ...);
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

template<IsConceptOrRole D>
auto& get_constructors_by_type(SearchSpace& search_space)
{
    if constexpr (std::is_same_v<D, Concept>)
    {
        return search_space.concepts_by_complexity;
    }
    else if constexpr (std::is_same_v<D, Role>)
    {
        return search_space.roles_by_complexity;
    }
    else
    {
        static_assert(dependent_false<D>::value, "Missing implementation for IsConceptOrRole.");
    }
}

template<typename... Ds>
auto make_constructors_by_complexity(SearchSpace& search_space)
{
    return std::tuple(std::ref(get_constructors_by_type<Ds>(search_space))...);
}

template<typename ConstructorImplType>
static bool refine_constructor(Problem problem,
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

            if (ref_search_space.concepts_by_complexity.size() < complexity)
                ref_search_space.concepts_by_complexity.resize(complexity + 1);

            ref_search_space.concepts_by_complexity[complexity].push_back(constructor);
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
        if (refine_concept_atomic_state<Static>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space))
            return;
        if (refine_concept_atomic_state<Fluent>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space))
            return;
        if (refine_concept_atomic_state<Derived>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space))
            return;

        for (size_t complexity = 2; complexity <= options.max_complexity; ++complexity)
        {
            if (refine_constructor<ConceptIntersectionImpl>(problem,
                                                            grammar,
                                                            options,
                                                            ref_constructor_repos,
                                                            ref_pruning_function,
                                                            ref_search_space,
                                                            ref_statistics,
                                                            complexity))
                return;

            if (refine_constructor<
                    ConceptUnionImpl>(problem, grammar, options, ref_constructor_repos, ref_pruning_function, ref_search_space, ref_statistics, complexity))
                return;
        }
    }
}

Result refine(Problem problem,
              const grammar::Grammar& grammar,
              const Options& options,
              VariadicConstructorFactory& ref_constructor_repos,
              RefinementPruningFunction& ref_pruning_function)
{
    auto result = Result();

    /* Initialize the search space. */

    auto search_space = SearchSpace();

    /* Refine within resource limits */

    refine_helper(problem, grammar, options, ref_constructor_repos, ref_pruning_function, search_space, result.statistics);

    /* Fetch and return result. */

    for (const auto& concepts : search_space.concepts_by_complexity)
    {
        result.concepts.insert(result.concepts.end(), concepts.begin(), concepts.end());
    }

    for (const auto& roles : search_space.roles_by_complexity)
    {
        result.roles.insert(result.roles.end(), roles.begin(), roles.end());
    }

    return result;
}

}
