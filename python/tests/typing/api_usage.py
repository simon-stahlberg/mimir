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
    cost: pymimir.NumericExpressionSpec = pymimir.Numeric.add(
        pymimir.Numeric.function("price", "?item"),
        pymimir.Numeric.constant(0.5),
    )
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
        pymimir.Numeric.subtract(cost, pymimir.Numeric.constant(1)),
        pymimir.Numeric.multiply(cost, pymimir.Numeric.constant(2)),
        pymimir.Numeric.divide(cost, pymimir.Numeric.constant(2)),
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
        [state],
        [],
        problem.goal,
    )
    pymimir.encode_virtual_node(context)
    pymimir.encode_expressive_state(context, state)
    pymimir.encode_expressive_goal(context, state, problem.goal)
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
