import gc
import math
from pathlib import Path
import weakref

import pytest

import pymimir as mm
from pymimir.advanced import lib, value_hash


@pytest.fixture(params=["grounded", "lifted"])
def graph(request):
    def create(edges, goal=1):
        nodes = sorted({0, goal} | {node for source, target, _ in edges for node in (source, target)})
        actions = "\n".join(
            f"""(:action edge{index} :parameters () :precondition (at n{source})
                 :effect (and (not (at n{source})) (at n{target})))"""
            for index, (source, target, _) in enumerate(edges)
        )
        domain = mm.Domain.from_pddl(f"""
            (define (domain graph) (:requirements :strips)
              (:constants {' '.join(f'n{node}' for node in nodes)})
              (:predicates (at ?x)) {actions})
        """)
        problem = mm.Problem.from_pddl(domain, f"""
            (define (problem graph) (:domain graph)
              (:init (at n0)) (:goal (at n{goal})))
        """, generator=request.param)
        return problem, TableHeuristic(problem, [value for _, _, value in edges])
    return create


class TableHeuristic(mm.QHeuristic):
    def __init__(self, problem, values):
        super().__init__(problem)
        self.values = values
        self.rows = []
        self.batch_sizes = []

    def evaluate(self, state, successors, goal=None):
        assert tuple(action for action, _ in successors) == state.applicable_actions()
        self.rows.append(tuple(str(action) for action, _ in successors))
        return [self.values[int(action.schema.name.removeprefix("edge"))] for action, _ in successors]

    def evaluate_batch(self, expansions, goal=None):
        self.batch_sizes.append(len(expansions))
        return super().evaluate_batch(expansions, goal)


def plan(result):
    return [str(action) for action in result.solution.plan]


def test_global_priority_and_unevaluated_goal_action(graph):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 1), (2, 1, -100)])
    result = mm.qgbfs(problem, heuristic)
    assert plan(result) == ["(edge0)", "(edge2)"]
    assert result.action_values == (10, None)
    assert result.solution.goal_state.holds(problem.goal)
    assert result.statistics.evaluated_candidates == 2


def test_fifo_ties(graph):
    values = (5, 5)
    problem, heuristic = graph([(0, 2, values[0]), (0, 3, values[1]), (2, 1, -1), (3, 1, -2)])
    first = problem.initial_state.applicable_actions()[0]
    result = mm.qgbfs(problem, heuristic)
    assert result.solution.plan[0] == first


@pytest.mark.parametrize("maximize,expected", [(True, "(edge1)"), (False, "(edge0)")])
def test_priority_direction(graph, maximize, expected):
    problem, heuristic = graph([(0, 2, -5), (0, 3, 10), (0, 4, 0), (2, 1, 0), (3, 1, 0), (4, 1, 0)])
    assert plan(mm.qgbfs(problem, heuristic, maximize=maximize))[0] == expected


def test_best_duplicate_in_batch_and_complete_rows(graph):
    problem, heuristic = graph([(0, 2, 5), (0, 3, 5), (2, 4, 1), (3, 4, 10), (4, 1, -1), (3, 0, 100)])
    result = mm.qgbfs(problem, heuristic, batch_target=2)
    assert plan(result) == ["(edge1)", "(edge3)", "(edge4)"]
    assert result.action_values == (5, 10, None)
    assert result.statistics.visited_states == 5
    assert result.statistics.expanded_states == 4
    assert result.statistics.generated_transitions == 6
    assert result.statistics.evaluated_candidates == 5
    assert 2 in heuristic.batch_sizes
    assert any("(edge5)" in row for row in heuristic.rows)


def test_no_reopening_or_scoring_already_reached_rows(graph):
    problem, heuristic = graph([(0, 2, 3), (2, 0, 100)])
    result = mm.qgbfs(problem, heuristic)
    assert result.status is mm.SearchStatus.EXHAUSTED
    assert result.partial_plan == ()
    assert result.action_values == ()
    assert result.statistics.visited_states == 2
    assert result.statistics.generated_transitions == 2
    assert result.statistics.evaluated_candidates == 1
    assert heuristic.rows == [("(edge0)",)]


def test_goal_returns_before_inference_and_stop_callback(graph):
    problem, heuristic = graph([(0, 1, -10), (0, 2, 100)])
    def stop(_):
        pytest.fail("goal must precede the batch stop callback")
    heuristic.evaluate_batch = lambda *args: pytest.fail("goal must precede inference")
    result = mm.qgbfs(problem, heuristic, should_stop=stop)
    assert plan(result) == ["(edge0)"]
    assert result.action_values == (None,)
    assert result.statistics.generated_transitions == 1
    assert result.statistics.evaluated_candidates == 0


def test_initial_goal_and_dead_end(graph):
    problem, heuristic = graph([], goal=0)
    result = mm.qgbfs(problem, heuristic)
    assert plan(result) == []
    assert result.statistics.visited_states == 1
    assert result.statistics.expanded_states == 0
    assert result.statistics.generated_transitions == 0
    assert result.statistics.evaluated_candidates == 0
    assert heuristic.batch_sizes == []
    problem, heuristic = graph([])
    result = mm.qgbfs(problem, heuristic)
    assert result.status is mm.SearchStatus.EXHAUSTED
    assert result.statistics.expanded_states == 1
    assert result.statistics.evaluated_candidates == 0


def test_stop_and_expansion_limit_return_scored_partial_path(graph):
    problem, heuristic = graph([(0, 2, 5), (2, 1, -1)])
    calls = []
    result = mm.qgbfs(problem, heuristic, should_stop=lambda expanded: calls.append(expanded) or True)
    assert result.status is mm.SearchStatus.STOPPED
    assert result.solution is None
    assert [str(action) for action in result.partial_plan] == ["(edge0)"]
    assert result.action_values == (5,)
    assert result.statistics.evaluated_candidates == 1
    assert calls == [1]
    result = mm.qgbfs(problem, heuristic, max_expanded_states=1)
    assert result.status is mm.SearchStatus.EXPANSION_LIMIT_REACHED
    assert [str(action) for action in result.partial_plan] == ["(edge0)"]
    result = mm.qgbfs(problem, heuristic, timeout_seconds=0)
    assert result.status is mm.SearchStatus.TIMED_OUT
    assert result.statistics.expanded_states == 0


@pytest.mark.parametrize("failure", ["rows", "values", "type", "exception", "stop"])
def test_callback_errors_are_propagated(graph, failure):
    problem, heuristic = graph([(0, 2, 1), (0, 3, 1)])
    error = RuntimeError("evaluation failed")
    def evaluate(*args):
        if failure == "rows": return []
        if failure == "values": return [[1]]
        if failure == "type": return [[True, 1]]
        raise error
    if failure != "stop":
        heuristic.evaluate_batch = evaluate
    def stop(_):
        raise error
    with pytest.raises((ValueError, TypeError, RuntimeError)) as caught:
        mm.qgbfs(problem, heuristic, should_stop=stop if failure == "stop" else None)
    if failure in ("exception", "stop"):
        assert caught.value is error


def test_bulk_callback_avoids_python_expansion_and_per_element_getters(graph, monkeypatch):
    problem, heuristic = graph([(0, 2, 5), (0, 3, 1), (2, 1, 0)])
    goal = problem.goal
    def unexpected(*args):
        pytest.fail("native Q-GBFS must transport already generated transitions")
    for name in ("mimir_transition_list_get_action", "mimir_transition_list_get_state",
                 "mimir_ground_action_apply", "mimir_state_generate_successors", "mimir_value_equals"):
        monkeypatch.setattr(lib, name, unexpected)
    monkeypatch.setattr(mm.State, "successor_states", unexpected)
    monkeypatch.setattr(mm.GroundAction, "apply", unexpected)
    # Avoid property lookups in the evaluator to isolate the transport contract.
    heuristic.evaluate = lambda state, successors, goal=None: [0.0] * len(successors)
    assert mm.qgbfs(problem, heuristic, goal=goal).is_solved


def test_callback_objects_can_outlive_batch_and_are_released(graph):
    problem, heuristic = graph([(0, 2, 5)])
    retained = []
    def evaluate(expansions, goal):
        retained.extend(expansions)
        return [[0] * len(successors) for _, successors in expansions]
    heuristic.evaluate_batch = evaluate
    mm.qgbfs(problem, heuristic)
    parent, successors = retained[0]
    action, successor = successors[0]
    assert action.apply(parent) == successor
    handles = [parent._handle, action._handle, successor._handle]
    refs = [weakref.ref(value) for value in (parent, action, successor)]
    del parent, successors, action, successor
    retained.clear()
    gc.collect()
    assert all(reference() is None for reference in refs)
    for handle in handles:
        with pytest.raises(ValueError):
            value_hash(handle)


@pytest.mark.parametrize("failure_index", [1, 2, 3, 4])
def test_wrapper_failure_releases_adopted_and_unadopted_handles(graph, monkeypatch, failure_index):
    import pymimir.model as model
    import pymimir.q_search as q_search
    problem, heuristic = graph([(0, 2, 1), (0, 3, 1)])
    start, goal = problem.initial_state, problem.goal
    allocated, released = [], []
    original_callback = q_search.QHEURISTIC_CALLBACK
    original_finalizer = model._create_finalizer
    original_free = lib.mimir_free_handle
    calls = 0
    def callback(function):
        def record(handles, offsets, count, output):
            allocated.extend(handles[index] for index in range(count + 2 * offsets[count]))
            return function(handles, offsets, count, output)
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
    monkeypatch.setattr(q_search, "QHEURISTIC_CALLBACK", callback)
    monkeypatch.setattr(model, "_create_finalizer", create_finalizer)
    monkeypatch.setattr(lib, "mimir_free_handle", release)
    with pytest.raises(RuntimeError, match="wrapper failed"):
        mm.qgbfs(problem, heuristic, start_state=start, goal=goal)
    gc.collect()
    assert len(allocated) == 5
    assert all(released.count(handle) <= 1 for handle in allocated)
    for handle in allocated:
        with pytest.raises(ValueError): value_hash(handle)


def test_builtin_state_batches_match_scalar_values(monkeypatch):
    examples = Path(__file__).resolve().parents[2] / "Tests" / "Examples" / "blocks_3"
    problem = mm.Problem.from_files(examples / "domain.pddl", examples / "p01.pddl", generator="grounded")
    states = [problem.initial_state]
    states.extend(successor for _, successor in states[0].successor_states())
    goal = problem.ground_condition()
    for kind in (mm.BlindHeuristic, mm.GoalCountHeuristic, mm.FFHeuristic, mm.LiftedFFHeuristic,
                 mm.AddHeuristic, mm.MaxHeuristic, mm.H2Heuristic, mm.SetAddHeuristic, mm.PerfectHeuristic):
        heuristic = kind(problem)
        for query_goal in (None, goal):
            expected = tuple(heuristic.evaluate(state, query_goal) for state in states)
            assert heuristic.evaluate_batch(states, query_goal) == expected
            assert heuristic.evaluate_batch([], query_goal) == ()
    heuristic = mm.GoalCountHeuristic(problem)
    monkeypatch.setattr(lib, "mimir_heuristic_evaluate", lambda *args: pytest.fail("scalar native call"))
    assert len(heuristic.evaluate_batch(states)) == len(states)


def test_python_state_batch_default(graph):
    problem, _ = graph([(0, 2, 0)])
    class Heuristic(mm.heuristics.Heuristic):
        def evaluate(self, state, goal=None):
            return len(state.fluent_atoms)
    states = [problem.initial_state, problem.initial_state.successor_states()[0][1]]
    assert Heuristic(problem).evaluate_batch(states) == [1, 1]


def test_alternate_start_and_goal_and_ownership(graph):
    problem, heuristic = graph([(0, 2, 4), (2, 1, -2)])
    start = problem.initial_state.successor_states()[0][1]
    result = mm.qgbfs(problem, heuristic, start_state=start)
    assert plan(result) == ["(edge1)"]
    assert result.action_values == (None,)
    assert mm.qgbfs(problem, heuristic, goal=problem.ground_condition()).is_solved
    other, _ = graph([(0, 1, 0)])
    with pytest.raises(ValueError, match="different problem"):
        mm.qgbfs(other, heuristic)
    with pytest.raises(ValueError, match="different problem"):
        mm.qgbfs(problem, heuristic, start_state=other.initial_state)
    with pytest.raises(ValueError, match="different problem"):
        mm.qgbfs(problem, heuristic, goal=other.goal)
    with pytest.raises(ValueError, match="different problems"):
        mm.GoalCountHeuristic(problem).evaluate_batch([other.initial_state])


@pytest.mark.parametrize("kwargs", [
    {"batch_target": 0}, {"batch_target": True}, {"maximize": 1},
    {"timeout_seconds": -1}, {"max_expanded_states": -1},
    {"should_stop": 1}, {"should_stop": lambda _: "yes"},
])
def test_invalid_search_arguments_fail_explicitly(graph, kwargs):
    problem, heuristic = graph([(0, 2, 0)])
    with pytest.raises((ValueError, TypeError)):
        mm.qgbfs(problem, heuristic, **kwargs)


@pytest.mark.parametrize("failure_index", [1, 2])
def test_result_wrapping_failure_releases_path_handles(graph, monkeypatch, failure_index):
    import pymimir.model as model
    problem, heuristic = graph([(0, 1, 5)])
    allocated, released = [], []
    original_copy = lib.mimir_plan_result_copy_scored_path
    original_free = lib.mimir_free_handle
    original_finalizer = model._create_finalizer
    wrappers = 0
    def copy(*args):
        count = original_copy(*args)
        allocated.extend(args[1])
        return count
    def release(handle):
        released.append(handle)
        original_free(handle)
    def finalizer(*args):
        nonlocal wrappers
        if allocated:
            wrappers += 1
            if wrappers == failure_index:
                args[1](*args[2:])
                raise RuntimeError("path wrapper failed")
        return original_finalizer(*args)
    monkeypatch.setattr(lib, "mimir_plan_result_copy_scored_path", copy)
    monkeypatch.setattr(lib, "mimir_free_handle", release)
    monkeypatch.setattr(model, "_create_finalizer", finalizer)
    with pytest.raises(RuntimeError, match="path wrapper failed"):
        mm.qgbfs(problem, heuristic)
    gc.collect()
    assert len(allocated) == 2
    assert all(released.count(handle) == 1 for handle in allocated)


@pytest.mark.parametrize("maximize", [True, False])
def test_better_priority_replaces_path_across_batches(graph, maximize):
    edges = [(0, 2, 10), (0, 3, 5), (0, 4, 1), (2, 4, 9),
             (2, 5, -5), (3, 1, 0), (4, 1, 0)]
    if not maximize:
        edges = [(source, target, -value) for source, target, value in edges]
    problem, heuristic = graph(edges)
    result = mm.qgbfs(problem, heuristic, maximize=maximize)
    assert plan(result) == ["(edge0)", "(edge3)", "(edge6)"]
    assert result.action_values == ((10, 9, None) if maximize else (-10, -9, None))
    assert result.statistics.expanded_states == 3
    assert result.solution.cost == 3


def test_priority_update_preserves_original_discovery_order(graph):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 1), (0, 4, 5), (2, 3, 5),
                                (2, 5, -1), (3, 1, 0), (4, 1, 0)])
    result = mm.qgbfs(problem, heuristic)
    assert plan(result) == ["(edge0)", "(edge3)", "(edge5)"]


def test_equal_priority_keeps_original_path(graph):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 5), (2, 3, 5), (2, 4, -1), (3, 1, 0)])
    result = mm.qgbfs(problem, heuristic)
    assert plan(result) == ["(edge1)", "(edge4)"]
    assert result.action_values == (5, None)


@pytest.mark.parametrize("invalid", [math.nan, math.inf, -math.inf])
@pytest.mark.parametrize("duplicate_target", [0, 2])
def test_nonfinite_scores_rejected_even_for_discarded_successors(graph, invalid, duplicate_target):
    problem, heuristic = graph([(0, 2, 1), (0, duplicate_target, invalid)])
    with pytest.raises(ValueError, match="row 0, action 1 must be finite"):
        mm.qgbfs(problem, heuristic)


def test_expansion_limit_finishes_pending_batch(graph):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 9), (2, 4, 100), (4, 1, 0)])
    result = mm.qgbfs(problem, heuristic, batch_target=3, max_expanded_states=2)
    assert result.status is mm.SearchStatus.EXPANSION_LIMIT_REACHED
    assert [str(action) for action in result.partial_plan] == ["(edge0)", "(edge2)"]
    assert result.action_values == (10, 100)
    assert result.statistics.expanded_states == 2
    assert result.statistics.evaluated_candidates == 3
    assert result.statistics.max_depth == 2


def test_goal_skips_other_pending_expansions(graph):
    problem, heuristic = graph([(0, 2, 10), (0, 3, 9), (2, 4, 0), (3, 1, 0)])
    original = heuristic.evaluate_batch
    def evaluate(expansions, goal):
        assert not heuristic.rows, "pending non-goal rows must not be evaluated after finding a goal"
        return original(expansions, goal)
    heuristic.evaluate_batch = evaluate
    result = mm.qgbfs(problem, heuristic, batch_target=3)
    assert plan(result) == ["(edge1)", "(edge3)"]
    assert result.action_values == (9, None)
    assert result.statistics.evaluated_candidates == 2
    assert result.statistics.generated_transitions == 4


def test_full_rows_retain_duplicates_and_closed_successors(graph):
    problem, heuristic = graph([(0, 2, 1), (0, 2, 2), (0, 0, 3), (2, 1, 0)])
    def evaluate(state, successors, goal):
        assert tuple(action for action, _ in successors) == state.applicable_actions()
        assert len(successors) == 3
        assert successors[0][1] == successors[1][1]
        assert successors[2][1] == state
        # Each score depends on its siblings being present.
        return [0, len(successors), -1]
    heuristic.evaluate = evaluate
    result = mm.qgbfs(problem, heuristic)
    assert plan(result) == ["(edge1)", "(edge3)"]
    assert result.action_values == (3, None)


def test_zero_expansion_limit_does_not_evaluate_or_call_stop(graph):
    problem, heuristic = graph([(0, 2, 1)])
    def unexpected(*args):
        pytest.fail("zero expansion budget must return before doing work")
    heuristic.evaluate_batch = unexpected
    result = mm.qgbfs(problem, heuristic, max_expanded_states=0, should_stop=unexpected)
    assert result.status is mm.SearchStatus.EXPANSION_LIMIT_REACHED
    assert result.statistics.expanded_states == 0
    assert result.statistics.evaluated_candidates == 0
    assert result.partial_plan == ()
