#ifndef MIMIR_SEARCH_TYPE_TRAITS_HPP_
#define MIMIR_SEARCH_TYPE_TRAITS_HPP_


namespace mimir
{

/// @brief Define type traits that can be accessed in the base class.
///
///        We use it to define member functions or attributes
///        that depend on the nested Configuration type.
/// @tparam T is the class for which we define the type traits.
template<typename T>
struct TypeTraits { };

}  // namespace mimir

#endif  // MIMIR_SEARCH_TYPE_TRAITS_HPP_
