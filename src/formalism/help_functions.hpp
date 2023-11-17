#ifndef MIMIR_FORMALISM_HELP_FUNCTIONS_HPP_
#define MIMIR_FORMALISM_HELP_FUNCTIONS_HPP_

#include <cstdint>
#include <functional>
#include <iostream>
#include <vector>

template<class T>
inline void
print_vector(std::ostream& os, const std::vector<T>& vector, const std::string& left = "[", const std::string& right = "]", const std::string& delimiter = ", ")
{
    os << left;
    for (size_t index = 0; index < vector.size(); ++index)
    {
        os << vector[index];

        if ((index + 1) < vector.size())
        {
            os << delimiter;
        }
    }
    os << right;
}

// --------------
// Hash functions
// --------------

template<typename T>
inline void hash_combine(size_t& seed, const T& val)
{
    seed ^= std::hash<T>()(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<>
inline void hash_combine(size_t& seed, const std::size_t& val)
{
    seed ^= val + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename... Types>
inline size_t hash_combine(const Types&... args)
{
    size_t seed = 0;
    (hash_combine(seed, args), ...);
    return seed;
}

template<class T>
inline std::size_t hash_vector(const std::vector<T>& vector)
{
    const auto hash_function = std::hash<T>();
    std::size_t aggregated_hash = 0;
    for (const auto& item : vector)
    {
        const auto item_hash = hash_function(item);
        hash_combine(aggregated_hash, item_hash);
    }
    return aggregated_hash;
}

// --------------------
// Comparison functions
// --------------------

template<typename T>
void compare_combine(std::int32_t& comparison, const T& lhs, const T& rhs)
{
    if (comparison == 0)
    {
        const std::less<T> less;

        if (less(lhs, rhs))
        {
            comparison = -1;
        }

        if (less(rhs, lhs))
        {
            comparison = 1;
        }
    }
}

template<typename... Types, std::size_t... Index>
constexpr bool less_for_each(const std::tuple<Types...>& left, const std::tuple<Types...>& right, std::index_sequence<Index...>)
{
    std::int32_t comparison = 0;
    (compare_combine(comparison, std::get<Index>(left), std::get<Index>(right)), ...);
    return comparison == -1;
}

template<typename... Types>
constexpr bool less_combine(const std::tuple<Types...>& left, const std::tuple<Types...>& right)
{
    // It might be sufficient to use std::less<std::tuples<Types...>>;
    // However, I haven't found any documentation if it is part of the C++ standard.
    // There's some indication that lexicographic comparison is removed in C++20.

    return less_for_each(left, right, std::index_sequence_for<Types...> {});
}

template<typename... Types, std::size_t... Index>
constexpr bool equal_to_for_each(const std::tuple<Types...>& left, const std::tuple<Types...>& right, std::index_sequence<Index...>)
{
    std::int32_t comparison = 0;
    (compare_combine(comparison, std::get<Index>(left), std::get<Index>(right)), ...);
    return comparison == 0;
}

template<typename... Types>
constexpr bool equal_to_combine(const std::tuple<Types...>& left, const std::tuple<Types...>& right)
{
    // It might be sufficient to use std::less<std::tuples<Types...>>;
    // However, I haven't found any documentation if it is part of the C++ standard.
    // There's some indication that lexicographic comparison is removed in C++20.

    return equal_to_for_each(left, right, std::index_sequence_for<Types...> {});
}

#endif  // MIMIR_FORMALISM_HELP_FUNCTIONS_HPP_
