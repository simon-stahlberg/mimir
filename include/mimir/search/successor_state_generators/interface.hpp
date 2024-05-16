#ifndef MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_SUCCESSOR_STATE_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/successor_state_generators/tags.hpp"

namespace mimir
{
/**
 * Dynamic interface class
 */
class IDynamicSSG
{
public:
    virtual ~IDynamicSSG() = default;

    [[nodiscard]] virtual State get_or_create_initial_state() = 0;

    /// @brief Expert interface for creating states.
    /// The user must ensure that the atoms are part of the problem from the applicable action generator.
    [[nodiscard]] virtual State get_or_create_state(const GroundAtomList& atoms) = 0;

    [[nodiscard]] virtual State get_or_create_successor_state(const State state, const GroundAction action) = 0;

    [[nodiscard]] virtual State get_non_extended_state(const State state) = 0;

    [[nodiscard]] virtual size_t get_state_count() const = 0;
};

/**
 * Static interface class
 */
template<typename Derived>
class IStaticSSG : public IDynamicSSG
{
private:
    IStaticSSG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    [[nodiscard]] State get_or_create_initial_state() override
    {  //
        return self().get_or_create_initial_state_impl();
    }

    [[nodiscard]] State get_or_create_state(const GroundAtomList& atoms) override
    {  //
        return self().get_or_create_state_impl(atoms);
    }

    [[nodiscard]] State get_or_create_successor_state(const State state, const GroundAction action) override
    {  //
        return self().get_or_create_successor_state_impl(state, action);
    }

    [[nodiscard]] State get_non_extended_state(const State state) override
    {  //
        return self().get_non_extended_state_impl(state);
    }

    [[nodiscard]] size_t get_state_count() const override
    {  //
        return self().get_state_count_impl();
    }
};

/**
 * General implementation class.
 *
 * Specialize the wrapped tag to provide your own implementation of a successor state generator.
 */
template<IsSSGDispatcher S>
class SSG : public IStaticSSG<SSG<S>>
{
};

}

#endif
