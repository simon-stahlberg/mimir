#ifndef MIMIR_SEARCH_OPEN_LIST_BASE_HPP_
#define MIMIR_SEARCH_OPEN_LIST_BASE_HPP_

#include <memory>

namespace mimir::search
{

template<typename Derived, typename T>
class OpenListBase {
private:
    OpenListBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

public:
    void insert(const T& item, double priority) {
        return self().insert_impl(item, priority);
    }

    T pop() {
        return self().pop_impl();
    }

    std::size_t size() const {
        return self().size_impl();
    }
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_OPEN_LIST_BASE_HPP_
