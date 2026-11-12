from __future__ import annotations

import gc
import weakref
from pathlib import Path

import pytest

import pymimir


EXAMPLES = Path(__file__).resolve().parents[2] / "Tests" / "Examples"


@pytest.fixture
def blocks() -> pymimir.Problem:
    base = EXAMPLES / "blocks_4"
    return pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")


def _assert_valid_plan(problem: pymimir.Problem, actions: list[pymimir.GroundAction]) -> None:
    state = problem.initial_state
    for action in actions:
        assert action.is_applicable(state), f"{action} is not applicable"
        state = action.apply(state)
    assert problem.goal.holds(state)


def test_plan_space_public_exports_are_singular() -> None:
    for name in pymimir.plan_space.__all__:
        assert name in pymimir.__all__
        assert getattr(pymimir, name) is getattr(pymimir.plan_space, name)


@pytest.fixture
def gripper() -> pymimir.Problem:
    base = EXAMPLES / "gripper"
    return pymimir.Problem.from_files(base / "domain.pddl", base / "p02.pddl", generator="grounded")


def _planned(problem: pymimir.Problem) -> pymimir.SerialPlan:
    """The plan of a state-space search as a serial plan: every action enters the empty plan at its only point."""

    result = pymimir.astar(problem, pymimir.FFHeuristic(problem))
    assert result.solution is not None
    plan = pymimir.SerialPlan.empty(problem)
    plan.apply(pymimir.SerialEdits(inserts=tuple(pymimir.SerialInsert(0, action.schema, action.arguments) for action in result.solution.plan)))
    return plan


def test_empty_serial_plan_has_one_unmet_goal_per_unsatisfied_goal_literal(blocks: pymimir.Problem) -> None:
    plan = pymimir.SerialPlan.empty(blocks)

    assert plan.problem is blocks
    assert plan.steps == ()
    assert [str(literal) for literal in plan.goal.literals] == [str(literal) for literal in blocks.goal.literals]
    assert not plan.is_solution()
    with pytest.raises(pymimir.MimirError, match="still has errors"):
        plan.to_actions()

    diagnosis = plan.diagnose()
    assert diagnosis.plan is plan
    assert not diagnosis.is_solution
    assert len(diagnosis) == 2
    assert [str(error.literal) for error in diagnosis.errors] == ["(on b2 b3)", "(on-table b1)"]
    for error in diagnosis.errors:
        assert error.kind is pymimir.SerialErrorKind.UNMET_GOAL
        assert error.step_id == -1
        assert error.culprit_step_id == -1
        assert error.candidate_indices
    stack = diagnosis.candidates[diagnosis.errors[0].candidate_indices[0]]
    assert stack == pymimir.GhostStep(blocks.domain.action("stack"), (blocks.object("b2"), blocks.object("b3")))
    assert plan.parameter_options(blocks.domain.action("stack"), 1) == blocks.all_objects
    assert [(support.producer_step_id, support.consumer_step_id) for support in diagnosis.supports] == [(-1, -1)] * 3
    assert [str(support.literal) for support in diagnosis.supports] == ["(clear b2)", "(on-table b3)", "(clear b1)"]
    assert diagnosis.unused_step_ids == ()


def test_serial_goals_with_negative_literals_round_trip(blocks: pymimir.Problem) -> None:
    on_b2_b3 = blocks.goal.literals[1]
    clear_b1 = blocks.ground_atom(blocks.domain.predicate("clear"), blocks.object("b1"))
    goal = blocks.ground_condition(on_b2_b3, blocks.ground_literal(clear_b1, positive=False))

    plan = pymimir.SerialPlan.empty(blocks, goal=goal)

    assert plan.goal.problem is blocks
    assert [str(literal) for literal in plan.goal.literals] == ["(on b2 b3)", "(not (clear b1))"]
    errors = plan.diagnose().errors
    assert [str(error.literal) for error in errors] == ["(on b2 b3)", "(not (clear b1))"]
    assert errors[1].literal.is_negative
    relabeled = pymimir.SerialPlan.empty(blocks).with_goal(plan.goal)
    assert [str(literal) for literal in relabeled.goal.literals] == ["(on b2 b3)", "(not (clear b1))"]
    assert pymimir.SerialPlan.empty(blocks, goal=blocks.ground_condition()).is_solution()

    base = EXAMPLES / "blocks_4"
    other = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.SerialPlan.empty(blocks, goal=other.goal)
    with pytest.raises(ValueError, match="different Problem"):
        plan.with_goal(other.goal)
    with pytest.raises(TypeError):
        pymimir.SerialPlan.empty(blocks, goal=blocks.goal.literals)  # type: ignore[arg-type]


def test_serial_plan_of_a_planner_plan_is_a_solution(blocks: pymimir.Problem) -> None:
    plan = _planned(blocks)

    assert plan.is_solution()
    assert plan.diagnose().is_solution
    actions = plan.to_actions()
    assert all(action.problem is blocks for action in actions)
    _assert_valid_plan(blocks, actions)
    # The steps that enter one point keep the order they were given in.
    steps = plan.steps
    assert [step.id for step in steps] == list(range(len(steps)))
    assert [(step.schema, step.binding) for step in steps] == [(action.schema, action.arguments) for action in actions]
    assert [str(action) for action in plan.clone().to_actions()] == [str(action) for action in actions]
    for index, action in enumerate(actions):
        assert {str(literal) for literal in plan.effects(index)} == {str(literal) for literal in action.effect.literals}
        assert {str(literal) for literal in plan.preconditions(index)} == {str(literal) for literal in action.precondition.literals}


def test_serial_edits_insert_rebind_move_and_delete_in_one_round(blocks: pymimir.Problem) -> None:
    b1, b2, b3 = (blocks.object(name) for name in ("b1", "b2", "b3"))
    pickup, stack = blocks.domain.action("pickup"), blocks.domain.action("stack")
    plan = pymimir.SerialPlan.empty(blocks)
    plan.apply(pymimir.SerialEdits(inserts=(pymimir.SerialInsert(0, stack, (b2, b3)), pymimir.SerialInsert(0, pickup, (b1,)))))
    assert [(step.id, step.schema.name, step.binding) for step in plan.steps] == [(0, "stack", (b2, b3)), (1, "pickup", (b1,))]
    # The stack fails because nothing holds b2, and nothing ever did.
    error = next(error for error in plan.diagnose().errors if str(error.literal) == "(holding b2)")
    assert (error.step_id, error.culprit_step_id) == (0, -1)

    # The pickup takes b2 and goes to the front; a putdown is inserted at the end and the stack is deleted.
    plan.apply(pymimir.SerialEdits(
        inserts=(pymimir.SerialInsert(2, blocks.domain.action("putdown"), (b1,)),),
        deleted_step_ids=(0,),
        moves=(pymimir.SerialMove(1, 0),),
        rebinds=(pymimir.SerialRebind(1, (b2,)),),
    ))
    assert [(step.id, step.schema.name, step.binding) for step in plan.steps] == [(1, "pickup", (b2,)), (2, "putdown", (b1,))]

    before = plan.steps
    with pytest.raises(ValueError, match="not part of the plan"):
        plan.apply(pymimir.SerialEdits(deleted_step_ids=(7,)))
    with pytest.raises(ValueError, match="insertion points"):
        plan.apply(pymimir.SerialEdits(moves=(pymimir.SerialMove(1, 3),)))
    with pytest.raises(ValueError):
        plan.apply(pymimir.SerialEdits(deleted_step_ids=(1,), moves=(pymimir.SerialMove(1, 0),)))
    with pytest.raises(ValueError):
        plan.apply(pymimir.SerialEdits(inserts=(pymimir.SerialInsert(0, stack, (b1,)),)))
    with pytest.raises(TypeError):
        plan.apply((pymimir.SerialMove(1, 0),))  # type: ignore[arg-type]
    assert plan.steps == before
    plan.apply(pymimir.SerialEdits())
    assert plan.steps == before


def test_serial_prune_to_achieved_goal_keeps_the_solved_part_of_a_plan(gripper: pymimir.Problem) -> None:
    reference = _planned(gripper)
    goal_literals = [str(literal) for literal in gripper.goal.literals]
    assert [str(literal) for literal in reference.prune_to_achieved_goal().goal.literals] == goal_literals

    # Without its last step the plan leaves a goal literal unmet.
    plan = reference.clone()
    plan.apply(pymimir.SerialEdits(deleted_step_ids=(plan.steps[-1].id,)))
    assert not plan.is_solution()
    pruned = plan.prune_to_achieved_goal()

    assert pruned.is_solution()
    assert pruned.problem is gripper
    achieved = [str(literal) for literal in pruned.goal.literals]
    assert set(achieved) < set(goal_literals)
    assert 0 < len(pruned.steps) < len(reference.steps)
    state = gripper.initial_state
    for action in pruned.to_actions():
        assert action.is_applicable(state)
        state = action.apply(state)
    assert pruned.goal.holds(state)

    reopened = pruned.with_goal(gripper.goal)
    errors = reopened.diagnose().errors
    assert {str(error.literal) for error in errors} == set(goal_literals) - set(achieved)
    assert all(error.kind is pymimir.SerialErrorKind.UNMET_GOAL for error in errors)
    assert [step.id for step in reopened.steps] == [step.id for step in pruned.steps]
    assert pruned.is_solution()


def test_serial_final_atoms_are_the_true_fluent_atoms_after_the_last_step(blocks: pymimir.Problem) -> None:
    empty = pymimir.SerialPlan.empty(blocks)
    assert set(empty.diagnose().final_atoms) == set(blocks.initial_state.fluent_atoms)

    reference = _planned(blocks)
    state = blocks.initial_state
    for action in reference.to_actions():
        state = action.apply(state)
    diagnosis = reference.diagnose()
    assert set(diagnosis.final_atoms) == set(state.fluent_atoms)
    # The atoms are read once.
    assert diagnosis.final_atoms is diagnosis.final_atoms


def test_serial_plan_and_diagnosis_outlive_their_problem_reference_and_free_handles(monkeypatch: pytest.MonkeyPatch) -> None:
    from pymimir import plan_space

    freed: list[int] = []
    original_free = plan_space.free_handle

    def tracked_free(handle: int) -> None:
        freed.append(handle)
        original_free(handle)

    monkeypatch.setattr(plan_space, "free_handle", tracked_free)
    base = EXAMPLES / "blocks_4"
    problem = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    plan = pymimir.SerialPlan.empty(problem)
    diagnosis = plan.diagnose()
    problem_ref = weakref.ref(problem)
    del problem
    gc.collect()
    assert problem_ref() is not None
    assert plan.problem.name == "blocksworld-300"
    assert len(plan.diagnose().errors) == 2

    plan_handle = plan._handle
    diagnosis_handle = diagnosis._handle
    errors = diagnosis.errors
    del diagnosis
    gc.collect()
    assert freed.count(diagnosis_handle) == 1
    assert freed.count(plan_handle) == 0
    # The snapshot owns its literals, so it stays readable after the native diagnosis is gone.
    assert str(errors[0].literal) == "(on b2 b3)"
    del errors, plan
    gc.collect()
    assert freed.count(plan_handle) == 1
    gc.collect()
    assert problem_ref() is None

    with pytest.raises(TypeError):
        pymimir.SerialPlan()
    with pytest.raises(TypeError):
        pymimir.SerialDiagnosis()
