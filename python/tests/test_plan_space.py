from __future__ import annotations

import gc
import weakref
from pathlib import Path

import pytest

import pymimir


EXAMPLES = Path(__file__).resolve().parents[2] / "Tests" / "Examples"


def _rows(values: list[int], arity: int) -> set[tuple[int, ...]]:
    assert arity > 0
    assert len(values) % arity == 0
    return {tuple(values[offset:offset + arity]) for offset in range(0, len(values), arity)}


@pytest.fixture
def blocks() -> pymimir.Problem:
    base = EXAMPLES / "blocks_4"
    return pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")


def _reference_plan(problem: pymimir.Problem) -> pymimir.PartialOrderPlan:
    result = pymimir.bfs(problem)
    assert result.solution is not None
    return pymimir.PartialOrderPlan.from_sequential_plan(problem, result.solution.plan)


def _assert_valid_plan(problem: pymimir.Problem, actions: list[pymimir.GroundAction]) -> None:
    state = problem.initial_state
    for action in actions:
        assert action.is_applicable(state), f"{action} is not applicable"
        state = action.apply(state)
    assert problem.goal.holds(state)


def test_plan_space_public_exports_are_singular() -> None:
    assert pymimir.PartialOrderPlan is pymimir.plan_space.PartialOrderPlan
    assert pymimir.FlawSet is pymimir.plan_space.FlawSet
    assert pymimir.oracle_targets is pymimir.plan_space.oracle_targets
    assert pymimir.encode_partial_plan is pymimir.plan_space.encode_partial_plan
    assert pymimir.partial_plan_relations is pymimir.plan_space.partial_plan_relations
    for name in pymimir.plan_space.__all__:
        assert name in pymimir.__all__
        assert getattr(pymimir, name) is getattr(pymimir.plan_space, name)


def test_empty_plan_has_one_open_condition_per_goal_atom(blocks: pymimir.Problem) -> None:
    plan = pymimir.PartialOrderPlan.empty(blocks)

    assert plan.problem is blocks
    assert plan.round == 0
    steps = plan.steps
    assert [step.kind for step in steps] == [pymimir.StepKind.INITIAL, pymimir.StepKind.GOAL]
    assert all(step.schema is None and step.binding == () and step.is_fully_bound for step in steps)
    assert plan.links == ()
    assert plan.tombstones == ()
    assert plan.is_before(0, 1)
    assert not plan.is_before(1, 0)
    assert plan.ordering_closure() == ((0, 1),)
    assert not plan.is_solution()
    with pytest.raises(pymimir.MimirError):
        plan.linearize()

    flaws = plan.compute_flaws()
    assert flaws.plan is plan
    assert len(flaws.flaws) == len(blocks.goal.literals)
    goal_atoms = {literal.atom for literal in blocks.goal.literals}
    assert {flaw.atom for flaw in flaws.flaws} == goal_atoms
    for flaw in flaws.flaws:
        assert flaw.kind is pymimir.FlawKind.OPEN_CONDITION
        assert flaw.step_id == 1
        assert flaw.link_id == -1
        assert flaw.parameter_index == -1
        assert flaw.resolvers[0].kind is pymimir.ResolverKind.NOOP
        for resolver in flaw.resolvers[1:]:
            assert resolver.kind in (pymimir.ResolverKind.LINK_EXISTING, pymimir.ResolverKind.ADOPT_GHOST)
            if resolver.kind is pymimir.ResolverKind.ADOPT_GHOST:
                ghost = flaws.ghosts[resolver.ghost_index]
                assert ghost.schema in blocks.domain.actions
                assert len(ghost.binding) == ghost.schema.arity
    assert any(
        ghost.schema.name == "unstack" and ghost.binding[0] is None and ghost.binding[1] == blocks.object("b1")
        for ghost in flaws.ghosts
    )


def test_adopting_a_ghost_creates_a_step_and_bind_resolvers_return_objects(blocks: pymimir.Problem) -> None:
    plan = pymimir.PartialOrderPlan.empty(blocks)
    flaws = plan.compute_flaws()
    clear_b1 = next(index for index, flaw in enumerate(flaws.flaws) if str(flaw.atom) == "(clear b1)")
    adopt = next(
        index
        for index, resolver in enumerate(flaws.flaws[clear_b1].resolvers)
        if resolver.kind is pymimir.ResolverKind.ADOPT_GHOST
        and flaws.ghosts[resolver.ghost_index].schema.name == "unstack"
    )
    chosen = [0] * len(flaws.flaws)
    chosen[clear_b1] = adopt

    result = plan.apply(flaws, chosen, [])

    assert result == pymimir.EditResult(1, (), (2,), ())
    assert plan.round == 1
    step = plan.steps[2]
    assert step.kind is pymimir.StepKind.STEP
    assert step.schema == blocks.domain.action("unstack")
    assert step.binding == (None, blocks.object("b1"))
    assert not step.is_fully_bound
    link = plan.links[0]
    assert (link.producer_id, link.consumer_id, str(link.atom)) == (2, 1, "(clear b1)")
    assert link.atom.predicate == blocks.domain.predicate("clear")

    flaws = plan.compute_flaws()
    unbound = next(flaw for flaw in flaws.flaws if flaw.kind is pymimir.FlawKind.UNBOUND_PARAMETER)
    assert unbound.atom is None
    assert (unbound.step_id, unbound.parameter_index) == (2, 0)
    candidates = {resolver.object for resolver in unbound.resolvers[1:]}
    assert all(resolver.kind is pymimir.ResolverKind.BIND for resolver in unbound.resolvers[1:])
    assert candidates == set(blocks.all_objects)


def test_apply_rejects_stale_or_foreign_flaw_sets(blocks: pymimir.Problem) -> None:
    plan = pymimir.PartialOrderPlan.empty(blocks)
    flaws = plan.compute_flaws()
    plan.apply(flaws, [0] * len(flaws.flaws), [])

    with pytest.raises(pymimir.MimirError, match="current state"):
        plan.apply(flaws, [0] * len(flaws.flaws), [])
    with pytest.raises(ValueError, match="different plan"):
        plan.clone().apply(flaws, [0] * len(flaws.flaws), [])
    fresh = plan.compute_flaws()
    with pytest.raises(ValueError):
        plan.apply(fresh, [0] * (len(fresh.flaws) + 1), [])
    with pytest.raises(ValueError):
        plan.apply(fresh, [0] * len(fresh.flaws), [0])
    with pytest.raises(TypeError):
        plan.apply(fresh, [0.5] * len(fresh.flaws), [])  # type: ignore[list-item]


def test_oracle_rollout_from_the_empty_plan_reaches_a_valid_solution(blocks: pymimir.Problem) -> None:
    reference = _reference_plan(blocks)
    assert reference.is_solution()
    assert len(reference.steps) == len(reference.linearize()) + 2
    current = pymimir.PartialOrderPlan.empty(blocks)

    rounds = 0
    while not current.is_solution():
        assert rounds < 40
        flaws = current.compute_flaws()
        targets = pymimir.oracle_targets(current, flaws, reference)
        assert len(targets.resolver_index_per_flaw) == len(flaws.flaws)
        current.apply(flaws, targets.resolver_index_per_flaw, targets.step_ids_to_delete)
        rounds += 1

    assert rounds > 0
    assert current.round == rounds
    linearized = current.linearize()
    assert all(isinstance(action, pymimir.GroundAction) for action in linearized)
    assert all(action.problem is blocks for action in linearized)
    _assert_valid_plan(blocks, linearized)
    assert len(current.steps) == len(reference.steps)
    clone = current.clone()
    assert clone.is_solution()
    assert clone.ordering_closure() == current.ordering_closure()
    assert [str(action) for action in clone.linearize()] == [str(action) for action in linearized]


def test_oracle_requests_deletion_of_spurious_steps_and_tombstones_record_them(blocks: pymimir.Problem) -> None:
    reference = _reference_plan(blocks)
    current = pymimir.PartialOrderPlan.empty(blocks, tombstone_capacity=4, tombstone_lifespan=3)
    flaws = current.compute_flaws()
    clear_b2 = next(index for index, flaw in enumerate(flaws.flaws) if str(flaw.atom) == "(clear b2)")
    # (putdown b2) never occurs in the optimal reference plan, so the oracle must ask to delete it.
    putdown = next(
        index
        for index, resolver in enumerate(flaws.flaws[clear_b2].resolvers)
        if resolver.kind is pymimir.ResolverKind.ADOPT_GHOST and flaws.ghosts[resolver.ghost_index].schema.name == "putdown"
    )
    chosen = [0] * len(flaws.flaws)
    chosen[clear_b2] = putdown
    created = current.apply(flaws, chosen, []).created_step_ids
    assert created == (2,)

    flaws = current.compute_flaws()
    targets = pymimir.oracle_targets(current, flaws, reference)
    assert targets.step_ids_to_delete == created
    assert targets.matched_step_count == 0
    result = current.apply(flaws, targets.resolver_index_per_flaw, targets.step_ids_to_delete)
    assert result.deleted_step_ids == created
    assert 2 not in [step.id for step in current.steps]
    tombstone = current.tombstones[0]
    assert tombstone.schema == blocks.domain.action("putdown")
    assert tombstone.binding == (blocks.object("b2"),)
    assert tombstone.age == 1


def test_empty_plan_with_a_subset_goal_has_one_flaw_per_subset_atom(blocks: pymimir.Problem) -> None:
    literals = blocks.goal.literals
    subset = blocks.ground_condition(*literals[:2])

    plan = pymimir.PartialOrderPlan.empty(blocks, goal=subset)

    assert [str(literal) for literal in plan.goal.literals] == [str(literal) for literal in literals[:2]]
    assert plan.goal.problem is blocks
    assert {str(flaw.atom) for flaw in plan.compute_flaws().flaws} == {str(literal.atom) for literal in literals[:2]}
    assert pymimir.PartialOrderPlan.empty(blocks, goal=blocks.ground_condition()).is_solution()
    assert [str(literal) for literal in pymimir.PartialOrderPlan.empty(blocks).goal.literals] == [str(literal) for literal in literals]
    reference = _reference_plan(blocks)
    relabeled = pymimir.PartialOrderPlan.from_sequential_plan(blocks, reference.linearize(), goal=subset)
    assert relabeled.is_solution()
    assert len(relabeled.goal.literals) == 2

    base = EXAMPLES / "blocks_4"
    other = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.PartialOrderPlan.empty(blocks, goal=other.goal)
    with pytest.raises(TypeError):
        pymimir.PartialOrderPlan.empty(blocks, goal=literals)  # type: ignore[arg-type]
    with pytest.raises(ValueError, match="different Problem"):
        plan.with_goal(other.goal)


def test_prune_to_achieved_goal_keeps_the_solved_part_of_a_rollout(blocks: pymimir.Problem) -> None:
    reference = _reference_plan(blocks)
    assert [str(literal) for literal in reference.prune_to_achieved_goal().goal.literals] == [str(literal) for literal in blocks.goal.literals]
    current = pymimir.PartialOrderPlan.empty(blocks)
    pruned: pymimir.PartialOrderPlan | None = None
    goal_atoms = {str(literal.atom) for literal in blocks.goal.literals}
    while not current.is_solution():
        flaws = current.compute_flaws()
        targets = pymimir.oracle_targets(current, flaws, reference)
        current.apply(flaws, targets.resolver_index_per_flaw, targets.step_ids_to_delete)
        candidate = current.prune_to_achieved_goal()
        assert candidate.is_solution()
        assert candidate.problem is blocks
        achieved = {str(literal.atom) for literal in candidate.goal.literals}
        assert achieved <= goal_atoms
        state = blocks.initial_state
        for action in candidate.linearize():
            assert action.is_applicable(state)
            state = action.apply(state)
        assert candidate.goal.holds(state)
        if pruned is None and 0 < len(achieved) < len(goal_atoms):
            pruned = candidate
    assert pruned is not None

    reopened = pruned.with_goal(blocks.goal)
    missing = goal_atoms - {str(literal.atom) for literal in pruned.goal.literals}
    flaws = reopened.compute_flaws()
    assert {str(flaw.atom) for flaw in flaws.flaws} == missing
    assert all(flaw.kind is pymimir.FlawKind.OPEN_CONDITION and flaw.step_id == 1 for flaw in flaws.flaws)
    assert len(reopened.steps) == len(pruned.steps)
    assert pruned.is_solution()

    with pymimir.EncodingContext() as context:
        context.begin_instance(blocks)
        layout = pymimir.encode_partial_plan(context, pruned, pruned.compute_flaws(), suffix="_plan")
        context.end_instance()
        relations = context.to_relations()
    assert len(layout.step_node_ids) == len(pruned.steps)
    assert layout.flaw_node_ids == ()
    assert len(layout.link_node_ids) == len(pruned.links)
    assert relations["plan_goal_plan"] == [layout.step_node_ids[1]]


def test_partial_plan_encoding_layout_matches_relations(blocks: pymimir.Problem) -> None:
    reference = _reference_plan(blocks)
    plan = pymimir.PartialOrderPlan.empty(blocks)
    flaws = plan.compute_flaws()
    plan.apply(flaws, pymimir.oracle_targets(plan, flaws, reference).resolver_index_per_flaw, [])
    flaws = plan.compute_flaws()
    assert flaws.flaws
    declared = dict(pymimir.partial_plan_relations(blocks.domain, "_plan"))

    with pymimir.EncodingContext() as context:
        context.begin_instance(blocks)
        object_ids = context.object_to_id
        layout = pymimir.encode_partial_plan(context, plan, flaws, suffix="_plan")
        step_ids = context.get_action_ids()
        auxiliary_ids = context.get_auxiliary_ids()
        context.end_instance()
        relations = context.to_relations()

    steps = plan.steps
    assert len(layout.step_node_ids) == len(steps)
    assert [layout.step_node_ids[index] for index, step in enumerate(steps) if step.kind is pymimir.StepKind.STEP] == step_ids
    assert relations["plan_initial_plan"] == [layout.step_node_ids[0]]
    assert relations["plan_goal_plan"] == [layout.step_node_ids[1]]
    assert all(len(layout.parameter_node_ids[index]) == (step.schema.arity if step.schema else 0) for index, step in enumerate(steps))
    assert len(layout.link_node_ids) == len(plan.links)
    assert len(layout.ghost_node_ids) == len(flaws.ghosts)
    assert len(layout.flaw_node_ids) == len(flaws.flaws)
    assert [len(resolvers) for resolvers in layout.resolver_node_ids] == [len(flaw.resolvers) for flaw in flaws.flaws]
    assert layout.tombstone_node_ids == ()

    resolves = _rows(relations["plan_resolves_plan"], 2)
    assert resolves == {
        (resolver_node, layout.flaw_node_ids[index])
        for index, resolvers in enumerate(layout.resolver_node_ids)
        for resolver_node in resolvers
    }
    assert relations["plan_res_noop_plan"] == [resolvers[0] for resolvers in layout.resolver_node_ids]
    bound = _rows(relations["plan_bound_plan"], 2)
    for index, step in enumerate(steps):
        for parameter, value in zip(layout.parameter_node_ids[index], step.binding):
            if value is not None:
                assert (parameter, object_ids[value]) in bound
    node_by_step_id = {step.id: layout.step_node_ids[index] for index, step in enumerate(steps)}
    closure = set(plan.ordering_closure())
    reduction = {(a, b) for a, b in closure if not any((a, step.id) in closure and (step.id, b) in closure for step in steps)}
    assert reduction < closure
    assert _rows(relations["plan_before_plan"], 2) == {(node_by_step_id[a], node_by_step_id[b]) for a, b in reduction}
    actions = [step.id for step in steps if step.kind is pymimir.StepKind.STEP]
    unordered = [(a, b) for a in actions for b in actions if a < b and (a, b) not in closure and (b, a) not in closure]
    assert unordered
    assert relations["plan_unordered_plan"] == [node_by_step_id[step_id] for pair in unordered for step_id in pair]
    for index, flaw in enumerate(flaws.flaws):
        if flaw.kind is pymimir.FlawKind.OPEN_CONDITION:
            assert flaw.atom is not None
            row = (layout.flaw_node_ids[index], *(object_ids[argument] for argument in flaw.atom.arguments))
            assert row in _rows(relations[f"plan_open_{flaw.atom.predicate.name}_plan"], 1 + flaw.atom.arity)

    allocated = set(step_ids) | set(auxiliary_ids)
    for name, values in relations.items():
        assert name in declared, name
        assert values
        assert len(values) % declared[name] == 0
        assert set(values) <= allocated | set(object_ids.values())


def test_relation_list_is_derived_from_the_domain(blocks: pymimir.Problem) -> None:
    relations = pymimir.partial_plan_relations(blocks.domain)
    names = [name for name, _ in relations]
    assert len(names) == len(set(names))
    by_name = dict(relations)
    assert by_name["plan_threat"] == 3
    assert by_name["plan_before"] == 2
    assert by_name["plan_unordered"] == 2
    assert by_name["plan_action_stack"] == 3
    assert by_name["plan_tombstone_action_pickup"] == 2
    assert by_name["plan_link_on"] == 3
    assert by_name["plan_open_arm-empty"] == 1
    assert pymimir.partial_plan_relations(blocks.domain, "_x")[0] == ("plan_initial_x", 1)
    with pytest.raises(TypeError):
        pymimir.partial_plan_relations(blocks)  # type: ignore[arg-type]


def test_encoding_rejects_foreign_context_plan_and_flaws(blocks: pymimir.Problem) -> None:
    plan = pymimir.PartialOrderPlan.empty(blocks)
    flaws = plan.compute_flaws()
    context = pymimir.EncodingContext()
    with pytest.raises(RuntimeError, match="no encoding instance"):
        pymimir.encode_partial_plan(context, plan, flaws)
    context.begin_instance(blocks)
    with pytest.raises(ValueError, match="different plan"):
        pymimir.encode_partial_plan(context, plan.clone(), flaws)
    base = EXAMPLES / "blocks_4"
    other = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_partial_plan(context, pymimir.PartialOrderPlan.empty(other), pymimir.PartialOrderPlan.empty(other).compute_flaws())
    with pytest.raises(TypeError):
        pymimir.PartialOrderPlan.from_sequential_plan(blocks, [blocks])  # type: ignore[list-item]
    with pytest.raises(pymimir.MimirError):
        pymimir.PartialOrderPlan.from_sequential_plan(blocks, [blocks.action("pickup", "b1")])
    context.close()


def test_plan_and_flaws_outlive_their_problem_reference_and_free_handles(monkeypatch: pytest.MonkeyPatch) -> None:
    from pymimir import plan_space

    freed: list[int] = []
    original_free = plan_space.free_handle

    def tracked_free(handle: int) -> None:
        freed.append(handle)
        original_free(handle)

    monkeypatch.setattr(plan_space, "free_handle", tracked_free)
    base = EXAMPLES / "blocks_4"
    problem = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    plan = pymimir.PartialOrderPlan.empty(problem)
    flaws = plan.compute_flaws()
    problem_ref = weakref.ref(problem)
    del problem
    gc.collect()
    assert problem_ref() is not None
    assert plan.problem.name == "blocksworld-300"
    assert len(plan.compute_flaws().flaws) == 5

    plan_handle = plan._handle
    flaws_handle = flaws._handle
    del flaws
    gc.collect()
    assert freed.count(flaws_handle) == 1
    assert freed.count(plan_handle) == 0
    del plan
    gc.collect()
    assert freed.count(plan_handle) == 1
    gc.collect()
    assert problem_ref() is None

    with pytest.raises(TypeError):
        pymimir.PartialOrderPlan()
    with pytest.raises(TypeError):
        pymimir.FlawSet()


@pytest.fixture
def gripper() -> pymimir.Problem:
    base = EXAMPLES / "gripper"
    return pymimir.Problem.from_files(base / "domain.pddl", base / "p02.pddl", generator="grounded")


def _serial_reference(problem: pymimir.Problem) -> pymimir.SerialPlan:
    reference = pymimir.serial_search(problem)
    assert reference is not None
    return reference


def _apply_oracle(current: pymimir.SerialPlan, reference: pymimir.SerialPlan) -> pymimir.SerialEditResult:
    diagnosis = current.diagnose()
    targets = pymimir.serial_oracle_targets(current, diagnosis, reference)
    assert len(targets.repair_index_per_error) == len(diagnosis.errors)
    return current.apply(diagnosis, targets.repair_index_per_error, targets.step_ids_to_delete)


def test_empty_serial_plan_has_one_unmet_goal_per_unsatisfied_goal_literal(blocks: pymimir.Problem) -> None:
    plan = pymimir.SerialPlan.empty(blocks)

    assert plan.problem is blocks
    assert plan.round == 0
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
        assert (error.step_id, error.parameter_index) == (-1, -1)
        assert error.repairs[0] == pymimir.SerialRepair(pymimir.SerialRepairKind.NOOP, -1, -1, None)
        assert all(repair.kind is pymimir.SerialRepairKind.INSERT for repair in error.repairs[1:])
    stack = diagnosis.candidates[diagnosis.errors[0].repairs[1].candidate_index]
    assert stack == pymimir.GhostStep(blocks.domain.action("stack"), (blocks.object("b2"), blocks.object("b3")))
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
    assert errors[1].literal is not None and errors[1].literal.is_negative
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


def test_serial_search_and_oracle_rollout_reach_a_valid_solution(blocks: pymimir.Problem) -> None:
    reference = _serial_reference(blocks)
    assert reference.is_solution()
    reference_actions = reference.to_actions()
    _assert_valid_plan(blocks, reference_actions)
    current = pymimir.SerialPlan.empty(blocks)

    rounds = 0
    while not current.is_solution():
        assert rounds < 40
        _apply_oracle(current, reference)
        rounds += 1

    assert rounds > 0
    assert current.round == rounds
    assert current.diagnose().is_solution
    actions = current.to_actions()
    assert all(action.problem is blocks for action in actions)
    _assert_valid_plan(blocks, actions)
    assert [str(action) for action in actions] == [str(action) for action in reference_actions]
    steps = current.steps
    assert all(step.is_fully_bound for step in steps)
    assert [(step.schema, step.binding) for step in steps] == [(action.schema, action.arguments) for action in actions]
    assert (steps[-1].purpose_step_id, str(steps[-1].purpose)) == (-1, "(on b2 b3)")
    assert all(step.purpose_step_id in {-1, *(other.id for other in steps)} for step in steps)
    assert [str(action) for action in current.clone().to_actions()] == [str(action) for action in actions]

    assert pymimir.serial_search(blocks, max_expansions=0) is None
    with pytest.raises(TypeError):
        pymimir.serial_search(blocks, max_expansions=1.5)  # type: ignore[arg-type]
    with pytest.raises(ValueError):
        pymimir.serial_search(blocks, timeout_seconds=-1.0)


def test_serial_apply_and_oracle_reject_stale_or_foreign_diagnoses(blocks: pymimir.Problem) -> None:
    reference = _serial_reference(blocks)
    plan = pymimir.SerialPlan.empty(blocks)
    diagnosis = plan.diagnose()
    result = plan.apply(diagnosis, [0, 1], [])
    assert result == pymimir.SerialEditResult(1, (), (0,), ())
    assert plan.round == 1

    with pytest.raises(pymimir.MimirError, match="current state"):
        plan.apply(diagnosis, [0, 0], [])
    with pytest.raises(ValueError, match="current state"):
        pymimir.serial_oracle_targets(plan, diagnosis, reference)
    with pytest.raises(ValueError, match="different plan"):
        plan.clone().apply(diagnosis, [0, 0], [])
    with pytest.raises(ValueError, match="different plan"):
        pymimir.serial_oracle_targets(plan.clone(), diagnosis, reference)
    with pytest.raises(ValueError, match="not a solution"):
        pymimir.serial_oracle_targets(plan, plan.diagnose(), plan.clone())
    fresh = plan.diagnose()
    with pytest.raises(ValueError):
        plan.apply(fresh, [0] * (len(fresh.errors) + 1), [])
    with pytest.raises(ValueError):
        plan.apply(fresh, [0] * len(fresh.errors), [7])
    with pytest.raises(TypeError):
        plan.apply(fresh, [0.5] * len(fresh.errors), [])  # type: ignore[list-item]
    assert plan.apply(fresh, [0] * len(fresh.errors), [0]).deleted_step_ids == (0,)
    assert plan.steps == ()


def test_serial_prune_to_achieved_goal_keeps_the_solved_part_of_a_rollout(gripper: pymimir.Problem) -> None:
    reference = _serial_reference(gripper)
    goal_literals = [str(literal) for literal in gripper.goal.literals]
    assert [str(literal) for literal in reference.prune_to_achieved_goal().goal.literals] == goal_literals
    current = pymimir.SerialPlan.empty(gripper)
    pruned: pymimir.SerialPlan | None = None
    while not current.is_solution():
        _apply_oracle(current, reference)
        candidate = current.prune_to_achieved_goal()
        assert candidate.is_solution()
        assert candidate.problem is gripper
        achieved = [str(literal) for literal in candidate.goal.literals]
        assert set(achieved) <= set(goal_literals)
        state = gripper.initial_state
        for action in candidate.to_actions():
            assert action.is_applicable(state)
            state = action.apply(state)
        assert candidate.goal.holds(state)
        if pruned is None and 0 < len(achieved) < len(goal_literals):
            pruned = candidate
    assert pruned is not None
    assert 0 < len(pruned.steps) < len(reference.steps)

    reopened = pruned.with_goal(gripper.goal)
    missing = set(goal_literals) - {str(literal) for literal in pruned.goal.literals}
    errors = reopened.diagnose().errors
    assert {str(error.literal) for error in errors} == missing
    assert all(error.kind is pymimir.SerialErrorKind.UNMET_GOAL for error in errors)
    assert [step.id for step in reopened.steps] == [step.id for step in pruned.steps]
    assert pruned.is_solution()


def test_serial_plan_encoding_layout_matches_relations(blocks: pymimir.Problem) -> None:
    reference = _serial_reference(blocks)
    plan = pymimir.SerialPlan.empty(blocks)
    _apply_oracle(plan, reference)
    diagnosis = plan.diagnose()
    steps = plan.steps
    assert steps and diagnosis.errors and diagnosis.candidates
    declared = dict(pymimir.serial_plan_relations(blocks.domain, "_plan"))

    with pymimir.EncodingContext() as context:
        context.begin_instance(blocks)
        object_ids = context.object_to_id
        layout = pymimir.encode_serial_plan(context, plan, diagnosis, suffix="_plan")
        step_ids = context.get_action_ids()
        auxiliary_ids = context.get_auxiliary_ids()
        context.end_instance()
        relations = context.to_relations()

    assert list(layout.step_node_ids) == step_ids
    assert relations["serial_initial_plan"] == [layout.initial_node_id]
    assert relations["serial_goal_plan"] == [layout.goal_node_id]
    chain = [layout.initial_node_id, *layout.step_node_ids, layout.goal_node_id]
    assert relations["serial_next_plan"] == [node for pair in zip(chain, chain[1:]) for node in pair]
    assert [len(parameters) for parameters in layout.parameter_node_ids] == [step.schema.arity for step in steps]
    assert len(layout.candidate_node_ids) == len(diagnosis.candidates)
    assert [len(parameters) for parameters in layout.candidate_parameter_node_ids] == [
        candidate.schema.arity for candidate in diagnosis.candidates
    ]
    assert len(layout.error_node_ids) == len(diagnosis.errors)
    assert [len(repairs) for repairs in layout.repair_node_ids] == [len(error.repairs) for error in diagnosis.errors]

    assert _rows(relations["serial_repairs_plan"], 2) == {
        (repair_node, layout.error_node_ids[index])
        for index, repairs in enumerate(layout.repair_node_ids)
        for repair_node in repairs
    }
    assert relations["serial_repair_noop_plan"] == [repairs[0] for repairs in layout.repair_node_ids]
    bound = _rows(relations["serial_bound_plan"], 2)
    for index, step in enumerate(steps):
        for parameter, value in zip(layout.parameter_node_ids[index], step.binding):
            if value is not None:
                assert (parameter, object_ids[value]) in bound
    node_by_step_id = {step.id: layout.step_node_ids[index] for index, step in enumerate(steps)}
    inserts = _rows(relations["serial_repair_insert_plan"], 2)
    kinds = set()
    for index, error in enumerate(diagnosis.errors):
        kinds.add(error.kind)
        assert error.literal is not None
        atom = error.literal.atom
        arguments = tuple(object_ids[argument] for argument in atom.arguments)
        if error.kind is pymimir.SerialErrorKind.UNMET_GOAL:
            row = (layout.error_node_ids[index], *arguments)
            assert row in _rows(relations[f"serial_error_goal_{atom.predicate.name}_plan"], 1 + atom.arity)
        else:
            row = (layout.error_node_ids[index], node_by_step_id[error.step_id], *arguments)
            assert row in _rows(relations[f"serial_error_pre_{atom.predicate.name}_plan"], 2 + atom.arity)
        for repair_index, repair in enumerate(error.repairs):
            if repair.kind is pymimir.SerialRepairKind.INSERT:
                assert (layout.repair_node_ids[index][repair_index], layout.candidate_node_ids[repair.candidate_index]) in inserts
    assert kinds == {pymimir.SerialErrorKind.UNSATISFIED_PRECONDITION, pymimir.SerialErrorKind.UNMET_GOAL}

    allocated = set(step_ids) | set(auxiliary_ids)
    for name, values in relations.items():
        assert name in declared, name
        assert values
        assert len(values) % declared[name] == 0
        assert set(values) <= allocated | set(object_ids.values())


def test_serial_relation_list_is_derived_from_the_domain(blocks: pymimir.Problem) -> None:
    relations = pymimir.serial_plan_relations(blocks.domain)
    names = [name for name, _ in relations]
    assert len(names) == len(set(names))
    by_name = dict(relations)
    assert by_name["serial_next"] == 2
    assert by_name["serial_repair_rebind"] == 3
    assert by_name["serial_action_stack"] == 3
    assert by_name["serial_support_on"] == 4
    assert by_name["serial_error_pre_not_clear"] == 3
    assert by_name["serial_error_goal_on"] == 3
    assert by_name["serial_support_arm-empty"] == 2
    assert by_name["serial_final_arm-empty"] == 1
    assert by_name["serial_final_unknown_on"] == 2
    assert pymimir.serial_plan_relations(blocks.domain, "_x")[0] == ("serial_initial_x", 1)
    with pytest.raises(TypeError):
        pymimir.serial_plan_relations(blocks)  # type: ignore[arg-type]


def test_serial_encoding_rejects_foreign_context_plan_and_stale_diagnosis(blocks: pymimir.Problem) -> None:
    plan = pymimir.SerialPlan.empty(blocks)
    diagnosis = plan.diagnose()
    context = pymimir.EncodingContext()
    with pytest.raises(RuntimeError, match="no encoding instance"):
        pymimir.encode_serial_plan(context, plan, diagnosis)
    context.begin_instance(blocks)
    with pytest.raises(ValueError, match="different plan"):
        pymimir.encode_serial_plan(context, plan.clone(), diagnosis)
    base = EXAMPLES / "blocks_4"
    other = pymimir.Problem.from_files(base / "domain.pddl", base / "p01.pddl", generator="grounded")
    other_plan = pymimir.SerialPlan.empty(other)
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_serial_plan(context, other_plan, other_plan.diagnose())
    plan.apply(diagnosis, [0] * len(diagnosis.errors), [])
    with pytest.raises(ValueError, match="current state"):
        pymimir.encode_serial_plan(context, plan, diagnosis)
    context.close()


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
