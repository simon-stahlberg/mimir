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

#include "mimir/search/match_tree/statistics.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"

namespace mimir::match_tree
{

/**
 * Parsing
 */

template<HasConjunctiveCondition Element>
struct ParseStatisticsNodeVisitor : public INodeVisitor<Element>
{
    Statistics& statistics;

    explicit ParseStatisticsNodeVisitor(Statistics& statistics) : statistics(statistics) {}

    void accept(const AtomSelectorNode_TFX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Fluent>& atom) override;
    void accept(const AtomSelectorNode_TFX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TF<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_TX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_FX<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_T<Element, Derived>& atom) override;
    void accept(const AtomSelectorNode_F<Element, Derived>& atom) override;
    void accept(const NumericConstraintSelectorNode_T<Element>& constraint) override;
    void accept(const NumericConstraintSelectorNode_TX<Element>& constraint) override;
    void accept(const ElementGeneratorNode_Perfect<Element>& generator) override;
    void accept(const ElementGeneratorNode_Imperfect<Element>& generator) override;
};

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_TFX<Element, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_TF<Element, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_TX<Element, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_FX<Element, P>& atom)
{
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_T<Element, P>& atom)
{
    atom.get_true_child()->visit(visitor);
}

template<HasConjunctiveCondition Element, DynamicPredicateTag P>
static void accept_impl(ParseStatisticsNodeVisitor<Element>& visitor, const AtomSelectorNode_F<Element, P>& atom)
{
    atom.get_false_child()->visit(visitor);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_T<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_F<Element, Fluent>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TFX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TF<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_TX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_FX<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_T<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const AtomSelectorNode_F<Element, Derived>& atom)
{
    accept_impl(*this, atom);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const NumericConstraintSelectorNode_T<Element>& constraint)
{
    constraint.get_true_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const NumericConstraintSelectorNode_TX<Element>& constraint)
{
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const ElementGeneratorNode_Perfect<Element>& generator)
{
    statistics.generator_distribution.push_back(generator.get_elements().size());
    statistics.perfect_generator_distribution.push_back(generator.get_elements().size());
}

template<HasConjunctiveCondition Element>
void ParseStatisticsNodeVisitor<Element>::accept(const ElementGeneratorNode_Imperfect<Element>& generator)
{
    statistics.generator_distribution.push_back(generator.get_elements().size());
    statistics.imperfect_generator_distribution.push_back(generator.get_elements().size());
}

template<HasConjunctiveCondition Element>
void parse_generator_distribution_iteratively(const Node<Element>& root, Statistics& ref_statistics)
{
    auto visitor = ParseStatisticsNodeVisitor<Element>(ref_statistics);
    root->visit(visitor);
}

template void parse_generator_distribution_iteratively(const Node<GroundActionImpl>& root, Statistics& ref_statistics);
template void parse_generator_distribution_iteratively(const Node<GroundAxiomImpl>& root, Statistics& ref_statistics);

/**
 * Printing
 */

std::ostream& operator<<(std::ostream& os, const Statistics& statistics)
{
    const auto num_elements = std::accumulate(statistics.generator_distribution.begin(), statistics.generator_distribution.end(), 0);
    const auto num_generators = statistics.generator_distribution.size();
    assert(num_generators > 0);
    const auto num_elements_per_generator = static_cast<double>(num_elements) / num_generators;

    const auto num_perfect_elements = std::accumulate(statistics.perfect_generator_distribution.begin(), statistics.perfect_generator_distribution.end(), 0);
    const auto num_perfect_generators = statistics.perfect_generator_distribution.size();
    const auto num_elements_per_perfect_generator =
        (num_perfect_generators > 0) ? static_cast<double>(num_perfect_elements) / num_perfect_generators : std::numeric_limits<double>::quiet_NaN();

    const auto num_imperfect_elements =
        std::accumulate(statistics.imperfect_generator_distribution.begin(), statistics.imperfect_generator_distribution.end(), 0);
    const auto num_imperfect_generators = statistics.imperfect_generator_distribution.size();
    const auto num_elements_per_imperfect_generator =
        (num_imperfect_generators > 0) ? static_cast<double>(num_imperfect_elements) / num_imperfect_generators : std::numeric_limits<double>::quiet_NaN();

    os << "[MatchTreeStatistics] Total time for construction: "
       << std::chrono::duration_cast<std::chrono::milliseconds>(statistics.construction_end_time_point - statistics.construction_start_time_point) << "\n"
       << "[MatchTreeStatistics] Total number of nodes: " << statistics.num_nodes << "\n"
       << "[MatchTreeStatistics] Total number of elements: " << num_elements << "\n"
       << "[MatchTreeStatistics] Perfect: " << ((statistics.is_perfect) ? "Yes" : "No") << "\n"
       << "[MatchTreeStatistics] Generators - total number: " << num_generators << "\n"
       << "[MatchTreeStatistics] Generators - average number of elements: " << num_elements_per_generator << "\n"
       << "[MatchTreeStatistics] Generators - distribution gini score: " << compute_gini_score(statistics.generator_distribution) << "\n"
       << "[MatchTreeStatistics] Perfect generators - total number: " << num_perfect_generators << "\n"
       << "[MatchTreeStatistics] Perfect generators - average number of elements: " << num_elements_per_perfect_generator << "\n"
       << "[MatchTreeStatistics] Perfect generators - distribution gini score: " << compute_gini_score(statistics.perfect_generator_distribution) << "\n"
       << "[MatchTreeStatistics] Imperfect generators - total number: " << num_imperfect_generators << "\n"
       << "[MatchTreeStatistics] Imperfect generators - average number of elements: " << num_elements_per_imperfect_generator << "\n"
       << "[MatchTreeStatistics] Imperfect generators - distribution gini score: " << compute_gini_score(statistics.imperfect_generator_distribution);

    return os;
}
}
