#ifndef MIMIR_COMMON_MIXINS_HPP_
#define MIMIR_COMMON_MIXINS_HPP_

#include <sstream>

/// @brief In this header, we define common mixins.
namespace mimir
{

/// @brief Provide common functionality to enhance usability such as printing or hashing.
template<typename Derived>
class FormattingMixin
{
private:
    /// @brief Avoid wrong template parameter passing declaring private default constructor.
    FormattingMixin() = default;
    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    // TODO (Dominik): Define common functionality here

    /// @brief Overload of the output stream insertion operator (operator<<).
    friend std::ostream& operator<<(std::ostream& os, const FormattingMixin& element)
    {
        os << element.str();
        return os;
    }

    /// @brief Compute a string representation of this object.
    void str(std::ostringstream& os) const { self().str_impl(os); }

    /// @brief Return a string representation.
    std::string str() const
    {
        std::ostringstream os;
        self().str_impl(os);
        return os.str();
    }
};

}  // namespace mimir

#endif  // MIMIR_COMMON_MIXINS_HPP_