#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_INTERFACE_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_INTERFACE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/actions.hpp"
#include "mimir/search/axioms.hpp"
#include "mimir/search/states.hpp"

#include <chrono>

namespace mimir
{
template<typename T>
class MatchTree;

/**
 * Interface class
 */
class IGroundedAAGEventHandler
{
public:
    virtual ~IGroundedAAGEventHandler() = default;

    /// @brief React on finishing delete-free exploration
    virtual void on_finish_delete_free_exploration(const GroundAtomList& reached_atoms,
                                                   const GroundActionList& instantiated_actions,
                                                   const GroundAxiomList& instantiated_axioms) = 0;

    virtual void on_finish_grounding_unrelaxed_actions(const GroundActionList& unrelaxed_actions) = 0;

    virtual void on_finish_build_action_match_tree(const MatchTree<GroundAction>& action_match_tree) = 0;

    virtual void on_finish_grounding_unrelaxed_axioms(const GroundAxiomList& unrelaxed_axioms) = 0;

    virtual void on_finish_build_axiom_match_tree(const MatchTree<GroundAxiom>& axiom_match_tree) = 0;
};

/**
 * Base class
 *
 * Collect statistics and call implementation of derived class.
 */
template<typename Derived>
class GroundedAAGEventHandlerBase : public IGroundedAAGEventHandler
{
protected:
private:
    GroundedAAGEventHandlerBase() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    void on_finish_delete_free_exploration(const GroundAtomList& reached_atoms,
                                           const GroundActionList& instantiated_actions,
                                           const GroundAxiomList& instantiated_axioms) override
    {  //
        self().on_finish_delete_free_exploration_impl(reached_atoms, instantiated_actions, instantiated_axioms);
    }

    void on_finish_grounding_unrelaxed_actions(const GroundActionList& unrelaxed_actions) override
    {  //
        self().on_finish_grounding_unrelaxed_actions_impl(unrelaxed_actions);
    }

    void on_finish_build_action_match_tree(const MatchTree<GroundAction>& action_match_tree) override
    {  //
        self().on_finish_build_action_match_tree_impl(action_match_tree);
    }

    void on_finish_grounding_unrelaxed_axioms(const GroundAxiomList& unrelaxed_axioms) override
    {  //
        self().on_finish_grounding_unrelaxed_axioms_impl(unrelaxed_axioms);
    }

    void on_finish_build_axiom_match_tree(const MatchTree<GroundAxiom>& axiom_match_tree) override
    {  //
        self().on_finish_build_axiom_match_tree_impl(axiom_match_tree);
    }
};
}

#endif