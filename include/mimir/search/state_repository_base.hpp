#ifndef MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
#define MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_

#include "config.hpp"
#include "declarations.hpp"
#include "state_base.hpp"
#include "state_builder_base.hpp"
#include "type_traits.hpp"

#include "../buffers/segmented_binary_vector.hpp"
#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"

#include <unordered_set>


namespace mimir
{

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<typename Derived>
class StateRepositoryBase : public UncopyableMixin<StateRepositoryBase<Derived>> {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    StateRepositoryBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    // Persistent storage
    SegmentedBinaryVector<100000> m_data;

    // Creates states uniquely
    std::unordered_set<State<C>> m_uniqueness;

    // Reuse memory to create states.
    StateBuilder<C> m_state_builder;

public:
    [[nodiscard]] State<C> get_or_create_initial_state(Problem problem) {
        return self().get_or_create_initial_state_impl(problem);
    }

    [[nodiscard]] State<C> get_or_create_successor_state(State<C> state, GroundAction action) {
        return self().get_or_create_successor_state_impl(state, action);
    }
};


/// @brief A concrete state repository.
template<Config C>
class StateRepository : public StateRepositoryBase<StateRepository<C>> {
private:
    // Implement Config independent functionality.
};


template<Config C>
struct TypeTraits<StateRepository<C>> {
    using ConfigType = C;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
