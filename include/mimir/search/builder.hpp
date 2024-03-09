#ifndef MIMIR_SEARCH_BUILDER_HPP_
#define MIMIR_SEARCH_BUILDER_HPP_

#include <flatmemory/flatmemory.hpp>

namespace mimir
{
/**
 * Interface class
 */
template<typename Derived>
class IBuilder
{
private:
    IBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] auto& get_flatmemory_builder() { return self().get_flatmemory_builder_impl(); }
    [[nodiscard]] const auto& get_flatmemory_builder() const { return self().get_flatmemory_builder_impl(); }
};

/**
 * Implementation class
 */
template<typename Tag>
class Builder : public IBuilder<Builder<Tag>>
{
};
}

#endif
