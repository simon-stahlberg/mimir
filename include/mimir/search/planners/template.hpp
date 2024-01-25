#ifndef MIMIR_SEARCH_PLANNERS_TEMPLATE_HPP_
#define MIMIR_SEARCH_PLANNERS_TEMPLATE_HPP_

#include "../config.hpp"

#include "../algorithms.hpp"

#include <string>


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class PlannerBase {
    // TODO: parse the domain, store preprocessed data that can be passed to all algorithms
    // e.g. causal graph etc
protected:
    std::string m_domain_file;
    std::string m_problem_file;

public:
    PlannerBase(const std::string& domain_file, const std::string& problem_file)
        : m_domain_file(domain_file), m_problem_file(problem_file) { }
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
 * Dispatcher class.
 *
 * Wrap the tag and variable number of template arguments.
 * 
 * Define required input template parameters using SFINAE
 * in the declaration file of your derived class.
*/
template<IsPlannerTag H, typename... Ts>
struct PlannerDispatcher {};

template<typename T>
struct is_planner_dispatcher : std::false_type {};

template<typename T>
concept IsPlannerDispatcher = is_planner_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Spezialize it with your dispatcher.
*/
template<IsPlannerDispatcher T>
class Planner : public PlannerBase<Planner<T>> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
