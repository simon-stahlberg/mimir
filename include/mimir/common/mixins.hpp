#ifndef MIMIR_COMMON_MIXINS_HPP_
#define MIMIR_COMMON_MIXINS_HPP_

#include <sstream>


namespace mimir::formalism {

/// @brief In this header, we define common primitive mixins that
///        express some constraints or define some orthogonal functionalities.

/// @brief Provide common functionality to enhance usability such as printing or hashing.
template<typename Derived>
class BaseMixin {
public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<Derived const&>(*this); }

    // TODO (Dominik): Define common functionality here

    /// @brief Overload of the output stream insertion operator (operator<<).
    friend std::ostream& operator<<(std::ostream& os, const BaseMixin& element) {
        // TODO (Dominik): implement
        return os;
    }
};


/// @brief Express that a type T is moveable but not copieable
template<typename Derived>
class UncopyableMixin {
public:
    UncopyableMixin() = default;
    // moveable but not copyable
    UncopyableMixin(const UncopyableMixin& other) = delete;
    UncopyableMixin& operator=(const UncopyableMixin& other) = delete;
    UncopyableMixin(UncopyableMixin&& other) = default;
    UncopyableMixin& operator=(UncopyableMixin&& other) = default;
};

}

#endif