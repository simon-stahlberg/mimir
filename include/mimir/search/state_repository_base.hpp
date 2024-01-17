#ifndef MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
#define MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_

#include "state_base.hpp"
#include "state_builder_base.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<typename Derived>
class StateRepositoryBase : public UncopyableMixin<StateRepositoryBase<Derived>> {
private:
    using Config = typename TypeTraits<Derived>::ConfigType;

    StateRepositoryBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    // Reuse memory to create successor states.
    // TODO (Dominik): maybe we dont need this.
    StateBuilder<Config> m_state_builder;

public:
    const State<Config>& get_or_create_initial_state(const Problem& problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    const State<Config>& get_or_create_successor_state(const State<Config>& state, const GroundAction& action) {
        return self().get_or_create_successor_state_impl(state, action);
    }

    const State<Config>& lookup_state(const ID<State<Config>>& state_id) {
        return self().lookup_state_impl(state_id);
    }
};


/// @brief A concrete state repository.
template<typename Config>
class StateRepository : public StateRepositoryBase<StateRepository<Config>> {
private:
    // Implement Config independent functionality.
};


template<typename Config>
struct TypeTraits<StateRepository<Config>> {
    using ConfigType = Config;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
