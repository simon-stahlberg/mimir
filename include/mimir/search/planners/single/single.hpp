#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_

#include "../interface.hpp"

#include "../../algorithms.hpp"

#include <tuple>
#include <utility>
#include <iostream>


namespace mimir
{


/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
template<IsAlgorithmTag... As>
struct SingleTag : public PlannerTag {};

/**
 * Specialized implementation class.
*/
template<IsAlgorithmTag A>
class Planner<SingleTag<A>>
    : public IPlanner<Planner<SingleTag<A>>>
{
private:

    // Give access to the private interface implementations.
    template<typename>
    friend class IPlanner;

    std::string m_domain_file;
    std::string m_problem_file;

    // TODO (Dominik): initialize it properly
    Domain m_domain;
    Problem m_problem;

    Algorithm<AlgorithmDispatcher<A>> m_algorithm;

    /* Implement IPlanner Interface*/
    template<typename>
    friend class IPlanner;

    std::tuple<SearchStatus, Plan> find_solution_impl() {
        auto plan = Plan();
        auto action_view_list = typename TypeTraits<std::remove_reference_t<decltype(m_algorithm)>>::ActionViewList();
        const auto status = m_algorithm.find_solution(action_view_list);
        for (const auto& action_view : action_view_list) {
            plan.push_back(action_view.str());
        }
        return std::make_tuple(status, plan);
    }

    const std::string& get_domain_file_impl() const { return m_domain_file; }
    const std::string& get_problem_file_impl() const { return m_problem_file; }

public:
    Planner(const std::string& domain_file, const std::string& problem_file)
        : m_algorithm(Algorithm<AlgorithmDispatcher<A>>(this->m_problem)) { }
};

}

#endif