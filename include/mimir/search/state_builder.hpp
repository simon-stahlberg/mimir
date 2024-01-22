#ifndef MIMIR_SEARCH_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_STATE_BUILDER_HPP_

#include "config.hpp"
#include "state.hpp"
#include "type_traits.hpp"

#include "../buffer/builder_base.hpp"


namespace mimir {

/**
 * Data types
*/
using state_id_type = uint32_t;


/**
 * Interface class
*/
template<typename Derived>
requires HasConfig<Derived>
class StateBuilderBase {
private:
    using C = typename TypeTraits<Derived>::Config;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(state_id_type id) { self().set_id_impl(id); }
};


/**
 * General implementation class
*/
template<typename C>
requires IsConfig<C>
class Builder<State<C>> : public BuilderBase<Builder<State<C>>>, public StateBuilderBase<Builder<State<C>>> { };


/**
 * Type traits.
*/
template<typename C>
requires IsConfig<C>
struct TypeTraits<Builder<State<C>>> {
    using Config = C;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_STATE_BUILDER_HPP_
