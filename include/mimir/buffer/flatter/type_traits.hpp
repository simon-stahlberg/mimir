
#ifndef MIMIR_BUFFER_FLATTER_TYPE_TRAITS_HPP_
#define MIMIR_BUFFER_FLATTER_TYPE_TRAITS_HPP_

#include <type_traits>


namespace mimir 
{
    /**
     * Base template assuming all types are static.
     * 
     * For dynamic types we need to write an offset instead of the data directly.
     */
    template<typename T>
    struct is_dynamic_type : std::false_type {};
}

#endif 
