#ifndef MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
#define MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_

#include "state_base.hpp"
#include "state_builder_base.hpp"
#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

/// @brief Top-level CRTP based interface for a StateRepository.
/// @tparam Derived
template<typename Derived>
class StateRepositoryBase : public UncopyableMixin<StateRepositoryBase<Derived>> {
private:
    using Configuration = typename TypeTraits<Derived>::ConfigurationType;

    StateRepositoryBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Common interface for state creation.
    ///        Take some arguments and return a state.
    /// @return
    State<Configuration> create(const StateBuilder<Configuration>& builder) {
        return self().create_impl(builder);
    }
};


/// @brief A concrete state repository.
template<typename Configuration>
class StateRepository : StateRepositoryBase<StateRepository<Configuration>> {
private:
    // Implement configuration independent functionality.
};


template<typename Configuration>
struct TypeTraits<StateRepository<Configuration>> {
    using ConfigurationType = Configuration;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_STATE_REPOSITORY_BASE_HPP_
