
#ifndef MIMIR_BUFFER_FLATTER_BUILDER_HPP_
#define MIMIR_BUFFER_FLATTER_BUILDER_HPP_

#include <type_traits>


namespace mimir 
{
    /**
     * Interface class.
    */
    template<typename Derived>
    class IBuilder {
        private:
            /// @brief Helper to cast to Derived.
            constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
            constexpr auto& self() { return static_cast<Derived&>(*this); }

        public:
            /// @brief Write serialized data to the stream.
            void finish() { self().finish_impl(); }

            /// @brief Clear all builders for reuse.
            void clear() { self().clear_impl(); }

            /// @brief Access the serialized buffer
            const uint8_t* get_data() const { return self().get_data_impl(); }
            size_t get_size() const { return self().get_size_impl(); }
    };

    
    /**
     * Implementation class.
     * 
     * Provide overload with your Tag.
    */
    template<typename Tag>
    class Builder : IBuilder<Builder<Tag>> {};
}

#endif 
