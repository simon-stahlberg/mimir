from collections.abc import Iterable, Mapping

import pymimir


def build_with_public_api() -> pymimir.Problem:
    domain_builder: pymimir.DomainBuilder = pymimir.DomainBuilder("built")
    requirements: pymimir.RequirementListBuilder = domain_builder.requirements()
    requirements.add(":adl").add(":typing").add(":action-costs").add(
        ":derived-predicates"
    ).close()
    types: pymimir.TypeListBuilder = domain_builder.types()
    types.add("item").close()
    constants: pymimir.ConstantListBuilder = domain_builder.constants()
    constants.add("home", "item").close()
    predicates: pymimir.PredicateListBuilder = domain_builder.predicates()
    predicates.add("ready", ("?item", "item"))
    predicates.add("done", ("?item", "item"))
    predicates.add("eligible", ("?item", "item")).close()
    functions: pymimir.NumericFunctionListBuilder = domain_builder.functions()
    functions.add("price", ("?item", "item")).close()

    actions: pymimir.ActionListBuilder = domain_builder.actions()
    action: pymimir.ActionSchemaBuilder = actions.add("finish")
    action.add_parameter("?item", "item")
    action.add_precondition("ready", "?item")
    action.add_precondition("done", "?item", positive=False)
    conditional: pymimir.ConditionalEffectBuilder = action.add_conditional_effect()
    conditional.add_condition("eligible", "?item")
    conditional.add_effect("done", "?item").close()
    cost: pymimir.NumericExpressionSpec = pymimir.Numeric.function("price", "?item") + pymimir.Numeric.constant(0.5)
    action.with_cost(cost).close().close()

    derived: pymimir.DerivedPredicateListBuilder = (
        domain_builder.derived_predicates()
    )
    body: pymimir.LogicalExpressionSpec = pymimir.Logic.or_(
        pymimir.Logic.atom("ready", "?item"),
        pymimir.Logic.exists(
            [("?other", "item")],
            pymimir.Logic.and_(
                pymimir.Logic.atom("ready", "?other"),
                pymimir.Logic.not_(pymimir.Logic.equal("?item", "?other")),
            ),
        ),
    )
    derived.define("eligible", body).close()
    domain: pymimir.Domain = domain_builder.build()

    problem_builder: pymimir.ProblemBuilder = pymimir.ProblemBuilder(
        domain, "built-problem", generator="grounded"
    )
    objects: pymimir.ProblemObjectListBuilder = problem_builder.objects()
    objects.add("a", "item").close()
    initial: pymimir.InitialStateBuilder = problem_builder.initial_state()
    initial.add_fact("ready", "a")
    initial.set_value(pymimir.Numeric.function("price", "a"), 2.5).close()
    goal: pymimir.GoalBuilder = problem_builder.goal()
    goal.add("done", "a").add("done", "home", positive=False).close()

    unused_logic: tuple[pymimir.LogicalExpressionSpec, ...] = (
        pymimir.Logic.true(),
        pymimir.Logic.false(),
        pymimir.Logic.imply(pymimir.Logic.true(), pymimir.Logic.true()),
        pymimir.Logic.forall([], pymimir.Logic.true()),
    )
    unused_costs: tuple[pymimir.NumericExpressionSpec, ...] = (
        cost - pymimir.Numeric.constant(1),
        cost * 2,
        cost / pymimir.Numeric.constant(2),
    )
    _ = unused_logic, unused_costs
    return problem_builder.build()


def use_public_api(
    problem: pymimir.Problem,
    heuristic: pymimir.heuristics.Heuristic,
) -> None:
    state: pymimir.State = problem.initial_state
    predicate: pymimir.Predicate = problem.domain.predicates[0]
    variable: pymimir.Variable = problem.variable("?value")
    atom: pymimir.Atom = problem.lifted_atom(predicate, variable)
    literal: pymimir.Literal = problem.literal(atom)
    condition: pymimir.ConjunctiveCondition = problem.condition(
        literal, variables=[variable]
    )
    bindings: tuple[dict[pymimir.Variable, pymimir.Object], ...] = (
        condition.bindings(state)
    )
    groundings: tuple[pymimir.GroundConjunctiveCondition, ...] = (
        condition.groundings(
            state,
            limit=1,
            omit_predicates=(predicate,),
        )
    )
    ground_atom: pymimir.GroundAtom = problem.atom(
        predicate.name, *(obj.name for obj in problem.all_objects[:predicate.arity])
    )
    ground_literal: pymimir.GroundLiteral = problem.ground_literal(ground_atom)
    ground_condition: pymimir.GroundConjunctiveCondition = problem.ground_condition(
        ground_literal
    )
    custom_state: pymimir.State = problem.state(
        *(ground_atom,) if ground_atom.is_fluent else ()
    )
    held: bool = custom_state.holds(ground_condition)
    actions: tuple[pymimir.GroundAction, ...] = state.applicable_actions()
    successors: tuple[tuple[pymimir.GroundAction, pymimir.State], ...] = state.successor_states()
    context: pymimir.learning.EncodingContext = pymimir.EncodingContext()
    context.begin_instance(problem)
    object_to_id: dict[pymimir.Object, int] = context.object_to_id
    object_id: int = context.get_object_id(problem.all_objects[0])
    action_id: int = context.new_action_id()
    virtual_id: int = context.new_virtual_id()
    existing_virtual_id: int = context.new_or_existing_virtual_id()
    auxiliary_id: int = context.new_or_existing_auxiliary_id("typing")
    same_auxiliary_id: int = context.get_auxiliary_id("typing")
    context_object_ids: list[int] = context.get_object_ids()
    context_action_ids: list[int] = context.get_action_ids()
    context_virtual_ids: list[int] = context.get_virtual_ids()
    context_auxiliary_ids: list[int] = context.get_auxiliary_ids()
    context_node_count: int = context.get_node_count()
    pymimir.learning.encode_state(
        context,
        state,
    )
    pymimir.encode_goal(
        context,
        state,
        problem.goal,
    )
    pymimir.encode_action_list(
        context,
        state,
        actions,
    )
    pymimir.encode_transition_effects(
        context,
        state,
        [action.apply(state) for action in actions],
        actions,
        [],
        problem.goal,
    )
    pymimir.encode_virtual_node(context)
    pymimir.encode_expressive_state(context, state)
    pymimir.encode_expressive_goal(context, state, problem.goal)
    serial_plan: pymimir.SerialPlan = pymimir.SerialPlan.empty(problem)
    serial_reference: pymimir.plan_space.SerialPlan | None = pymimir.serial_search(problem, max_expansions=1000, timeout_seconds=1.0)
    serial_steps: tuple[pymimir.SerialStep, ...] = serial_plan.steps
    serial_purpose: pymimir.GroundLiteral | None = serial_steps[0].purpose if serial_steps else None
    serial_round: int = serial_plan.round
    serial_diagnosis: pymimir.SerialDiagnosis = serial_plan.diagnose()
    serial_errors: tuple[pymimir.SerialError, ...] = serial_diagnosis.errors
    serial_candidates: tuple[pymimir.GhostStep, ...] = serial_diagnosis.candidates
    serial_supports: tuple[pymimir.SerialSupport, ...] = serial_diagnosis.supports
    serial_unused: tuple[int, ...] = serial_diagnosis.unused_step_ids
    serial_diagnosed: bool = serial_diagnosis.is_solution
    serial_owner: pymimir.SerialPlan = serial_diagnosis.plan
    serial_error_kind: pymimir.SerialErrorKind | None = serial_errors[0].kind if serial_errors else None
    serial_error_literal: pymimir.GroundLiteral | None = serial_errors[0].literal if serial_errors else None
    serial_repair: pymimir.SerialRepair | None = serial_errors[0].repairs[0] if serial_errors else None
    serial_repair_kind: pymimir.SerialRepairKind = pymimir.SerialRepairKind.NOOP
    serial_layout: pymimir.SerialEncodingLayout = pymimir.encode_serial_plan(context, serial_plan, serial_diagnosis, suffix="_serial")
    if serial_reference is not None:
        serial_targets: pymimir.SerialOracleTargets = pymimir.serial_oracle_targets(serial_plan, serial_diagnosis, serial_reference)
        serial_edit: pymimir.SerialEditResult = serial_plan.apply(
            serial_diagnosis,
            serial_targets.repair_index_per_error,
            serial_targets.step_ids_to_delete,
        )
        serial_actions: list[pymimir.GroundAction] = serial_reference.to_actions()
        _ = serial_edit.dropped_error_indices, serial_actions
    serial_solved: bool = serial_plan.is_solution()
    serial_copy: pymimir.SerialPlan = serial_plan.clone()
    serial_goal: pymimir.GroundConjunctiveCondition = serial_plan.goal
    serial_relabeled: pymimir.SerialPlan = serial_plan.with_goal(problem.goal)
    serial_pruned: pymimir.SerialPlan = serial_relabeled.prune_to_achieved_goal()
    serial_subset: pymimir.SerialPlan = pymimir.SerialPlan.empty(problem, goal=serial_pruned.goal)
    serial_relations: list[tuple[str, int]] = pymimir.serial_plan_relations(problem.domain, "_serial")
    _ = serial_purpose, serial_round, serial_candidates, serial_supports, serial_unused, serial_diagnosed, serial_owner
    _ = serial_error_kind, serial_error_literal, serial_repair, serial_repair_kind, serial_layout.repair_node_ids
    _ = serial_solved, serial_copy, serial_goal, serial_subset, serial_relations
    context.end_instance()
    relation_buffer: pymimir.RelationBuffer = context.to_relation_buffer()
    relation_values: memoryview = relation_buffer.values
    relation_descriptors: tuple[pymimir.RelationDescriptor, ...] = (
        relation_buffer.descriptors
    )
    buffered_relations: dict[str, list[int]] = relation_buffer.to_relations()
    relations: dict[str, list[int]] = context.to_relations()
    node_sizes: list[int] = context.node_sizes
    object_indices: list[int] = context.object_indices
    action_indices: list[int] = context.action_indices
    virtual_indices: list[int] = context.virtual_indices
    auxiliary_indices: list[int] = context.auxiliary_indices
    context.close()
    with pymimir.EncodingContext() as managed_context:
        managed_context.begin_instance(problem)
        managed_context.end_instance()
    managed_context.close()
    hierarchy: Mapping[str, str | None] = problem.domain.type_hierarchy
    requirements: tuple[str, ...] = problem.requirements
    declared_objects: tuple[pymimir.Object, ...] = problem.declared_objects
    all_objects: tuple[pymimir.Object, ...] = problem.all_objects
    static_atoms: tuple[pymimir.GroundAtom, ...] = state.static_atoms
    fluent_atoms: tuple[pymimir.GroundAtom, ...] = state.fluent_atoms
    derived_atoms: tuple[pymimir.GroundAtom, ...] = state.derived_atoms
    atoms: tuple[pymimir.GroundAtom, ...] = state.atoms
    _ = hierarchy, requirements, declared_objects, all_objects
    _ = object_to_id, object_id, action_id, virtual_id, existing_virtual_id
    _ = auxiliary_id, same_auxiliary_id, context_object_ids, context_action_ids
    _ = context_virtual_ids, context_auxiliary_ids, context_node_count
    _ = relations, node_sizes, object_indices, action_indices
    _ = relation_values, relation_descriptors, buffered_relations
    _ = virtual_indices, auxiliary_indices
    _ = static_atoms, fluent_atoms, derived_atoms, atoms

    def visit_state(value: pymimir.State) -> None:
        pass

    def visit_transition(value: pymimir.Transition) -> None:
        pass

    def visit_depth(value: int) -> None:
        pass

    def visit_value(value: float) -> None:
        pass

    results: Iterable[pymimir.SearchResult] = (
        pymimir.bfs(
            problem,
            start_state=state,
            goal=problem.goal,
            timeout_seconds=1.0,
            max_expanded_states=10,
            on_expand=visit_state,
            on_goal=visit_state,
            on_generate=visit_transition,
            on_discover=visit_transition,
            on_prune=visit_transition,
            on_finish_depth=visit_depth,
        ),
        pymimir.ucs(problem, on_expand=visit_state, on_prune=visit_transition),
        pymimir.astar(problem, heuristic, on_finish_f_layer=visit_value),
        pymimir.gbfs(problem, heuristic, on_new_best_h=visit_value),
        pymimir.iw(problem, max_width=2, on_discover=visit_transition),
    )
    for result in results:
        solved: bool = result.is_solved
        elapsed: float = result.statistics.elapsed_seconds
        if result.solution is not None:
            plan: tuple[pymimir.GroundAction, ...] = result.solution.plan
            cost: float = result.solution.cost
            goal_state: pymimir.State = result.solution.goal_state
            _ = plan, cost, goal_state
        _ = solved, elapsed

    for schema in problem.domain.actions:
        static: tuple[pymimir.Literal, ...] = schema.static_preconditions
        fluent: tuple[pymimir.Literal, ...] = schema.fluent_preconditions
        derived: tuple[pymimir.Literal, ...] = schema.derived_preconditions
        for conditional in schema.conditional_effects:
            effect: pymimir.Effect = conditional.effect
            effect_literals: tuple[pymimir.Literal, ...] = effect.literals
            _ = static, fluent, derived, effect_literals

    for action_value in actions:
        effect_value: pymimir.GroundEffect = action_value.effect
        added: tuple[pymimir.GroundAtom, ...] = effect_value.add_atoms
        deleted: tuple[pymimir.GroundAtom, ...] = effect_value.delete_atoms
        for ground_conditional in action_value.conditional_effects:
            satisfied: bool = ground_conditional.is_satisfied(state)
            _ = satisfied
        _ = added, deleted, bindings, held

    space = pymimir.StateSpace(problem, max_states=100, seed=1)
    indexed_state: pymimir.State = space[0]
    state_slice: tuple[pymimir.State, ...] = space[:2]
    label: pymimir.StateLabel = space.label(space.initial_state)
    transitions: tuple[pymimir.Transition, ...] = space.successors(space.initial_state)
    samples: tuple[pymimir.State, ...] = space.sample_states(2)
    _ = label, transitions, samples, indexed_state, state_slice


def reject_invalid_search_calls(
    problem: pymimir.Problem,
    heuristic: pymimir.heuristics.Heuristic,
) -> None:
    pymimir.ucs(problem, unknown_keyword=True)  # type: ignore[call-arg]
    pymimir.astar(
        problem,
        heuristic,
        on_finish_f_layer=lambda _value: "not None",  # type: ignore[arg-type]
    )


def use_batched_heuristics(
    problem: pymimir.Problem,
    heuristic: pymimir.heuristics.Heuristic,
    q_heuristic: pymimir.QHeuristic,
) -> None:
    state = problem.initial_state
    detector: pymimir.DeadEndDetector = pymimir.DisjunctiveDeadEndDetector(
        [pymimir.H2DeadEndDetector(problem)]
    )
    is_dead_end: bool = detector.is_dead_end(state, problem.goal)
    _ = is_dead_end
    successors = state.successor_states()
    heuristic.evaluate_batch([state], problem.goal)
    q_heuristic.evaluate(state, successors, problem.goal)
    q_heuristic.evaluate_batch([(state, successors)], problem.goal)
    result: pymimir.SearchResult = pymimir.qgbfs(
        problem, q_heuristic, batch_target=4, maximize=True,
        should_stop=lambda expanded: expanded >= 10,
        dead_end_detector=detector,
    )
    partial: tuple[pymimir.GroundAction, ...] = result.partial_plan
    scores: tuple[float | None, ...] = result.action_values
    generated: int | None = result.statistics.generated_transitions
    _ = partial, scores, generated

    beam_result: pymimir.SearchResult = pymimir.beam(
        problem, heuristic, beam_size=4, max_depth=10, dead_end_detector=detector,
    )
    qbeam_result: pymimir.SearchResult = pymimir.qbeam(
        problem, q_heuristic, beam_size=4, max_depth=10, maximize=False,
        start_state=state, goal=problem.goal, timeout_seconds=1, max_expanded_states=100,
        should_stop=lambda expanded: expanded > 10,
    )
    _ = beam_result, qbeam_result


def inspect_numeric_values(problem: pymimir.Problem) -> None:
    atom: pymimir.GroundAtom = problem.atom("ready", "a")
    call: pymimir.GroundFunctionCall = problem.function_call("price", "a")
    truth: bool = problem.initial_state.value(atom)
    value: float = problem.initial_state.value(call * 2 + 1)
    condition: pymimir.NumericComparison = call.greater_than_or_equal(1)
    satisfied: bool = problem.initial_state.holds(condition)
    schema: pymimir.Action = problem.domain.actions[0]
    cost: pymimir.NumericExpression = schema.cost_expression
    requirements: tuple[pymimir.NumericComparison, ...] = schema.precondition.numeric_conditions
    updates: tuple[pymimir.NumericUpdate, ...] = schema.effect.numeric_effects
    assert isinstance(truth, bool) and isinstance(value, float) and isinstance(satisfied, bool)
    assert cost is not None and requirements is not None and updates is not None
