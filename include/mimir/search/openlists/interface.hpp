#ifndef MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_
#define MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_

#include "mimir/search/openlists/tags.hpp"

#include <concepts>
#include <cstdint>

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IOpenList
{
private:
    IOpenList() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    template<typename T>
    void insert(const T& item, double priority)
    {
        return self().insert_impl(item, priority);
    }

    [[nodiscard]] auto pop() { return self().pop_impl(); }

    [[nodiscard]] std::size_t size() const { return self().size_impl(); }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsOpenListDispatcher T>
class OpenList : public IOpenList<OpenList<T>>
{
};

}

#endif