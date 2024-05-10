#ifndef MIMIR_SEARCH_AXIOM_EVALUATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/axiom_evaluators/tags.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/types.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IDynamicAE
{
public:
    virtual ~IDynamicAE() = default;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(FlatBitsetBuilder& ref_ground_atoms) = 0;
};

/**
 * Static interface class.
 */
template<typename Derived>
class IStaticAE : public IDynamicAE
{
private:
    IStaticAE() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void generate_and_apply_axioms(FlatBitsetBuilder& ref_ground_atoms) override
    {  //
        self().generate_and_apply_axioms_impl(ref_ground_atoms);
    }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsAEDispatcher A>
class AE : public IStaticAE<AE<A>>
{
};

}

#endif
