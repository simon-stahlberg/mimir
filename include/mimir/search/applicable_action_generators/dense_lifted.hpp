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

using GroundFunctionToValue = std::unordered_map<GroundFunction, double>;

/// @brief The StaticConsistencyGraph encodes the part of the consisteny graph
///        that is static for each state.
class StaticConsistencyGraph
{
private:
    Problem m_problem;

    /// @brief The ActionExtension provides additional accessors to action information
    class ActionExtension
    {
    private:
    public:
        // Define getters here
    };

    class Vertex
    {
    private:
        size_t param_index;
        size_t object_index;

    public:
        Vertex(size_t param_index, size_t object_index);

        size_t get_param_index() const;
        size_t get_object_index() const;
    };

    struct Edge
    {
    };

    std::unordered_map<Action, ActionExtension> action_extensions;

public:
    explicit StaticConsistencyGraph(Problem problem);
};

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
    PDDLFactories& m_pddl_factories;

    DenseActionSet m_actions;
    std::vector<ConstActionView> m_actions_by_index;
    Builder<ActionDispatcher<DenseStateTag>> m_action_builder;

    GroundFunctionToValue m_ground_function_value_costs;

    std::unordered_map<Action, std::vector<std::vector<size_t>>> m_partitions;
    std::unordered_map<Action, std::vector<Assignment>> m_to_vertex_assignment;
    std::unordered_map<Action, std::vector<AssignmentPair>> m_statically_consistent_assignments;

    GroundLiteral ground_literal(const Literal& literal, const ObjectList& binding) const;

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_preconditions_hold(const Action& action, ConstStateView state) const;

    void nullary_case(const Action& action, ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

    void unary_case(const Action& action, ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

    void general_case(const std::vector<std::vector<bool>>& assignment_sets,
                      const Action& action,
                      ConstStateView state,
                      std::vector<ConstActionView>& out_applicable_actions);

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<LiftedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Ground an action and return a view onto it.
    ConstActionView ground_action(const Action& action, ObjectList&& binding);

    /// @brief Return all actions.
    [[nodiscard]] const DenseActionSet& get_actions() const;

    /// @brief Return the action with the given id.
    [[nodiscard]] ConstActionView get_action(size_t action_id) const;
};

}  // namespace mimir

#endif