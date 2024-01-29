#ifndef MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_

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

enum SearchStatus {IN_PROGRESS, OUT_OF_TIME, OUT_OF_MEMORY, FAILED, SOLVED};

/**
 * Interface class.
*/
template<typename Derived>
class IAlgorithm {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using StateView = View<StateDispatcher<S, P>>;
    using ActionView = View<ActionDispatcher<P, S>>;
    using ActionViewList = std::vector<ActionView>;

    IAlgorithm() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    SearchStatus find_solution(ActionViewList& out_plan) { return self().find_solution_impl(out_plan); }
};


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct AlgorithmTag {};

template<typename DerivedTag>
concept IsAlgorithmTag = std::derived_from<DerivedTag, AlgorithmTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
*/
template<IsAlgorithmTag A>
struct AlgorithmDispatcher {};

template<typename T>
struct is_algorithm_dispatcher : std::false_type {};

template<IsAlgorithmTag A>
struct is_algorithm_dispatcher<AlgorithmDispatcher<A>> : std::true_type {};

template<typename T>
concept IsAlgorithmDispatcher = is_algorithm_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
*/
template<IsAlgorithmDispatcher A>
class Algorithm : public IAlgorithm<Algorithm<A>> {};



}
#endif