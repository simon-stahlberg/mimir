#ifndef MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_

#include "../config.hpp"
#include "../state_builder.hpp"


namespace mimir
{

/// @brief Concrete implementation of a grounded state.
template<>
class Builder<State<Lifted>> : public BuilderBase<Builder<State<Lifted>>>, public StateBuilderBase<Builder<State<Lifted>>> {
    uint32_t m_id;

    /* Implement BuilderBase interface */
    uint32_t calculate_size_impl() const {
        return sizeof(uint32_t);
    }

    void finish_impl() {
        this->m_buffer.write(m_id);
    }

    friend class BuilderBase<Builder<State<Lifted>>>;

    /* Implement StateBuilderBase interface */
    void set_id_impl(uint32_t id) { m_id = id; }

    friend class StateBuilderBase<Builder<State<Lifted>>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_
