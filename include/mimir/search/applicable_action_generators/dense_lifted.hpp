#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/actions/dense.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/consistency_graph.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/states.hpp"

#include <flatmemory/details/view_const.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{

using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<LiftedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>
{
private:
    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    flat::DenseActionSet m_actions;
    std::vector<DenseAction> m_actions_by_index;
    Builder<ActionDispatcher<DenseStateTag>> m_action_builder;

    GroundFunctionToValue m_ground_function_value_costs;

    std::unordered_map<Action, consistency_graph::Graphs> m_static_consistency_graphs;

    GroundLiteral ground_literal(const Literal& literal, const ObjectList& binding) const;

    /**
     * Precondition
     */

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_preconditions_hold(const Action& action, DenseState state) const;

    void nullary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions);

    void unary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions);

    void general_case(const AssignmentSet& assignment_sets, const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions);

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const DenseState state, std::vector<DenseAction>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Ground an action and return a view onto it.
    DenseAction ground_action(const Action& action, ObjectList&& binding);

    /// @brief Return all actions.
    [[nodiscard]] const flat::DenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] DenseAction get_action(size_t action_id) const;
};

}  // namespace mimir

#endif