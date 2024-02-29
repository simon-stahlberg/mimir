#ifndef MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_
#define MIMIR_SEARCH_OPENLISTS_INTERFACE_HPP_

#include "../../common/mixins.hpp"
#include "../type_traits.hpp"

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IOpenList
{
private:
    using T = typename TypeTraits<Derived>::ValueType;

    IOpenList() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void insert(const T& item, double priority) { return self().insert_impl(item, priority); }

    [[nodiscard]] T pop() { return self().pop_impl(); }

    [[nodiscard]] std::size_t size() const { return self().size_impl(); }
};

/**
 * ID base class.
 *
 * Derive from it to provide your own implementation.
 *
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
 */
struct OpenListTag
{
};

template<class DerivedTag>
concept IsOpenListTag = std::derived_from<DerivedTag, OpenListTag>;

/**
 * Dispatcher class.
 *
 * Wrap the tag to dispatch the correct overload.
 * The template parameters are arguments that all specializations have in common.
 * Do not add your specialized arguments here, add them to your derived tag instead.
 */
template<IsOpenListTag O>
struct OpenListDispatcher
{
};

template<typename T>
struct is_openlist_dispatcher : std::false_type
{
};

template<IsOpenListTag O>
struct is_openlist_dispatcher<OpenListDispatcher<O>> : std::true_type
{
};

template<typename T>
concept IsOpenListDispatcher = is_openlist_dispatcher<T>::value;

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