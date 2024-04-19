#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"

namespace mimir
{

/**
 * We use the same order of propositions as the order of ground atoms in the factory
 * to forward iterate over the ground atoms that are true in a given state.
 * Similarly, we create the MatchTree top down to forward traverse the nodes vector as well.
 */
class MatchTree
{
private:
    using NodeID = size_t;

    struct GroundAtomNode
    {
        size_t m_ground_atom_id;
        NodeID m_true_successor;
        NodeID m_false_successor;
    };

    struct SingleLeafNode
    {
        ConstDenseActionViewProxy action;
    };

    struct MultiLeafNode
    {
        std::vector<ConstDenseActionViewProxy> actions;
    };

    using Node = std::variant<GroundAtomNode, SingleLeafNode, MultiLeafNode>;

    std::vector<Node> m_nodes;

    void get_applicable_actions(const ConstDenseStateViewProxy& state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions);
};

/**
 * Fully specialized implementation class.
 */
template<>
class AAG<GroundedAAGDispatcher<DenseStateTag>> : public IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>
{
private:
    using ConstStateView = ConstView<StateDispatcher<DenseStateTag>>;
    using ConstActionView = ConstView<ActionDispatcher<DenseStateTag>>;

    Problem m_problem;
    PDDLFactories& m_pddl_factories;

    Builder<StateDispatcher<DenseStateTag>> m_state_builder;

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions) {}

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return the action with the given id.
    [[nodiscard]] ConstActionView get_action(size_t action_id) const { throw std::runtime_error("not implemented"); }
};
}

#endif