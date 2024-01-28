#ifndef MIMIR_SEARCH_STATES_TEMPLATE_HPP_
#define MIMIR_SEARCH_STATES_TEMPLATE_HPP_

#include "../config.hpp"
#include "../type_traits.hpp"

#include "../../buffer/builder_base.hpp"
#include "../../buffer/view_base.hpp"
#include "../../buffer/wrappers/bitset.hpp"


namespace mimir {

/**
 * Data types
*/
using state_id_type = uint32_t;


/**
 * Interface class
*/
template<typename Derived>
class StateBuilderBase
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    StateBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_id(state_id_type id) { self().set_id_impl(id); }

    void set_num_atoms(size_t num_atoms) { self().set_num_atoms_impl(num_atoms); }
    void add_atom(size_t atom_id) { self().set_atom(atom_id); }
    void set_atoms() { self().set_atoms_impl(); }
};

template<typename Derived>
class StateViewBase
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    StateViewBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] state_id_type get_id() const { return self().get_id_impl(); }

    [[nodiscard]] BitsetView get_atoms() const { return self().get_atoms_impl(); }
};


/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct StateBaseTag {};

template<typename DerivedTag>
concept IsStateTag = std::derived_from<DerivedTag, StateBaseTag>;


/**
 * Dispatcher class.
 *
 * Wrap the tag and variable number of template arguments.
 *
 * Define required input template parameters using SFINAE
 * in the declaration file of your derived class.
*/
template<IsStateTag S, typename... Ts>
struct StateDispatcher {};

template<typename T>
struct is_state_dispatcher : std::false_type {};

template<typename T>
concept IsStateDispatcher = is_state_dispatcher<T>::value;


/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a state representation.
*/
template<IsStateDispatcher S>
class Builder<S> : public BuilderBase<Builder<S>>, public StateBuilderBase<Builder<S>> { };

template<IsStateDispatcher S>
class View<S> : public ViewBase<View<S>>, public StateViewBase<View<S>> { };


} // namespace mimir



#endif  // MIMIR_SEARCH_STATES_TEMPLATE_HPP_