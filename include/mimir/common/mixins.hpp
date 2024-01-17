#ifndef MIMIR_COMMON_MIXINS_HPP_
#define MIMIR_COMMON_MIXINS_HPP_

#include "id.hpp"

#include <sstream>


/// @brief In this header, we define common mixins.
///        We distinguish two types of mixins with different naming conventions.
///           1. Mixins that add functionality should use present participle to emphasize action,
///              e.g., FormattingMixin
///           2. Mixins that add constraints should be adjectives,
///              e.g., UncopyableMixin
namespace mimir {

/// @brief Provide common functionality to enhance usability such as printing or hashing.
template<typename Derived>
class FormattingMixin {
private:
    /// @brief Avoid wrong template parameter passing declaring private default constructor.
    FormattingMixin() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    // TODO (Dominik): Define common functionality here

    /// @brief Overload of the output stream insertion operator (operator<<).
    friend std::ostream& operator<<(std::ostream& os, const FormattingMixin& element) {
        os << element.str();
        return os;
    }

    /// @brief Compute a string representation of this object.
    void str(std::ostringstream& os) const {
        self().str_impl(os);
    }

    /// @brief Return a string representation.
    std::string str() const {
        std::ostringstream os;
        self().str_impl(os);
        return os.str();
    }
};


/// @brief Express that a type T is moveable but not copieable
template<typename Derived>
class UncopyableMixin {
private:
    /// @brief Avoid wrong template parameter passing declaring private default constructor.
    UncopyableMixin() = default;
    friend Derived;

public:
    // moveable but not copyable
    UncopyableMixin(const UncopyableMixin& other) = delete;
    UncopyableMixin& operator=(const UncopyableMixin& other) = delete;
    UncopyableMixin(UncopyableMixin&& other) = default;
    UncopyableMixin& operator=(UncopyableMixin&& other) = default;
};


/// @brief Provide functionality to construct and get an ID.
template<typename Derived> 
class IDMixin {
private:
    ID<Derived> m_id;

public:
    explicit IDMixin(int id) : m_id(ID<Derived>(id)) { }

    ID<Derived> get_id() const { return m_id; }
};

}  // namespace mimir

#endif  // MIMIR_COMMON_MIXINS_HPP_