#ifndef MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_
#define MIMIR_SEARCH_ALGORITHMS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/config.hpp"
#include "mimir/search/search_nodes.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/successor_state_generators.hpp"
#include "mimir/search/type_traits.hpp"

namespace mimir
{

enum SearchStatus
{
    IN_PROGRESS,
    OUT_OF_TIME,
    OUT_OF_MEMORY,
    FAILED,
    SOLVED
};

/**
 * Interface class.
 */
template<typename Derived>
class IAlgorithm
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using ConstStateView = ConstView<StateDispatcher<S, P>>;
    using ConstActionView = ConstView<ActionDispatcher<P, S>>;
    using ConstActionViewList = std::vector<ConstActionView>;

    IAlgorithm() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    SearchStatus find_solution(ConstActionViewList& out_plan) { return self().find_solution_impl(out_plan); }
};

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct AlgorithmTag
{
};

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
struct AlgorithmDispatcher
{
};

template<typename T>
struct is_algorithm_dispatcher : std::false_type
{
};

template<IsAlgorithmTag A>
struct is_algorithm_dispatcher<AlgorithmDispatcher<A>> : std::true_type
{
};

template<typename T>
concept IsAlgorithmDispatcher = is_algorithm_dispatcher<T>::value;

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsAlgorithmDispatcher A>
class Algorithm : public IAlgorithm<Algorithm<A>>
{
};

}
#endif