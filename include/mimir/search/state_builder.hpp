#ifndef MIMIR_SEARCH_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_STATE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state.hpp"
#include "search_node.hpp"

#include "../buffer/builder_base.hpp"


namespace mimir {

/**
 * Interface class
*/
template<typename Derived>
class StateBuilderBase {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    // TODO (Dominik): implement
    void set_id(state_id_type id) { self().set_id_impl(id); }
};


/**
 * Implementation class
*/
template<Config C>
class Builder<State<C>> : public BuilderBase<Builder<State<C>>>, public StateBuilderBase<Builder<State<C>>> { };


/**
 * Type traits.
*/
template<Config C>
struct TypeTraits<Builder<State<C>>> {
    using ConfigType = C;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_STATE_BUILDER_HPP_
