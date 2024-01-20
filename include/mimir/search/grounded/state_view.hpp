#ifndef MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_
#define MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_

#include "../state_view.hpp"

#include "../../buffer/char_stream_utils.hpp"


namespace mimir
{

/// @brief Concrete implementation of a lifted state.
template<>
class View<State<Grounded>> : public ViewBase<View<State<Grounded>>>, public StateViewBase<View<State<Grounded>>> {
private:
    static constexpr size_t s_id_offset = sizeof(data_size_type);
    static constexpr size_t s_data_ofset = sizeof(data_size_type) + sizeof(state_id_type);

    /* Implement ViewBase interface */
    [[nodiscard]] bool operator_equal_impl(const View<State<Grounded>>& other) const {
        if (get_size() != other.get_size()) return false;
        // Compare all data after the id.
        return (std::memcmp(
            get_data() + s_data_ofset,
            other.get_data() + s_data_ofset,
            get_size() - s_data_ofset) == 0);
    }

    [[nodiscard]] size_t hash_impl() const {
        // Hash all data after the id.
        size_t seed = get_size();
        int64_t hash[2];
        MurmurHash3_x64_128(get_data() + s_data_ofset, get_size() - s_data_ofset, seed, hash);
        return static_cast<std::size_t>(hash[0] + 0x9e3779b9 + (hash[1] << 6) + (hash[1] >> 2));
    }

    friend class ViewBase<View<State<Grounded>>>;

    /* Implement SearchNodeViewBase interface */
    [[nodiscard]] state_id_type get_id_impl() const {
        return read_value<state_id_type>(this->get_data() + s_id_offset);
    }

    friend class StateViewBase<View<State<Grounded>>>;

public:
    /// @brief Create a view on a SearchNode.
    explicit View(char* data) : ViewBase<View<State<Grounded>>>(data) { }
};

}  // namespace mimir


namespace std {
    template<>
    struct hash<mimir::View<mimir::State<mimir::Grounded>>>
    {
        std::size_t operator()(const mimir::View<mimir::State<mimir::Grounded>>& view) const {
            return view.hash();
        }
    };
}  // namespace std

#endif  // MIMIR_SEARCH_GROUNDED_STATE_VIEW_HPP_
