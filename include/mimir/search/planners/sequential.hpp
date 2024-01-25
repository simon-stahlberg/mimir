#ifndef MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_
#define MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_

#include "template.hpp"


namespace mimir {

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
template<IsAlgorithmTag... As>
struct SequentialTag : public PlannerBaseTag {};

/**
 * Spezialized implementation class.
*/
template<IsAlgorithmTag... As>
class Planner<SequentialTag<As...>>
    : public PlannerBase<Planner<SequentialTag<As...>>> {
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class PlannerBase;

public:
    Planner(const std::string& domain_file, const std::string& problem_file)
        : PlannerBase<Planner<SequentialTag<As...>>>(domain_file, problem_file) { }

    std::tuple<PlannerStatus, Plan> find_solution_impl() {
        // TODO (Dominik): run algorithms sequentially.
        return std::make_tuple(PlannerStatus::SOLVED, Plan());
    }
};


/**
 * Type traits.
*/
template<IsAlgorithmTag... As>
struct TypeTraits<Planner<SequentialTag<As...>>> {
    using Algorithms = std::tuple<As...>;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_
