#ifndef MIMIR_SEARCH_PLANNERS_INTERFACE_HPP_
#define MIMIR_SEARCH_PLANNERS_INTERFACE_HPP_

#include "mimir/formalism/problem/declarations.hpp"
#include "mimir/search/algorithms/interface.hpp"

#include <loki/common/filesystem.hpp>
#include <string>

namespace mimir
{

using Plan = std::vector<std::string>;

/**
 * Interface class
 */
template<typename Derived>
class IPlanner
{
private:
    IPlanner() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    const Domain& get_domain() const { return self().get_domain_impl(); }
    const Problem& get_problem() const { return self().get_problem_impl(); }

    std::tuple<SearchStatus, Plan> find_solution() { return self().find_solution_impl(); }
};

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct PlannerTag
{
};

template<class DerivedTag>
concept IsPlannerTag = std::derived_from<DerivedTag, PlannerTag>;

/**
 * General implementation class.
 *
 * Specialize it with your tag.
 * No dispatcher because this is the topmost template.
 */
template<IsPlannerTag T>
class Planner : public IPlanner<Planner<T>>
{
};

}

#endif
