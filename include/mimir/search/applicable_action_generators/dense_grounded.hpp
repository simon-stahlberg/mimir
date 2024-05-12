#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/dense_grounded/match_tree.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"

#include <variant>

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<GroundedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    LiftedDenseAAG m_lifted_aag;

    MatchTree<DenseAction> m_action_match_tree;
    MatchTree<DenseAxiom> m_axiom_match_tree;

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const DenseState state, DenseActionList& out_applicable_actions);

    void generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms, FlatBitsetBuilder& ref_derived_atoms);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return all actions.
    [[nodiscard]] const FlatDenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] DenseAction get_action(size_t action_id) const;
};

/**
 * Types
 */

using GroundedDenseAAG = AAG<GroundedAAGDispatcher<DenseStateTag>>;

}

#endif