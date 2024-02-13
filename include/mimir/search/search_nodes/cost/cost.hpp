#ifndef MIMIR_SEARCH_SEARCH_NODES_COST_HPP_
#define MIMIR_SEARCH_SEARCH_NODES_COST_HPP_


#include "../../states.hpp"
#include "../../actions.hpp"

#include <flatmemory/flatmemory.hpp>

#include <cassert>


namespace mimir
{

/**
 * Data types
*/
enum SearchNodeStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};


/**
 * Flatmemory builder and view
*/
using CostSearchNodeLayout    = flatmemory::Tuple<SearchNodeStatus, int32_t, int32_t>;

using CostSearchNodeBuilder   = flatmemory::Builder<CostSearchNodeLayout>;
using CostSearchNodeView      = flatmemory::View<CostSearchNodeLayout>;
using CostSearchNodeVector    = flatmemory::FixedSizedTypeVector<CostSearchNodeLayout>;

/**
 * Wrappers for more meaningful access
*/
class CostSearchNodeBuilderWrapper {
private:
    CostSearchNodeBuilder m_builder;

public:
    CostSearchNodeBuilderWrapper(CostSearchNodeBuilder builder) : m_builder(std::move(builder)) { }

    void finish() { m_builder.finish(); }
    uint8_t* get_data() { return m_builder.buffer().data(); }
    size_t get_size() { return m_builder.buffer().size(); }
    const CostSearchNodeBuilder& get_flatmemory_builder() { return m_builder; }

    void set_status(SearchNodeStatus status) { m_builder.get<0>() = status; }
    void set_g_value(int32_t g_value) { m_builder.get<1>() = g_value; }
    void set_parent_state_id(int32_t parent_state_id) { m_builder.get<2>() = parent_state_id; }
};


class CostSearchNodeViewWrapper {
private:
    CostSearchNodeView m_view;

public:
    CostSearchNodeViewWrapper(CostSearchNodeView view) : m_view(view) { }

    SearchNodeStatus& get_status() { return m_view.get<0>(); }
    int32_t& get_g_value() { return m_view.get<1>(); }
    int32_t& get_parent_state_id() { return m_view.get<2>(); }
};


}

#endif