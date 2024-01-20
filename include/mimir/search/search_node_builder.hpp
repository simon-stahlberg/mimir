#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "grounded/state_view.hpp"
#include "lifted/state_view.hpp"
#include "declarations.hpp"
#include "search_node.hpp"

#include "../buffer/builder_base.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir {

/**
 * Interface class
*/
template<typename Derived>
class SearchNodeBuilderBase {
private:
    using C = typename TypeTraits<Derived>::ConfigType;

    SearchNodeBuilderBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void set_status(SearchNodeStatus status) { self().set_status_impl(status); }
    void set_g_value(int g_value) { self().set_g_value_impl(g_value); }
    void set_parent_state(View<State<C>> parent_state) { self().set_parent_state_impl(parent_state); }
    void set_ground_action(GroundAction creating_action) { self().set_ground_action_impl(creating_action); }
};


/**
 * Implementation class
*/
template<Config C>
class Builder<SearchNode<C>> : public BuilderBase<Builder<SearchNode<C>>>, public SearchNodeBuilderBase<Builder<SearchNode<C>>> {
private:
    SearchNodeStatus m_status;
    int m_g_value;
    View<State<C>> m_parent_state;
    GroundAction m_creating_action;

    /* Implement BuilderBase interface */
    uint32_t calculate_size_impl() const {
        return sizeof(SearchNodeStatus) + sizeof(int) + sizeof(char*) + sizeof(GroundAction);
    }

    void finish_impl() {
        DataSizeType size = this->calculate_size();
        this->m_buffer.write(size);
        this->m_buffer.write(m_status);
        this->m_buffer.write(m_g_value);
        this->m_buffer.write(m_parent_state.get_data());
        this->m_buffer.write(m_creating_action);
    }

    friend class BuilderBase<Builder<SearchNode<C>>>;

    /* Implement SearchNodeBuilderBase interface */
    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(View<State<C>> parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(GroundAction creating_action) { m_creating_action = creating_action; }

    friend class SearchNodeBuilderBase<Builder<SearchNode<C>>>;

public:
    Builder() : m_parent_state(View<State<C>>(nullptr)) { }

    /// @brief Construct a builder with custom default values.
    Builder(SearchNodeStatus status, int g_value, View<State<C>> parent_state, GroundAction creating_action)
        : m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) {
        this->finish();
    }
};


/**
 * Type traits.
*/
template<Config C>
struct TypeTraits<Builder<SearchNode<C>>> {
    using ConfigType = C;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
