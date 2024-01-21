#ifndef MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_

#include "../config.hpp"
#include "../state_builder.hpp"


namespace mimir
{

/**
 * Implementation class
 *
 * The grounded state builder extends the builder base memory layout as follows:
 *  __________________________________
 * |                |          |      |
 * | data_size_type | state_id | TODO |
 * |________________|__________|______|
 *
 *
*/
template<>
class Builder<State<Grounded>> : public BuilderBase<Builder<State<Grounded>>>, public StateBuilderBase<Builder<State<Grounded>>> {
    state_id_type m_id;

    /* Implement BuilderBase interface */
    data_size_type calculate_size_impl() const {
        return sizeof(state_id_type);
    }

    void finish_impl() {
        this->m_buffer.write(m_id);
    }

    friend class BuilderBase<Builder<State<Grounded>>>;

    /* Implement StateBuilderBase interface */
    void set_id_impl(state_id_type id) { m_id = id; }

    friend class StateBuilderBase<Builder<State<Grounded>>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_GROUNDED_STATE_BUILDER_HPP_
