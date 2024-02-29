#ifndef MIMIR_SEARCH_VIEW_CONST_HPP_
#define MIMIR_SEARCH_VIEW_CONST_HPP_

#include <cstddef>

namespace mimir
{
/**
 * Interface class
 */
template<typename Derived>
class IConstView
{
private:
    IConstView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] bool operator==(const IConstView& other) const { return self().are_equal_impl(other); }

    [[nodiscard]] size_t hash() const { return self().hash_impl(); }
};

/**
 * Implementation class
 */
template<typename T>
class ConstView : public IConstView<ConstView<T>>
{
};
}

#endif
