#ifndef MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_
#define MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_

#include "../state_view.hpp"

#include "../../buffer/char_stream_utils.hpp"


namespace mimir
{

/// @brief Concrete implementation of a lifted state.
template<>
class View<State<Lifted>> : public ViewBase<View<State<Lifted>>>, public StateViewBase<View<State<Lifted>>> {
private:
    static constexpr size_t s_id_offset = sizeof(DataSizeType);

    /* Implement ViewBase has no interface: no methods must be overriden */

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] uint32_t get_id_impl() const {
        return read_value<uint32_t>(this->get_data() + s_id_offset);
    }

    friend class StateViewBase<View<State<Lifted>>>;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<State<Lifted>>>(data) { }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_
