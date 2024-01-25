#ifndef MIMIR_SEARCH_OPENLISTS_TEMPLATE_HPP_
#define MIMIR_SEARCH_OPENLISTS_TEMPLATE_HPP_

#include "../type_traits.hpp"

#include "../../common/mixins.hpp"


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
 * ID base class.
 * 
 * Derive from it to provide your own implementation.
 * 
 * Define new template parameters to your derived tag
 * in the declaration file of your derived class.
*/
struct OpenListBaseTag {};

template<class DerivedTag>
concept IsOpenListTag = std::derived_from<DerivedTag, OpenListBaseTag>;


/**
 * General implementation class.
 *
 * Spezialize it using our wrapper dispatch class.
 * in the spezialization file of your derived class.
*/
template<IsOpenListTag T>
class OpenList : public OpenListBase<OpenList<T>> { };


}  // namespace mimir

#endif  // MIMIR_SEARCH_OPENLIST_TEMPLATE_HPP_
