#ifndef MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
#define MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_

#include "config.hpp"
#include "type_traits.hpp"
#include "state_base.hpp"
#include "declarations.hpp"
#include "search_node.hpp"

#include "../buffer/builder_base.hpp"
#include "../formalism/problem/declarations.hpp"


namespace mimir {

// Interface class
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
    void set_parent_state(State<C> parent_state) { self().set_parent_state_impl(parent_state); }
    void set_ground_action(GroundAction creating_action) { self().set_ground_action_impl(creating_action); }
};


// Implementation class
template<Config C>
class Builder<SearchNode<C>> : public BuilderBase<Builder<SearchNode<C>>>, public SearchNodeBuilderBase<Builder<SearchNode<C>>> {
private:
    SearchNodeStatus m_status;
    int m_g_value;
    State<C> m_parent_state;
    GroundAction m_creating_action;

    void set_status_impl(SearchNodeStatus status) { m_status = status; }
    void set_g_value_impl(int g_value) { m_g_value = g_value; }
    void set_parent_state_impl(State<C> parent_state) { m_parent_state = parent_state; }
    void set_ground_action_impl(GroundAction creating_action) { m_creating_action = creating_action; }

    void finish_impl() {
        this->m_buffer.write(reinterpret_cast<const char*>(&m_status), sizeof(SearchNodeStatus));
        this->m_buffer.write(reinterpret_cast<const char*>(&m_g_value), sizeof(int));
        this->m_buffer.write(reinterpret_cast<const char*>(&m_parent_state), sizeof(State<C>));
        this->m_buffer.write(reinterpret_cast<const char*>(&m_creating_action), sizeof(GroundAction));
    }

    friend class BuilderBase<Builder<SearchNode<C>>>;
    friend class SearchNodeBuilderBase<Builder<SearchNode<C>>>;

public:
    Builder() = default;
    Builder(SearchNodeStatus status, int g_value, State<C> parent_state, GroundAction creating_action)
        : m_status(status), m_g_value(g_value), m_parent_state(parent_state), m_creating_action(creating_action) { }
};


template<Config C>
struct TypeTraits<Builder<SearchNode<C>>> {
    using ConfigType = C;
};



} // namespace mimir



#endif  // MIMIR_SEARCH_SEARCH_NODE_BUILDER_HPP_
