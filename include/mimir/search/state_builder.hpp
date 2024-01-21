#ifndef MIMIR_SEARCH_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_STATE_BUILDER_HPP_

#include "config.hpp"
#include "state_tag.hpp"
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
class StateBuilderBase {
private:
    using C = typename TypeTraits<Derived>::ConfigTag;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(state_id_type id) { self().set_id_impl(id); }
};


/**
 * Implementation class
*/
template<Config C>
class Builder<StateTag<C>> : public BuilderBase<Builder<StateTag<C>>>, public StateBuilderBase<Builder<StateTag<C>>> { };


/**
 * Type traits.
*/
template<Config C>
struct TypeTraits<Builder<StateTag<C>>> {
    using ConfigTag = C;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_STATE_BUILDER_HPP_
