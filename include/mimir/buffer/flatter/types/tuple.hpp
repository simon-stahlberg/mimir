
#ifndef MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
#define MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
 
#include "../../byte_stream.hpp"

#include "../layout.hpp"
#include "../builder.hpp"
#include "../view.hpp"
#include "../type_traits.hpp"

#include <algorithm>
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
        private:
            /**
             * For static data types, the data at the offset will be the actual data.
             * For dynamic data types,the data at the offset will be an offset to the data.
            */
            static constexpr std::array<size_t, sizeof...(Ts) + 1> calculate_layout() {
                std::array<size_t, sizeof...(Ts) + 1> layout{};
                size_t index = 0;
                size_t cur_pos = 0;

                // First element must work with alignment.
                layout[0] = cur_pos;

                auto add_padding = [](size_t offset, size_t align) -> size_t {
                    return (align - (offset % align)) % align;
                };

                ([&] {
                    bool is_dynamic = is_dynamic_type<Ts>::value;
                    if (is_dynamic) {
                        cur_pos += add_padding(cur_pos, alignof(uint16_t));
                        cur_pos += sizeof(uint16_t);
                    } else {
                        if (index > 0) {
                            cur_pos += add_padding(cur_pos, Layout<Ts>::alignment);
                        }
                        cur_pos += Layout<Ts>::header_size;  // static size of the type
                    }
                    layout[++index] = cur_pos;
                }(), ...);

                return layout;
            }

        public:
            static constexpr std::array<size_t, sizeof...(Ts) + 1> offsets = calculate_layout();

            static constexpr size_t header_size = offsets.back();
            static constexpr size_t alignment = std::max({Layout<Ts>::alignment...});
    };

    
    /**
     * Type traits
     * 
     * A tuple is dynamic if at least one of its nested builders is dynamic.
    */
    template<typename... Ts>
    struct is_dynamic_type<TupleTag<Ts...>> : std::bool_constant<(is_dynamic_type<Ts>::value || ...)> {};


    /**
     * Builder
    */
    template<typename... Ts> 
    class Builder<TupleTag<Ts...>> : public IBuilder<Builder<TupleTag<Ts...>>> {
    private:
        std::tuple<Builder<Ts>...> m_data;
        ByteStream m_header_buffer;
        ByteStream m_dynamic_buffer;

        uint16_t offset = Layout<TupleTag<Ts...>>::header_size;

        /* Implement IBuilder interface. */
        template<typename>
        friend class IBuilder;

        template<std::size_t I = 0>
        void finish_rec_impl() {
            if constexpr (I < sizeof...(Ts)) {
                // Recursively call finish
                auto& builder = std::get<I>(m_data);
                builder.finish();
                
                // Write padding to satisfy alignment requirements
                m_header_buffer.write_padding(Layout<TupleTag<Ts...>>::offsets[I] - m_header_buffer.get_size());

                bool is_dynamic = is_dynamic_type<std::tuple_element_t<I, std::tuple<Ts...>>>::value;
                if (is_dynamic) {
                    m_header_buffer.write(offset);
                    m_dynamic_buffer.write(builder.get_data(), builder.get_size());     
                    offset += builder.get_size();
                } else {
                    m_header_buffer.write(builder.get_data(), builder.get_size());
                }

                // Call finish of next data
                finish_rec_impl<I + 1>();
            }
        }


        void finish_impl() {
            // Build header and dynamic buffer
            finish_rec_impl<0>();
            // Concatenate all buffers
            m_header_buffer.write(m_dynamic_buffer.get_data(), m_dynamic_buffer.get_size());  
        }


        template<std::size_t I = 0>
        void clear_rec_impl() {
            if constexpr (I < sizeof...(Ts)) {
                auto& builder = std::get<I>(m_data);
                builder.clear();

                // Call clear of next data
                clear_rec_impl<I + 1>();
            }
        }


        void clear_impl() {
            // Clear all nested builder.
            clear_rec_impl<0>();
            // Clear this builder.
            m_header_buffer.clear(),
            m_dynamic_buffer.clear();
        }

        const uint8_t* get_data_impl() const { return m_header_buffer.get_data(); }
        size_t get_size_impl() const { return m_header_buffer.get_size(); }
    };


    /**
     * View
    */
    template<typename... Ts>
    class View<TupleTag<Ts...>> {
    private:
        const uint8_t* m_data;

    public:
        View(const uint8_t* data) : m_data(data) {}

        template<std::size_t I>
        auto get() const {
            // Compute the offset for the I-th element.
            // This requires a more complex implementation that calculates offsets based on TupleLayout.
            size_t offset = Layout<TupleTag<Ts...>>::offsets[I];
            return read_value<View<std::tuple_element_t<I, std::tuple<Ts...>>>>(m_data + offset);
        }
    };
}


#endif
