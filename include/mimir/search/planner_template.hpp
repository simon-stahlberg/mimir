#ifndef MIMIR_SEARCH_PLANNER_TEMPLATE_HPP_
#define MIMIR_SEARCH_PLANNER_TEMPLATE_HPP_

#include "config.hpp"

#include "algorithm_base.hpp"

#include <string>


namespace mimir
{

/**
 * Configuration classes to dispatch parallel or sequential planning
*/
struct ParallelBaseTag {};

struct SequentialBaseTag {};


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
 * General implementation.
*/
template<typename T>
class Planner : public PlannerBase<Planner<T>> {
};


/**
 * Configuration classes to dispatch parallel or sequential planning
*/
template<IsAlgorithmTag... As>
struct Parallel : public ParallelBaseTag {};

template<IsAlgorithmTag... As>
struct Sequential : public SequentialBaseTag {};


/**
 * Concepts.
*/
template<typename T>
struct is_parallel : std::false_type {};

template<IsAlgorithmTag... As>
struct is_parallel<Parallel<As...>> : std::true_type {};

template<typename T>
struct is_sequential : std::false_type {};

template<IsAlgorithmTag... As>
struct is_sequential<Sequential<As...>> : std::true_type {};

template<typename T>
concept IsConcurrencyMode = is_parallel<T>::value || is_sequential<T>::value;


/**
 * Spezialized implementations accepting a variable number of algorithms
*/
template<IsAlgorithmTag... As>
class Planner<Parallel<As...>> : public PlannerBase<Planner<Parallel<As...>>> {

};

template<IsAlgorithmTag... As>
class Planner<Sequential<As...>> : public PlannerBase<Planner<Sequential<As...>>> {

};



}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_HPP_
