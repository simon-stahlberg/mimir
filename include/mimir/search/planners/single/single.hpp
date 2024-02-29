#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_

#include "../../../formalism/parser.hpp"
#include "../../algorithms.hpp"
#include "../interface.hpp"

#include <iostream>
#include <tuple>
#include <utility>

namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
 */
template<IsAlgorithmTag A>
struct SingleTag : public PlannerTag
{
};

/**
 * Specialized implementation class.
 */
template<IsAlgorithmTag A>
class Planner<SingleTag<A>> : public IPlanner<Planner<SingleTag<A>>>
{
private:
    // Give access to the private interface implementations.
    template<typename>
    friend class IPlanner;

    fs::path m_domain_file;
    fs::path m_problem_file;

    PDDLParser m_parser;

    Algorithm<AlgorithmDispatcher<A>> m_algorithm;

    /* Implement IPlanner Interface*/
    template<typename>
    friend class IPlanner;

    std::tuple<SearchStatus, Plan> find_solution_impl()
    {
        auto plan = Plan();
        auto action_view_list = typename TypeTraits<std::remove_reference_t<decltype(m_algorithm)>>::ConstActionViewList();
        const auto status = m_algorithm.find_solution(action_view_list);
        for (const auto& action_view : action_view_list)
        {
            plan.push_back(action_view.str());
        }
        return std::make_tuple(status, plan);
    }

    const fs::path& get_domain_file_path_impl() const { return m_domain_file; }
    const fs::path& get_problem_file_path_impl() const { return m_problem_file; }

public:
    Planner(const fs::path& domain_file_path, const fs::path& problem_file_path) :
        m_parser(PDDLParser(domain_file_path, problem_file_path)),
        m_algorithm(Algorithm<AlgorithmDispatcher<A>>(m_parser.get_problem(), m_parser.get_factories()))
    {
    }
};

}

#endif