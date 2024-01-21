#ifndef MIMIR_SEARCH_OPEN_LIST_BASE_HPP_
#define MIMIR_SEARCH_OPEN_LIST_BASE_HPP_

#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{


template<typename Derived>
class OpenListBase : public UncopyableMixin<OpenListBase<Derived>> {
private:
    using T = typename TypeTraits<Derived>::ValueType;

    OpenListBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void insert(const T& item, double priority) {
        return self().insert_impl(item, priority);
    }

    [[nodiscard]] T pop() {
        return self().pop_impl();
    }

    [[nodiscard]] std::size_t size() const {
        return self().size_impl();
    }
};


}  // namespace mimir

#endif  // MIMIR_SEARCH_OPEN_LIST_BASE_HPP_
