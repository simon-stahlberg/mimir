#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"

namespace mimir
{
/**
 * Implementation of https://arxiv.org/pdf/1109.6051.pdf section 5
 *
 * Dominik (2024-04-20): I tried an implementation with std::variant
 * which has similar performance so I switched the to implementation
 * using virtual, which is simpler.
 * It seems that whenever runtime polymorphism is needed virtual is the more elegant choice.
 */
class MatchTree
{
private:
    class INode
    {
    public:
        virtual ~INode() = default;
        virtual void get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions) = 0;
    };

    class SelectorNode : public INode
    {
    private:
        size_t m_atom_id;
        std::unique_ptr<INode> m_true_succ;
        std::unique_ptr<INode> m_false_succ;
        std::unique_ptr<INode> m_dontcare_succ;

    public:
        SelectorNode(size_t ground_atom_id, std::unique_ptr<INode>&& true_succ, std::unique_ptr<INode>&& false_succ, std::unique_ptr<INode>&& dontcare_succ);

        void get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions) override;
    };

    class GeneratorNode : public INode
    {
    private:
        std::vector<ConstDenseActionViewProxy> m_actions;

    public:
        explicit GeneratorNode(std::vector<ConstDenseActionViewProxy> actions);

        void get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions) override;
    };

    size_t m_num_nodes;
    std::unique_ptr<INode> m_root_node;

    std::unique_ptr<INode> build_recursively(const size_t atom_id, size_t const num_atoms, const std::vector<ConstDenseActionViewProxy>& actions);

public:
    MatchTree();
    MatchTree(const size_t num_atoms, const std::vector<ConstDenseActionViewProxy>& actions);

    void get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions);

    [[nodiscard]] size_t get_num_nodes() const;
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

    AAG<LiftedAAGDispatcher<DenseStateTag>> m_lifted_aag;

    Builder<StateDispatcher<DenseStateTag>> m_state_builder;

    MatchTree m_match_tree;

    /* Implement IStaticAAG interface */
    friend class IStaticAAG<AAG<GroundedAAGDispatcher<DenseStateTag>>>;

    void generate_applicable_actions_impl(const ConstStateView state, std::vector<ConstActionView>& out_applicable_actions);

public:
    AAG(Problem problem, PDDLFactories& pddl_factories);

    /// @brief Return the action with the given id.
    [[nodiscard]] ConstActionView get_action(size_t action_id) const;
};
}

#endif