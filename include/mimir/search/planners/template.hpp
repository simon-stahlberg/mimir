#ifndef MIMIR_SEARCH_PLANNERS_TEMPLATE_HPP_
#define MIMIR_SEARCH_PLANNERS_TEMPLATE_HPP_

#include "../config.hpp"

#include "../algorithms.hpp"

#include <string>


namespace mimir
{

using Plan = std::vector<std::string>;


/**
 * Interface class
*/
template<typename Derived>
class PlannerBase {
private:
    PlannerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

protected:
    std::string m_domain_file;
    std::string m_problem_file;

    // TODO (Dominik): initialize it properly
    Domain m_domain;
    Problem m_problem;

    // TODO: store preprocessed data that can be passed to all algorithms, e.g., causal graph etc

    PlannerBase(const std::string& domain_file, const std::string& problem_file)
        : m_domain_file(domain_file), m_problem_file(problem_file)
        , m_domain(nullptr), m_problem(nullptr) { }

public:
    const std::string& get_domain_file() const { return m_domain_file; }
    const std::string& get_problem_file() const { return m_problem_file; }

    /// @brief Find a plan.
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
struct PlannerBaseTag {};

template<class DerivedTag>
concept IsPlannerTag = std::derived_from<DerivedTag, PlannerBaseTag>;


/**
 * General implementation class.
 *
 * Spezialize it with your tag.
 * No dispatcher because this is the topmost template.
*/
template<IsPlannerTag T>
class Planner : public PlannerBase<Planner<T>> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
