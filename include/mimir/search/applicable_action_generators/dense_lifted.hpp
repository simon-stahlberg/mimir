#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/search/actions/dense.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/internal_representation.hpp"
#include "mimir/search/states.hpp"

#include <flatmemory/details/view_const.hpp>
#include <unordered_map>
#include <vector>

namespace mimir
{

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<LiftedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<DenseStateTag>>;
    using ConstActionView = ConstView<ActionDispatcher<DenseStateTag>>;

    Problem m_problem;

    DenseActionSet m_actions;
    std::vector<ConstActionView> m_actions_by_index;
    Builder<ActionDispatcher<DenseStateTag>> m_action_builder;

    // TODO: If we separate Function from GroundFunction, then we can turn this into a vector
    std::map<GroundFunction, double> m_initial_ground_function_values;

    PDDLFactories& m_pddl_factories;
    std::vector<FlatAction> m_flat_actions;
    std::unordered_map<Action, std::vector<std::vector<size_t>>> m_partitions;
    std::unordered_map<Action, std::vector<Assignment>> m_to_vertex_assignment;
    std::unordered_map<Action, std::vector<AssignmentPair>> m_statically_consistent_assignments;

    void ground_variables(const std::vector<ParameterIndexOrConstantId>& variables, const ObjectList& binding, ObjectList& out_terms) const;

    GroundLiteral ground_literal(const FlatLiteral& literal, const ObjectList& binding) const;

    ConstActionView ground_action(const FlatAction& flat_action, ObjectList&& binding);

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_preconditions_hold(const FlatAction& flat_action, ConstStateView state) const;

    void nullary_case(const FlatAction& flat_action, ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

    void unary_case(const FlatAction& flat_action, ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

    void general_case(const std::vector<std::vector<bool>>& assignment_sets,
                      const FlatAction& flat_action,
                      ConstStateView state,
                      std::vector<ConstActionView>& out_applicable_actions);

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return all actions.
    [[nodiscard]] const DenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] ConstActionView get_action(size_t action_id) const;
};

}  // namespace mimir

#endif