#ifndef MIMIR_SEARCH_TYPE_TRAITS_HPP_
#define MIMIR_SEARCH_TYPE_TRAITS_HPP_


namespace mimir
{

/**
 * Type traits.
 *
 * Define type traits that can be accessed in the base class
 * by specialization the trait for a type T.
 * This allows us to add constraints on the template parameters
 * in the attributes or functions in the interface of the base class.
*/
template<typename T>
struct TypeTraits { };

}  // namespace mimir

#endif  // MIMIR_SEARCH_TYPE_TRAITS_HPP_
