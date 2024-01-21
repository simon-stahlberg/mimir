#ifndef MIMIR_SEARCH_HEURISTIC_HPP_
#define MIMIR_SEARCH_HEURISTIC_HPP_

#include "state_view.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

/**
 * Interface class
*/
template<typename Derived>
class HeuristicBase : public UncopyableMixin<HeuristicBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    HeuristicBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] double compute_heuristic(View<State<C>> state) {
        return self().compute_heuristic_impl(state);
    }
};


/**
 * Implementation class
 *
 * We provide specializations for
 * - Zero, a heuristic that always returns 0 in heursitics/zero.hpp
*/
template<typename T>
class Heuristic : public HeuristicBase<Heuristic<T>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_HEURISTIC_HPP_
