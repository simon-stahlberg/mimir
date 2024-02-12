#ifndef MIMIR_SEARCH_STATES_BITSET_INTERFACE_HPP_
#define MIMIR_SEARCH_STATES_BITSET_INTERFACE_HPP_

#include "../interface.hpp"
#include "../../types.hpp"

#include <flatmemory/flatmemory.hpp>


namespace mimir
{

/**
 * Derived ID class.
 *
 * Define name and template parameters of your own implementation.
*/
class BitsetStateTag : public StateTag {};


/**
 * Interface class
*/
template<typename Derived>
class IBitsetStateBuilder
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    IBitsetStateBuilder() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] Bitset& get_atoms_bitset() { return self().get_atoms_bitset_impl(); }
};


template<typename Derived>
class IBitsetStateView
{
private:
    using P = typename TypeTraits<Derived>::PlanningModeTag;

    IBitsetStateView() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] ConstBitsetView get_atoms_bitset() const { return self().get_atoms_bitset_impl(); }
};



/**
 * Type traits.
*/
template<IsPlanningModeTag P>
struct TypeTraits<Builder<StateDispatcher<BitsetStateTag, P>>>
{
    using PlanningModeTag = P;
};

template<IsPlanningModeTag P>
struct TypeTraits<View<StateDispatcher<BitsetStateTag, P>>>
{
    using PlanningModeTag = P;
};


} 

#endif 