#ifndef MIMIR_SEARCH_HEURISTIC_BASE_HPP_
#define MIMIR_SEARCH_HEURISTIC_BASE_HPP_

#include "config.hpp"
#include "declarations.hpp"
#include "state_view.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

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

}  // namespace mimir

#endif  // MIMIR_SEARCH_OPEN_LIST_BASE_HPP_
