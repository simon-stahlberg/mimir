#ifndef MIMIR_SEARCH_ALGORITHM_BASE_HPP_
#define MIMIR_SEARCH_ALGORITHM_BASE_HPP_

#include "../common/mixins.hpp"


namespace mimir
{

/// @brief CRTP based interface for a search algorithm
/// @tparam Derived
template<typename Derived>
class AlgorithmBase : public UncopyableMixin<AlgorithmBase<Derived>> {
private:
    AlgorithmBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void search() {
        self().search_impl();
    }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHM_BASE_HPP_
