#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/applicable_action_generators/tags.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/states.hpp"

namespace mimir
{

/**
 * Dynamic interface class.
 */
class IDynamicAAG
{
public:
    virtual ~IDynamicAAG() = default;

    /// @brief Generate all applicable actions for a given state.
    virtual void generate_applicable_actions(const State state, GroundActionList& out_applicable_actions) = 0;

    /// @brief Generate all applicable axioms for a given set of ground atoms by running fixed point computation.
    virtual void generate_and_apply_axioms(FlatBitsetBuilder& ref_state_atoms) = 0;

    /// @brief Return the action with the given id.
    [[nodiscard]] virtual GroundAction get_action(size_t action_id) const = 0;

    /* Getters */
    [[nodiscard]] virtual Problem get_problem() const = 0;
    [[nodiscard]] virtual PDDLFactories& get_pddl_factories() = 0;
    [[nodiscard]] virtual const PDDLFactories& get_pddl_factories() const = 0;
};

/**
 * Static interface class.
 */
template<typename Derived>
class IStaticAAG : public IDynamicAAG
{
private:
    IStaticAAG() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void generate_applicable_actions(const State state, GroundActionList& out_applicable_actions) override
    {
        self().generate_applicable_actions_impl(state, out_applicable_actions);
    }

    void generate_and_apply_axioms(FlatBitsetBuilder& ref_state_atoms) override
    {  //
        self().generate_and_apply_axioms_impl(ref_state_atoms);
    }

    [[nodiscard]] Problem get_problem() const override
    {
        //
        return self().get_problem_impl();
    }

    [[nodiscard]] PDDLFactories& get_pddl_factories() override
    {  //
        return self().get_pddl_factories_impl();
    }

    [[nodiscard]] const PDDLFactories& get_pddl_factories() const override
    {  //
        return self().get_pddl_factories_impl();
    }
};

/**
 * General implementation class.
 *
 * Specialize it with your dispatcher.
 */
template<IsAAGDispatcher A>
class AAG : public IStaticAAG<AAG<A>>
{
};

}

#endif
