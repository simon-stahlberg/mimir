#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_SINGLE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/planners/interface.hpp"

#include <tuple>

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

    Domain m_domain;
    Problem m_problem;

    Algorithm<AlgorithmDispatcher<A>> m_algorithm;

    /* Implement IPlanner Interface*/
    template<typename>
    friend class IPlanner;

    std::tuple<SearchStatus, Plan> find_solution_impl()
    {
        auto action_view_list = typename TypeTraits<std::remove_reference_t<decltype(m_algorithm)>>::ConstActionViewList();
        const auto status = m_algorithm.find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }

    const Domain& get_domain_impl() const { return m_domain; }
    const Problem& get_problem_impl() const { return m_problem; }

public:
    Planner(const Domain& domain, const Problem& problem, PDDLFactories& factories) :
        m_domain(domain),
        m_problem(problem),
        m_algorithm(Algorithm<AlgorithmDispatcher<A>>(problem, factories))
    {
    }
};

}

#endif
