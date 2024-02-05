
#ifndef MIMIR_BUFFER_FLATTER_TYPES_UINT16_HPP_
#define MIMIR_BUFFER_FLATTER_TYPES_UINT16_HPP_
 
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
    struct Uint16Tag {};


    /**
     * Layout
    */
    template<>
    class Layout<Uint16Tag> {
        public:
            static constexpr size_t header_size = sizeof(uint16_t); 
            static constexpr size_t alignment = alignof(uint16_t);
    };

    
    /**
     * Type traits
    */
    template<>
    struct is_dynamic_type<Uint16Tag> : std::false_type{};


    /**
     * Builder
    */
    template<>
    class Builder<Uint16Tag> : public IBuilder<Builder<Uint16Tag>> {
        private:
            uint16_t m_data;
            ByteStream m_header_buffer;

            /* Implement IBuilder interface. */
            template<typename>
            friend class IBuilder;

            void finish_impl() {
                m_header_buffer.write<uint16_t>(m_data);
            }

            void clear_impl() {}

            uint8_t* get_data_impl() { return m_header_buffer.get_data(); }
            size_t get_size_impl() const { return m_header_buffer.get_size(); }

        public:
            void set_uint16(uint16_t value) {
                m_data = value;
            }
    };


    /**
     * View
    */
    template<>
    class View<Uint16Tag> {
    private:
        uint8_t* m_data;

    public:
        View(uint8_t* data) : m_data(data) {}

        uint16_t& get() const {
            return read_value<uint16_t>(m_data);
        }
    };
}


#endif
