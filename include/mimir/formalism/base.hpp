#ifndef MIMIR_FORMALISM_BASE_HPP_
#define MIMIR_FORMALISM_BASE_HPP_

#include <sstream>


namespace mimir::formalism {

template<typename Derived>
class Base {
public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<Derived const&>(*this); }

    // TODO (Dominik): Define common functionality here

    /// @brief Overload of the output stream insertion operator (operator<<).
    friend std::ostream& operator<<(std::ostream& os, const Base& element) {
        // TODO (Dominik): implement
        return os;
    }
};


/// @brief Express that a type T is meant to be copieable
template<typename T>
using CopyableBase = Base<T>;


/// @brief Express that a type T is meant to be moveable but not copieable
template<typename Derived>
class MoveableBase : Base<MoveableBase<Derived>> {
public:
    // moveable but not copyable
    MoveableBase(const MoveableBase& other) = delete;
    MoveableBase& operator=(const MoveableBase& other) = delete;
    MoveableBase(MoveableBase&& other) = default;
    MoveableBase& operator=(MoveableBase&& other) = default;
};

}

#endif