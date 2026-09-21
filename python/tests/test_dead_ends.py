import gc
import math

import pytest

import pymimir as mm


@pytest.fixture
def problem():
    domain = mm.Domain.from_pddl("""
        (define (domain detection) (:requirements :strips)
          (:predicates (ready) (safe) (dead) (goal))
          (:action lose :parameters () :precondition (ready) :effect (and (not (ready)) (dead)))
          (:action advance :parameters () :precondition (ready) :effect (and (not (ready)) (safe)))
          (:action loop :parameters () :precondition (dead) :effect (dead))
          (:action finish :parameters () :precondition (safe) :effect (goal)))
    """)
    return mm.Problem.from_pddl(domain, """
        (define (problem p) (:domain detection) (:init (ready)) (:goal (goal)))
    """, generator="grounded")


def test_detection_is_goal_relative_and_reuses_h2(problem):
    initial = problem.initial_state
    lose = next(action for action in initial.applicable_actions() if action.schema.name == "lose")
    dead = lose.apply(initial)
    detector = mm.H2DeadEndDetector(problem)
    heuristic = mm.H2Heuristic(problem)
    hindsight = problem.ground_condition(problem.fact("dead"))

    assert detector.is_dead_end(dead, problem.goal)
    assert not detector.is_dead_end(dead, hindsight)
    assert not detector.is_dead_end(initial, problem.goal)
    assert heuristic.evaluate(dead) == math.inf
    assert heuristic.evaluate(initial) == 2
    assert heuristic.evaluate_batch([dead, initial, dead]) == (math.inf, 2, math.inf)


def test_disjunction_owns_children_and_empty_returns_false(problem):
    initial = problem.initial_state
    dead = next(action for action in initial.applicable_actions() if action.schema.name == "lose").apply(initial)
    children = [mm.H2DeadEndDetector(problem)]
    detector = mm.DisjunctiveDeadEndDetector(children)
    children.clear()
    gc.collect()
    assert detector.is_dead_end(dead, problem.goal)
    assert not mm.DisjunctiveDeadEndDetector([]).is_dead_end(dead, problem.goal)
    with pytest.raises(TypeError):
        mm.DisjunctiveDeadEndDetector([object()])


@pytest.mark.parametrize("search", [mm.bfs, mm.ucs, mm.astar, mm.gbfs, mm.iw, mm.beam])
def test_search_accepts_detector(problem, search):
    args = [problem]
    if search in (mm.astar, mm.gbfs, mm.beam):
        args.append(mm.BlindHeuristic(problem))
    result = search(*args, dead_end_detector=mm.H2DeadEndDetector(problem))
    assert result.is_solved
    assert [action.schema.name for action in result.solution.plan] == ["advance", "finish"]


@pytest.mark.parametrize("search", [mm.qgbfs, mm.qbeam])
@pytest.mark.parametrize("maximize", [True, False])
def test_q_model_scores_full_row_then_dead_end_is_masked(problem, search, maximize):
    class Model(mm.QHeuristic):
        def __init__(self):
            super().__init__(problem)
            self.rows = []

        def evaluate(self, state, successors, goal=None):
            actions = tuple(action for action, _ in successors)
            assert actions == state.applicable_actions()
            self.rows.append([action.schema.name for action in actions])
            assert self.rows[-1] == ["lose", "advance"]
            assert successors[0][1].holds(problem.fact("dead"))
            return [100 if maximize else -100, 0]

    model = Model()
    result = search(problem, model, maximize=maximize, dead_end_detector=mm.H2DeadEndDetector(problem))
    assert result.is_solved
    assert model.rows == [["lose", "advance"]]
    assert result.statistics.evaluated_candidates == 2
    assert [action.schema.name for action in result.solution.plan] == ["advance", "finish"]


def test_beam_scores_dead_end_before_masking(problem):
    class Model(mm.heuristics.Heuristic):
        def __init__(self):
            super().__init__(problem)
            self.scored = []

        def evaluate(self, state, goal=None):
            self.scored.append(state)
            return 0 if state.holds(problem.fact("dead")) else 10

    model = Model()
    result = mm.beam(problem, model, beam_size=1, dead_end_detector=mm.H2DeadEndDetector(problem))
    assert result.is_solved
    assert len(model.scored) == 2
    assert any(state.holds(problem.fact("dead")) for state in model.scored)


def test_invalid_inputs_are_not_dead_end_labels(problem):
    detector = mm.H2DeadEndDetector(problem)
    other = mm.Problem.from_pddl(problem.domain,
        "(define (problem other) (:domain detection) (:init (ready)) (:goal (goal)))")
    with pytest.raises(ValueError):
        detector.is_dead_end(other.initial_state, other.goal)
    with pytest.raises(ValueError):
        mm.bfs(other, dead_end_detector=detector)
    with pytest.raises(TypeError):
        mm.bfs(problem, dead_end_detector=object())
    with pytest.raises(TypeError):
        detector.is_dead_end(problem.initial_state, None)
