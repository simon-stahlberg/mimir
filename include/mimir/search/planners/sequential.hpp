#ifndef MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_
#define MIMIR_SEARCH_PLANNERS_SEQUENTIAL_HPP_

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
struct SequentialTag : public PlannerBaseTag {};

/**
 * Specialized implementation class.
*/
template<IsAlgorithmTag... As>
class Planner<SequentialTag<As...>>
    : public PlannerBase<Planner<SequentialTag<As...>>> {
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class PlannerBase;

    /*
        All algorithms are currently initialized with the same arguments.
        This looks sufficient since the planner only passes
        very general parameters like memory limit or time limits.
        We can use template meta programming to pass different parameters to different algorithms.
    */
    std::tuple<Algorithm<AlgorithmDispatcher<As>>...> m_algorithms;

public:
    Planner(const std::string& domain_file, const std::string& problem_file)
        : PlannerBase<Planner<SequentialTag<As...>>>(domain_file, problem_file)
        , m_algorithms(std::make_tuple(Algorithm<AlgorithmDispatcher<As>>(this->m_problem)...)) { }

    std::tuple<SearchStatus, Plan> find_solution_impl() {
        auto plan = Plan();
        std::apply([&plan](auto&... algorithm) {
            (..., [&]{
                auto action_view_list = typename TypeTraits<std::remove_reference_t<decltype(algorithm)>>::ActionViewList();
                const auto status = algorithm.find_solution(action_view_list);
                if (status == SearchStatus::SOLVED) {
                    for (const auto& action_view : action_view_list) {
                        plan.push_back(action_view.str());
                    }
                    return;
                }
            }());
        }, m_algorithms);
        return std::make_tuple(SearchStatus::SOLVED, plan);
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
