#ifndef MIMIR_SEARCH_STATE_BASE_HPP_
#define MIMIR_SEARCH_STATE_BASE_HPP_

#include "config.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a State.
/// @tparam Derived
template<typename Derived>
class StateBase {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    StateBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    int m_id;

public:
    // Define common interface for states.
    int get_id() const { return m_id; }
};


/// @brief A concrete state.
template<Config C>
class StateImpl : public StateBase<StateImpl<C>> {
private:
    // Implement configuration independent functionality.
};


template<Config C>
struct TypeTraits<StateImpl<C>> {
    using ConfigType = C;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_BASE_HPP_
