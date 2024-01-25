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
 * Dispatcher class.
 * 
 * Define the required template arguments of your implementation.
*/
template<IsAlgorithmTag... As>
struct is_planner_dispatcher<PlannerDispatcher<SequentialTag<As...>>> : std::true_type {};


/**
 * Spezialized implementation class.
*/
template<IsAlgorithmTag... As>
class Planner<PlannerDispatcher<SequentialTag<As...>>>
    : public PlannerBase<Planner<PlannerDispatcher<SequentialTag<As...>>>> {
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class PlannerBase;

public:
    Planner(const std::string& domain_file, const std::string& problem_file) 
        : PlannerBase<Planner<PlannerDispatcher<SequentialTag<As...>>>>(domain_file, problem_file) { }
};


/**
 * Type traits.
*/
template<IsPlanningModeTag P, IsStateTag S, IsActionTag A>
struct TypeTraits<Heuristic<HeuristicDispatcher<BlindTag, P, S, A>>> {
    using PlanningModeTag = P;
    using StateTag = S;
    using ActionTag = A;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_
