#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_HPP_

#include "template.hpp"

#include "../algorithms.hpp"

#include <tuple>
#include <utility>



namespace mimir {

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
template<IsAlgorithmTag... As>
struct SingleTag : public PlannerBaseTag {};

/**
 * Specialized implementation class.
*/
template<IsAlgorithmTag A>
class Planner<SingleTag<A>>
    : public PlannerBase<Planner<SingleTag<A>>> {
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class PlannerBase;

    Algorithm<AlgorithmDispatcher<A>> m_algorithm;

public:
    Planner(const std::string& domain_file, const std::string& problem_file)
        : PlannerBase<Planner<SingleTag<A>>>(domain_file, problem_file)
        , m_algorithm(Algorithm<AlgorithmDispatcher<A>>(this->m_problem)) { }

    std::tuple<SearchStatus, Plan> find_solution_impl() {
        auto plan = Plan();
        auto action_view_list = typename TypeTraits<std::remove_reference_t<decltype(m_algorithm)>>::ActionViewList();
        const auto status = m_algorithm.find_solution(action_view_list);
        for (const auto& action_view : action_view_list) {
            plan.push_back(action_view.str());
        }
        return std::make_tuple(status, plan);
    }
};


/**
 * Type traits.
*/
template<IsAlgorithmTag A>
struct TypeTraits<Planner<SingleTag<A>>> {
    using Algorithm = A;
};



}  // namespace mimir

#endif  // MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_
