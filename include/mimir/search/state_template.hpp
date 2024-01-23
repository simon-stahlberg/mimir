#ifndef MIMIR_SEARCH_STATE_TEMPLATE_HPP_
#define MIMIR_SEARCH_STATE_TEMPLATE_HPP_

#include "config.hpp"
#include "type_traits.hpp"

#include "../buffer/view_base.hpp"
#include "../buffer/byte_stream_utils.hpp"


namespace mimir {

/**
 * Data types
*/
using state_id_type = uint32_t;


/**
 * Interface classes
*/
template<typename Derived>
requires HasPlanningModeTag<Derived>
class StateBuilderBase {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(state_id_type id) { self().set_id_impl(id); }
};


template<typename Derived>
requires HasPlanningModeTag<Derived>
class StateViewBase {
private:
    using P = typename TypeTraits<Derived>::PlanningMode;

    StateViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /* Mutable getters. */

    /* Immutable getters. */
    state_id_type get_id() const { return self().get_id_impl(); }
};


/**
 * ID class.
 *
 * Derive from it to provide your own implementation of a successor state generator.
*/
struct StateBaseTag {};


/**
 * Concepts
*/
template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateBaseTag>;


/**
 * General implementation classes.
 *
 * Spezialize them with your derived tag to provide your own implementation of a state view and state builder.
*/
template<IsStateTag S>
class Builder<S> : public BuilderBase<Builder<S>>, public StateBuilderBase<Builder<S>> { };

template<IsStateTag S>
class View<S> : public ViewBase<View<S>>, public StateViewBase<View<S>> { };


} // namespace mimir



#endif  // MIMIR_SEARCH_STATE_TEMPLATE_HPP_
