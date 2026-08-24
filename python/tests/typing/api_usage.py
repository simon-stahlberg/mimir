from collections.abc import Iterable

import mimir


def use_public_api(
    problem: mimir.Problem,
    heuristic: mimir.heuristics.Heuristic,
) -> None:
    state: mimir.State = problem.initial_state
    predicate: mimir.model.Predicate = problem.domain.predicates[0]
    variable: mimir.model.Variable = problem.variable("?value")
    atom: mimir.model.Atom = problem.atom(predicate, variable)
    literal: mimir.model.Literal = problem.literal(atom)
    condition: mimir.model.ConjunctiveCondition = problem.condition(
        literal, variables=[variable]
    )
    bindings: tuple[dict[mimir.model.Variable, mimir.model.Object], ...] = (
        condition.bindings(state)
    )
    ground_atom: mimir.model.GroundAtom = problem.fact(
        predicate.name, *(obj.name for obj in problem.objects[:predicate.arity])
    )
    ground_literal: mimir.model.GroundLiteral = problem.ground_literal(ground_atom)
    ground_condition: mimir.model.GroundConjunctiveCondition = problem.ground_condition(
        ground_literal
    )
    custom_state: mimir.State = problem.state(
        *(ground_atom,) if ground_atom.is_fluent else ()
    )
    held: bool = custom_state.holds(ground_condition)
    actions: tuple[mimir.GroundAction, ...] = state.applicable_actions()

    def visit_state(value: mimir.State) -> None:
        pass

    def visit_transition(value: mimir.Transition) -> None:
        pass

    def visit_depth(value: int) -> None:
        pass

    def visit_value(value: float) -> None:
        pass

    results: Iterable[mimir.SearchResult] = (
        mimir.bfs(
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
        mimir.ucs(problem, on_expand=visit_state, on_prune=visit_transition),
        mimir.astar(problem, heuristic, on_finish_f_layer=visit_value),
        mimir.gbfs(problem, heuristic, on_new_best_h=visit_value),
        mimir.iw(problem, max_width=2, on_discover=visit_transition),
    )
    for result in results:
        solved: bool = result.is_solved
        elapsed: float = result.statistics.elapsed_seconds
        if result.solution is not None:
            plan: tuple[mimir.GroundAction, ...] = result.solution.plan
            cost: float = result.solution.cost
            goal_state: mimir.State = result.solution.goal_state
            _ = plan, cost, goal_state
        _ = solved, elapsed

    for schema in problem.domain.actions:
        static: tuple[mimir.model.Literal, ...] = schema.static_preconditions
        fluent: tuple[mimir.model.Literal, ...] = schema.fluent_preconditions
        derived: tuple[mimir.model.Literal, ...] = schema.derived_preconditions
        for conditional in schema.conditional_effects:
            effect: mimir.model.Effect = conditional.effect
            effect_literals: tuple[mimir.model.Literal, ...] = effect.literals
            _ = static, fluent, derived, effect_literals

    for action_value in actions:
        effect_value: mimir.model.GroundEffect = action_value.effect
        added: tuple[mimir.model.GroundAtom, ...] = effect_value.add_atoms
        deleted: tuple[mimir.model.GroundAtom, ...] = effect_value.delete_atoms
        for ground_conditional in action_value.conditional_effects:
            satisfied: bool = ground_conditional.is_satisfied(state)
            _ = satisfied
        _ = added, deleted, bindings, held

    space = mimir.StateSpace(problem, max_states=100, seed=1)
    indexed_state: mimir.State = space[0]
    state_slice: tuple[mimir.State, ...] = space[:2]
    label: mimir.StateLabel = space.label(space.initial_state)
    transitions: tuple[mimir.Transition, ...] = space.successors(space.initial_state)
    samples: tuple[mimir.State, ...] = space.sample_states(2)
    _ = label, transitions, samples, indexed_state, state_slice


def reject_invalid_search_calls(
    problem: mimir.Problem,
    heuristic: mimir.heuristics.Heuristic,
) -> None:
    mimir.ucs(problem, unknown_keyword=True)  # type: ignore[call-arg]
    mimir.astar(
        problem,
        heuristic,
        on_finish_f_layer=lambda _value: "not None",  # type: ignore[arg-type]
    )
