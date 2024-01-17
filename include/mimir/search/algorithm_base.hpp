#ifndef MIMIR_SEARCH_ALGORITHM_BASE_HPP_
#define MIMIR_SEARCH_ALGORITHM_BASE_HPP_

#include "search_space.hpp"
#include "state_base.hpp"
#include "type_traits.hpp"

#include "grounded/state_builder.hpp"
#include "grounded/state_repository.hpp"
#include "grounded/successor_generator.hpp"
#include "lifted/state_builder.hpp"
#include "lifted/state_repository.hpp"
#include "lifted/successor_generator.hpp"

#include "../common/mixins.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir
{

/// @brief CRTP based interface for a search algorithm
/// @tparam Derived
template<typename Derived>
class AlgorithmBase : public UncopyableMixin<AlgorithmBase<Derived>> {
private:
    using Config = typename TypeTraits<Derived>::ConfigType;

    AlgorithmBase(Problem problem)
        : m_problem(problem)
        , m_initial_state(m_state_repository.get_or_create_initial_state(problem))  { }

    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

    Problem m_problem;

    State<Config> m_initial_state;

    StateRepository<Config> m_state_repository;

    SuccessorGenerator<Config> m_successor_generator;

    SearchSpace<Config> m_search_space;

public:
    void find_solution() {
        self().find_solution_impl();
    }

    const State<Config>& get_initial_state() const {
        return m_initial_state;
    }
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_ALGORITHM_BASE_HPP_
