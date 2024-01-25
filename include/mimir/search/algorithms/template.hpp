#ifndef MIMIR_SEARCH_ALGORITHMS_TEMPLATE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_TEMPLATE_HPP_

#include "../actions.hpp"
#include "../applicable_action_generators.hpp"
#include "../config.hpp"
#include "../search_nodes.hpp"
#include "../states.hpp"
#include "../successor_state_generators.hpp"
#include "../type_traits.hpp"

#include "../../buffer/containers/vector.hpp"
#include "../../common/mixins.hpp"
#include "../../formalism/problem/declarations.hpp"


namespace mimir
{

enum SearchStatus {IN_PROGRESS, TIMEOUT, FAILED, SOLVED};

/**
 * Interface class.
*/
template<typename Derived>
class AlgorithmBase : public UncopyableMixin<AlgorithmBase<Derived>> {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using AG = typename TypeTraits<Derived>::AAGTag;
    using SG = typename TypeTraits<Derived>::SSGTag;
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<A, P, S>>;
    using ActionViewList = std::vector<ActionView>;

    AlgorithmBase(const Problem& problem)
        : m_problem(problem)
        , m_state_repository(SSG<SSGDispatcher<SG, P, S, A>>())
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem)) {}

    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;
    SSG<SSGDispatcher<SG, P, S, A>> m_state_repository;
    StateView m_initial_state;
    AAG<AAGDispatcher<AG, P, S, A>> m_successor_generator;

public:
    SearchStatus find_solution(ActionViewList& out_plan) {
        return self().find_solution_impl(out_plan);
    }
};


/**
 * ID base class.
 * 
 * Derive from it to provide your own implementation.
 * 
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct AlgorithmBaseTag {};

template<typename DerivedTag>
concept IsAlgorithmTag = std::derived_from<DerivedTag, AlgorithmBaseTag>;


/**
 * General implementation class.
 *
 * Spezialize it with your derived tag to provide your own implementation of an algorithm.
*/
template<IsAlgorithmTag A>
class Algorithm : public AlgorithmBase<Algorithm<A>> { };



}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHMS_TEMPLATE_HPP_
