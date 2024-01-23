#ifndef MIMIR_SEARCH_ALGORITHM_TEMPLATE_HPP_
#define MIMIR_SEARCH_ALGORITHM_TEMPLATE_HPP_

#include "config.hpp"
#include "search_node.hpp"
#include "search_node_view.hpp"
#include "search_node_builder.hpp"
#include "type_traits.hpp"
#include "applicable_action_generators.hpp"
#include "successor_state_generators.hpp"
#include "grounded/state_builder.hpp"
#include "grounded/state_view.hpp"
#include "lifted/state_builder.hpp"
#include "lifted/state_view.hpp"

#include "../buffer/containers/vector.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

enum SearchStatus {IN_PROGRESS, TIMEOUT, FAILED, SOLVED};

/**
 * Interface class.
*/
template<typename Derived, IsPlanningModeTag P, IsApplicableActionGeneratorTag AG>
class AlgorithmBase : public UncopyableMixin<AlgorithmBase<Derived, P, AG>> {
private:
    AlgorithmBase(const Problem& problem)
        : m_problem(problem)
        , m_state_repository(DefaultSuccessorStateGenerator<P>())
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem))
        , m_search_nodes(AutomaticVector(Builder<SearchNode<P>>(SearchNodeStatus::CLOSED, 0, View<State<P>>(nullptr), nullptr))) { }

    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;
    DefaultSuccessorStateGenerator<P>  m_state_repository;
    View<State<P>> m_initial_state;
    ApplicableActionGenerator<ApplicableActionGeneratorInstantiation<AG, P>> m_successor_generator;
    AutomaticVector<SearchNode<P>> m_search_nodes;

public:
    SearchStatus find_solution(GroundActionList& out_plan) {
        return self().find_solution_impl(out_plan);
    }
};


/**
 * ID class.
 *
 * Derive from it to provide your own implementation of an algorithm.
*/
struct AlgorithmBaseTag {};


/**
 * Concepts
*/
template<typename DerivedTag>
concept IsAlgorithmTag = std::derived_from<DerivedTag, AlgorithmBaseTag>;


/**
 * General implementation class.
 *
 * Spezialize it with your derived tag to provide your own implementation of an algorithm.
*/
template<IsAlgorithmTag A, IsPlanningModeTag P, IsApplicableActionGeneratorTag AG = DefaultApplicableActionGeneratorTag>
class Algorithm : public AlgorithmBase<Algorithm<A, P, AG>, P, AG> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHM_TEMPLATE_HPP_
