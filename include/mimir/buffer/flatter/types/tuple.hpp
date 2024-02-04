
#ifndef MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
#define MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
 
#include "../../byte_stream.hpp"

#include "../layout.hpp"
#include "../builder.hpp"
#include "../view.hpp"
#include "../type_traits.hpp"

#include <cstdint>
#include <tuple>


namespace mimir
{
    /**
     * Dispatcher for tuple.
    */
    template<typename... Ts>
    struct TupleTag {};


    /**
     * Layout
    */
    template<typename... Ts>
    class Layout<TupleTag<Ts...>> {
        static constexpr std::array<size_t, sizeof...(Ts)> calculate_layout() {
            std::array<size_t, sizeof...(Ts)> layout{};
            size_t current_offset = 0;
            size_t index = 0;

            auto add_padding = [](size_t offset, size_t align) -> size_t {
                return (align - (offset % align)) % align;
            };

            ([&] {
                // Query alignment from Builder<Ts>, assuming it correctly accounts for dynamic types
                size_t align = alignment_of<typename Builder<Ts>::Type>(); // Assuming Builder<T> has a Type alias for T
                if (index > 0) {
                    size_t padding = add_padding(current_offset, align);
                    current_offset += padding;
                }
                layout[index++] = current_offset;
                // Use Builder<Ts>::size to get the serialized size of the element
                current_offset += Builder<Ts>::size; // Adjust this line
            }(), ...);

            return layout;
        }

        static constexpr std::array<size_t, sizeof...(Ts)> offsets = calculate_layout();
        static constexpr size_t total_size = offsets.back() + Builder<std::tuple_element_t<sizeof...(Ts) - 1, std::tuple<Ts...>>>::size;
    };

    
    /**
     * Type traits
    */
    template<typename... Ts>
    struct is_dynamic_type<Layout<TupleTag<Ts...>>> : std::bool_constant<(is_dynamic_type<Layout<Ts>>::value || ...)> {};


    /**
     * Builder
    */
    template<typename... Ts>  // Every type should have a id class to dispatch layout, builder, and view
    class Builder<TupleTag<Ts...>> : public IBuilder<Builder<TupleTag<Ts...>>> {
    private:
        std::tuple<Builder<Ts>...> m_data;  // Here i want to dispatch a builder
        ByteStream m_buffer;

        /* Implement IBuilder interface. */
        template<typename>
        friend class IBuilder;

        template<std::size_t I = 0>
        void finish_impl() {
            // TODO
        }

        void clear_impl() {
            // TODO
        }

        const uint8_t* get_data_impl() const { return m_buffer.get_data(); }
        size_t get_size() const { return m_buffer.get_size(); }
    };


    /**
     * View
    */
    template<typename... Ts>
    class TupleView {
    private:
        const uint8_t* m_data;

    public:
        TupleView(const uint8_t* data) : m_data(data) {}

        template<std::size_t I>
        auto get() const {
            // Compute the offset for the I-th element.
            // This requires a more complex implementation that calculates offsets based on TupleLayout.
            size_t offset = /* Calculate offset based on TupleLayout */;
            return read_value<std::tuple_element_t<I, std::tuple<Ts...>>>(m_data + offset);
        }
    };
}


#endif
