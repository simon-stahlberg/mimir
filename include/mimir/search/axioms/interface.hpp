#ifndef MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOMS_INTERFACE_HPP_

#include "mimir/search/axioms/tags.hpp"
#include "mimir/search/flat_types.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Interface class
 */
template<typename Derived>
class IAxiomBuilder
{
private:
    IAxiomBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] uint32_t& get_id() { return self().get_id_impl(); }
    [[nodiscard]] Axiom& get_axiom() { return self().get_axiom_impl(); }
    [[nodiscard]] FlatObjectListBuilder& get_objects() { return self().get_objects_impl(); }
};

/**
 * Interface class
 */
template<typename Derived>
class IAxiomView
{
private:
    IAxiomView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] uint32_t get_id() const { return self().get_id_impl(); }
    [[nodiscard]] Axiom get_axiom() const { return self().get_axiom_impl(); }
    [[nodiscard]] FlatObjectList get_objects() const { return self().get_objects_impl(); }
};

}

#endif