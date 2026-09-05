import ctypes
import gc

import pytest

import pymimir as mm
import pymimir.model as model_api
from pymimir.advanced import free_handle, lib, value_hash


@pytest.fixture(params=["grounded", "lifted"])
def problem(request):
    domain = mm.Domain.from_pddl("""
        (define (domain successors)
          (:requirements :strips :negative-preconditions :conditional-effects :derived-predicates)
          (:predicates (ready ?x) (done ?x) (enabled ?x))
          (:derived (enabled ?x) (ready ?x))
          (:action finish
            :parameters (?x)
            :precondition (enabled ?x)
            :effect (and (done ?x) (when (ready ?x) (not (ready ?x)))))
          (:action also-finish
            :parameters (?x)
            :precondition (enabled ?x)
            :effect (and (done ?x) (when (ready ?x) (not (ready ?x))))))
    """)
    return mm.Problem.from_pddl(domain, """
        (define (problem successors-problem)
          (:domain successors)
          (:objects a b)
          (:init (ready a) (ready b))
          (:goal (and (done a) (done b))))
    """, generator=request.param)


@pytest.mark.parametrize("actions_first", [False, True])
def test_successor_order_and_effects_match_applicable_actions(problem, actions_first):
    state = problem.initial_state
    # Also exercise a non-initial state, where grounded generation is rebuilt.
    for _ in range(2):
        reference_state = state if actions_first else problem.state(*state.fluent_atoms)
        expected_actions = reference_state.applicable_actions()
        transitions = state.successor_states()
        cached_actions = state.applicable_actions()

        assert tuple(action for action, _ in transitions) == expected_actions
        assert state.applicable_actions() is cached_actions
        for index, (action, successor) in enumerate(transitions):
            assert action is cached_actions[index]
            assert successor.problem is problem
            expected = action.apply(state)
            assert successor == expected
            assert set(successor.derived_atoms) == set(expected.derived_atoms)
        assert len({successor for _, successor in transitions}) < len(transitions)

        repeated = state.successor_states()
        assert repeated == transitions
        assert all(new_state is not old_state for (_, new_state), (_, old_state) in zip(repeated, transitions))
        assert all(new_action is old_action for (new_action, _), (old_action, _) in zip(repeated, transitions))
        state = transitions[0][1]

    assert state.successor_states() == ()
    assert state.applicable_actions() == ()
    assert state.successor_states() == ()


@pytest.mark.parametrize("actions_first", [False, True])
def test_successors_use_bulk_transfer_without_per_element_native_calls(problem, actions_first, monkeypatch):
    state = problem.initial_state
    if actions_first:
        state.applicable_actions()
    calls = []

    def record(name):
        original = getattr(lib, name)

        def counted(*args):
            calls.append(name)
            return original(*args)

        return counted

    bulk_calls = (
        "mimir_state_generate_successors",
        "mimir_transition_list_count",
        "mimir_transition_list_copy_handles",
    )
    for name in bulk_calls:
        monkeypatch.setattr(lib, name, record(name))

    def unexpected(*args):
        raise AssertionError("successor expansion made a per-element native call")

    for name in (
        "mimir_state_generate_applicable_actions",
        "mimir_action_list_get",
        "mimir_transition_list_get_action",
        "mimir_transition_list_get_state",
        "mimir_ground_action_is_applicable",
        "mimir_ground_action_apply",
        "mimir_value_equals",
    ):
        monkeypatch.setattr(lib, name, unexpected)

    transitions = state.successor_states()

    assert len(transitions) == 4
    assert calls == list(bulk_calls)
    assert all(action is cached for (action, _), cached in zip(transitions, state.applicable_actions()))


@pytest.mark.parametrize("failure_index", [1, 2, 3])
def test_successor_wrapping_failure_releases_all_handles(problem, monkeypatch, failure_index):
    state = problem.initial_state
    allocated = []
    released = []
    original_copy = lib.mimir_transition_list_copy_handles
    original_free = lib.mimir_free_handle
    original_finalizer = model_api._create_finalizer
    wrappers = 0

    def copy(*args):
        count = original_copy(*args)
        allocated.extend(args[1])
        allocated.extend(args[2])
        return count

    def release(handle):
        released.append(handle)
        original_free(handle)

    def create_finalizer(*args):
        nonlocal wrappers
        wrappers += 1
        if wrappers == failure_index:
            # Match _create_finalizer's ownership contract on failure.
            args[1](*args[2:])
            raise RuntimeError("injected wrapper failure")
        return original_finalizer(*args)

    monkeypatch.setattr(lib, "mimir_transition_list_copy_handles", copy)
    monkeypatch.setattr(lib, "mimir_free_handle", release)
    monkeypatch.setattr(model_api, "_create_finalizer", create_finalizer)
    with pytest.raises(RuntimeError, match="injected wrapper failure"):
        state.successor_states()
    gc.collect()

    assert len(allocated) == 8
    assert all(released.count(handle) == 1 for handle in allocated)
    for handle in allocated:
        with pytest.raises(ValueError, match="invalid native value"):
            value_hash(handle)


def test_native_successor_transfer_rejects_small_buffer(problem):
    state = problem.initial_state
    result = lib.mimir_state_generate_successors(state._handle, problem._handle, None, -1)
    try:
        count = lib.mimir_transition_list_count(result)
        states = (ctypes.c_int * count)()
        with pytest.raises(ValueError, match="capacity"):
            lib.mimir_transition_list_copy_handles(result, None, states, count - 1)
        assert list(states) == [0] * count
    finally:
        free_handle(result)


def test_native_successor_generation_rejects_foreign_problem(problem):
    state = problem.initial_state
    other = mm.Problem.from_pddl(problem.domain, """
        (define (problem other) (:domain successors)
          (:objects a) (:init (ready a)) (:goal (done a)))
    """)
    with pytest.raises(ValueError, match="different problem"):
        lib.mimir_state_generate_successors(state._handle, other._handle, None, -1)
