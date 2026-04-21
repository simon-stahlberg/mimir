#include "mimir/formalism/translator/novelty_compilation.hpp"

#include "mimir/formalism/problem.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/lifted/kpkc.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/state_repository.hpp"

#include <algorithm>
#include <gtest/gtest.h>

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::tests
{

TEST(MimirTests, MimirFormalismNoveltyCompilationApplicabilityTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x) (q ?x)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        "    (:action delete-p "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (not (p ?x)))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-problem) "
        "    (:domain novelty-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");
    auto translator = NoveltyTranslator(1);
    const auto novelty_problem = translator.translate(problem);
    const auto& compiled_problem = novelty_problem.get_problem();
    const auto& compiled_domain = compiled_problem->get_domain();

    EXPECT_EQ(novelty_problem.get_k(), 1);

    EXPECT_TRUE(compiled_domain->get_name_to_predicate<FluentTag>().contains("not-novel-p"));
    EXPECT_TRUE(compiled_domain->get_name_to_predicate<FluentTag>().contains("not-novel-q"));
    EXPECT_TRUE(compiled_domain->get_name_to_predicate<DerivedTag>().contains("novel-witness-make-q"));
    EXPECT_TRUE(compiled_domain->get_name_to_predicate<DerivedTag>().contains("novel-witness-delete-p"));
    EXPECT_EQ(compiled_domain->get_axioms().size(), 1);

    const auto initial_atom = problem->get_or_create_ground_atom(problem->get_domain()->get_predicate<FluentTag>("p"),
                                                                 ObjectList { problem->get_problem_or_domain_object("a") });
    const auto compiled_initial_atom = novelty_problem.get_compiled_atom(initial_atom);
    const auto compiled_initial_not_novel_atom = novelty_problem.get_not_novel_atom(initial_atom);

    EXPECT_TRUE(std::any_of(compiled_problem->get_initial_literals<FluentTag>().begin(),
                            compiled_problem->get_initial_literals<FluentTag>().end(),
                            [&](const auto& literal)
                            {
                                return literal->get_polarity() && literal->get_atom() == compiled_initial_not_novel_atom;
                            }));

    auto context = SearchContextImpl::create(compiled_problem, SearchContextImpl::Options(SearchContextImpl::LiftedOptions()));
    auto& state_repository = *context->get_state_repository();
    auto& applicable_action_generator = *context->get_applicable_action_generator();

    const auto [initial_state, initial_metric_value] = state_repository.get_or_create_initial_state();
    auto applicable_actions = GroundActionList {};
    for (const auto& action : applicable_action_generator.create_applicable_action_generator(initial_state))
    {
        applicable_actions.push_back(action);
    }

    ASSERT_EQ(applicable_actions.size(), 1);
    EXPECT_EQ(applicable_actions.front()->get_action()->get_name(), "make-q");

    const auto [successor_state, successor_metric_value] =
        state_repository.get_or_create_successor_state(initial_state, applicable_actions.front(), initial_metric_value);

    auto successor_actions = GroundActionList {};
    for (const auto& action : applicable_action_generator.create_applicable_action_generator(successor_state))
    {
        successor_actions.push_back(action);
    }

    EXPECT_TRUE(successor_actions.empty());
    (void) successor_metric_value;
    (void) compiled_initial_atom;
}

TEST(MimirTests, MimirFormalismNoveltyCompilationStateAtomsTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-state-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x) (q ?x) (r ?x)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (q ?x))) "
        "    (:action make-r "
        "        :parameters (?x) "
        "        :precondition (and (p ?x)) "
        "        :effect (and (r ?x))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-state-problem) "
        "    (:domain novelty-state-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");
    auto translator = NoveltyTranslator(1);
    const auto novelty_problem = translator.translate(problem);
    const auto& compiled_problem = novelty_problem.get_problem();

    const auto object = problem->get_problem_or_domain_object("a");
    const auto q_atom = problem->get_or_create_ground_atom(problem->get_domain()->get_predicate<FluentTag>("q"), ObjectList { object });
    const auto r_atom = problem->get_or_create_ground_atom(problem->get_domain()->get_predicate<FluentTag>("r"), ObjectList { object });

    const auto compiled_atoms = novelty_problem.create_compiled_state_atoms(GroundAtomList<FluentTag> { q_atom }, GroundAtomList<FluentTag> { r_atom });

    auto repository = StateRepositoryImpl::create(KPKCLiftedAxiomEvaluatorImpl::create(compiled_problem));
    const auto [state, metric_value] = repository->get_or_create_state(compiled_atoms, compiled_problem->get_initial_function_to_value<FluentTag>());

    EXPECT_TRUE(state.get_atoms<FluentTag>().get(novelty_problem.get_compiled_atom(q_atom)->get_index()));
    EXPECT_TRUE(state.get_atoms<FluentTag>().get(novelty_problem.get_not_novel_atom(q_atom)->get_index()));
    EXPECT_TRUE(state.get_atoms<FluentTag>().get(novelty_problem.get_not_novel_atom(r_atom)->get_index()));
    (void) metric_value;
}

TEST(MimirTests, MimirFormalismNoveltyCompilationValidationTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-validation-test) "
        "    (:requirements :strips) "
        "    (:predicates (not-novel-p ?x) (p ?x)) "
        "    (:action make-p "
        "        :parameters (?x) "
        "        :precondition (and (not-novel-p ?x)) "
        "        :effect (and (p ?x))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-validation-problem) "
        "    (:domain novelty-validation-test) "
        "    (:objects a) "
        "    (:init (not-novel-p a)) "
        "    (:goal (p a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");

    // k != 1 throws
    EXPECT_THROW(NoveltyTranslator(2).translate(problem), std::runtime_error);

    // name collision throws
    auto translator = NoveltyTranslator(1);
    EXPECT_THROW(translator.translate(problem), std::runtime_error);
}

TEST(MimirTests, MimirFormalismNoveltyCompilationWitnessValidationTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-witness-validation-test) "
        "    (:requirements :strips) "
        "    (:predicates (novel-witness-make-p ?x) (p ?x)) "
        "    (:action make-p "
        "        :parameters (?x) "
        "        :precondition (and (novel-witness-make-p ?x)) "
        "        :effect (and (p ?x))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-witness-validation-problem) "
        "    (:domain novelty-witness-validation-test) "
        "    (:objects a) "
        "    (:init (novel-witness-make-p a)) "
        "    (:goal (p a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");

    // witness name collision throws
    auto translator = NoveltyTranslator(1);
    EXPECT_THROW(translator.translate(problem), std::runtime_error);
}

TEST(MimirTests, MimirFormalismNoveltyCompilationSingleUseTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-single-use-test) "
        "    (:requirements :strips) "
        "    (:predicates (p ?x)) "
        "    (:action make-p "
        "        :parameters (?x) "
        "        :precondition () "
        "        :effect (and (p ?x))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-single-use-problem) "
        "    (:domain novelty-single-use-test) "
        "    (:objects a) "
        "    (:init ) "
        "    (:goal (p a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");
    auto translator = NoveltyTranslator(1);
    EXPECT_NO_THROW(translator.translate(problem));

    // Using again throws
    EXPECT_THROW(translator.translate(problem), std::runtime_error);
}

TEST(MimirTests, MimirFormalismNoveltyCompilationConditionalEffectsTest)
{
    const auto domain_content = std::string(
        "(define (domain novelty-cond-eff-test) "
        "    (:requirements :strips :conditional-effects) "
        "    (:predicates (p ?x) (q ?x)) "
        "    (:action make-q "
        "        :parameters (?x) "
        "        :precondition () "
        "        :effect (and (when (p ?x) (q ?x)))) "
        ")");

    const auto problem_content = std::string(
        "(define (problem novelty-cond-eff-problem) "
        "    (:domain novelty-cond-eff-test) "
        "    (:objects a) "
        "    (:init (p a)) "
        "    (:goal (q a)) "
        ")");

    const auto problem = ProblemImpl::create(domain_content, "", problem_content, "");
    auto translator = NoveltyTranslator(1);
    const auto novelty_problem = translator.translate(problem);
    const auto& compiled_problem = novelty_problem.get_problem();
    const auto& compiled_domain = compiled_problem->get_domain();

    const auto& axioms = compiled_domain->get_axioms();
    EXPECT_EQ(axioms.size(), 1);

    auto context = SearchContextImpl::create(compiled_problem, SearchContextImpl::Options(SearchContextImpl::LiftedOptions()));
    auto& state_repository = *context->get_state_repository();
    auto& applicable_action_generator = *context->get_applicable_action_generator();

    const auto [initial_state, initial_metric_value] = state_repository.get_or_create_initial_state();

    auto applicable_actions = GroundActionList {};
    for (const auto& action : applicable_action_generator.create_applicable_action_generator(initial_state))
    {
        applicable_actions.push_back(action);
    }

    ASSERT_EQ(applicable_actions.size(), 1);
    EXPECT_EQ(applicable_actions.front()->get_action()->get_name(), "make-q");

    const auto [successor_state, successor_metric_value] =
        state_repository.get_or_create_successor_state(initial_state, applicable_actions.front(), initial_metric_value);

    const auto q_atom = problem->get_or_create_ground_atom(problem->get_domain()->get_predicate<FluentTag>("q"),
                                                           ObjectList { problem->get_problem_or_domain_object("a") });
    EXPECT_TRUE(successor_state.get_atoms<FluentTag>().get(novelty_problem.get_compiled_atom(q_atom)->get_index()));
    EXPECT_TRUE(successor_state.get_atoms<FluentTag>().get(novelty_problem.get_not_novel_atom(q_atom)->get_index()));

    (void) successor_metric_value;
}

}
