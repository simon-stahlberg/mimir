#ifndef MIMIR_SEARCH_PLANNERS_SINGLE_HPP_
#define MIMIR_SEARCH_PLANNERS_SINGLE_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/translations.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/planners/interface.hpp"

#include <tuple>

namespace mimir
{

/**
 * Specialized implementation class.
 */
class SinglePlanner : public IPlanner
{
private:
    // Give access to the private interface implementations.
    template<typename>
    friend class IPlanner;

    Domain m_domain;
    Problem m_problem;

    std::unique_ptr<IAlgorithm> m_algorithm;

    /* Implement IPlanner Interface*/
    template<typename>
    friend class IPlanner;

    std::tuple<SearchStatus, Plan> find_solution_impl()
    {
        auto action_view_list = VActionList {};
        const auto status = m_algorithm->find_solution(action_view_list);
        return std::make_tuple(status, to_plan(action_view_list));
    }

    const Domain& get_domain_impl() const { return m_domain; }
    const Problem& get_problem_impl() const { return m_problem; }

public:
    SinglePlanner(const Domain& domain, const Problem& problem, PDDLFactories& factories, std::unique_ptr<IAlgorithm>&& algorithm) :
        m_domain(domain),
        m_problem(problem),
        m_algorithm(std::move(algorithm))
    {
    }
};

}

#endif
