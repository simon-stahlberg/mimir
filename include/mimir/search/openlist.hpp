#ifndef MIMIR_SEARCH_OPEN_LIST_HPP_
#define MIMIR_SEARCH_OPEN_LIST_HPP_

#include "type_traits.hpp"

#include "../common/mixins.hpp"


namespace mimir
{

/**
 * Interface class
*/
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


/**
 * Implementation class
 *
 * We provide specializations for
 * - PriorityQueueOpenListTag, a priority based open list in openlists/priority_queue.hpp
*/
template<typename OpenListTag>
class OpenList : public OpenListBase<OpenList<OpenListTag>> { };

}  // namespace mimir

#endif  // MIMIR_SEARCH_OPEN_LIST_HPP_
