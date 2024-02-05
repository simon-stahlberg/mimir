
#ifndef MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
#define MIMIR_BUFFER_FLATTER_TYPES_TUPLE_HPP_
 
#include "../../byte_stream.hpp"
#include "../../byte_stream_utils.hpp"

#include "../layout.hpp"
#include "../builder.hpp"
#include "../view.hpp"
#include "../type_traits.hpp"
#include "../types.hpp"

#include <algorithm>
#include <cassert>
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
             * For static data types, the offset will point to the actual data.
             * For dynamic data types, the offset will point to an offset to the actual data.
            */
            static constexpr std::array<offset_type, sizeof...(Ts) + 1> calculate_layout() {
                std::array<offset_type, sizeof...(Ts) + 1> layout{};
                size_t index = 0;
                offset_type cur_pos = 0;

                // First element must work with alignment.
                layout[0] = cur_pos;

                ([&] {
                    bool is_dynamic = is_dynamic_type<Ts>::value;
                    if (is_dynamic) {
                        cur_pos += compute_amount_padding(cur_pos, alignof(offset_type));
                        cur_pos += sizeof(offset_type);
                    } else {
                        cur_pos += compute_amount_padding(cur_pos, Layout<Ts>::alignment);
                        cur_pos += Layout<Ts>::size;
                    }
                    layout[++index] = cur_pos;
                }(), ...);

                return layout;
            }

        public:
            static constexpr std::array<offset_type, sizeof...(Ts) + 1> offsets = calculate_layout();

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
            ByteStream m_buffer;
            ByteStream m_dynamic_buffer;

            /* Implement IBuilder interface. */
            template<typename>
            friend class IBuilder;

            template<std::size_t I = 0>
            void finish_rec_impl() {
                assert(m_buffer.get_size() == Layout<TupleTag<Ts...>>::offsets[I]);

                // offset is the first position to write the dynamic data
                offset_type offset = Layout<TupleTag<Ts...>>::offsets.back();
                if constexpr (I < sizeof...(Ts)) {
                    // Recursively call finish
                    auto& nested_builder = std::get<I>(m_data);
                    nested_builder.finish();
                    
                    // Write padding to satisfy alignment requirements
                    m_buffer.write_padding(Layout<TupleTag<Ts...>>::offsets[I] - m_buffer.get_size());

                    bool is_dynamic = is_dynamic_type<std::tuple_element_t<I, std::tuple<Ts...>>>::value;
                    if (is_dynamic) {
                        m_buffer.write(offset);
                        m_dynamic_buffer.write(nested_builder.get_data(), nested_builder.get_size());     
                        offset += nested_builder.get_size();
                    } else {
                        m_buffer.write(nested_builder.get_data(), nested_builder.get_size());
                    }

                    // Call finish of next data
                    finish_rec_impl<I + 1>();
                }
            }


            void finish_impl() {
                // Build header and dynamic buffer
                finish_rec_impl<0>();
                // Concatenate all buffers
                m_buffer.write(m_dynamic_buffer.get_data(), m_dynamic_buffer.get_size());  
                // Write alignment padding
                m_buffer.write_padding(compute_amount_padding(m_buffer.get_size(), Layout<TupleTag<Ts...>>::alignment));
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
                // Clear all nested builders.
                clear_rec_impl<0>();
                // Clear this builder.
                m_buffer.clear(),
                m_dynamic_buffer.clear();
            }

            uint8_t* get_data_impl() { return m_buffer.get_data(); }
            size_t get_size_impl() const { return m_buffer.get_size(); }

        public:
            template<std::size_t I>
            auto& get_builder() {
                return std::get<I>(m_data);
            }
    };


    /**
     * View
    */
    template<typename... Ts>
    class View<TupleTag<Ts...>> {
    private:
        uint8_t* m_data;

    public:
        View(uint8_t* data) : m_data(data) {}

        /**
         * Returns a View to the I-th element.
         * 
         * If the I-th type is dynamic we must add the offset to the actual data first.
        */
        template<std::size_t I>
        auto get() {
            offset_type offset = Layout<TupleTag<Ts...>>::offsets[I];
            if constexpr (is_dynamic_type<std::tuple_element_t<I, std::tuple<Ts...>>>::value) {
                offset = read_value<offset_type>(m_data + offset);
            }
            return View<std::tuple_element_t<I, std::tuple<Ts...>>>(m_data + offset);
        }
    };
}


#endif
