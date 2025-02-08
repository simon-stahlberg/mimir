/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_SEARCH_MATCH_TREE2_HPP_
#define MIMIR_SEARCH_MATCH_TREE2_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/dense_state.hpp"

#include <concepts>
#include <cstdint>
#include <queue>
#include <vector>

namespace mimir::match_tree
{

/**
 * Types
 */

using Split = std::variant<GroundAtom<Fluent>, GroundAtom<Derived>, GroundNumericConstraint>;
using SplitList = std::vector<Split>;

/**
 * Base classes
 */

/// MatchTree can handle elements that have a conjunctive condition, e.g., GroundActionImpl and GroundAxiomImpl.
template<typename T>
concept HasConjunctiveCondition = requires(const T a) {
    { a.get_conjunctive_condition() } -> std::same_as<const GroundConjunctiveCondition&>;
};

/// @brief `Node` implements the interface of nodes.
template<HasConjunctiveCondition Element>
class INode
{
public:
    virtual ~INode() = default;

    virtual void generate_applicable_actions(const DenseState& state,
                                             std::vector<const INode<Element>*>& ref_applicable_nodes,
                                             std::vector<const Element*>& ref_applicable_elements) const = 0;
};

template<HasConjunctiveCondition Element>
using Node = std::unique_ptr<INode<Element>>;

/* Customization point 1: NodeScoringFunction*/

template<HasConjunctiveCondition Element>
struct QueueEntry
{
    double score;
    const INode<Element>* node;
    SplitList splits;
};

template<HasConjunctiveCondition Element>
struct QueueEntryComparator
{
    bool operator()(const QueueEntry<Element>& lhs, const QueueEntry<Element>& rhs) const { return lhs.score > rhs.score; }
};

/// @brief `INodeScoringFunction` allows computing a score for a node.
template<HasConjunctiveCondition Element>
class IQueueEntryScoringFunction
{
public:
    virtual ~IQueueEntryScoringFunction() = default;

    virtual double compute_score(const QueueEntry<Element>& entry, size_t distance_to_root) = 0;
};

template<HasConjunctiveCondition Element>
using QueueEntryScoringFunction = std::unique_ptr<IQueueEntryScoringFunction<Element>>;

/* Customization point 2: SplitScoringFunction */

/// @brief `ISplitScoringFunction` computes the best split for a given set of elements.
/// @tparam Element
template<HasConjunctiveCondition Element>
class ISplitScoringFunction
{
public:
    virtual ~ISplitScoringFunction() = default;

    virtual Split compute_best_split(const SplitList& past_splits, std::span<const Element*> elements) = 0;
};

template<HasConjunctiveCondition Element>
using SplitScoringFunction = std::unique_ptr<ISplitScoringFunction<Element>>;

/**
 * Derived classes
 */

template<HasConjunctiveCondition Element>
class MinDepthQueueEntryScoringFunction : public IQueueEntryScoringFunction<Element>
{
public:
    double compute_score(const QueueEntry<Element>& entry, size_t distance_to_root) override { return static_cast<double>(distance_to_root); }
};

template<HasConjunctiveCondition Element>
class FrequencySplitScoringFunction : public ISplitScoringFunction<Element>
{
private:
    const PDDLRepositories& m_pddl_repositories;

public:
    explicit FrequencySplitScoringFunction(const PDDLRepositories& pddl_repositories) : m_pddl_repositories(pddl_repositories) {}

    Split compute_best_split(const SplitList& past_splits, std::span<const Element*> elements) override
    {
        std::cout << "compute_best_split" << std::endl;

        /* Collect frequencies */
        auto fluent_atom_frequencies = std::unordered_map<GroundAtom<Fluent>, size_t> {};
        auto derived_atom_frequencies = std::unordered_map<GroundAtom<Derived>, size_t> {};
        auto numeric_constraint_frequencies = std::unordered_map<GroundNumericConstraint, size_t> {};

        for (const auto& element : elements)
        {
            const auto& conjunctive_condition = element->get_conjunctive_condition();

            /* Fluent */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            /* Derived */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            /* Numeric constraint */
            for (const auto& numeric_constraint : conjunctive_condition.get_numeric_constraints())
            {
                ++numeric_constraint_frequencies[numeric_constraint.get()];
            }
        }

        std::cout << "fluent_atom_frequencies: " << fluent_atom_frequencies << std::endl;
        std::cout << "derived_atom_frequencies: " << derived_atom_frequencies << std::endl;
        std::cout << "numeric_constraint_frequencies: " << numeric_constraint_frequencies << std::endl;

        /* Erase previous splits */
        for (const auto& split : past_splits)
        {
            std::visit(
                [&](auto&& arg) -> void
                {
                    using T = std::decay_t<decltype(arg)>;
                    if constexpr (std::is_same_v<T, GroundAtom<Fluent>>)
                    {
                        fluent_atom_frequencies.erase(arg);
                    }
                    else if constexpr (std::is_same_v<T, GroundAtom<Derived>>)
                    {
                        derived_atom_frequencies.erase(arg);
                    }
                    else if constexpr (std::is_same_v<T, GroundNumericConstraint>)
                    {
                        numeric_constraint_frequencies.erase(arg);
                    }
                    else
                    {
                        static_assert(dependent_false<T>::value,
                                      "FrequencySplitScoringFunction::compute_best_split(past_splits, elements): Missing implementation for Split type.");
                    }
                },
                split);
        }

        /* Compute highest frequency */
        auto highest_frequency = size_t(0);
        for (const auto& [atom, frequency] : fluent_atom_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }
        for (const auto& [atom, frequency] : derived_atom_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }
        for (const auto& [constraint, frequency] : numeric_constraint_frequencies)
        {
            highest_frequency = std::max(highest_frequency, frequency);
        }

        std::cout << "Highest frequency: " << highest_frequency << std::endl;

        /* Collect splits with highest frequency */
        auto highest_frequency_splits = SplitList {};
        for (const auto& [atom, frequency] : fluent_atom_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(atom);
        }
        for (const auto& [atom, frequency] : derived_atom_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(atom);
        }
        for (const auto& [constraint, frequency] : numeric_constraint_frequencies)
        {
            if (frequency == highest_frequency)
                highest_frequency_splits.push_back(constraint);
        }

        std::cout << "Highest frequency splits: " << highest_frequency_splits << std::endl;

        /* Break ties in favor of lexicographically smallest */
        // Note: in case we do not get deterministic results, we should work on normal form of function expressions in the translator.
        std::sort(highest_frequency_splits.begin(),
                  highest_frequency_splits.end(),
                  [](auto&& lhs, auto&& rhs)
                  {
                      std::stringstream ss_lhs;
                      ss_lhs << lhs;
                      std::stringstream ss_rhs;
                      ss_rhs << rhs;
                      return ss_lhs.str() < ss_rhs.str();
                  });

        std::cout << "Final split: " << highest_frequency_splits.front() << std::endl;

        return highest_frequency_splits.front();
    }
};

/* Nodes */
template<HasConjunctiveCondition Element, DynamicPredicateTag P>
class AtomSelectorNode : public INode<Element>
{
private:
    GroundAtom<P> m_atom;

    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    explicit AtomSelectorNode(GroundAtom<P> atom) : m_atom(atom) {}

    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        (state.get_atoms<P>().get(m_atom->get_index())) ? ref_applicable_nodes.push_back(m_true_succ) : ref_applicable_nodes.push_back(m_false_succ);
        ref_applicable_nodes.push_back(m_dontcare_succ);
    }
};

template<HasConjunctiveCondition Element>
class NumericConstraintSelectorNode : public INode<Element>
{
private:
    GroundNumericConstraint m_constraint;

    Node<Element> m_true_succ;
    Node<Element> m_false_succ;
    Node<Element> m_dontcare_succ;

public:
    void
    generate_applicable_actions(const DenseState& state, std::vector<const INode<Element>*>& ref_applicable_nodes, std::vector<const Element*>&) const override
    {
        evaluate(m_constraint, state.get_numeric_variables()) ? ref_applicable_nodes.push_back(m_true_succ) : ref_applicable_nodes.push_back(m_false_succ);
        ref_applicable_nodes.push_back(m_dontcare_succ);
    }
};

template<HasConjunctiveCondition Element>
class ElementGeneratorNode : public INode<Element>
{
private:
    std::span<Element> m_elements;

public:
    void generate_applicable_actions(const DenseState& state,
                                     std::vector<const INode<Element>*>&,
                                     std::vector<const Element*>& ref_applicable_elements) const override
    {
        ref_applicable_elements.insert(ref_applicable_elements.end(), m_elements.begin(), m_elements.end());
    }
};

/* MatchTree */
template<HasConjunctiveCondition Element>
class MatchTree
{
private:
    std::vector<const Element*> m_elements;  ///< ATTENTION: must remain persistent. Swapping elements is allowed.
    Node<Element> m_root;                    ///< the root node.

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    using QueueType = std::priority_queue<QueueEntry<Element>, std::vector<QueueEntry<Element>>, QueueEntryComparator<Element>>;

    std::tuple<Node<Element>> instantiate_node_and_update_queue(const Split& split, std::span<const Element*> elements, QueueType& queue)
    {
        // TODO split the elements. First find the amount of true, false, and dontcare together with their index,
        // Then swap them in elements such that true are in the front, then false, then dont cares.
    }

    void build_iteratively(const QueueEntryScoringFunction<Element>& queue_scoring_function, const SplitScoringFunction<Element>& split_scoring_function)
    {
        auto queue = QueueType {};

        auto root_split = split_scoring_function->compute_best_split(SplitList {}, std::span<const Element*>(m_elements.begin(), m_elements.end()));

        std::cout << "root_split: " << root_split << std::endl;

        exit(1);
    }

public:
    MatchTree(std::vector<const Element*> elements,  //
              const QueueEntryScoringFunction<Element>& queue_scoring_function,
              const SplitScoringFunction<Element>& split_scoring_function) :
        m_elements(std::move(elements))
    {
        build_iteratively(queue_scoring_function, split_scoring_function);
    }
    // Uncopieable and unmoveable to prohibit invalidating spans on m_elements.
    MatchTree(const MatchTree& other) = delete;
    MatchTree& operator=(const MatchTree& other) = delete;
    MatchTree(MatchTree&& other) = delete;
    MatchTree& operator=(MatchTree&& other) = delete;

    void generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements)
    {
        m_evaluate_stack.clear();
        out_applicable_elements.clear();

        m_evaluate_stack.push_back(m_root.get());

        while (!m_evaluate_stack.empty())
        {
            const auto node = m_evaluate_stack.back();
            m_evaluate_stack.pop_back();

            node->generate_applicable_actions(state, m_evaluate_stack, out_applicable_elements);
        }
    }
};

}

#endif
