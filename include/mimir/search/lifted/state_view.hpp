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
    static constexpr size_t s_id_offset =  sizeof(DataSizeType);
    static constexpr size_t s_data_ofset = sizeof(DataSizeType) + sizeof(uint32_t);

    /* Implement ViewBase interface */
    [[nodiscard]] bool operator_equal_impl(const View<State<Lifted>>& other) const {
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

    friend class ViewBase<View<State<Lifted>>>;

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


namespace std {
    template<>
    struct hash<mimir::View<mimir::State<mimir::Lifted>>>
    {
        std::size_t operator()(const mimir::View<mimir::State<mimir::Lifted>>& view) const {
            return view.hash();
        }
    };
}  // namespace std

#endif  // MIMIR_SEARCH_LIFTED_STATE_VIEW_HPP_
