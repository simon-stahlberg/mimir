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

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

/**
 * Parsing
 */

template<formalism::HasConjunctiveCondition E>
struct ParseStatisticsNodeVisitor : public INodeVisitor<E>
{
    Statistics& statistics;

    explicit ParseStatisticsNodeVisitor(Statistics& statistics) : statistics(statistics) {}

    void accept(const AtomSelectorNode_TFX<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_TF<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_TX<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_FX<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_T<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_F<E, FluentTag>& atom) override;
    void accept(const AtomSelectorNode_TFX<E, DerivedTag>& atom) override;
    void accept(const AtomSelectorNode_TF<E, DerivedTag>& atom) override;
    void accept(const AtomSelectorNode_TX<E, DerivedTag>& atom) override;
    void accept(const AtomSelectorNode_FX<E, DerivedTag>& atom) override;
    void accept(const AtomSelectorNode_T<E, DerivedTag>& atom) override;
    void accept(const AtomSelectorNode_F<E, DerivedTag>& atom) override;
    void accept(const NumericConstraintSelectorNode_T<E>& constraint) override;
    void accept(const NumericConstraintSelectorNode_TX<E>& constraint) override;
    void accept(const ElementGeneratorNode_Perfect<E>& generator) override;
    void accept(const ElementGeneratorNode_Imperfect<E>& generator) override;
};

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_TFX<E, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_TF<E, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_TX<E, P>& atom)
{
    atom.get_true_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_FX<E, P>& atom)
{
    atom.get_false_child()->visit(visitor);
    atom.get_dontcare_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_T<E, P>& atom)
{
    atom.get_true_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E, IsFluentOrDerivedTag P>
static void accept_impl(ParseStatisticsNodeVisitor<E>& visitor, const AtomSelectorNode_F<E, P>& atom)
{
    atom.get_false_child()->visit(visitor);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TFX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TF<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_FX<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_T<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_F<E, FluentTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TFX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TF<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_TX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_FX<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_T<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const AtomSelectorNode_F<E, DerivedTag>& atom)
{
    accept_impl(*this, atom);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const NumericConstraintSelectorNode_T<E>& constraint)
{
    constraint.get_true_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const NumericConstraintSelectorNode_TX<E>& constraint)
{
    constraint.get_true_child()->visit(*this);
    constraint.get_dontcare_child()->visit(*this);
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const ElementGeneratorNode_Perfect<E>& generator)
{
    statistics.generator_distribution.push_back(generator.get_elements().size());
    statistics.perfect_generator_distribution.push_back(generator.get_elements().size());
}

template<formalism::HasConjunctiveCondition E>
void ParseStatisticsNodeVisitor<E>::accept(const ElementGeneratorNode_Imperfect<E>& generator)
{
    statistics.generator_distribution.push_back(generator.get_elements().size());
    statistics.imperfect_generator_distribution.push_back(generator.get_elements().size());
}

template<formalism::HasConjunctiveCondition E>
void parse_generator_distribution_iteratively(const Node<E>& root, Statistics& ref_statistics)
{
    auto visitor = ParseStatisticsNodeVisitor<E>(ref_statistics);
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
    const auto num_elements_per_generator =
        (num_generators > 0) ? static_cast<double>(num_elements) / num_generators : std::numeric_limits<double>::quiet_NaN();

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
       << std::chrono::duration_cast<std::chrono::milliseconds>(statistics.construction_end_time_point - statistics.construction_start_time_point).count()
       << " ms" << "\n"
       << "[MatchTreeStatistics] Total number of nodes: " << statistics.num_nodes << "\n"
       << "[MatchTreeStatistics] Total number of elements: " << num_elements << "\n"
       << "[MatchTreeStatistics] Generators - total number: " << num_generators << "\n"
       << "[MatchTreeStatistics] Generators - average number of elements: " << num_elements_per_generator << "\n"
       << "[MatchTreeStatistics] Perfect generators - total number: " << num_perfect_generators << "\n"
       << "[MatchTreeStatistics] Perfect generators - average number of elements: " << num_elements_per_perfect_generator << "\n"
       << "[MatchTreeStatistics] Imperfect generators - total number: " << num_imperfect_generators << "\n"
       << "[MatchTreeStatistics] Imperfect generators - average number of elements: " << num_elements_per_imperfect_generator;

    return os;
}
}
