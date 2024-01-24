#ifndef MIMIR_SEARCH_STATES_TEMPLATE_HPP_
#define MIMIR_SEARCH_STATES_TEMPLATE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"

#include "../../buffer/builder_base.hpp"
#include "../../buffer/view_base.hpp"


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
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(state_id_type id) { self().set_id_impl(id); }
};

template<typename Derived>
class StateViewBase {
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    StateViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    state_id_type get_id() const { return self().get_id_impl(); }
};


/**
 * ID class
*/
struct StateBaseTag {};

template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateBaseTag>;


/**
 * Wrapper class.
 *
 * Wrap the tag and the planning mode to be able use a given planning mode.
*/
template<IsStateTag S, IsPlanningModeTag P>
struct WrappedStateTag {};

template<typename T>
struct is_wrapped_state_tag : std::false_type {};

template<IsStateTag S, IsPlanningModeTag P>
struct is_wrapped_state_tag<WrappedStateTag<S, P>> : std::true_type {};

template<typename T>
concept IsWrappedStateTag = is_wrapped_state_tag<T>::value;


/**
 * General implementation class.
 *
 * Spezialize the wrapped tag to provide your own implementation of a state representation.
*/
template<IsWrappedStateTag S>
class Builder<S> : public BuilderBase<Builder<S>>, public StateBuilderBase<Builder<S>> { };

template<IsWrappedStateTag S>
class View<S> : public ViewBase<View<S>>, public StateViewBase<View<S>> { };


} // namespace mimir



#endif  // MIMIR_SEARCH_STATES_TEMPLATE_HPP_
