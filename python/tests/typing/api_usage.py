from collections.abc import Iterable

import pymimir


def use_public_api(
    problem: pymimir.Problem,
    heuristic: pymimir.heuristics.Heuristic,
) -> None:
    state: pymimir.State = problem.initial_state
    predicate: pymimir.model.Predicate = problem.domain.predicates[0]
    variable: pymimir.model.Variable = problem.variable("?value")
    atom: pymimir.model.Atom = problem.atom(predicate, variable)
    literal: pymimir.model.Literal = problem.literal(atom)
    condition: pymimir.model.ConjunctiveCondition = problem.condition(
        literal, variables=[variable]
    )
    bindings: tuple[dict[pymimir.model.Variable, pymimir.model.Object], ...] = (
        condition.bindings(state)
    )
    ground_atom: pymimir.model.GroundAtom = problem.fact(
        predicate.name, *(obj.name for obj in problem.objects[:predicate.arity])
    )
    ground_literal: pymimir.model.GroundLiteral = problem.ground_literal(ground_atom)
    ground_condition: pymimir.model.GroundConjunctiveCondition = problem.ground_condition(
        ground_literal
    )
    custom_state: pymimir.State = problem.state(
        *(ground_atom,) if ground_atom.is_fluent else ()
    )
    held: bool = custom_state.holds(ground_condition)
    actions: tuple[pymimir.GroundAction, ...] = state.applicable_actions()

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
        static: tuple[pymimir.model.Literal, ...] = schema.static_preconditions
        fluent: tuple[pymimir.model.Literal, ...] = schema.fluent_preconditions
        derived: tuple[pymimir.model.Literal, ...] = schema.derived_preconditions
        for conditional in schema.conditional_effects:
            effect: pymimir.model.Effect = conditional.effect
            effect_literals: tuple[pymimir.model.Literal, ...] = effect.literals
            _ = static, fluent, derived, effect_literals

    for action_value in actions:
        effect_value: pymimir.model.GroundEffect = action_value.effect
        added: tuple[pymimir.model.GroundAtom, ...] = effect_value.add_atoms
        deleted: tuple[pymimir.model.GroundAtom, ...] = effect_value.delete_atoms
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
