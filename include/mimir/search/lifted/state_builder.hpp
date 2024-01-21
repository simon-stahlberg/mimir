#ifndef MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_

#include "../state_builder.hpp"


namespace mimir
{

/**
 * No ID class since we want to provide a single implementation.
*/


/**
 * Implementation class
 *
 * The lifted state builder extends the builder base memory layout as follows:
 *  __________________________________
 * |                |          |      |
 * | data_size_type | state_id | TODO |
 * |________________|__________|______|
 *
 *
*/
template<>
class Builder<StateTag<LiftedTag>> : public BuilderBase<Builder<StateTag<LiftedTag>>>, public StateBuilderBase<Builder<StateTag<LiftedTag>>> {
    state_id_type m_id;

    /* Implement BuilderBase interface */
    data_size_type calculate_size_impl() const {
        return sizeof(state_id_type);
    }

    void finish_impl() {
        this->m_buffer.write(m_id);
    }

    friend class BuilderBase<Builder<StateTag<LiftedTag>>>;

    /* Implement StateBuilderBase interface */
    void set_id_impl(state_id_type id) { m_id = id; }

    friend class StateBuilderBase<Builder<StateTag<LiftedTag>>>;
};

}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_BUILDER_HPP_
