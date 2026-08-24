from abc import ABC, abstractmethod
from pathlib import Path
import copy
from concurrent.futures import ThreadPoolExecutor
import gc
import importlib.util
import inspect
import os
import pickle
import subprocess
import sys
import textwrap
import threading
from typing import get_type_hints
import weakref

import pytest

import mimir


EXAMPLES = Path(__file__).resolve().parents[2] / "Tests" / "Examples"


@pytest.fixture
def blocks():
    base = EXAMPLES / "blocks_3"
    return mimir.Problem.from_files(
        base / "domain.pddl",
        base / "p01.pddl",
        generator="grounded",
    )


def test_release_and_public_search_surface():
    assert mimir.advanced.lib.mimir_abi_version() == 12
    assert mimir.__version__ == "0.4.0"
    assert not hasattr(mimir, "brfs")
    assert not hasattr(mimir, "astar_eager")
    assert not hasattr(mimir, "gbfs_lazy")
    assert not hasattr(mimir, "StateSpaceSampler")
    assert inspect.signature(mimir.astar).parameters["heuristic"].default is inspect.Parameter.empty
    assert inspect.signature(mimir.bfs).parameters["timeout_seconds"].kind is inspect.Parameter.KEYWORD_ONLY


def test_root_exports_only_core_model_and_concrete_heuristic_types():
    root_model_types = {"Domain", "Problem", "State", "GroundAction"}
    assert all(getattr(mimir, name) is getattr(mimir.model, name)
               for name in root_model_types)
    assert all(not hasattr(mimir, name)
               for name in set(mimir.model.__all__) - root_model_types)
    assert not hasattr(mimir, "Heuristic")
    assert mimir.BlindHeuristic is mimir.heuristics.BlindHeuristic
    assert mimir.heuristics.Heuristic.__abstractmethods__ == {"evaluate"}


def test_every_search_has_a_complete_runtime_signature():
    expected_parameters = {
        "bfs": {"problem", "start_state", "goal", "timeout_seconds",
                "max_expanded_states", "on_expand", "on_goal", "on_generate",
                "on_discover", "on_prune", "on_finish_depth"},
        "ucs": {"problem", "start_state", "goal", "timeout_seconds",
                "max_expanded_states", "on_expand", "on_goal", "on_generate",
                "on_discover", "on_prune"},
        "astar": {"problem", "heuristic", "start_state", "goal",
                  "timeout_seconds", "max_expanded_states", "on_expand", "on_goal",
                  "on_generate", "on_discover", "on_prune", "on_finish_f_layer"},
        "gbfs": {"problem", "heuristic", "start_state", "goal",
                 "timeout_seconds", "max_expanded_states", "on_expand", "on_goal",
                 "on_generate", "on_discover", "on_prune", "on_new_best_h"},
        "iw": {"problem", "max_width", "start_state", "goal",
               "timeout_seconds", "max_expanded_states", "on_expand", "on_goal",
               "on_generate", "on_discover"},
    }
    for name, expected in expected_parameters.items():
        function = getattr(mimir, name)
        signature = inspect.signature(function)
        assert set(signature.parameters) == expected
        assert all(
            parameter.kind is not inspect.Parameter.VAR_KEYWORD
            for parameter in signature.parameters.values()
        )
        assert get_type_hints(function)["return"] is mimir.SearchResult


def test_result_conveniences_are_properties():
    assert isinstance(mimir.SearchStatistics.elapsed_seconds, property)
    assert isinstance(mimir.SearchResult.is_solved, property)
    assert mimir.Solution is mimir.search.Solution


def test_public_modules_are_implementations_and_legacy_modules_are_gone():
    assert mimir.model.Problem is mimir.Problem
    assert mimir.search.SearchResult is mimir.SearchResult
    assert mimir.state_space.StateSpace is mimir.StateSpace
    for name in (
        "mimir.wrapper_formalism",
        "mimir.wrapper_search",
        "mimir.heuristics_api",
        "mimir.search_api",
        "mimir.state_space_api",
    ):
        assert importlib.util.find_spec(name) is None


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
def test_equality_is_an_ordinary_static_predicate(generator):
    domain = mimir.Domain.from_pddl("""
        (define (domain equality-api)
          (:requirements :strips :equality :negative-preconditions)
          (:predicates (done ?x))
          (:action choose
            :parameters (?x ?y)
            :precondition (not (= ?x ?y))
            :effect (done ?x)))
    """)
    problem = mimir.Problem.from_pddl(domain, """
        (define (problem equality-problem)
          (:domain equality-api)
          (:objects a b)
          (:init)
          (:goal (= a a)))
    """, generator=generator)
    equals = domain.predicate("=")
    assert equals.name == "="
    assert equals.arity == 2
    assert tuple(parameter.type_name for parameter in equals.parameters) == ("object", "object")
    assert equals.predicate_type is mimir.model.PredicateType.STATIC

    x = problem.variable("?x")
    y = problem.variable("?y")
    atom = problem.atom(equals, x, y)
    negative = problem.literal(atom, positive=False)
    assert negative.atom.predicate == equals
    assert not negative.is_positive

    bindings = problem.condition(negative, variables=(x, y)).bindings(problem.initial_state)
    assert {(binding[x].name, binding[y].name) for binding in bindings} == {("a", "b"), ("b", "a")}

    a = problem.object("a")
    variable_object = problem.literal(problem.atom(equals, x, a), positive=False)
    assert [binding[x].name for binding in problem.condition(
        variable_object, variables=(x,)).bindings(problem.initial_state)
    ] == ["b"]
    reflexive = problem.fact("=", "a", "a")
    non_reflexive = problem.fact("=", "a", "b")
    assert problem.initial_state.holds(reflexive)
    assert not problem.initial_state.holds(non_reflexive)
    assert problem.condition(
        problem.literal(problem.atom(equals, a, a)), variables=()
    ).bindings(problem.initial_state) == ({},)
    assert problem.initial_state.holds(problem.goal)
    assert all(action.objects[0] != action.objects[1]
               for action in problem.initial_state.applicable_actions())


def test_derived_equality_uses_the_ordinary_predicate_truth():
    domain = mimir.Domain.from_pddl("""
        (define (domain derived-equality)
          (:requirements :strips :equality :derived-predicates :negative-preconditions)
          (:predicates (unused))
          (:derived (same ?x ?y) (= ?x ?y)))
    """)
    problem = mimir.Problem.from_pddl(domain, """
        (define (problem derived-equality-problem)
          (:domain derived-equality)
          (:objects a b)
          (:init)
          (:goal (and (same a a) (not (same a b)))))
    """)
    assert problem.initial_state.holds(problem.goal)


@pytest.fixture
def extended_state_problem():
    domain = mimir.Domain.from_pddl('''
        (define (domain extended-state-api)
          (:requirements :strips :derived-predicates :negative-preconditions
                         :conditional-effects)
          (:predicates (enabled) (done))
          (:derived (ready) (enabled))
          (:action enable
            :parameters ()
            :precondition (not (enabled))
            :effect (enabled))
          (:action finish
            :parameters ()
            :precondition (ready)
            :effect (done))
          (:action conditional
            :parameters ()
            :precondition ()
            :effect (when (ready) (done))))
    ''')
    return mimir.Problem.from_pddl(domain, '''
        (define (problem extended-state-problem)
          (:domain extended-state-api)
          (:init)
          (:goal (done)))
    ''')


def test_every_python_state_producer_exposes_derived_truth(extended_state_problem):
    problem = extended_state_problem
    enabled = problem.fact('enabled')
    ready = problem.fact('ready')
    done = problem.fact('done')
    initial = problem.initial_state
    custom = problem.state(enabled)
    successor = problem.action('enable').apply(initial)

    assert not initial.holds(ready)
    assert custom.holds(ready)
    assert successor.holds(ready)
    assert successor == custom
    assert hash(successor) == hash(custom)
    assert tuple(successor) == (enabled,)
    assert len(successor) == 1
    assert 'ready' not in str(successor)

    finish = problem.action('finish')
    assert finish.is_applicable(successor)
    assert not finish.is_applicable(initial)

    conditional = problem.action('conditional')
    conditional_effect = conditional.conditional_effects[0]
    assert not conditional_effect.is_satisfied(initial)
    assert conditional_effect.is_satisfied(successor)
    assert not conditional.apply(initial).holds(done)
    assert conditional.apply(successor).holds(done)

    ready_atom = problem.atom(problem.domain.predicate('ready'))
    ready_condition = problem.condition(problem.literal(ready_atom))
    assert ready_condition.bindings(initial) == ()
    assert ready_condition.bindings(successor) == ({},)
    ready_goal = problem.ground_condition(ready)
    goal_count = mimir.GoalCountHeuristic(problem)
    assert goal_count.evaluate(initial, ready_goal) == 1.0
    assert goal_count.evaluate(successor, ready_goal) == 0.0

    space = mimir.StateSpace(problem, max_states=32)
    assert space.initial_state == initial
    assert not space.initial_state.holds(ready)
    indexed = next(
        state for state in space
        if state.holds(ready) and not state.holds(done)
    )
    assert indexed == successor
    assert space.label(indexed).depth == 1
    transition = next(
        item for item in space.successors(initial)
        if item.target == successor
    )
    assert transition.target.holds(ready)
    assert transition.action.apply(transition.source) == transition.target


def test_search_and_heuristic_callbacks_receive_extended_states(extended_state_problem):
    problem = extended_state_problem
    ready = problem.fact('ready')
    observed = {
        'expand': [],
        'goal': [],
        'generate': [],
        'discover': [],
        'prune': [],
    }

    def observe_state(name):
        def callback(state):
            observed[name].append(state.holds(ready))
        return callback

    def observe_transition(name):
        def callback(transition):
            observed[name].append((
                transition.source.holds(ready),
                transition.target.holds(ready),
            ))
            assert transition.action.apply(transition.source) == transition.target
        return callback

    result = mimir.bfs(
        problem,
        on_expand=observe_state('expand'),
        on_goal=observe_state('goal'),
        on_generate=observe_transition('generate'),
        on_discover=observe_transition('discover'),
        on_prune=observe_transition('prune'),
    )
    assert result.is_solved
    assert all(observed.values())
    assert result.solution is not None
    assert result.solution.goal_state.holds(ready)

    class DerivedHeuristic(mimir.heuristics.Heuristic):
        def __init__(self, heuristic_problem):
            super().__init__(heuristic_problem)
            self.evaluations = []
            self.preferences = []

        def evaluate(self, state, goal=None):
            self.evaluations.append(state.holds(ready))
            return 0.0

        def is_preferred(self, state, action, goal=None):
            self.preferences.append(state.holds(ready))
            return False

    heuristic = DerivedHeuristic(problem)
    assert mimir.gbfs(problem, heuristic).is_solved
    assert heuristic.evaluations
    assert heuristic.preferences


def test_equality_is_requirement_gated():
    domain = mimir.Domain.from_pddl(
        "(define (domain no-equality) (:requirements :strips) (:predicates (p)))")
    assert not domain.has_predicate("=")
    with pytest.raises(KeyError):
        domain.predicate("=")


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
def test_problem_owns_action_generation_and_search(blocks, generator):
    problem = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
        generator=generator,
    )
    assert problem.generator == generator
    assert problem.initial_state.applicable_actions()
    assert mimir.bfs(problem).is_solved
    assert len(mimir.StateSpace(problem, max_states=1000)) > 1
    custom_state = problem.state()
    assert isinstance(custom_state.applicable_actions(), tuple)
    assert mimir.bfs(problem, start_state=custom_state).status in {
        mimir.SearchStatus.SOLVED,
        mimir.SearchStatus.EXHAUSTED,
    }
    goal_count = mimir.GoalCountHeuristic(problem)
    assert isinstance(goal_count.preferred_actions(problem.initial_state), tuple)
    assert mimir.PerfectHeuristic(problem).evaluate(problem.initial_state) >= 0.0


def test_problem_defaults_to_lifted_generator():
    base = EXAMPLES / "blocks_3"
    for loader in (
        mimir.Problem.from_file,
        mimir.Problem.from_pddl,
        mimir.Problem.from_files,
    ):
        assert inspect.signature(loader).parameters["generator"].default == "lifted"

    problem = mimir.Problem.from_files(
        base / "domain.pddl",
        base / "p01.pddl",
    )

    assert problem.generator == "lifted"

    with pytest.raises(ValueError, match="grounded.*lifted"):
        mimir.Problem.from_files(
            base / "domain.pddl",
            base / "p01.pddl",
            generator="clique",
        )


def test_grounded_rpg_heuristic_rejects_lifted_problem(blocks):
    problem = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
        generator="lifted",
    )
    with pytest.raises(ValueError, match="grounded"):
        mimir.FFHeuristic(problem)


def test_explicit_loaders_and_raw_constructors(blocks):
    assert blocks.name == "blocksworld-300"
    assert blocks.domain.name == "blocksworld"
    assert isinstance(blocks.objects, tuple)
    with pytest.raises(TypeError):
        mimir.Domain(EXAMPLES / "blocks_3" / "domain.pddl")
    with pytest.raises(TypeError):
        mimir.Problem()
    with pytest.raises(TypeError):
        mimir.State(123, blocks)
    with pytest.raises(TypeError):
        mimir.GroundAction(123, blocks)


def test_legacy_getters_and_factories_are_not_public(blocks):
    values = (
        blocks.domain,
        blocks,
        blocks.objects[0],
        blocks.domain.predicates[0],
        blocks.domain.actions[0],
        blocks.initial_state,
        blocks.goal,
    )
    for value in values:
        assert not any(name.startswith("get_") for name in dir(value))
        assert not any(name.startswith("new_") for name in dir(value))

    assert not hasattr(mimir.GroundAction, "get_cost")
    assert not hasattr(mimir.GroundAction, "new")
    assert not hasattr(mimir.model.Atom, "new")


def test_effect_views_are_typed_read_only_values(blocks):
    action = blocks.domain.actions[0]
    conditional = action.conditional_effects[0]
    effect = conditional.effect
    assert isinstance(effect.parameters, tuple)
    assert isinstance(effect.literals, tuple)
    assert conditional.effect_literal in effect.literals
    assert isinstance(action.static_preconditions, tuple)
    assert isinstance(action.fluent_preconditions, tuple)
    assert isinstance(action.derived_preconditions, tuple)

    ground_action = next(iter(blocks.initial_state.applicable_actions()))
    ground_effect = ground_action.effect
    assert ground_effect.problem == blocks
    assert isinstance(ground_effect.add_atoms, tuple)
    assert isinstance(ground_effect.delete_atoms, tuple)

    for effect_type in (
        mimir.model.Effect,
        mimir.model.ConditionalEffect,
        mimir.model.GroundEffect,
        mimir.model.GroundConditionalEffect,
    ):
        with pytest.raises(TypeError):
            effect_type()


def test_pddl_errors_are_structured():
    with pytest.raises(mimir.PddlError) as captured:
        mimir.Domain.from_pddl("(define (domain broken) (:predicates (p))")
    assert captured.value.document_type == "domain"
    assert captured.value.error_code in {"lexical", "syntax"}
    assert captured.value.line is not None


def test_properties_factories_and_state_construction(blocks):
    clear = blocks.domain.predicate("clear")
    assert clear.name == "clear"
    assert clear.predicate_type is mimir.model.PredicateType.FLUENT
    assert clear.is_fluent
    fact = blocks.fact("clear", "b1")
    assert fact.predicate == clear
    custom = blocks.state(fact, fact)
    assert len(custom) == 1
    assert fact in custom
    assert tuple(custom) == custom.fluent_atoms
    with pytest.raises(TypeError):
        blocks.ground_atom("clear", blocks.objects[0])
    with pytest.raises(ValueError):
        blocks.ground_atom(clear)
    with pytest.raises(KeyError):
        blocks.domain.predicate("missing")


@pytest.mark.parametrize("kind", ["atom", "ground_atom"])
def test_atom_predicates_retain_the_domain_without_retaining_the_problem(kind):
    problem = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    domain = problem.domain

    if kind == "atom":
        source_predicate = domain.predicate("clear")
        variable = problem.variable("?block")
        atom = problem.atom(source_predicate, variable)
        predicate = atom.predicate
        del source_predicate, variable
    else:
        atom = problem.initial_fluent_atoms[0]
        predicate = atom.predicate

    predicate_name = predicate.name
    problem_handle = problem._handle
    problem_reference = weakref.ref(problem)
    assert predicate._owner is domain

    del atom, problem
    gc.collect()

    assert problem_reference() is None
    with pytest.raises(ValueError, match="invalid native value"):
        mimir.advanced.value_hash(problem_handle)
    assert predicate.name == predicate_name


def test_variadic_state_contract(blocks):
    first, second = blocks.initial_fluent_atoms[:2]
    assert len(blocks.state()) == 0
    assert tuple(blocks.state(first)) == (first,)
    assert set(blocks.state(first, second)) == {first, second}
    assert tuple(blocks.state(first, first)) == (first,)
    with pytest.raises(TypeError, match="GroundAtom"):
        blocks.state([first])


def test_state_rejects_non_fluent_facts():
    domain = mimir.Domain.from_pddl("""
        (define (domain state-test)
          (:requirements :strips)
          (:predicates (fixed) (changed))
          (:action change
            :parameters ()
            :precondition (fixed)
            :effect (changed)))
    """)
    problem = mimir.Problem.from_pddl(domain, """
        (define (problem state-test-problem)
          (:domain state-test)
          (:init (fixed))
          (:goal (changed)))
    """)
    fixed = problem.fact("fixed")
    assert fixed.is_static
    with pytest.raises(ValueError, match="fluent atoms only"):
        problem.state(fixed)


def test_variadic_ground_condition_contract(blocks):
    first = blocks.fact("clear", "b1")
    second = blocks.fact("on-table", "b2")
    negative = blocks.ground_literal(second, positive=False)

    assert len(blocks.ground_condition()) == 0
    positive_condition = blocks.ground_condition(first)
    assert positive_condition.literals == (blocks.ground_literal(first),)
    mixed = blocks.ground_condition(first, negative)
    assert mixed.literals == (blocks.ground_literal(first), negative)
    with pytest.raises(TypeError, match="GroundAtom or GroundLiteral"):
        blocks.ground_condition([first])


def test_ground_condition_rejects_foreign_values(blocks):
    other = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    foreign = other.fact("clear", "b1")
    with pytest.raises(ValueError):
        blocks.ground_condition(foreign)
    with pytest.raises(ValueError):
        blocks.ground_condition(other.ground_literal(foreign))


def test_factory_ownership_errors_are_values(blocks):
    other = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    with pytest.raises(ValueError):
        blocks.ground_atom(blocks.domain.predicate("clear"), other.objects[0])
    with pytest.raises(ValueError):
        blocks.state(other.fact("clear", "b1"))


def test_search_result_field_invariants(blocks):
    statistics = mimir.SearchStatistics(0, 0, 0, 0.0, 0.0)
    solution = mimir.Solution((), 0.0, blocks.initial_state)
    solved = mimir.SearchResult(mimir.SearchStatus.SOLVED, solution, statistics)
    assert solved.is_solved

    with pytest.raises(ValueError, match="solved search results"):
        mimir.SearchResult(mimir.SearchStatus.SOLVED, None, statistics)

    for status in set(mimir.SearchStatus) - {mimir.SearchStatus.SOLVED}:
        result = mimir.SearchResult(status, None, statistics)
        assert not result.is_solved
        with pytest.raises(ValueError, match="unsolved search results"):
            mimir.SearchResult(status, solution, statistics)


def test_named_action_cost_and_checked_apply(blocks):
    state = blocks.initial_state
    action = next(iter(state.applicable_actions()))
    successor = action.apply(state)
    assert isinstance(successor, mimir.State)
    assert action.cost >= 0.0
    assert state != successor
    assert inspect.signature(action.apply).parameters.keys() == {"state"}

    inapplicable = next(
        action for action in (
            blocks.action("move-b-to-t", "b1", "b2"),
            blocks.action("move-b-to-t", "b2", "b1"),
        ) if not action.is_applicable(state)
    )
    with pytest.raises(ValueError, match="not applicable"):
        inapplicable.apply(state)


@pytest.mark.parametrize("search_name", ["bfs", "ucs"])
def test_uninformed_searches_return_typed_results(blocks, search_name):
    result = getattr(mimir, search_name)(blocks)
    assert result.status is mimir.SearchStatus.SOLVED
    assert result.solution is not None
    assert isinstance(result.solution.plan, tuple)
    assert result.solution.cost == sum(action.cost for action in result.solution.plan)
    assert result.solution.goal_state.holds(blocks.goal)
    assert result.statistics.expanded_states > 0


@pytest.mark.parametrize("search_name", ["astar", "gbfs"])
def test_informed_searches_require_and_use_heuristics(blocks, search_name):
    heuristic = mimir.GoalCountHeuristic(blocks)
    result = getattr(mimir, search_name)(blocks, heuristic)
    assert result.is_solved
    assert heuristic.evaluate(blocks.initial_state) > 0


def test_iw_and_expansion_limits(blocks):
    result = mimir.iw(blocks, max_width=3)
    assert result.status in {mimir.SearchStatus.SOLVED, mimir.SearchStatus.EXHAUSTED}
    limited = mimir.bfs(blocks, max_expanded_states=0)
    assert limited.status is mimir.SearchStatus.EXPANSION_LIMIT_REACHED
    assert limited.solution is None
    assert limited.statistics.expanded_states == 0


def test_zero_timeout_is_distinct(blocks):
    result = mimir.bfs(blocks, timeout_seconds=0)
    assert result.status is mimir.SearchStatus.TIMED_OUT


def test_initial_goal_is_tested_but_not_expanded(blocks):
    expanded = []
    goals = []
    result = mimir.bfs(
        blocks,
        goal=blocks.ground_condition(),
        on_expand=expanded.append,
        on_goal=goals.append,
    )
    assert result.status is mimir.SearchStatus.SOLVED
    assert expanded == []
    assert goals == [blocks.initial_state]
    assert result.statistics.expanded_states == 0
    assert result.statistics.generated_states == 1


@pytest.mark.parametrize("search_name", ["bfs", "ucs", "astar", "gbfs", "iw"])
def test_exact_and_insufficient_expansion_budgets(blocks, search_name):
    def run(limit):
        keywords = {"max_expanded_states": limit}
        if search_name in {"astar", "gbfs"}:
            return getattr(mimir, search_name)(blocks, mimir.BlindHeuristic(blocks), **keywords)
        if search_name == "iw":
            return mimir.iw(blocks, max_width=3, **keywords)
        return getattr(mimir, search_name)(blocks, **keywords)

    complete = run(None)
    exact = run(complete.statistics.expanded_states)
    insufficient = run(complete.statistics.expanded_states - 1)
    one = run(1)

    assert exact.status is mimir.SearchStatus.SOLVED
    assert exact.statistics.expanded_states == complete.statistics.expanded_states
    assert insufficient.status is mimir.SearchStatus.EXPANSION_LIMIT_REACHED
    assert one.status is mimir.SearchStatus.EXPANSION_LIMIT_REACHED
    assert one.statistics.expanded_states == 1


def test_custom_python_heuristic_drives_native_search(blocks):
    class CountingHeuristic(mimir.heuristics.Heuristic):
        def __init__(self, problem):
            super().__init__(problem)
            self.calls = 0

        def evaluate(self, state, goal=None):
            self.calls += 1
            return 0.0

    heuristic = CountingHeuristic(blocks)
    assert mimir.astar(blocks, heuristic).is_solved
    assert heuristic.calls > 1


def test_custom_preferred_actions_are_used_by_gbfs(blocks):
    class PreferredHeuristic(mimir.heuristics.Heuristic):
        def __init__(self, problem):
            super().__init__(problem)
            self.preferred_calls = 0

        def evaluate(self, state, goal=None):
            return 0.0

        def is_preferred(self, state, action, goal=None):
            self.preferred_calls += 1
            return False

    heuristic = PreferredHeuristic(blocks)
    assert mimir.gbfs(blocks, heuristic).is_solved
    assert heuristic.preferred_calls > 0


def test_custom_heuristic_exception_is_rethrown(blocks):
    class BrokenHeuristic(mimir.heuristics.Heuristic):
        def evaluate(self, state, goal=None):
            raise LookupError("heuristic failed")

    with pytest.raises(LookupError, match="heuristic failed"):
        mimir.astar(blocks, BrokenHeuristic(blocks))
    assert mimir.bfs(blocks).is_solved


def test_transition_callbacks_and_exceptions(blocks):
    transitions = []
    result = mimir.bfs(blocks, on_generate=transitions.append)
    assert result.is_solved
    assert transitions
    transition = transitions[0]
    assert isinstance(transition, mimir.Transition)
    assert transition.action.apply(transition.source) == transition.target
    assert not hasattr(transition, "cost")

    def fail(_state):
        raise RuntimeError("callback failed")

    with pytest.raises(RuntimeError, match="callback failed"):
        mimir.bfs(blocks, on_expand=fail)


@pytest.mark.parametrize(
    "run",
    [
        lambda problem, fail: mimir.bfs(problem, on_expand=fail),
        lambda problem, fail: mimir.bfs(problem, on_goal=fail),
        lambda problem, fail: mimir.bfs(problem, on_generate=fail),
        lambda problem, fail: mimir.bfs(problem, on_discover=fail),
        lambda problem, fail: mimir.bfs(problem, on_prune=fail),
        lambda problem, fail: mimir.bfs(problem, on_finish_depth=fail),
        lambda problem, fail: mimir.astar(
            problem, mimir.BlindHeuristic(problem), on_finish_f_layer=fail),
        lambda problem, fail: mimir.gbfs(
            problem, mimir.GoalCountHeuristic(problem), on_new_best_h=fail),
    ],
)
def test_every_callback_trampoline_rethrows_and_search_recovers(blocks, run):
    def fail(_value):
        raise LookupError("callback trampoline failed")

    with pytest.raises(LookupError, match="callback trampoline failed"):
        run(blocks, fail)
    assert mimir.bfs(blocks).is_solved


def test_alternate_goal_controls_evaluation_and_search(blocks):
    alternate = blocks.ground_condition()
    heuristic = mimir.GoalCountHeuristic(blocks)
    assert heuristic.evaluate(blocks.initial_state, alternate) == 0.0
    result = mimir.astar(
        blocks,
        heuristic,
        goal=alternate,
        max_expanded_states=0,
    )
    assert result.status is mimir.SearchStatus.SOLVED
    assert result.solution is not None
    assert result.solution.plan == ()


def test_condition_binding_limit_contract(blocks):
    condition = blocks.domain.actions[0].precondition
    unlimited = condition.bindings(blocks.initial_state)
    assert condition.bindings(blocks.initial_state, limit=None) == unlimited
    assert condition.bindings(blocks.initial_state, limit=0) == ()
    assert len(condition.bindings(blocks.initial_state, limit=1)) <= 1
    assert all(tuple(binding) == condition.parameters for binding in unlimited)
    with pytest.raises(ValueError, match="nonnegative"):
        condition.bindings(blocks.initial_state, limit=-1)
    for invalid in (True, 1.5, "1"):
        with pytest.raises(TypeError, match="int or None"):
            condition.bindings(blocks.initial_state, limit=invalid)


def test_variadic_condition_and_binding_validation(blocks, monkeypatch):
    variable = blocks.variable("?value")
    empty = blocks.condition()
    assert empty.bindings(blocks.initial_state) == ({},)
    with pytest.raises(ValueError, match="duplicates"):
        blocks.condition(variables=(variable, variable))
    with pytest.raises(TypeError, match="Literal"):
        blocks.condition([blocks.domain.actions[0].precondition.literals[0]])
    with pytest.raises(TypeError, match="state must be a State"):
        empty.bindings("not a state")

    other = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    with pytest.raises(ValueError, match="different problems"):
        empty.bindings(other.initial_state)

    one_parameter = blocks.condition(variables=(variable,))
    monkeypatch.setattr(
        mimir.model.lib,
        "mimir_binding_list_get_binding_size",
        lambda _handle, _index: 2,
    )
    with pytest.raises(RuntimeError, match="binding size"):
        one_parameter.bindings(blocks.initial_state, limit=1)


def test_public_relationships_validate_types_and_ownership(blocks):
    other = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    state = blocks.initial_state
    atom = blocks.initial_fluent_atoms[0]
    literal = blocks.ground_literal(atom)

    with pytest.raises(TypeError, match="GroundAtom"):
        state.contains("not an atom")
    with pytest.raises(ValueError, match="different problems"):
        state.contains(other.initial_fluent_atoms[0])
    with pytest.raises(TypeError, match="GroundLiteral"):
        state.literal_holds(atom)
    with pytest.raises(ValueError, match="different problems"):
        state.literal_holds(other.ground_literal(other.initial_fluent_atoms[0]))
    with pytest.raises(TypeError, match="name must be str"):
        blocks.domain.has_predicate(1)
    with pytest.raises(TypeError, match="child and parent"):
        blocks.domain.is_type_compatible("object", None)
    with pytest.raises(TypeError, match="positive must be bool"):
        blocks.ground_literal(atom, positive=1)
    assert state.literal_holds(literal)


def test_every_builtin_heuristic_honors_an_alternate_goal(blocks):
    alternate = blocks.ground_condition()
    heuristic_types = (
        mimir.BlindHeuristic,
        mimir.GoalCountHeuristic,
        mimir.FFHeuristic,
        mimir.LiftedFFHeuristic,
        mimir.AddHeuristic,
        mimir.MaxHeuristic,
        mimir.H2Heuristic,
        mimir.SetAddHeuristic,
        mimir.PerfectHeuristic,
    )
    for heuristic_type in heuristic_types:
        heuristic = heuristic_type(blocks)
        assert heuristic.evaluate(blocks.initial_state, alternate) == 0.0

    perfect = mimir.PerfectHeuristic(blocks)
    alternate_handle = perfect._handle_for(alternate)
    assert perfect._handle_for(alternate) == alternate_handle
    equal_alternate = blocks.ground_condition()
    assert equal_alternate == alternate
    assert perfect.evaluate(blocks.initial_state, equal_alternate) == 0.0


def test_perfect_heuristic_releases_collected_alternate_goal(blocks):
    state = blocks.initial_state
    heuristic = mimir.PerfectHeuristic(blocks)
    default_value = heuristic.evaluate(state)
    alternate = blocks.ground_condition()
    alternate_handle = heuristic._handle_for(alternate)
    assert heuristic.evaluate(state, alternate) == 0.0
    alternate_reference = weakref.ref(alternate)

    del alternate
    gc.collect()

    assert alternate_reference() is None
    with pytest.raises(ValueError, match="invalid native value"):
        mimir.advanced.value_hash(alternate_handle)
    assert heuristic.evaluate(state) == default_value


def test_perfect_heuristic_cache_is_identity_copied(blocks):
    heuristic = mimir.PerfectHeuristic(blocks)
    cache = heuristic._alternate_handles

    assert copy.copy(cache) is cache
    assert copy.deepcopy(cache) is cache
    with pytest.raises(TypeError):
        pickle.dumps(cache)


def test_perfect_heuristic_cache_allows_reentrant_gc(blocks):
    heuristic = mimir.PerfectHeuristic(blocks)
    cache = heuristic._alternate_handles
    alternate = blocks.ground_condition()
    alternate._literals.append(alternate)
    heuristic._handle_for(alternate)
    heuristic._finalizer.detach()
    alternate_reference = weakref.ref(alternate)
    del alternate

    completed = threading.Event()

    def collect_while_locked():
        with cache._lock:
            gc.collect()
        completed.set()

    try:
        thread = threading.Thread(target=collect_while_locked, daemon=True)
        thread.start()
        thread.join(timeout=2)

        assert completed.is_set(), "alternate-goal cleanup deadlocked while re-entering the cache lock"
        assert alternate_reference() is None
    finally:
        if completed.is_set():
            cache.close()
        mimir.advanced.free_handle(heuristic._handle)


def test_perfect_heuristic_same_goal_cache_is_thread_safe(blocks, monkeypatch):
    heuristic = mimir.PerfectHeuristic(blocks)
    goal = blocks.ground_condition()
    original_factory = mimir.advanced.lib.mimir_heuristic_perfect
    start = threading.Barrier(8)
    counter_lock = threading.Lock()
    native_lock = threading.Lock()
    factory_calls = 0
    created_handles = []

    def create(problem_handle, goal_handle):
        nonlocal factory_calls
        with counter_lock:
            factory_calls += 1
        with native_lock:
            handle = int(original_factory(problem_handle, goal_handle))
            created_handles.append(handle)
            return handle

    monkeypatch.setattr(mimir.advanced.lib, "mimir_heuristic_perfect", create)

    def get_handle(_index):
        start.wait()
        return heuristic._handle_for(goal)

    try:
        with ThreadPoolExecutor(max_workers=8) as executor:
            handles = tuple(executor.map(get_handle, range(8)))
        assert len(set(handles)) == 1
        assert factory_calls == 1
    finally:
        heuristic._finalizer()
        for handle in set(created_handles):
            mimir.advanced.free_handle(handle)


def test_native_heuristics_validate_state_and_action_ownership(blocks):
    other = mimir.Problem.from_files(
        EXAMPLES / "blocks_3" / "domain.pddl",
        EXAMPLES / "blocks_3" / "p01.pddl",
    )
    heuristic = mimir.GoalCountHeuristic(blocks)
    action = next(iter(blocks.initial_state.applicable_actions()))
    foreign_action = next(iter(other.initial_state.applicable_actions()))

    with pytest.raises(TypeError, match="state must be a State"):
        heuristic.preferred_actions("not a state")
    with pytest.raises(ValueError, match="different problems"):
        heuristic.preferred_actions(other.initial_state)
    with pytest.raises(TypeError, match="action must be a GroundAction"):
        heuristic.is_preferred(blocks.initial_state, "not an action")
    with pytest.raises(ValueError, match="different problems"):
        heuristic.is_preferred(blocks.initial_state, foreign_action)
    with pytest.raises(TypeError, match="state must be a State"):
        heuristic.is_preferred("not a state", action)


def test_blind_heuristic_rejects_a_null_native_handle(blocks, monkeypatch):
    import mimir.heuristics as heuristics_api

    monkeypatch.setattr(heuristics_api.lib, "mimir_heuristic_blind", lambda: 0)
    with pytest.raises(ValueError, match="could not create BlindHeuristic"):
        mimir.BlindHeuristic(blocks)


def test_failed_finalizer_registration_releases_the_new_native_handle(blocks, monkeypatch):
    from mimir.advanced import _native

    created_handles = []
    released_handles = []
    original_create = mimir.advanced.lib.mimir_goal_create
    original_free = mimir.advanced.lib.mimir_free_handle

    def create(*args):
        handle = int(original_create(*args))
        created_handles.append(handle)
        return handle

    def free(handle):
        released_handles.append(int(handle))
        original_free(handle)

    def reject_finalizer(*_args, **_kwargs):
        raise RuntimeError("finalizer registration failed")

    monkeypatch.setattr(mimir.advanced.lib, "mimir_goal_create", create)
    monkeypatch.setattr(mimir.advanced.lib, "mimir_free_handle", free)
    monkeypatch.setattr(_native.weakref, "finalize", reject_finalizer)

    with pytest.raises(RuntimeError, match="finalizer registration failed"):
        blocks.ground_condition()

    assert len(created_handles) == 1
    assert released_handles.count(created_handles[0]) == 1
    with pytest.raises(ValueError, match="invalid native value"):
        mimir.advanced.value_hash(created_handles[0])


def test_failures_before_finalizer_registration_release_native_handles_once(blocks, monkeypatch):
    created_handles = []
    failed_handles = []
    released_handles = []
    original_load_domain = mimir.advanced.lib.mimir_load_domain_from_string
    original_free = mimir.advanced.lib.mimir_free_handle

    class AbstractDomain(mimir.Domain, ABC):
        @abstractmethod
        def missing_method(self):
            pass

    class AbstractState(mimir.State, ABC):
        @abstractmethod
        def missing_method(self):
            pass

    class RejectProblemAssignment(mimir.State):
        def __setattr__(self, name, value):
            if name == "_problem":
                raise RuntimeError("problem assignment failed")
            super().__setattr__(name, value)

    def load_domain(*args):
        handle = int(original_load_domain(*args))
        created_handles.append(handle)
        return handle

    def free(handle):
        released_handles.append(int(handle))
        original_free(handle)

    monkeypatch.setattr(mimir.advanced.lib, "mimir_load_domain_from_string", load_domain)
    monkeypatch.setattr(mimir.advanced.lib, "mimir_free_handle", free)

    release_count = len(released_handles)
    with pytest.raises(TypeError, match="abstract"):
        AbstractDomain.from_pddl("(define (domain allocation-test) (:predicates (p)))")
    domain_handle = created_handles[-1]
    failed_handles.append(domain_handle)
    assert released_handles[release_count:].count(domain_handle) == 1
    with pytest.raises(ValueError, match="invalid native value"):
        mimir.advanced.value_hash(domain_handle)

    state_handle = int(mimir.advanced.lib.mimir_problem_get_initial_state(blocks._handle))
    failed_handles.append(state_handle)
    release_count = len(released_handles)
    with pytest.raises(TypeError, match="abstract"):
        AbstractState._from_handle(state_handle, blocks)
    assert released_handles[release_count:].count(state_handle) == 1
    with pytest.raises(ValueError, match="invalid native value"):
        mimir.advanced.value_hash(state_handle)

    state_handle = int(mimir.advanced.lib.mimir_problem_get_initial_state(blocks._handle))
    failed_handles.append(state_handle)
    release_count = len(released_handles)
    with pytest.raises(TypeError, match="states require a problem owner"):
        mimir.State._from_handle(state_handle, None)
    assert released_handles[release_count:].count(state_handle) == 1

    state_handle = int(mimir.advanced.lib.mimir_problem_get_initial_state(blocks._handle))
    failed_handles.append(state_handle)
    release_count = len(released_handles)
    with pytest.raises(RuntimeError, match="problem assignment failed"):
        RejectProblemAssignment._from_handle(state_handle, blocks)
    assert released_handles[release_count:].count(state_handle) == 1

    gc.collect()
    assert all(released_handles.count(handle) == 1 for handle in failed_handles)


def test_failure_after_finalizer_registration_releases_native_handles_once(blocks, monkeypatch):
    allocated_handles = []
    released_handles = []
    original_free = mimir.advanced.lib.mimir_free_handle

    class RejectHandleAssignment:
        def __setattr__(self, name, value):
            if name == "_handle":
                allocated_handles.append(int(value))
                raise RuntimeError("handle assignment failed")
            super().__setattr__(name, value)

    class RejectingDomain(RejectHandleAssignment, mimir.Domain):
        pass

    class RejectingState(RejectHandleAssignment, mimir.State):
        pass

    class RejectingGoalCount(RejectHandleAssignment, mimir.GoalCountHeuristic):
        pass

    class RejectingBlind(RejectHandleAssignment, mimir.BlindHeuristic):
        pass

    class RejectingGroundCondition(
        RejectHandleAssignment,
        mimir.model.GroundConjunctiveCondition,
    ):
        pass

    class RejectingPerfect(RejectHandleAssignment, mimir.PerfectHeuristic):
        pass

    class RejectingStateSpace(RejectHandleAssignment, mimir.StateSpace):
        pass

    def free(handle):
        released_handles.append(int(handle))
        original_free(handle)

    monkeypatch.setattr(mimir.advanced.lib, "mimir_free_handle", free)
    constructors = (
        lambda: RejectingDomain.from_pddl(
            "(define (domain assignment-test) (:predicates (p)))"
        ),
        lambda: RejectingState._from_handle(
            mimir.advanced.lib.mimir_problem_get_initial_state(blocks._handle),
            blocks,
        ),
        lambda: RejectingGoalCount(blocks),
        lambda: RejectingBlind(blocks),
        lambda: RejectingGroundCondition._from_literals([], blocks),
        lambda: RejectingPerfect(blocks),
        lambda: RejectingStateSpace(blocks, max_states=32),
    )

    for construct in constructors:
        allocation_count = len(allocated_handles)
        release_count = len(released_handles)
        with pytest.raises(RuntimeError, match="handle assignment failed"):
            construct()

        assert len(allocated_handles) == allocation_count + 1
        handle = allocated_handles[-1]
        assert released_handles[release_count:].count(handle) == 1
        with pytest.raises(ValueError, match="invalid native value"):
            mimir.advanced.value_hash(handle)

    gc.collect()
    assert all(released_handles.count(handle) == 1 for handle in allocated_handles)


def test_state_space_sequence_labels_transitions_and_sampling(blocks):
    space = mimir.StateSpace(blocks, max_states=1000, seed=7)
    assert len(space) > 1
    assert space[0] == space.initial_state
    assert space[-1] in tuple(space)
    assert isinstance(space[:2], tuple)
    label = space.label(space.initial_state)
    assert label.depth == 0
    assert label.steps_to_goal is not None
    assert space.transition_count > 0
    assert all(t.source == space.initial_state for t in space.successors(space.initial_state))
    first = space.sample_states(4)
    space.reseed(7)
    assert first == space.sample_states(4)
    assert space.sample_states(0) == ()


def test_state_space_queries_do_not_scan_the_sequence(blocks, monkeypatch):
    space = mimir.StateSpace(blocks, max_states=1000, seed=7)
    state = space.initial_state

    def fail_iteration(_space):
        raise AssertionError("state-space query scanned the full sequence")

    monkeypatch.setattr(mimir.StateSpace, "__iter__", fail_iteration)
    assert space.label(state).depth == 0
    assert space.successors(state)


def test_state_space_sampling_indices_are_cached(blocks):
    space = mimir.StateSpace(blocks, max_states=1000, seed=7)
    first_dead_ends, first_distances = space._sampling_indices()
    second_dead_ends, second_distances = space._sampling_indices()
    assert first_dead_ends is second_dead_ends
    assert first_distances is second_distances
    assert 0 in first_distances
    with pytest.raises(LookupError, match="no states at distance"):
        space.sample_state_at_distance(10_000)

    unreachable = blocks.state()
    if unreachable not in tuple(space):
        with pytest.raises(ValueError, match="not present"):
            space.label(unreachable)


def test_package_contains_typing_marker():
    assert (Path(mimir.__file__).with_name("py.typed")).is_file()
    assert not Path(mimir.__file__).with_name("__init__.pyi").exists()


def test_native_loader_does_not_search_the_legacy_parent_layout(monkeypatch):
    from mimir.advanced import _native

    parent = os.path.normpath(os.path.join(
        os.path.dirname(os.path.abspath(_native.__file__)),
        "..",
        _native._lib_filename(),
    ))
    monkeypatch.setattr(
        _native.os.path,
        "exists",
        lambda path: os.path.normpath(path) == parent,
    )
    monkeypatch.setattr(_native.os.path, "isdir", lambda _path: False)

    path, searched = _native._find_lib()

    assert path == _native._lib_filename()
    assert parent not in searched


def test_python_package_type_contract(tmp_path):
    cache = tmp_path / "mypy-cache"
    cache.mkdir()
    package = Path(__file__).resolve().parents[1] / "mimir"
    usage = Path(__file__).with_name("typing") / "api_usage.py"
    completed = subprocess.run(
        [
            sys.executable,
            "-m",
            "mypy",
            "--cache-dir",
            str(cache),
            str(package),
            str(usage),
        ],
        cwd=Path(__file__).resolve().parents[2],
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert completed.returncode == 0, completed.stdout + completed.stderr


@pytest.mark.parametrize("kind", ["state", "heuristic", "goal", "state_space"])
def test_native_owners_are_identity_copied_and_cannot_be_pickled(blocks, kind):
    factories = {
        "state": lambda: blocks.initial_state,
        "heuristic": lambda: mimir.GoalCountHeuristic(blocks),
        "goal": lambda: blocks.goal,
        "state_space": lambda: mimir.StateSpace(blocks, max_states=32),
    }
    value = factories[kind]()

    assert copy.copy(value) is value
    assert copy.deepcopy(value) is value
    with pytest.raises(TypeError):
        pickle.dumps(value)


def test_deepcopy_of_search_values_preserves_native_owner_identity(blocks):
    transitions = []
    result = mimir.bfs(blocks, on_generate=transitions.append)
    transition = transitions[0]

    copied_result, copied_transition = copy.deepcopy((result, transition))

    assert copied_result is not result
    assert copied_result.solution is not None
    assert result.solution is not None
    assert copied_result.solution.goal_state is result.solution.goal_state
    assert all(a is b for a, b in zip(
        copied_result.solution.plan, result.solution.plan))
    assert copied_transition is not transition
    assert copied_transition.source is transition.source
    assert copied_transition.action is transition.action
    assert copied_transition.target is transition.target

    del result, transition, transitions
    gc.collect()

    assert copied_transition.action.apply(
        copied_transition.source) == copied_transition.target


def test_stale_numeric_accessor_raises_instead_of_terminating(blocks):
    from mimir.advanced import free_handle

    action = next(iter(blocks.initial_state.applicable_actions()))
    free_handle(action._handle)
    with pytest.raises(ReferenceError):
        _ = action.cost


def test_native_failures_do_not_terminate_a_python_subprocess():
    code = textwrap.dedent(f"""
        from pathlib import Path
        import mimir
        from mimir.advanced import free_handle

        base = Path({str(EXAMPLES / 'blocks_3')!r})
        problem = mimir.Problem.from_files(base / 'domain.pddl', base / 'p01.pddl')
        action = next(iter(problem.initial_state.applicable_actions()))
        free_handle(action._handle)
        action._finalizer.detach()
        try:
            action.cost
        except ReferenceError:
            pass
        else:
            raise AssertionError('stale handle did not raise ReferenceError')

        try:
            mimir.Domain.from_pddl('(define (domain broken) (:predicates (p))')
        except mimir.PddlError as error:
            assert error.line is not None
            assert error.column is not None
        else:
            raise AssertionError('malformed PDDL did not raise PddlError')
    """)
    environment = os.environ.copy()
    environment["PYTHONPATH"] = str(Path(__file__).resolve().parents[1])
    completed = subprocess.run(
        [sys.executable, "-c", code],
        cwd=Path(__file__).resolve().parents[2],
        env=environment,
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert completed.returncode == 0, completed.stderr


def test_callback_abort_becomes_a_python_error_instead_of_terminating():
    code = textwrap.dedent(f"""
        import ctypes
        from pathlib import Path
        import mimir
        from mimir.advanced import free_handle, lib
        from mimir.advanced.heuristics import HEURISTIC_CALLBACK

        base = Path({str(EXAMPLES / 'blocks_3')!r})
        problem = mimir.Problem.from_files(base / 'domain.pddl', base / 'p01.pddl')
        state = problem.initial_state
        goal = problem.goal

        @HEURISTIC_CALLBACK
        def abort(state_handle, _result):
            free_handle(state_handle)
            return 0

        heuristic_handle = int(lib.mimir_heuristic_callback(
            problem._handle,
            ctypes.cast(abort, ctypes.c_void_p),
            ctypes.c_void_p(),
        ))
        try:
            try:
                lib.mimir_heuristic_evaluate(
                    heuristic_handle,
                    state._handle,
                    goal._handle,
                )
            except mimir.MimirError as error:
                assert 'callback aborted' in str(error).lower()
            else:
                raise AssertionError('callback abort did not become a Python error')
        finally:
            free_handle(heuristic_handle)
    """)
    environment = os.environ.copy()
    environment["PYTHONPATH"] = str(Path(__file__).resolve().parents[1])
    completed = subprocess.run(
        [sys.executable, "-c", code],
        cwd=Path(__file__).resolve().parents[2],
        env=environment,
        capture_output=True,
        text=True,
        timeout=30,
    )
    assert completed.returncode == 0, completed.stderr
