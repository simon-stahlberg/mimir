#ifndef MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_

#include "../config.hpp"
#include "../state_builder.hpp"


namespace mimir
{

/// @brief Concrete implementation of a grounded state.
template<>
class Builder<State<Grounded>> : public BuilderBase<Builder<State<Grounded>>>, public StateBuilderBase<Builder<State<Grounded>>> {
    uint32_t m_id;

    /* Implement BuilderBase interface */
    uint32_t calculate_size_impl() const {
        return sizeof(uint32_t);
    }

    void finish_impl() {
        DataSizeType size = this->calculate_size();
        this->m_buffer.write(reinterpret_cast<const char*>(&size), sizeof(DataSizeType));
        this->m_buffer.write(reinterpret_cast<const char*>(&m_id), sizeof(uint32_t));
    }

    friend class BuilderBase<Builder<State<Grounded>>>;

    /* Implement StateBuilderBase interface */
    void set_id_impl(uint32_t id) { m_id = id; }

    friend class StateBuilderBase<Builder<State<Grounded>>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
