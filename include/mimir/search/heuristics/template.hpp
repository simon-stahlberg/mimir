#ifndef MIMIR_SEARCH_HEURISTICS_TEMPLATE_HPP_
#define MIMIR_SEARCH_HEURISTICS_TEMPLATE_HPP_

#include "../actions.hpp"
#include "../states.hpp"
#include "../type_traits.hpp"

#include "../../common/mixins.hpp"
#include "../../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class HeuristicBase {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;
    using S = typename TypeTraits<Derived>::StateTag;
    using A = typename TypeTraits<Derived>::ActionTag;
    using StateView = View<StateDispatcher<S, P>>;

    HeuristicBase(Problem problem) : m_problem(problem) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;

public:
    [[nodiscard]] double compute_heuristic(StateView state) {
        return self().compute_heuristic_impl(state);
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
struct HeuristicBaseTag {};

template<class DerivedTag>
concept IsHeuristicTag = std::derived_from<DerivedTag, HeuristicBaseTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag and variable number of template arguments.
 *
 * Define required input template parameters using SFINAE
 * in the declaration file of your derived class.
*/
template<IsHeuristicTag H, typename... Ts>
struct HeuristicDispatcher {};

template<typename T>
struct is_heuristic_dispatcher : std::false_type {};

template<typename T>
concept IsHeuristicDispatcher = is_heuristic_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Spezialize it with your dispatcher.
*/
template<IsHeuristicDispatcher T>
class Heuristic : public HeuristicBase<Heuristic<T>> { };




}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTIC_TEMPLATE_HPP_
