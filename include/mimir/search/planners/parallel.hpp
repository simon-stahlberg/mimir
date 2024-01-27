#ifndef MIMIR_SEARCH_PLANNERS_PARALLEL_HPP_
#define MIMIR_SEARCH_PLANNERS_PARALLEL_HPP_

#include "template.hpp"


namespace mimir {

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
template<IsAlgorithmTag... As>
struct ParallelTag : public PlannerBaseTag {};

/**
 * Specialized implementation class.
*/
template<IsAlgorithmTag... As>
class Planner<ParallelTag<As...>>
    : public PlannerBase<Planner<ParallelTag<As...>>> {
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class PlannerBase;

public:
    Planner(const std::string& domain_file, const std::string& problem_file)
        : PlannerBase<Planner<ParallelTag<As...>>>(domain_file, problem_file) { }

    std::tuple<SearchStatus, Plan> find_solution_impl() {
        // TODO (Dominik): run algorithms sequentially.
        return std::make_tuple(SearchStatus::SOLVED, Plan());
    }
};


/**
 * Type traits.
*/
template<IsAlgorithmTag... As>
struct TypeTraits<Planner<ParallelTag<As...>>> {
    using Algorithms = std::tuple<As...>;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_PLANNERS_PARALLEL_HPP_
