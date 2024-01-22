#ifndef MIMIR_SEARCH_HEURISTIC_HPP_
#define MIMIR_SEARCH_HEURISTIC_HPP_

#include "grounded/state_view.hpp"
#include "lifted/state_view.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/**
 * Interface class.
 *
 * We provide implementations for
 * - ZeroHeuristic, a heuristic that always returns 0 in heuristics/zero.hpp
*/
template<typename Derived>
class HeuristicBase : public UncopyableMixin<HeuristicBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::Config;

    HeuristicBase(Problem problem) : m_problem(problem) { }
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;

public:
    [[nodiscard]] double compute_heuristic(StateView<C> state) {
        return self().compute_heuristic_impl(state);
    }
};

/**
 * Concepts
*/
template<class Derived>
concept IsHeuristic = std::derived_from<Derived, HeuristicBase<Derived>>;

}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTIC_HPP_
