from __future__ import annotations

import copy
import ctypes
import gc
import pickle
from pathlib import Path

import pytest

import pymimir


EXAMPLES = Path(__file__).resolve().parents[2] / "Tests" / "Examples"


def _rows(values: list[int], arity: int) -> list[tuple[int, ...]]:
    assert arity > 0
    assert len(values) % arity == 0
    return sorted(
        tuple(values[offset:offset + arity])
        for offset in range(0, len(values), arity)
    )


@pytest.fixture
def gripper() -> pymimir.Problem:
    base = EXAMPLES / "gripper"
    return pymimir.Problem.from_files(
        base / "domain.pddl",
        base / "p01.pddl",
        generator="grounded",
    )


@pytest.fixture
def rich_problem() -> pymimir.Problem:
    domain = pymimir.Domain.from_pddl("""
        (define (domain rich-learning)
          (:requirements :strips :typing :equality :derived-predicates)
          (:types vehicle - object car - vehicle)
          (:predicates
            (linked ?from - vehicle ?to - vehicle)
            (flag)
            (finished)
            (ready ?vehicle - vehicle)
            (powered ?vehicle - vehicle))
          (:derived (powered ?vehicle - vehicle) (ready ?vehicle))
          (:action consume
            :parameters (?vehicle - vehicle)
            :precondition (ready ?vehicle)
            :effect (not (ready ?vehicle))))
    """)
    return pymimir.Problem.from_pddl(domain, """
        (define (problem rich-problem)
          (:domain rich-learning)
          (:objects a - car b - vehicle)
          (:init (linked a b) (flag) (ready a))
          (:goal (and (powered a) (ready b) (flag) (finished))))
    """, generator="grounded")


def _transition_problem() -> pymimir.Problem:
    domain = pymimir.Domain.from_pddl("""
        (define (domain transition-learning)
          (:requirements :strips)
          (:predicates (p ?value) (q))
          (:action change
            :parameters (?value)
            :precondition (q)
            :effect (and (p ?value) (not (q)))))
    """)
    return pymimir.Problem.from_pddl(domain, """
        (define (problem transition-problem)
          (:domain transition-learning)
          (:objects a)
          (:init (q))
          (:goal (p a)))
    """, generator="grounded")


def _expressive_problem() -> pymimir.Problem:
    domain = pymimir.Domain.from_pddl("""
        (define (domain expressive-learning)
          (:requirements :strips :typing :equality)
          (:types item)
          (:constants c - item)
          (:predicates (edge ?from - item ?to - item)))
    """)
    return pymimir.Problem.from_pddl(domain, """
        (define (problem expressive-problem)
          (:domain expressive-learning)
          (:objects a - item)
          (:init (edge c a))
          (:goal (edge c a)))
    """, generator="grounded")


def test_learning_public_exports_are_singular_and_complete() -> None:
    assert pymimir.EncodingContext is pymimir.learning.EncodingContext
    assert pymimir.RelationBuffer is pymimir.learning.RelationBuffer
    assert pymimir.RelationDescriptor is pymimir.learning.RelationDescriptor
    assert pymimir.encode_state is pymimir.learning.encode_state
    assert pymimir.encode_goal is pymimir.learning.encode_goal
    assert pymimir.encode_action_list is pymimir.learning.encode_action_list
    assert (
        pymimir.encode_transition_effects
        is pymimir.learning.encode_transition_effects
    )
    assert pymimir.encode_virtual_node is pymimir.learning.encode_virtual_node
    assert (
        pymimir.encode_expressive_state
        is pymimir.learning.encode_expressive_state
    )
    assert (
        pymimir.encode_expressive_goal
        is pymimir.learning.encode_expressive_goal
    )
    for removed in (
        "NodeIndex",
        "NodeReference",
        "encode_action_batch",
        "encode_states",
        "encode_goals",
        "encode_ground_actions",
    ):
        assert not hasattr(pymimir, removed)
        assert not hasattr(pymimir.learning, removed)


def test_context_owns_batch_instance_boundaries_and_metadata(
    gripper: pymimir.Problem,
) -> None:
    context = pymimir.EncodingContext()
    with pytest.raises(RuntimeError, match="no encoding instance"):
        _ = context.problem

    context.begin_instance(gripper)
    objects = gripper.all_objects
    assert context.problem is gripper
    assert context.id_offset == 0
    assert context.object_to_id == {
        value: position for position, value in enumerate(objects)
    }
    assert context.get_object_ids() == [0, 1, 2, 3, 4, 5]
    assert context.new_virtual_id() == 6
    assert context.new_action_id() == 7
    assert context.new_or_existing_virtual_id() == 6
    assert context.new_or_existing_auxiliary_id("shared") == 8
    assert context.new_or_existing_auxiliary_id("shared") == 8
    assert context.get_auxiliary_id("shared") == 8
    assert context.get_node_count() == 9
    context.end_instance()

    context.begin_instance(gripper)
    assert context.id_offset == 9
    assert context.get_object_ids() == [9, 10, 11, 12, 13, 14]
    context.end_instance()

    assert context.batch_count == 2
    assert context.node_count == 15
    assert context.node_sizes == [9, 6]
    assert context.object_sizes == [6, 6]
    assert context.object_indices == [0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14]
    assert context.action_sizes == [1, 0]
    assert context.action_indices == [7]
    assert context.virtual_sizes == [1, 0]
    assert context.virtual_indices == [6]
    assert context.auxiliary_sizes == [1, 0]
    assert context.auxiliary_indices == [8]
    assert context.to_relations() == {}


def test_standard_encoders_accumulate_until_to_relations(
    rich_problem: pymimir.Problem,
) -> None:
    state = rich_problem.initial_state
    action = state.applicable_actions()[0]
    context = pymimir.EncodingContext()
    context.begin_instance(rich_problem)
    flag = rich_problem.ground_literal(rich_problem.fact("flag"))
    duplicate_flag_goal = rich_problem.ground_condition(flag, flag)

    assert pymimir.encode_state(context, state, suffix="_state") is None
    assert (
        pymimir.encode_goal(
            context,
            state,
            rich_problem.goal,
            suffix="_goal",
        )
        is None
    )
    pymimir.encode_goal(
        context,
        state,
        duplicate_flag_goal,
        suffix="_duplicate",
    )
    pymimir.encode_expressive_goal(
        context,
        state,
        duplicate_flag_goal,
        suffix="_duplicate",
    )
    assert (
        pymimir.encode_action_list(
            context,
            state,
            [action, action],
            suffix="_available",
        )
        is None
    )
    with pytest.raises(RuntimeError, match="while an instance is active"):
        context.to_relations()
    with pytest.raises(RuntimeError, match="while an instance is active"):
        context.to_relation_buffer()
    context.end_instance()

    encoded = context.to_relations()
    assert _rows(encoded["type_relation_object_state"], 1) == [(0,), (1,)]
    assert _rows(encoded["type_relation_vehicle_state"], 1) == [(0,), (1,)]
    assert _rows(encoded["type_relation_car_state"], 1) == [(0,)]
    assert _rows(encoded["relation_=_state"], 2) == [(0, 0), (1, 1)]
    assert _rows(encoded["relation_linked_state"], 2) == [(0, 1)]
    assert encoded["relation_flag_state"] == [0, 1]
    assert _rows(encoded["relation_ready_state"], 1) == [(0,)]
    assert _rows(encoded["relation_powered_state"], 1) == [(0,)]
    assert _rows(encoded["relation_powered_goal_goal_true"], 1) == [(0,)]
    assert _rows(encoded["relation_ready_goal_goal_false"], 1) == [(1,)]
    assert encoded["relation_flag_goal_goal_true"] == [0, 1]
    assert encoded["relation_finished_goal_goal_false"] == [0, 1]
    assert _rows(encoded["relation_flag_duplicate_goal_true"], 1) == [
        (0,), (0,), (1,), (1,),
    ]
    assert _rows(encoded["expressive_relation_flag_duplicate_goal_true"], 1) == [
        (0,), (0,), (1,), (1,),
    ]
    assert _rows(encoded["action_consume_available"], 2) == [(2, 0), (3, 0)]
    assert context.node_sizes == [4]
    assert context.action_indices == [2, 3]

    returned = context.to_relations()
    returned["relation_ready_state"].clear()
    assert context.to_relations()["relation_ready_state"] == [0]


def test_two_instances_receive_prefix_offsets(gripper: pymimir.Problem) -> None:
    state = gripper.initial_state
    actions = state.applicable_actions()
    context = pymimir.EncodingContext()
    for _ in range(2):
        context.begin_instance(gripper)
        pymimir.encode_state(context, state)
        pymimir.encode_action_list(context, state, actions)
        context.end_instance()

    encoded = context.to_relations()
    assert context.node_sizes == [12, 12]
    assert context.object_indices == [
        0, 1, 2, 3, 4, 5,
        12, 13, 14, 15, 16, 17,
    ]
    assert context.action_indices == [
        6, 7, 8, 9, 10, 11,
        18, 19, 20, 21, 22, 23,
    ]
    assert encoded["constant_relation_rooma"] == [0, 12]
    assert encoded["constant_relation_roomb"] == [1, 13]
    assert _rows(encoded["relation_room"], 1) == [(0,), (1,), (12,), (13,)]


def test_state_encodes_constants_ancestors_empty_types_and_nullaries() -> None:
    domain = pymimir.Domain.from_pddl("""
        (define (domain typed-context)
          (:requirements :strips :typing)
          (:types entity unused - object car - entity)
          (:constants shared - car)
          (:predicates (flag) (done)))
    """)
    problem = pymimir.Problem.from_pddl(domain, """
        (define (problem typed-context-problem)
          (:domain typed-context)
          (:objects local-car - car local-entity - entity)
          (:init (flag))
          (:goal (done)))
    """, generator="grounded")
    context = pymimir.EncodingContext()
    context.begin_instance(problem)
    pymimir.encode_state(context, problem.initial_state, suffix="_typed")
    context.end_instance()

    relation_buffer = context.to_relation_buffer()
    constant_descriptor = next(
        descriptor
        for descriptor in relation_buffer.descriptors
        if descriptor.name == "constant_relation_shared_typed"
    )
    assert constant_descriptor.length == 1

    encoded = relation_buffer.to_relations()
    assert context.object_indices == [0, 1, 2]
    assert encoded["constant_relation_shared_typed"] == [0]
    assert encoded["type_relation_object_typed"] == [0, 1, 2]
    assert encoded["type_relation_entity_typed"] == [0, 1, 2]
    assert encoded["type_relation_car_typed"] == [0, 1]
    assert encoded["type_relation_unused_typed"] == []
    assert encoded["relation_flag_typed"] == [0, 1, 2]


def test_nullary_predicates_use_encoder_specific_nodes() -> None:
    domain = pymimir.Domain.from_pddl("""
        (define (domain nullary-learning)
          (:requirements :strips :derived-predicates)
          (:predicates
            (static-on)
            (fluent-on)
            (derived-on)
            (false-static))
          (:derived (derived-on) (fluent-on))
          (:action clear
            :parameters ()
            :precondition (fluent-on)
            :effect (not (fluent-on))))
    """)
    first = pymimir.Problem.from_pddl(domain, """
        (define (problem first-nullary)
          (:domain nullary-learning)
          (:objects a b)
          (:init (static-on) (fluent-on))
          (:goal (and
            (static-on)
            (fluent-on)
            (derived-on)
            (false-static))))
    """, generator="grounded")
    second = pymimir.Problem.from_pddl(domain, """
        (define (problem second-nullary)
          (:domain nullary-learning)
          (:objects c)
          (:init (static-on) (fluent-on))
          (:goal (and
            (static-on)
            (fluent-on)
            (derived-on)
            (false-static))))
    """, generator="grounded")
    empty = pymimir.Problem.from_pddl(domain, """
        (define (problem empty-nullary)
          (:domain nullary-learning)
          (:init (static-on) (fluent-on))
          (:goal (and
            (static-on)
            (fluent-on)
            (derived-on)
            (false-static))))
    """, generator="grounded")
    context = pymimir.EncodingContext()

    for problem in (first, second):
        state = problem.initial_state
        context.begin_instance(problem)
        pymimir.encode_state(context, state, suffix="_broadcast")
        pymimir.encode_goal(
            context,
            state,
            problem.goal,
            suffix="_broadcast",
        )
        pymimir.encode_expressive_state(
            context,
            state,
            suffix="_broadcast",
        )
        pymimir.encode_expressive_goal(
            context,
            state,
            problem.goal,
            suffix="_broadcast",
        )
        context.end_instance()

    expected_objects = [0, 1, 4]
    encoded = context.to_relations()
    assert context.node_sizes == [4, 1]
    assert context.object_indices == expected_objects
    for name in (
        "type_relation_object_broadcast",
        "expressive_type_relation_object_broadcast",
        "relation_static-on_broadcast",
        "relation_fluent-on_broadcast",
        "relation_derived-on_broadcast",
        "relation_static-on_broadcast_goal_true",
        "relation_fluent-on_broadcast_goal_true",
        "relation_derived-on_broadcast_goal_true",
        "relation_false-static_broadcast_goal_false",
        "expressive_relation_static-on_broadcast",
        "expressive_relation_fluent-on_broadcast",
        "expressive_relation_derived-on_broadcast",
        "expressive_relation_static-on_broadcast_goal_true",
        "expressive_relation_fluent-on_broadcast_goal_true",
        "expressive_relation_derived-on_broadcast_goal_true",
        "expressive_relation_false-static_broadcast_goal_false",
    ):
        assert _rows(encoded[name], 1) == _rows(expected_objects, 1)

    transition_context = pymimir.EncodingContext()
    for problem in (first, second):
        state = problem.initial_state
        disabled = problem.action("clear").apply(state)
        transition_context.begin_instance(problem)
        pymimir.encode_transition_effects(
            transition_context,
            state,
            [disabled],
            [],
            problem.goal,
            suffix="_broadcast",
        )
        pymimir.encode_transition_effects(
            transition_context,
            disabled,
            [state],
            [],
            problem.goal,
            suffix="_broadcast",
        )
        transition_context.end_instance()

    expected_positive = [3, 6]
    expected_negative = [2, 5]
    transition_relations = transition_context.to_relations()
    assert transition_context.node_sizes == [4, 3]
    assert _rows(transition_relations["fluent-on_broadcast_pos"], 1) == _rows(
        expected_positive,
        1,
    )
    assert _rows(transition_relations["fluent-on_broadcast_neg"], 1) == _rows(
        expected_negative,
        1,
    )
    assert _rows(
        transition_relations["fluent-on_broadcast_pos_goal"],
        1,
    ) == _rows(expected_positive, 1)
    assert _rows(
        transition_relations["fluent-on_broadcast_neg_goal"],
        1,
    ) == _rows(expected_negative, 1)
    assert _rows(
        transition_relations["derived-on_broadcast_pos"],
        1,
    ) == _rows(expected_positive, 1)
    assert _rows(
        transition_relations["derived-on_broadcast_neg"],
        1,
    ) == _rows(expected_negative, 1)
    assert _rows(
        transition_relations["derived-on_broadcast_pos_goal"],
        1,
    ) == _rows(expected_positive, 1)
    assert _rows(
        transition_relations["derived-on_broadcast_neg_goal"],
        1,
    ) == _rows(expected_negative, 1)

    empty_context = pymimir.EncodingContext()
    empty_state = empty.initial_state
    empty_disabled = empty.action("clear").apply(empty_state)
    empty_context.begin_instance(empty)
    pymimir.encode_state(empty_context, empty_state, suffix="_empty")
    pymimir.encode_goal(
        empty_context,
        empty_state,
        empty.goal,
        suffix="_empty",
    )
    pymimir.encode_expressive_state(
        empty_context,
        empty_state,
        suffix="_empty",
    )
    pymimir.encode_expressive_goal(
        empty_context,
        empty_state,
        empty.goal,
        suffix="_empty",
    )
    pymimir.encode_transition_effects(
        empty_context,
        empty_state,
        [empty_disabled],
        [],
        empty.goal,
        suffix="_empty",
    )
    pymimir.encode_transition_effects(
        empty_context,
        empty_disabled,
        [empty_state],
        [],
        empty.goal,
        suffix="_empty",
    )
    empty_context.end_instance()

    empty_relations = empty_context.to_relations()
    assert empty_context.node_sizes == [2]
    for name in (
        "type_relation_object_empty",
        "expressive_type_relation_object_empty",
        "relation_static-on_empty",
        "relation_fluent-on_empty",
        "relation_derived-on_empty",
        "relation_static-on_empty_goal_true",
        "relation_fluent-on_empty_goal_true",
        "relation_derived-on_empty_goal_true",
        "relation_false-static_empty_goal_false",
        "expressive_relation_static-on_empty",
        "expressive_relation_fluent-on_empty",
        "expressive_relation_derived-on_empty",
        "expressive_relation_static-on_empty_goal_true",
        "expressive_relation_fluent-on_empty_goal_true",
        "expressive_relation_derived-on_empty_goal_true",
        "expressive_relation_false-static_empty_goal_false",
    ):
        assert empty_relations[name] == []
    assert _rows(empty_relations["fluent-on_empty_pos"], 1) == [(1,)]
    assert _rows(empty_relations["fluent-on_empty_neg"], 1) == [(0,)]
    assert _rows(empty_relations["fluent-on_empty_pos_goal"], 1) == [(1,)]
    assert _rows(empty_relations["fluent-on_empty_neg_goal"], 1) == [(0,)]
    assert _rows(empty_relations["derived-on_empty_pos"], 1) == [(1,)]
    assert _rows(empty_relations["derived-on_empty_neg"], 1) == [(0,)]
    assert _rows(empty_relations["derived-on_empty_pos_goal"], 1) == [(1,)]
    assert _rows(empty_relations["derived-on_empty_neg_goal"], 1) == [(0,)]


def test_goal_preserves_python_literal_duplicates(
    rich_problem: pymimir.Problem,
) -> None:
    state = rich_problem.initial_state
    literal = rich_problem.ground_literal(rich_problem.fact("ready", "a"))
    duplicate_goal = rich_problem.ground_condition(literal, literal)
    assert len(duplicate_goal.literals) == 2

    context = pymimir.EncodingContext()
    context.begin_instance(rich_problem)
    pymimir.encode_goal(context, state, duplicate_goal)
    context.end_instance()

    assert context.to_relations()["relation_ready_goal_true"] == [0, 0]


def test_transition_effects_are_native_and_use_action_category_ids() -> None:
    problem = _transition_problem()
    state = problem.initial_state
    action = problem.action("change", "a")
    successor = action.apply(state)
    context = pymimir.EncodingContext()
    context.begin_instance(problem)
    assert (
        pymimir.encode_transition_effects(
            context,
            state,
            [successor, state, successor],
            [(0, 1), (1, 2), (0, 1)],
            problem.goal,
            suffix="_test",
        )
        is None
    )
    context.end_instance()

    encoded = context.to_relations()
    assert context.action_indices == [1, 2, 3]
    assert encoded["p_test_pos"] == [1, 0, 3, 0]
    assert encoded["p_test_pos_goal"] == [1, 0, 3, 0]
    assert _rows(encoded["q_test_neg"], 1) == [(1,), (3,)]
    assert encoded["effect_relation_test"] == [1, 2, 2, 3, 1, 2]


def test_virtual_node_links_only_declared_objects(gripper: pymimir.Problem) -> None:
    context = pymimir.EncodingContext()
    context.begin_instance(gripper)
    assert pymimir.encode_virtual_node(context) is None
    context.end_instance()

    assert context.virtual_indices == [6]
    assert context.to_relations()["virtual_node_link"] == [
        6, 2,
        6, 3,
        6, 4,
        6, 5,
    ]


def test_expressive_encoders_share_ordered_pair_ids() -> None:
    problem = _expressive_problem()
    state = problem.initial_state
    constant = problem.object("c")
    declared = problem.object("a")
    false_goal = problem.ground_condition(
        problem.ground_literal(problem.fact("edge", "a", "c"))
    )
    context = pymimir.EncodingContext()
    context.begin_instance(problem)
    pymimir.encode_expressive_state(context, state, suffix="_state")
    pair_from_native = context.get_auxiliary_id((constant, declared))
    pair_id = context.new_or_existing_auxiliary_id((constant, declared))
    reverse_pair_id = context.new_or_existing_auxiliary_id((declared, constant))
    pymimir.encode_expressive_goal(
        context,
        state,
        problem.goal,
        suffix="_goal",
    )
    pymimir.encode_expressive_goal(
        context,
        state,
        false_goal,
        suffix="_false",
    )
    object_ids = context.object_to_id
    context.end_instance()

    constant_id = object_ids[constant]
    declared_id = object_ids[declared]
    assert pair_from_native == pair_id
    expected_pairs = [constant_id, pair_id, reverse_pair_id, declared_id]
    encoded = context.to_relations()
    assert encoded["expressive_relation_edge_state"] == expected_pairs
    assert encoded["expressive_relation_edge_goal_goal_true"] == expected_pairs
    assert encoded["expressive_relation_edge_false_goal_false"] == [
        declared_id,
        reverse_pair_id,
        pair_id,
        constant_id,
    ]
    assert _rows(encoded["expressive_relation_=_state"], 4) == [
        (constant_id,) * 4,
        (declared_id,) * 4,
    ]
    assert sorted(encoded["expressive_type_relation_item_state"]) == [
        constant_id,
        declared_id,
    ]
    assert len(encoded["expressive_composition"]) == 2**3 * 3
    assert sorted(context.auxiliary_indices) == sorted([pair_id, reverse_pair_id])


def test_native_relations_materialize_only_when_requested(
    gripper: pymimir.Problem,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    original = (
        pymimir.advanced.lib.mimir_learning_encoding_context_get_relation_count
    )
    original_bulk_copy = (
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_all_relation_values
    )
    calls = 0
    bulk_copy_calls = 0

    def counted(handle: int) -> int:
        nonlocal calls
        calls += 1
        return int(original(handle))

    def counted_bulk_copy(
        handle: int,
        destination: ctypes.c_void_p,
        capacity: int,
    ) -> int:
        nonlocal bulk_copy_calls
        bulk_copy_calls += 1
        return int(original_bulk_copy(handle, destination, capacity))

    monkeypatch.setattr(
        pymimir.advanced.lib,
        "mimir_learning_encoding_context_get_relation_count",
        counted,
    )
    monkeypatch.setattr(
        pymimir.advanced.lib,
        "mimir_learning_encoding_context_copy_all_relation_values",
        counted_bulk_copy,
    )
    context = pymimir.EncodingContext()
    for _ in range(2):
        context.begin_instance(gripper)
        pymimir.encode_state(context, gripper.initial_state)
        context.end_instance()
    assert calls == 0

    context.to_relations()
    assert calls == 1
    assert bulk_copy_calls == 1


def test_relation_buffer_is_a_packed_independent_snapshot(
    rich_problem: pymimir.Problem,
) -> None:
    state = rich_problem.initial_state
    context = pymimir.EncodingContext()
    context.begin_instance(rich_problem)
    pymimir.encode_state(context, state, suffix="_state")
    context.end_instance()

    relation_buffer = context.to_relation_buffer()
    assert isinstance(relation_buffer.descriptors, tuple)
    assert relation_buffer.descriptors
    assert [descriptor.offset for descriptor in relation_buffer.descriptors] == [
        sum(previous.length for previous in relation_buffer.descriptors[:index])
        for index in range(len(relation_buffer.descriptors))
    ]
    assert len({descriptor.name for descriptor in relation_buffer.descriptors}) == len(
        relation_buffer.descriptors
    )
    flag = next(
        descriptor
        for descriptor in relation_buffer.descriptors
        if descriptor.name == "relation_flag_state"
    )
    assert flag.length == 2
    with pytest.raises(AttributeError):
        flag.length = 1  # type: ignore[misc]

    values = relation_buffer.values
    assert isinstance(values, memoryview)
    assert not values.readonly
    assert values.format == "B"
    assert values.nbytes == 4 * sum(
        descriptor.length for descriptor in relation_buffer.descriptors
    )
    with pytest.raises(BufferError):
        relation_buffer._storage.append(0)

    ready = next(
        descriptor
        for descriptor in relation_buffer.descriptors
        if descriptor.name == "relation_ready_state"
    )
    alias = ctypes.c_int32.from_buffer(values, ready.offset * 4)
    original = alias.value
    alias.value = 1234
    assert relation_buffer.to_relations()[ready.name] == [1234]
    assert context.to_relations()[ready.name] == [original]
    del alias
    values.release()

    context.begin_instance(rich_problem)
    pymimir.encode_state(context, state, suffix="_later")
    context.end_instance()
    context.close()
    del context
    gc.collect()

    assert relation_buffer.to_relations()[ready.name] == [1234]
    assert not any(
        descriptor.name.endswith("_later")
        for descriptor in relation_buffer.descriptors
    )


def test_empty_relation_buffer_and_descriptor_validation() -> None:
    context = pymimir.EncodingContext()
    relation_buffer = context.to_relation_buffer()
    context.close()

    assert relation_buffer.descriptors == ()
    assert relation_buffer.values.nbytes == 0
    assert relation_buffer.to_relations() == {}

    descriptor = pymimir.RelationDescriptor("empty", 0, 0)
    assert pymimir.RelationBuffer(bytearray(), [descriptor]).to_relations() == {
        "empty": []
    }
    with pytest.raises(ValueError, match="contiguous"):
        pymimir.RelationBuffer(
            bytearray(4),
            [pymimir.RelationDescriptor("bad", 1, 0)],
        )
    with pytest.raises(ValueError, match="duplicate"):
        pymimir.RelationBuffer(
            bytearray(),
            [descriptor, descriptor],
        )
    with pytest.raises(OverflowError, match="Int32.MaxValue"):
        pymimir.RelationBuffer(
            bytearray(),
            [pymimir.RelationDescriptor("huge", 0, 2**31)],
        )


def test_context_rejects_invalid_lifecycle_ownership_and_literals(
    gripper: pymimir.Problem,
    rich_problem: pymimir.Problem,
) -> None:
    context = pymimir.EncodingContext()
    with pytest.raises(TypeError, match="problem must be a Problem"):
        context.begin_instance(object())  # type: ignore[arg-type]
    with pytest.raises(RuntimeError, match="no encoding instance"):
        context.end_instance()

    context.begin_instance(gripper)
    with pytest.raises(RuntimeError, match="already active"):
        context.begin_instance(gripper)
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_state(context, rich_problem.initial_state)
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_goal(context, gripper.initial_state, rich_problem.goal)
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_action_list(
            context,
            gripper.initial_state,
            rich_problem.initial_state.applicable_actions(),
        )
    with pytest.raises(ValueError, match="different Problem"):
        pymimir.encode_transition_effects(
            context,
            gripper.initial_state,
            [rich_problem.initial_state],
            [],
            gripper.goal,
        )
    with pytest.raises(TypeError, match="State"):
        pymimir.encode_transition_effects(
            context,
            gripper.initial_state,
            [object()],  # type: ignore[list-item]
            [],
            gripper.goal,
        )
    assert context.get_action_count() == 0
    context.end_instance()

    with pytest.raises(ValueError, match="exact Domain"):
        context.begin_instance(rich_problem)

    rich_context = pymimir.EncodingContext()
    rich_context.begin_instance(rich_problem)
    negative = rich_problem.ground_condition(
        rich_problem.ground_literal(
            rich_problem.fact("ready", "a"),
            positive=False,
        )
    )
    with pytest.raises(ValueError, match="positive"):
        pymimir.encode_goal(
            rich_context,
            rich_problem.initial_state,
            negative,
        )
    with pytest.raises(TypeError, match="suffix must be str"):
        pymimir.encode_state(
            rich_context,
            rich_problem.initial_state,
            suffix=None,  # type: ignore[arg-type]
        )
    with pytest.raises(ValueError, match="null character"):
        pymimir.encode_state(
            rich_context,
            rich_problem.initial_state,
            suffix="bad\0suffix",
        )
    with pytest.raises(TypeError, match="GroundAction"):
        pymimir.encode_action_list(
            rich_context,
            rich_problem.initial_state,
            [object()],  # type: ignore[list-item]
        )
    assert rich_context.get_action_count() == 0
    rich_context.end_instance()


@pytest.mark.parametrize("invalid_index", [-(2**31) - 1, 2**31])
def test_transition_relation_indices_reject_int32_overflow(
    invalid_index: int,
) -> None:
    problem = _transition_problem()
    state = problem.initial_state
    context = pymimir.EncodingContext()
    context.begin_instance(problem)
    with pytest.raises(OverflowError, match="does not fit in int32"):
        pymimir.encode_transition_effects(
            context,
            state,
            [state],
            [(invalid_index, 0)],
            problem.goal,
        )
    assert context.get_action_count() == 0
    context.end_instance()


def test_transition_validation_is_atomic() -> None:
    problem = _transition_problem()
    state = problem.initial_state
    action = problem.action("change", "a")
    successor = action.apply(state)
    context = pymimir.EncodingContext()
    context.begin_instance(problem)
    with pytest.raises(ValueError, match="source index"):
        pymimir.encode_transition_effects(
            context,
            state,
            [successor],
            [(-1, 0)],
            problem.goal,
        )
    assert context.get_action_count() == 0
    context.end_instance()


def test_context_close_is_idempotent_and_frees_exactly_once(
    gripper: pymimir.Problem,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    from pymimir import learning

    original_free = learning.free_handle
    freed: list[int] = []

    def tracked_free(handle: int) -> None:
        freed.append(handle)
        original_free(handle)

    monkeypatch.setattr(learning, "free_handle", tracked_free)
    context = pymimir.EncodingContext()
    assert copy.copy(context) is context
    assert copy.deepcopy(context) is context
    with pytest.raises(TypeError, match="cannot be pickled"):
        pickle.dumps(context)
    handle = context._handle
    context.begin_instance(gripper)
    _ = context.object_to_id
    context.new_or_existing_auxiliary_id("owned")
    context.close()
    context.close()
    assert context._current_problem is None
    assert context._object_to_id is None
    assert context._auxiliary_ids == {}
    assert freed.count(handle) == 1
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.end_instance()
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.begin_instance(gripper)
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.to_relations()
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.to_relation_buffer()
    del context
    gc.collect()
    assert freed.count(handle) == 1


def test_context_manager_closes_after_relations_are_copied(
    gripper: pymimir.Problem,
) -> None:
    with pymimir.EncodingContext() as context:
        context.begin_instance(gripper)
        pymimir.encode_state(context, gripper.initial_state)
        context.end_instance()
        relations = context.to_relations()
        assert context.batch_count == 1

    assert relations["relation_room"] == [0, 1]
    relations["relation_room"].append(99)
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        _ = context.batch_count
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.to_relations()
    with pytest.raises(RuntimeError, match="EncodingContext is closed"):
        context.to_relation_buffer()


def test_context_finalizer_frees_an_unclosed_context(
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    from pymimir import learning

    original_free = learning.free_handle
    freed: list[int] = []

    def tracked_free(handle: int) -> None:
        freed.append(handle)
        original_free(handle)

    monkeypatch.setattr(learning, "free_handle", tracked_free)
    context = pymimir.EncodingContext()
    handle = context._handle
    del context
    gc.collect()
    assert freed.count(handle) == 1


def test_context_detects_a_stale_native_handle(gripper: pymimir.Problem) -> None:
    context = pymimir.EncodingContext()
    pymimir.advanced.free_handle(context._handle)
    context._finalizer.detach()
    with pytest.raises(ReferenceError, match="EncodingContext"):
        context.begin_instance(gripper)
    with pytest.raises(ReferenceError, match="EncodingContext"):
        context.to_relation_buffer()
    with pytest.raises(ReferenceError, match="EncodingContext"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_all_relation_values(
            context._handle,
            ctypes.c_void_p(),
            0,
        )


def test_native_relation_and_metadata_copies_reject_small_buffers(
    gripper: pymimir.Problem,
) -> None:
    context = pymimir.EncodingContext()
    context.begin_instance(gripper)
    pymimir.encode_state(context, gripper.initial_state)
    context.end_instance()
    destination = (ctypes.c_int * 1)()

    with pytest.raises(ValueError, match="destination capacity"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_object_indices(
            context._handle,
            ctypes.cast(destination, ctypes.c_void_p),
            1,
        )

    relation_count = int(
        pymimir.advanced.lib.mimir_learning_encoding_context_get_relation_count(
            context._handle
        )
    )
    relation_index = next(
        index
        for index in range(relation_count)
        if int(
            pymimir.advanced.lib.mimir_learning_encoding_context_get_relation_value_count(
                context._handle,
                index,
            )
        ) > 1
    )
    with pytest.raises(ValueError, match="destination capacity"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_relation_values(
            context._handle,
            relation_index,
            ctypes.cast(destination, ctypes.c_void_p),
            1,
        )

    total_value_count = sum(
        int(
            pymimir.advanced.lib.mimir_learning_encoding_context_get_relation_value_count(
                context._handle,
                index,
            )
        )
        for index in range(relation_count)
    )
    assert total_value_count > 1
    with pytest.raises(ValueError, match="destination capacity"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_all_relation_values(
            context._handle,
            ctypes.cast(destination, ctypes.c_void_p),
            1,
        )
    with pytest.raises(ValueError, match="destination capacity"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_all_relation_values(
            context._handle,
            ctypes.cast(destination, ctypes.c_void_p),
            -1,
        )
    with pytest.raises(ValueError, match="relation values"):
        pymimir.advanced.lib.mimir_learning_encoding_context_copy_all_relation_values(
            context._handle,
            ctypes.c_void_p(),
            total_value_count,
        )
