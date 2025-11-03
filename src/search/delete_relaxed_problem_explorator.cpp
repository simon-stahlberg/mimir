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

#include "mimir/search/delete_relaxed_problem_explorator.hpp"

#include "mimir/algorithms/souffle.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/translator/delete_relax.hpp"
#include "mimir/search/applicability.hpp"
#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/grounded/grounded.hpp"
#include "mimir/search/applicable_action_generators/lifted/kpkc.hpp"
#include "mimir/search/axiom_evaluators/grounded/event_handlers/default.hpp"
#include "mimir/search/axiom_evaluators/grounded/grounded.hpp"
#include "mimir/search/axiom_evaluators/lifted/kpkc.hpp"
#include "mimir/search/match_tree/match_tree.hpp"
#include "mimir/search/state_unpacked.hpp"

#include <algorithm>
#include <string>

using namespace mimir::formalism;

namespace mimir::search
{

/**
 * Symbols
 */

static std::string to_datalog_symbol(std::string symbol)
{
    std::ranges::replace(symbol, '-', '_');
    return symbol;
}

template<IsStaticOrFluentOrDerivedTag P>
static std::string to_datalog_symbol(const PredicateImpl<P>& predicate)
{
    if (predicate.get_name() == "=")
        return fmt::format("equal_{}", predicate.get_index());
    else
        return fmt::format("{}_{}", to_datalog_symbol(predicate.get_name()), predicate.get_index());
}

static std::string to_datalog_symbol(const ActionImpl& action, const ConditionalEffectImpl& effect)
{
    return fmt::format("{}_{}_{}", to_datalog_symbol(action.get_name()), effect.get_index(), action.get_index());
}

static std::string to_datalog_symbol(const AxiomImpl& axiom)
{
    return fmt::format("{}_{}", to_datalog_symbol(*axiom.get_literal()->get_atom()->get_predicate()), axiom.get_index());
}

static std::string to_datalog_symbol(const VariableImpl& variable) { return fmt::format("x{}", variable.get_index()); }

static std::string to_datalog_symbol(const ObjectImpl& object) { return fmt::format("{}", object.get_index()); }

static std::string to_datalog_symbol(const TermImpl& term)
{
    return std::visit([](auto&& arg) -> std::string { return to_datalog_symbol(*arg); }, term.get_variant());
}

/**
 * Relations
 */

template<IsStaticOrFluentOrDerivedTag P>
static std::string to_datalog_relation(const PredicateImpl<P>& predicate)
{
    return fmt::format(
        "{}({})",
        to_datalog_symbol(predicate),
        fmt::join(predicate.get_parameters()
                      | std::views::transform([](auto&& parameter) { return fmt::format("{}:number", to_datalog_symbol(*parameter->get_variable())); }),
                  ","));
}

static std::string to_datalog_relation(const ActionImpl& action, const ConditionalEffectImpl& effect)
{
    auto add_parameters = [](const auto& parameters, std::vector<std::string>& out)
    {
        auto range = parameters | std::views::transform([](auto&& param) { return fmt::format("{}:number", to_datalog_symbol(*param->get_variable())); });

        for (auto&& s : range)
            out.push_back(std::move(s));
    };

    auto parameters = std::vector<std::string> {};

    add_parameters(action.get_parameters(), parameters);
    add_parameters(effect.get_conjunctive_condition()->get_parameters(), parameters);

    return fmt::format("{}({})", to_datalog_symbol(action, effect), fmt::join(parameters, ","));
}

static std::string to_datalog_relation(const AxiomImpl& axiom)
{
    auto add_parameters = [](const auto& parameters, std::vector<std::string>& out)
    {
        auto range = parameters | std::views::transform([](auto&& param) { return fmt::format("{}:number", to_datalog_symbol(*param->get_variable())); });

        for (auto&& s : range)
            out.push_back(std::move(s));
    };

    auto parameters = std::vector<std::string> {};

    add_parameters(axiom.get_parameters(), parameters);

    return fmt::format("{}({})", to_datalog_symbol(axiom), fmt::join(parameters, ","));
}

static void create_datalog_predicate_relations(const ProblemImpl& problem, std::ostream& out)
{
    boost::hana::for_each(problem.get_domain()->get_hana_predicates(),
                          [&](auto&& pair)
                          {
                              const auto& predicates = boost::hana::second(pair);

                              for (const auto& predicate : predicates)
                              {
                                  fmt::print(out, ".decl {}\n", to_datalog_relation(*predicate));
                              }
                          });

    for (const auto& derived_predicate : problem.get_derived_predicates())
    {
        fmt::print(out, ".decl {}\n", to_datalog_relation(*derived_predicate));
    }
}

static void create_datalog_action_relations(const ActionImpl& action, std::ostream& out)
{
    for (const auto& cond_effect : action.get_conditional_effects())
    {
        fmt::print(out, ".decl {}\n", to_datalog_relation(action, *cond_effect));

        fmt::print(out, ".output {}(IO=grounder)\n", to_datalog_symbol(action, *cond_effect));
    }
}

static void create_datalog_action_relations(const DomainImpl& domain, std::ostream& out)
{
    for (const auto& action : domain.get_actions())
        create_datalog_action_relations(*action, out);
}

static void create_datalog_axiom_relations(const AxiomImpl& axiom, std::ostream& out)
{
    fmt::print(out, ".decl {}\n", to_datalog_relation(axiom));

    fmt::print(out, ".output {}(IO=grounder)\n", to_datalog_symbol(axiom));
}

static void create_datalog_axiom_relations(const ProblemImpl& problem, std::ostream& out)
{
    for (const auto& axiom : problem.get_problem_and_domain_axioms())
        create_datalog_axiom_relations(*axiom, out);
}

/**
 * Atoms
 */

template<IsStaticOrFluentOrDerivedTag P>
static std::string to_datalog_atom(const AtomImpl<P>& atom)
{
    return fmt::format("{}({})",
                       to_datalog_symbol(*atom.get_predicate()),
                       fmt::join(atom.get_terms() | std::views::transform([](auto&& term) { return to_datalog_symbol(*term); }), ","));
}

static std::string to_datalog_atom(const AxiomImpl& axiom)
{
    return fmt::format("{}({})",
                       to_datalog_symbol(axiom),
                       fmt::join(axiom.get_parameters() | std::views::transform([](auto&& param) { return to_datalog_symbol(*param->get_variable()); }), ","));
}

static std::string to_datalog_atom(const ActionImpl& action, const ConditionalEffectImpl& effect)
{
    auto add_parameters = [](const auto& parameters, std::vector<std::string>& out)
    {
        auto range = parameters | std::views::transform([](auto&& param) { return to_datalog_symbol(*param->get_variable()); });

        for (auto&& s : range)
            out.push_back(std::move(s));
    };

    auto parameters = std::vector<std::string> {};

    add_parameters(action.get_parameters(), parameters);
    add_parameters(effect.get_conjunctive_condition()->get_parameters(), parameters);

    return fmt::format("{}({})", to_datalog_symbol(action, effect), fmt::join(parameters, ","));
}

/**
 * Rules
 */

static void create_datalog_axiom_rule(const AxiomImpl& axiom, std::ostream& out)
{
    auto axiom_cond = axiom.get_conjunctive_condition();

    auto body_atoms = std::vector<std::string> {};

    auto add_positive = [](const auto& literals, std::vector<std::string>& out)
    {
        auto range = literals | std::views::filter([](auto&& lit) { return lit->get_polarity(); })
                     | std::views::transform([](auto&& lit) { return to_datalog_atom(*lit->get_atom()); });

        for (auto&& s : range)
            out.push_back(std::move(s));
    };

    // Collect from action condition
    add_positive(axiom_cond->get_literals<StaticTag>(), body_atoms);
    add_positive(axiom_cond->get_literals<FluentTag>(), body_atoms);
    add_positive(axiom_cond->get_literals<DerivedTag>(), body_atoms);

    if (body_atoms.empty())
    {
        fmt::print(out, "{}.\n", to_datalog_atom(axiom));
        fmt::print(out, "{}.\n", to_datalog_atom(*axiom.get_literal()->get_atom()));
    }
    else
    {
        fmt::print(out, "{} :- {}.\n", to_datalog_atom(axiom), fmt::join(body_atoms, ", "));
        fmt::print(out, "{} :- {}.\n", to_datalog_atom(*axiom.get_literal()->get_atom()), fmt::join(body_atoms, ", "));
    }
}

static void create_datalog_axiom_rules(const ProblemImpl& problem, std::ostream& out)
{
    for (const auto& axiom : problem.get_problem_and_domain_axioms())
        create_datalog_axiom_rule(*axiom, out);
}

static void create_datalog_action_rule(const ActionImpl& action, std::ostream& out)
{
    auto action_cond = action.get_conjunctive_condition();

    for (const auto& cond_effect : action.get_conditional_effects())
    {
        auto effect_cond = cond_effect->get_conjunctive_condition();
        auto effect = cond_effect->get_conjunctive_effect();

        auto body_atoms = std::vector<std::string> {};

        auto add_positive = [](const auto& literals, std::vector<std::string>& out)
        {
            auto range = literals | std::views::filter([](auto&& lit) { return lit->get_polarity(); })
                         | std::views::transform([](auto&& lit) { return to_datalog_atom(*lit->get_atom()); });

            for (auto&& s : range)
                out.push_back(std::move(s));
        };

        // Collect from action condition
        add_positive(action_cond->get_literals<StaticTag>(), body_atoms);
        add_positive(action_cond->get_literals<FluentTag>(), body_atoms);
        add_positive(action_cond->get_literals<DerivedTag>(), body_atoms);

        // Collect from effect condition
        add_positive(effect_cond->get_literals<StaticTag>(), body_atoms);
        add_positive(effect_cond->get_literals<FluentTag>(), body_atoms);
        add_positive(effect_cond->get_literals<DerivedTag>(), body_atoms);

        if (body_atoms.empty())
            fmt::print(out, "{}.\n", to_datalog_atom(action, *cond_effect));
        else
            fmt::print(out, "{} :- {}.\n", to_datalog_atom(action, *cond_effect), fmt::join(body_atoms, ", "));

        auto head_atoms = std::vector<std::string> {};

        // Collect from effect
        add_positive(effect->get_literals(), head_atoms);

        for (const auto& head_atom : head_atoms)
            fmt::print(out, "{} :- {}.\n", head_atom, to_datalog_atom(action, *cond_effect));
    }
}

static void create_datalog_action_rules(const DomainImpl& domain, std::ostream& out)
{
    for (const auto& action : domain.get_actions())
        create_datalog_action_rule(*action, out);
}

/**
 * Facts
 */

static void create_datalog_initial_facts(const ProblemImpl& problem, std::ostream& out)
{
    boost::hana::for_each(
        problem.get_hana_initial_literals(),
        [&](auto&& pair)
        {
            const auto& literals = boost::hana::second(pair);

            for (const auto& literal : literals)
            {
                if (literal->get_polarity())
                {
                    fmt::print(out,
                               "{}({}).\n",
                               to_datalog_symbol(*literal->get_atom()->get_predicate()),
                               fmt::join(literal->get_atom()->get_objects() | std::views::transform([](auto&& object) { return object->get_index(); }), ","));
                }
            }
        });
}

static std::string create_datalog_program(const ProblemImpl& problem)
{
    std::stringstream ss;

    create_datalog_predicate_relations(problem, ss);
    create_datalog_action_relations(*problem.get_domain(), ss);
    create_datalog_axiom_relations(problem, ss);

    create_datalog_action_rules(*problem.get_domain(), ss);
    create_datalog_axiom_rules(problem, ss);

    create_datalog_initial_facts(problem, ss);

    return ss.str();
}

/// A custom output write stream
struct ActionGrounderWriteStream : public souffle::WriteStream
{
    ProblemImpl& m_problem;
    Action m_action;

    GroundActionSet& m_ref_ground_actions;

    explicit ActionGrounderWriteStream(const std::map<std::string, std::string>& rwOperation,
                                       const souffle::SymbolTable& symbolTable,
                                       const souffle::RecordTable& recordTable,
                                       ProblemImpl& problem,
                                       Action action,
                                       GroundActionSet& ref_ground_actions) :
        WriteStream(rwOperation, symbolTable, recordTable),
        m_problem(problem),
        m_action(action),
        m_ref_ground_actions(ref_ground_actions)
    {
        assert(arity >= m_action->get_arity());
    }

    void writeNullary() override { m_ref_ground_actions.insert(m_problem.ground(m_action, ObjectList {})); }

    void writeNextTuple(const souffle::RamDomain* tup) override
    {
        auto binding = ObjectList {};
        binding.reserve(m_action->get_arity());
        for (size_t col = 0; col < m_action->get_arity(); ++col)
        {
            const auto object_index = tup[col];
            binding.push_back(m_problem.get_repositories().get_object(object_index));
        }
        m_ref_ground_actions.insert(m_problem.ground(m_action, binding));
    }
};

struct AxiomGrounderWriteStream : public souffle::WriteStream
{
    ProblemImpl& m_problem;
    Axiom m_axiom;

    GroundAxiomSet& m_ref_ground_axioms;

    explicit AxiomGrounderWriteStream(const std::map<std::string, std::string>& rwOperation,
                                      const souffle::SymbolTable& symbolTable,
                                      const souffle::RecordTable& recordTable,
                                      ProblemImpl& problem,
                                      Axiom axiom,
                                      GroundAxiomSet& ref_ground_axioms) :
        WriteStream(rwOperation, symbolTable, recordTable),
        m_problem(problem),
        m_axiom(axiom),
        m_ref_ground_axioms(ref_ground_axioms)
    {
        assert(arity == m_axiom->get_arity());
    }

    void writeNullary() override { m_ref_ground_axioms.insert(m_problem.ground(m_axiom, ObjectList {})); }

    void writeNextTuple(const souffle::RamDomain* tup) override
    {
        auto binding = ObjectList {};
        binding.reserve(m_axiom->get_arity());
        for (size_t col = 0; col < m_axiom->get_arity(); ++col)
        {
            const auto object_index = tup[col];
            binding.push_back(m_problem.get_repositories().get_object(object_index));
        }
        m_ref_ground_axioms.insert(m_problem.ground(m_axiom, binding));
    }
};

/// Factory of the custom output write stream
struct GrounderWriteStreamFactory : public souffle::WriteStreamFactory
{
    std::unordered_map<std::string, Action> m_relation_to_action;
    std::unordered_map<std::string, Axiom> m_relation_to_axiom;

    ProblemImpl& m_problem;
    GroundActionSet& m_ref_ground_actions;
    GroundAxiomSet& m_ref_ground_axioms;

    explicit GrounderWriteStreamFactory(ProblemImpl& problem, GroundActionSet& ref_ground_actions, GroundAxiomSet& ref_ground_axioms) :
        m_problem(problem),
        m_ref_ground_actions(ref_ground_actions),
        m_ref_ground_axioms(ref_ground_axioms)
    {
        for (const auto& action : problem.get_domain()->get_actions())
        {
            for (const auto& cond_effect : action->get_conditional_effects())
            {
                m_relation_to_action.emplace(to_datalog_symbol(*action, *cond_effect), action);
            }
        }
        for (const auto& axiom : problem.get_problem_and_domain_axioms())
        {
            m_relation_to_axiom.emplace(to_datalog_symbol(*axiom), axiom);
        }
    }

    std::unique_ptr<souffle::WriteStream>
    getWriter(const std::map<std::string, std::string>& rwOperation, const souffle::SymbolTable& symbolTable, const souffle::RecordTable& recordTable) override
    {
        const auto& relation = rwOperation.at("name");

        if (auto it = m_relation_to_action.find(relation); it != m_relation_to_action.end())
            return std::make_unique<ActionGrounderWriteStream>(rwOperation, symbolTable, recordTable, m_problem, it->second, m_ref_ground_actions);

        if (auto it = m_relation_to_axiom.find(relation); it != m_relation_to_axiom.end())
            return std::make_unique<AxiomGrounderWriteStream>(rwOperation, symbolTable, recordTable, m_problem, it->second, m_ref_ground_axioms);

        throw std::logic_error("Could not find structure for relation.");
    }

    const std::string& getName() const override { return name; }

    const std::string name = "grounder";
};

DeleteRelaxedProblemExplorator::DeleteRelaxedProblemExplorator(Problem problem) : m_problem(problem), m_ground_actions(), m_ground_axioms()
{
    mimir::datalog::solve(create_datalog_program(*problem), std::make_shared<GrounderWriteStreamFactory>(*m_problem, m_ground_actions, m_ground_axioms));
}

GroundActionList DeleteRelaxedProblemExplorator::create_ground_actions() const
{
    auto ground_actions = GroundActionList {};
    for (const auto& ground_action : m_ground_actions)
    {
        if (is_statically_applicable(ground_action->get_conjunctive_condition(), m_problem->get_positive_static_initial_atoms_bitset()))
        {
            ground_actions.push_back(ground_action);
        }
    }
    return ground_actions;
}

GroundAxiomList DeleteRelaxedProblemExplorator::create_ground_axioms() const
{
    auto ground_axioms = GroundAxiomList {};
    for (const auto& ground_axiom : m_ground_axioms)
    {
        if (is_statically_applicable(ground_axiom->get_conjunctive_condition(), m_problem->get_positive_static_initial_atoms_bitset()))
        {
            ground_axioms.push_back(ground_axiom);
        }
    }
    return ground_axioms;
}

GroundedAxiomEvaluator DeleteRelaxedProblemExplorator::create_grounded_axiom_evaluator(const match_tree::Options& options,
                                                                                       GroundedAxiomEvaluatorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedAxiomEvaluatorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_axiom_instantiation();
    auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& pddl_repositories = problem.get_repositories();

    /* Initialize bookkeeping to map ground axioms into corresponding partition. */
    const auto num_partitions = problem.get_problem_and_domain_axiom_partitioning().size();
    auto axiom_to_partition = std::unordered_map<Axiom, size_t> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        const auto& partition = problem.get_problem_and_domain_axiom_partitioning()[i];
        for (const auto& axiom : partition.get_axioms())
        {
            axiom_to_partition.emplace(axiom, i);
        }
    }

    /* Store ground axioms in corresponding partition. */
    auto ground_axiom_partitioning = std::vector<GroundAxiomList>(num_partitions);
    for (const auto& ground_axiom : create_ground_axioms())
    {
        ground_axiom_partitioning.at(axiom_to_partition.at(ground_axiom->get_axiom())).push_back(ground_axiom);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_axiom_instantiation(total_time);

    event_handler->on_start_build_axiom_match_trees();
    start_time = std::chrono::high_resolution_clock::now();

    /* Create a MatchTree for each partition. */
    auto match_tree_partitioning = std::vector<std::unique_ptr<match_tree::MatchTreeImpl<GroundAxiomImpl>>> {};
    for (size_t i = 0; i < num_partitions; ++i)
    {
        event_handler->on_start_build_axiom_match_tree(i);

        const auto& ground_axioms = ground_axiom_partitioning.at(i);

        auto match_tree = match_tree::MatchTreeImpl<GroundAxiomImpl>::create(pddl_repositories, ground_axioms, options);

        event_handler->on_finish_build_axiom_match_tree(*match_tree);

        match_tree_partitioning.push_back(std::move(match_tree));
    }

    end_time = std::chrono::high_resolution_clock::now();
    total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_build_axiom_match_trees(total_time);

    return GroundedAxiomEvaluatorImpl::create(m_problem, std::move(match_tree_partitioning), std::move(event_handler));
}

GroundedApplicableActionGenerator
DeleteRelaxedProblemExplorator::create_grounded_applicable_action_generator(const match_tree::Options& options,
                                                                            GroundedApplicableActionGeneratorImpl::EventHandler event_handler) const
{
    if (!event_handler)
    {
        event_handler = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl::create();
    }

    event_handler->on_start_ground_action_instantiation();
    const auto start_time = std::chrono::high_resolution_clock::now();

    auto& problem = *m_problem;
    auto& pddl_repositories = problem.get_repositories();

    auto ground_actions = create_ground_actions();

    const auto end_time = std::chrono::high_resolution_clock::now();
    const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    event_handler->on_finish_ground_action_instantiation(total_time);

    event_handler->on_start_build_action_match_tree();

    auto match_tree = match_tree::MatchTreeImpl<GroundActionImpl>::create(pddl_repositories, ground_actions, options);

    event_handler->on_finish_build_action_match_tree(*match_tree);

    return GroundedApplicableActionGeneratorImpl::create(m_problem, std::move(match_tree), std::move(event_handler));
}

const Problem& DeleteRelaxedProblemExplorator::get_problem() const { return m_problem; }
}