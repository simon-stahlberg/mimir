import math

import pytest

import pymimir as mm
from pymimir.heuristics import Heuristic
from test_qgbfs import graph, plan


class StateHeuristic(Heuristic):
    def __init__(self, problem, values):
        super().__init__(problem)
        self.values = values
        self.calls = []

    def evaluate(self, state, goal=None):
        node = next(index for index in self.values if state.holds(self.problem.atom("at", f"n{index}")))
        self.calls.append(node)
        return self.values[node]


def test_qbeam_batches_whole_layer_and_selects_best_duplicate(graph):
    problem, heuristic = graph([(0, 2, 5), (0, 3, 5), (2, 4, 1), (3, 4, 10), (4, 1, -1)])
    result = mm.qbeam(problem, heuristic, beam_size=2)
    assert plan(result) == ["(edge1)", "(edge3)", "(edge4)"]
    assert result.action_values == (5, 10, None)
    assert heuristic.batch_sizes == [1, 2]
    assert result.statistics.visited_states == 5
    assert result.statistics.expanded_states == 4
    assert result.statistics.generated_transitions == 5
    assert result.statistics.evaluated_candidates == 4
    assert result.statistics.max_depth == 3


def test_qbeam_preserves_duplicate_and_closed_successors_in_each_row(graph):
    problem, heuristic = graph([(0, 0, 99), (0, 2, 1), (0, 2, 8), (2, 0, 99), (2, 3, 2), (3, 1, 0)])
    result = mm.qbeam(problem, heuristic)
    assert plan(result) == ["(edge2)", "(edge4)", "(edge5)"]
    assert heuristic.rows == [("(edge0)", "(edge1)", "(edge2)"), ("(edge3)", "(edge4)")]
    assert result.action_values == (8, 2, None)
    assert result.statistics.evaluated_candidates == 5


@pytest.mark.parametrize("maximize,first", [(True, "(edge0)"), (False, "(edge1)")])
def test_qbeam_score_direction(graph, maximize, first):
    problem, heuristic = graph([(0, 2, 3), (0, 3, 1), (2, 1, 0), (3, 1, 0)])
    assert plan(mm.qbeam(problem, heuristic, maximize=maximize))[0] == first


@pytest.mark.parametrize("width,solved", [(1, False), (2, True)])
def test_width_one_is_greedy_and_wider_beam_keeps_alternative(graph, width, solved):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 1), (3, 1, 0)])
    result = mm.qbeam(problem, heuristic, beam_size=width)
    assert result.is_solved is solved
    if not solved:
        assert result.status is mm.SearchStatus.DEAD_END
        assert [str(action) for action in result.partial_plan] == ["(edge0)"]
        assert result.action_values == (10,)


@pytest.mark.parametrize("kind", ["q", "state"])
def test_width_pruned_successors_can_be_reached_later(graph, kind):
    problem, q = graph([(0, 2, 10), (0, 3, 1), (2, 3, 5), (3, 1, 0)])
    result = (mm.qbeam(problem, q) if kind == "q"
              else mm.beam(problem, StateHeuristic(problem, {2: 0, 3: 1})))
    assert plan(result) == ["(edge0)", "(edge2)", "(edge3)"]
    assert result.statistics.visited_states == 4


@pytest.mark.parametrize("kind", ["q", "state"])
def test_equal_scores_keep_first_path_and_generation_order(graph, kind):
    problem, q = graph([(0, 2, 0), (0, 3, 0), (0, 2, 0), (2, 1, 0), (3, 1, 0)])
    result = (mm.qbeam(problem, q) if kind == "q"
              else mm.beam(problem, StateHeuristic(problem, {2: 1, 3: 1})))
    assert plan(result) == ["(edge0)", "(edge3)"]


def test_state_heuristic_scores_unique_nonclosed_candidates_once(graph):
    problem, _ = graph([(0, 0, 0), (0, 2, 0), (0, 2, 0), (0, 3, 0), (2, 4, 0), (3, 4, 0), (4, 1, 0)])
    heuristic = StateHeuristic(problem, {2: 5, 3: 1, 4: 3})
    result = mm.beam(problem, heuristic, beam_size=2)
    assert plan(result) == ["(edge3)", "(edge5)", "(edge6)"]
    assert result.action_values == (1, 3, None)
    assert heuristic.calls == [2, 3, 4]
    assert result.statistics.evaluated_candidates == 3


@pytest.mark.parametrize("kind", ["q", "state"])
@pytest.mark.parametrize("depth,status,length", [(0, "DEPTH_LIMIT_REACHED", 0), (1, "DEPTH_LIMIT_REACHED", 1), (2, "SOLVED", 2)])
def test_depth_limits_and_exact_goal_depth(graph, kind, depth, status, length):
    problem, q = graph([(0, 2, 8), (2, 1, 0)])
    result = (mm.qbeam(problem, q, max_depth=depth) if kind == "q"
              else mm.beam(problem, mm.BlindHeuristic(problem), max_depth=depth))
    assert result.status is getattr(mm.SearchStatus, status)
    assert len(result.action_values) == length
    assert result.statistics.max_depth == length
    assert len(result.solution.plan if result.is_solved else result.partial_plan) == length


@pytest.mark.parametrize("kind", ["q", "state"])
@pytest.mark.parametrize("goal", [0, 1])
def test_goal_avoids_evaluation(graph, kind, goal):
    problem, q = graph([(0, 1, math.nan)], goal=goal)
    h = StateHeuristic(problem, {})
    result = mm.qbeam(problem, q) if kind == "q" else mm.beam(problem, h)
    assert result.is_solved
    assert result.action_values == (() if goal == 0 else (None,))
    assert result.statistics.evaluated_candidates == 0
    assert not q.rows and not h.calls


@pytest.mark.parametrize("kind", ["q", "state"])
def test_cycle_exhaustion_and_terminal_dead_end_are_distinct(graph, kind):
    for edges, status in [([(0, 0, 0)], mm.SearchStatus.EXHAUSTED), ([], mm.SearchStatus.DEAD_END)]:
        problem, q = graph(edges)
        result = mm.qbeam(problem, q) if kind == "q" else mm.beam(problem, mm.BlindHeuristic(problem))
        assert result.status is status
        assert result.statistics.evaluated_candidates == 0


@pytest.mark.parametrize("kind", ["q", "state"])
def test_limits_finish_collected_batch_and_return_scored_path(graph, kind):
    problem, q = graph([(0, 2, 8), (0, 3, 1), (2, 4, 5), (3, 1, 0)])
    result = (mm.qbeam(problem, q, beam_size=2, max_expanded_states=2) if kind == "q" else
              mm.beam(problem, StateHeuristic(problem, {2: 0, 3: 1, 4: 2}), beam_size=2, max_expanded_states=2))
    assert result.status is mm.SearchStatus.EXPANSION_LIMIT_REACHED
    assert [str(action) for action in result.partial_plan] == ["(edge0)", "(edge2)"]
    assert result.statistics.expanded_states == 2
    assert result.statistics.evaluated_candidates == 3
    assert result.action_values == ((8, 5) if kind == "q" else (0, 2))


@pytest.mark.parametrize("kind", ["q", "state"])
@pytest.mark.parametrize("limit,status", [({"max_expanded_states": 0}, "EXPANSION_LIMIT_REACHED"), ({"timeout_seconds": 0}, "TIMED_OUT")])
def test_zero_limits_avoid_callbacks(graph, kind, limit, status):
    problem, q = graph([(0, 2, 0)])
    def unexpected(_):
        raise AssertionError("must not be called")
    result = (mm.qbeam(problem, q, should_stop=unexpected, **limit) if kind == "q" else
              mm.beam(problem, StateHeuristic(problem, {}), should_stop=unexpected, **limit))
    assert result.status is getattr(mm.SearchStatus, status)
    assert result.statistics.expanded_states == result.statistics.evaluated_candidates == 0


@pytest.mark.parametrize("kind", ["q", "state"])
def test_stop_preserves_scored_path_and_propagates_errors(graph, kind):
    problem, q = graph([(0, 2, 8), (2, 1, 0)])
    run = mm.qbeam if kind == "q" else mm.beam
    heuristic = q if kind == "q" else StateHeuristic(problem, {2: 3})
    counts = []
    result = run(problem, heuristic, should_stop=lambda count: counts.append(count) or True)
    assert result.status is mm.SearchStatus.STOPPED
    assert counts == [1]
    assert result.action_values == ((8,) if kind == "q" else (3,))
    error = RuntimeError("callback failed")
    def fail(*args):
        raise error
    for kwargs in [{"should_stop": fail}, {}]:
        if not kwargs:
            heuristic.evaluate = fail
        with pytest.raises(RuntimeError) as caught:
            run(problem, heuristic, **kwargs)
        assert caught.value is error


@pytest.mark.parametrize("value", [math.nan, math.inf, -math.inf])
def test_qbeam_rejects_nonfinite_even_on_closed_action(graph, value):
    problem, q = graph([(0, 0, value), (0, 2, 1)])
    with pytest.raises(ValueError, match="must be finite"):
        mm.qbeam(problem, q)


@pytest.mark.parametrize("value", [math.nan, -1, -math.inf])
def test_beam_rejects_invalid_state_scores(graph, value):
    problem, _ = graph([(0, 2, 0)])
    with pytest.raises(ValueError):
        mm.beam(problem, StateHeuristic(problem, {2: value}))


def test_beam_treats_positive_infinity_as_dead_end(graph):
    problem, _ = graph([(0, 2, 0)])
    result = mm.beam(problem, StateHeuristic(problem, {2: math.inf}))
    assert result.status is mm.SearchStatus.DEAD_END
    assert not result.partial_plan


@pytest.mark.parametrize("kind", ["q", "state"])
@pytest.mark.parametrize("kwargs,error", [({"beam_size": 0}, ValueError), ({"beam_size": True}, TypeError),
    ({"max_depth": -1}, ValueError), ({"max_depth": 1.5}, TypeError), ({"max_depth": 2**31}, ValueError),
    ({"max_expanded_states": 2**32}, ValueError), ({"should_stop": 1}, TypeError), ({"timeout_seconds": -1}, ValueError)])
def test_validation(graph, kind, kwargs, error):
    problem, q = graph([(0, 2, 0)])
    with pytest.raises(error):
        if kind == "q": mm.qbeam(problem, q, **kwargs)
        else: mm.beam(problem, mm.BlindHeuristic(problem), **kwargs)


@pytest.mark.parametrize("kind", ["q", "state"])
def test_alternate_start_goal_and_ownership(graph, kind):
    problem, q = graph([(0, 2, 3), (2, 1, 0)])
    run = mm.qbeam if kind == "q" else mm.beam
    heuristic = q if kind == "q" else mm.BlindHeuristic(problem)
    start = problem.initial_state.successor_states()[0][1]
    assert len(run(problem, heuristic, start_state=start).solution.plan) == 1
    assert run(problem, heuristic, goal=problem.ground_condition()).is_solved
    other, _ = graph([(0, 1, 0)])
    for kwargs in [{"start_state": other.initial_state}, {"goal": other.goal}]:
        with pytest.raises(ValueError, match="different problem"):
            run(problem, heuristic, **kwargs)


@pytest.mark.parametrize("kind", ["q", "state"])
@pytest.mark.parametrize("edges,cap,status", [
    ([], 1, "DEAD_END"),
    ([(0, 0, 0)], 1, "EXHAUSTED"),
    ([(0, 2, 0)], 2, "DEAD_END"),
])
def test_terminal_layer_at_exact_budget_reports_natural_termination(graph, kind, edges, cap, status):
    problem, q = graph(edges)
    result = (mm.qbeam(problem, q, max_expanded_states=cap) if kind == "q" else
              mm.beam(problem, mm.BlindHeuristic(problem), max_expanded_states=cap))
    assert result.status is getattr(mm.SearchStatus, status)
    assert result.statistics.expanded_states == cap


def test_infinite_candidates_at_exact_budget_report_dead_end(graph):
    problem, _ = graph([(0, 2, 0)])
    result = mm.beam(problem, StateHeuristic(problem, {2: math.inf}), max_expanded_states=1)
    assert result.status is mm.SearchStatus.DEAD_END
    assert result.statistics.evaluated_candidates == 1


@pytest.mark.parametrize("kind", ["q", "state"])
def test_unexpanded_beam_members_prevent_reporting_dead_end(graph, kind):
    problem, q = graph([(0, 2, 10), (0, 3, 0), (3, 1, 0)])
    result = (mm.qbeam(problem, q, beam_size=2, max_expanded_states=2) if kind == "q" else
              mm.beam(problem, StateHeuristic(problem, {2: 0, 3: 1}), beam_size=2, max_expanded_states=2))
    assert result.status is mm.SearchStatus.EXPANSION_LIMIT_REACHED
    assert result.statistics.expanded_states == 2


@pytest.mark.parametrize("cap,status", [(2, "EXPANSION_LIMIT_REACHED"), (3, "DEAD_END")])
def test_infinite_candidates_distinguish_partial_and_complete_layers(graph, cap, status):
    problem, _ = graph([(0, 2, 0), (0, 3, 0), (2, 4, 0), (3, 4, 0)])
    result = mm.beam(problem, StateHeuristic(problem, {2: 0, 3: 1, 4: math.inf}),
                     beam_size=2, max_expanded_states=cap)
    assert result.status is getattr(mm.SearchStatus, status)


@pytest.mark.parametrize("kind", ["beam", "qbeam", "qgbfs"])
def test_initial_goal_skips_native_setup_and_heuristic_binding(graph, monkeypatch, kind):
    import pymimir.q_search as q_search
    from pymimir.advanced import lib
    problem, q = graph([(0, 2, 0)])
    goal = problem.ground_condition()
    heuristic = mm.BlindHeuristic(problem) if kind == "beam" else q
    def unexpected(*args, **kwargs):
        raise AssertionError("initial goal must precede setup")
    monkeypatch.setattr(q_search, "_heuristic_handle", unexpected)
    monkeypatch.setattr(q_search, "QHEURISTIC_CALLBACK", unexpected)
    monkeypatch.setattr(lib, "mimir_" + kind + "_search", unexpected)
    options = {} if kind == "qgbfs" else {"max_depth": 0}
    result = getattr(mm, kind)(problem, heuristic, goal=goal, timeout_seconds=0,
                               max_expanded_states=0, should_stop=unexpected, **options)
    assert result.is_solved
    assert result.statistics == mm.SearchStatistics(0, 1, 0, 0, 0, 1, 0, 0)
    assert result.solution.goal_state == problem.initial_state
    assert result.action_values == result.solution.plan == ()


class BatchStateHeuristic(StateHeuristic):
    def __init__(self, problem, values):
        super().__init__(problem, values)
        self.batches = []

    def evaluate(self, state, goal=None):
        raise AssertionError("scalar callback must not be used")

    def evaluate_batch(self, states, goal=None):
        self.batches.append(tuple(states))
        assert goal == self.problem.goal
        return [StateHeuristic.evaluate(self, state, goal) for state in states]


def test_python_batch_override_receives_ordered_unique_nonclosed_states(graph):
    problem, _ = graph([(0, 0, 0), (0, 2, 0), (0, 2, 0), (0, 3, 0), (2, 4, 0), (3, 4, 0), (4, 1, 0)])
    heuristic = BatchStateHeuristic(problem, {2: 5, 3: 1, 4: 3})
    result = mm.beam(problem, heuristic, beam_size=2)
    assert plan(result) == ["(edge3)", "(edge5)", "(edge6)"]
    assert heuristic.calls == [2, 3, 4]
    assert [len(batch) for batch in heuristic.batches] == [2, 1]
    assert result.action_values == (1, 3, None)
    assert result.statistics.evaluated_candidates == 3


@pytest.mark.parametrize("values,error", [([], ValueError), ([1, 2], ValueError), ([math.nan], ValueError),
                                        ([-1], ValueError), ([-math.inf], ValueError), ([True], TypeError)])
def test_python_batch_override_rejects_invalid_outputs(graph, values, error):
    problem, _ = graph([(0, 2, 0)])
    heuristic = BatchStateHeuristic(problem, {})
    heuristic.evaluate_batch = lambda states, goal=None: values
    with pytest.raises(error):
        mm.beam(problem, heuristic)


def test_python_batch_error_preserves_exception_identity(graph):
    problem, _ = graph([(0, 2, 0)])
    heuristic = BatchStateHeuristic(problem, {})
    error = RuntimeError("batch failed")
    def fail(states, goal=None):
        raise error
    heuristic.evaluate_batch = fail
    with pytest.raises(RuntimeError) as caught:
        mm.beam(problem, heuristic)
    assert caught.value is error


def test_python_batch_states_can_be_retained_and_release_their_handles(graph):
    import gc
    import weakref
    from pymimir.advanced import value_hash
    problem, _ = graph([(0, 2, 0), (0, 3, 0)])
    heuristic = BatchStateHeuristic(problem, {2: 1, 3: 2})
    mm.beam(problem, heuristic)
    retained = heuristic.batches[0]
    assert retained[0].holds(problem.atom("at", "n2"))
    handles = [state._handle for state in retained]
    refs = [weakref.ref(state) for state in retained]
    del retained
    heuristic.batches.clear()
    gc.collect()
    assert all(reference() is None for reference in refs)
    for handle in handles:
        with pytest.raises(ValueError): value_hash(handle)


@pytest.mark.parametrize("failure_index", [1, 2])
def test_python_batch_wrapper_failure_releases_adopted_and_unadopted_handles(graph, monkeypatch, failure_index):
    import gc
    import pymimir._base as handles
    import pymimir.search as search
    from pymimir.advanced import lib, value_hash
    problem, _ = graph([(0, 2, 0), (0, 3, 0)])
    heuristic = BatchStateHeuristic(problem, {})
    start, goal = problem.initial_state, problem.goal
    allocated, released = [], []
    original_callback = search.HEURISTIC_BATCH_CALLBACK
    original_finalizer = handles._create_finalizer
    original_free = lib.mimir_free_handle
    calls = 0
    def callback(function):
        def record(handles, count, output):
            allocated.extend(handles[index] for index in range(count))
            return function(handles, count, output)
        return original_callback(record)
    def create_finalizer(*args):
        nonlocal calls
        calls += 1
        if calls == failure_index:
            args[1](*args[2:])
            raise RuntimeError("wrapper failed")
        return original_finalizer(*args)
    def release(handle):
        released.append(handle)
        original_free(handle)
    monkeypatch.setattr(search, "HEURISTIC_BATCH_CALLBACK", callback)
    monkeypatch.setattr(handles, "_create_finalizer", create_finalizer)
    monkeypatch.setattr(lib, "mimir_free_handle", release)
    with pytest.raises(RuntimeError, match="wrapper failed"):
        mm.beam(problem, heuristic, start_state=start, goal=goal)
    gc.collect()
    assert len(allocated) == 2
    assert all(released.count(handle) <= 1 for handle in allocated)
    for handle in allocated:
        with pytest.raises(ValueError): value_hash(handle)


def test_initial_goal_fast_path_handles_derived_and_negative_literals():
    domain = mm.Domain.from_pddl("""
        (define (domain derived-goal) (:requirements :strips :derived-predicates :negative-preconditions)
          (:predicates (ready) (blocked) (reachable))
          (:derived (reachable) (ready))
          (:action block :parameters () :precondition (ready) :effect (blocked)))
    """)
    problem = mm.Problem.from_pddl(domain, """
        (define (problem derived-goal) (:domain derived-goal) (:init (ready))
          (:goal (and (reachable) (not (blocked)))))
    """, generator="grounded")
    heuristic = BatchStateHeuristic(problem, {})
    result = mm.beam(problem, heuristic)
    assert result.is_solved
    assert not heuristic.batches
    assert result.statistics.setup_seconds == 0


def test_batch_override_receives_alternate_goal(graph):
    problem, _ = graph([(0, 2, 0), (2, 3, 0)])
    goal = problem.ground_condition(problem.atom("at", "n3"))
    heuristic = BatchStateHeuristic(problem, {})
    seen = []
    def evaluate(states, supplied_goal=None):
        seen.append(supplied_goal)
        return [1] * len(states)
    heuristic.evaluate_batch = evaluate
    result = mm.beam(problem, heuristic, goal=goal)
    assert result.is_solved
    assert seen == [goal]


def test_scalar_searches_keep_using_scalar_override(graph):
    problem, _ = graph([(0, 2, 0), (2, 1, 0)])
    heuristic = StateHeuristic(problem, {0: 2, 2: 1, 1: 0})
    def unexpected(*args):
        raise AssertionError("existing GBFS must retain scalar evaluation")
    heuristic.evaluate_batch = unexpected
    assert mm.gbfs(problem, heuristic).is_solved
