import math

import pytest
import pymimir as m


DOMAIN = """
(define (domain resources)
  (:requirements :strips :typing :negative-preconditions :conditional-effects :action-costs)
  (:types truck)
  (:predicates (ready ?t - truck) (done ?t - truck) (flag))
  (:functions (total-cost) (fuel ?t - truck))
  (:action drive
    :parameters (?t - truck)
    :precondition (and (ready ?t) (not (done ?t)))
    :effect (and (not (ready ?t)) (done ?t)
                 (when (flag) (ready ?t))
                 (increase (total-cost) (+ (fuel ?t) 1)))))
"""
PROBLEM = """
(define (problem one-truck)
  (:domain resources)
  (:objects truck1 - truck)
  (:init (ready truck1) (= (fuel truck1) 20) (= (total-cost) 0))
  (:goal (done truck1))
  (:metric minimize (total-cost)))
"""


@pytest.fixture
def problem():
    return m.Problem.from_pddl(m.Domain.from_pddl(DOMAIN), PROBLEM)


def test_boolean_and_numeric_values_use_ground_references(problem):
    state = problem.initial_state
    ready = problem.atom("ready", "truck1")
    fuel = problem.function_call("fuel", "truck1")
    assert state.value(ready) is True
    assert state.value(problem.atom("done", "truck1")) is False
    assert state.value(fuel) == 20.0
    assert state.value(2 * fuel + 1) == 41.0
    assert state.value(-fuel / 2) == -10.0
    assert state.holds(fuel.greater_than_or_equal(20))
    assert not state.holds(fuel.less_than(20))
    assert ready.arguments == fuel.arguments == (problem.object("truck1"),)
    assert fuel.function == problem.domain.function("fuel")
    assert {fuel: 20}[problem.function_call("fuel", "truck1")] == 20
    with pytest.raises(TypeError, match="state.holds"):
        bool(fuel.equal_to(20))


def test_schema_and_ground_inspection_have_matching_shapes(problem):
    schema = problem.domain.action("drive")
    action = problem.action("drive", "truck1")
    for value in (schema, action):
        assert len(value.precondition.literals) == 2
        assert value.precondition.numeric_conditions == ()
        assert len(value.effect.literals) == 2
        assert value.effect.numeric_effects == ()
        assert len(value.conditional_effects) == 1
        conditional = value.conditional_effects[0]
        assert len(conditional.condition.literals) == 1
        assert conditional.condition.numeric_conditions == ()
        assert len(conditional.effect.literals) == 1
        assert conditional.effect.numeric_effects == ()
        assert isinstance(value.cost_expression, m.NumericBinaryExpression)
        assert value.cost_expression.operator is m.NumericOperator.ADD
        assert value.cost_expression.right.value == 1
    lifted = schema.cost_expression.left
    ground = action.cost_expression.left
    assert isinstance(lifted, m.FunctionCall)
    assert isinstance(ground, m.GroundFunctionCall)
    assert lifted.arguments == schema.parameters
    assert ground.arguments == action.arguments
    assert lifted.function == ground.function
    assert problem.initial_state.value(action.cost_expression) == action.cost == 21
    assert problem.initial_state.holds(action.precondition)
    successor = action.apply(problem.initial_state)
    assert successor.value(problem.atom("done", "truck1")) is True
    assert successor.value(problem.atom("ready", "truck1")) is False
    assert successor.value(ground) == 20
    result = m.ucs(problem)
    assert result.solution.cost == 21


def test_queries_reject_unbound_and_foreign_references(problem):
    function = problem.domain.function("fuel")
    variable = problem.variable("?t", "truck")
    lifted = problem.lifted_function_call(function, variable)
    assert lifted.arguments == (variable,)
    with pytest.raises(ValueError, match="ground"):
        problem.initial_state.value(lifted)
    other = m.Problem.from_pddl(problem.domain, PROBLEM)
    foreign = other.function_call("fuel", "truck1")
    with pytest.raises(ValueError, match="different problem"):
        problem.initial_state.value(foreign)
    with pytest.raises(ValueError, match="same owner"):
        _ = foreign + problem.function_call("fuel", "truck1")
    with pytest.raises(ValueError):
        problem.function_call("fuel")
    with pytest.raises(ValueError):
        problem.lifted_function_call(function, other.object("truck1"))
    with pytest.raises(ValueError, match="finite"):
        m.NumericConstant(float("nan"))


def test_numeric_builders_reuse_expressions_for_static_costs():
    fuel = m.Numeric.function("fuel", "?t")
    domain = (m.DomainBuilder("costs")
              .requirements().add(":strips").add(":action-costs").close()
              .predicates().add("done", ("?t", "object")).close()
              .functions().add("fuel", ("?t", "object")).close()
              .actions().add("drive").add_parameter("?t")
              .add_effect("done", "?t").with_cost(fuel * 2 + 1).close().close().build())
    problem = (m.ProblemBuilder(domain, "p").objects().add("truck1").close()
               .initial_state().set_value(m.Numeric.function("fuel", "truck1"), 20).close()
               .goal().add("done", "truck1").close().build())
    assert problem.action("drive", "truck1").cost == 41
    assert problem.initial_state.value(problem.function_call("fuel", "truck1")) == 20


def test_numeric_builders_execute_and_inspect():
    fuel = m.Numeric.function("fuel", "?t")
    domain = (m.DomainBuilder("numeric")
              .requirements().add(":strips").add(":numeric-fluents").add(":conditional-effects").add(":action-costs").close()
              .predicates().add("done", ("?t", "object")).close()
              .functions().add("fuel", ("?t", "object")).add("rate", ("?t", "object")).close()
              .actions().add("drive").add_parameter("?t")
              .add_precondition(fuel.greater_than_or_equal(2)).decrease(fuel, 2).with_cost(m.Numeric.function("rate", "?t"))
              .add_conditional_effect().add_condition(fuel.equal_to(2)).add_effect("done", "?t").close()
              .close().close().build())
    ground = m.Numeric.function("fuel", "a")
    problem = (m.ProblemBuilder(domain, "p").objects().add("a").close()
               .initial_state().set_value(ground, 4).set_value(m.Numeric.function("rate", "a"), 1.5).close()
               .goal().add(ground.equal_to(0)).add("done", "a").close().build())
    action = problem.action("drive", "a")
    fuel_ref = problem.function_call("fuel", "a")
    assert len(domain.action("drive").precondition.numeric_conditions) == 1
    assert domain.action("drive").effect.numeric_effects[0].operator == m.NumericUpdateOperator.DECREASE
    assert action.effect.numeric_effects[0].target == fuel_ref
    assert len(action.conditional_effects[0].condition.numeric_conditions) == 1
    initial = problem.initial_state
    assert action.cost == 1.5
    next_state = action.apply(initial)
    assert next_state.value(fuel_ref) == 2
    assert not next_state.value(problem.atom("done", "a"))
    assert not next_state.holds(problem.goal)
    last = action.apply(next_state)
    assert last.holds(problem.goal)
    assert not action.is_applicable(last)
    assert initial.value(fuel_ref) == 4
    custom = problem.state(numeric_values={fuel_ref: 2})
    assert custom == next_state
    assert len({initial, next_state, last}) == 3
    condition = problem.ground_condition(numeric_conditions=[fuel_ref.greater_than(1)])
    assert initial.holds(condition)
    assert not last.holds(condition)
    assert condition != problem.ground_condition(numeric_conditions=[fuel_ref.greater_than(4)])
    lifted = condition.lift()
    assert len(lifted.numeric_conditions) == 1
    assert len(lifted.bindings(initial)) == 1
    assert lifted.bindings(last) == ()
    assert initial.applicable_actions() == (action,)
    assert last.applicable_actions() == ()
    schema_condition = domain.action("drive").precondition
    assert len(schema_condition.bindings(initial)) == 1
    assert schema_condition.bindings(last) == ()
    with pytest.raises(ValueError, match="complete"):
        problem.state()


@pytest.mark.parametrize("operation, expected", [("assign", 3), ("increase", 7), ("decrease", 1), ("scale-up", 12), ("scale-down", 4 / 3)])
def test_numeric_pddl_updates_and_conditional_inspection(operation, expected):
    domain = m.Domain.from_pddl(f"""(define (domain numerical)
        (:requirements :strips :numeric-fluents :conditional-effects)
        (:functions (fuel))
        (:action act :parameters () :precondition (>= (fuel) 4)
          :effect (when (= (fuel) 4) ({operation} (fuel) 3))))""")
    problem = m.Problem.from_pddl(domain, """(define (problem p) (:domain numerical)
        (:init (= (fuel) 4)) (:goal (> (fuel) 0)))""")
    action = problem.action("act")
    assert domain.action("act").conditional_effects == ()
    assert len(domain.action("act").conditional_numeric_effects) == 1
    assert len(domain.action("act").conditional_numeric_effects[0].effect.numeric_effects) == 1
    assert action.conditional_effects == ()
    assert len(action.conditional_numeric_effects[0].effect.numeric_effects) == 1
    assert action.conditional_numeric_effects[0].effect.literals == ()
    # State values are stored on the 1e-9 comparison grid.
    assert action.apply(problem.initial_state).value(problem.function_call("fuel")) == pytest.approx(expected, abs=1e-9)


def test_missing_numeric_initialization_is_undefined(problem):
    uninitialized = m.Problem.from_pddl(problem.domain, PROBLEM.replace("(= (fuel truck1) 20)", ""))
    fuel = uninitialized.function_call("fuel", "truck1")
    assert math.isnan(uninitialized.initial_state.value(fuel))
    assert not uninitialized.initial_state.holds(fuel.greater_than_or_equal(0))
    # PDDL 2.1: an undefined cost makes the action inapplicable.
    assert uninitialized.initial_state.applicable_actions() == ()


def test_nested_numeric_goals_fail_explicitly():
    domain = m.Domain.from_pddl(DOMAIN.replace(":action-costs)", ":action-costs :numeric-fluents)"))
    with pytest.raises(m.PddlError, match="numeric comparisons"):
        m.Problem.from_pddl(domain, PROBLEM.replace("(:goal (done truck1))", "(:goal (not (> (fuel truck1) 0)))"))


def test_custom_state_numeric_mapping_validation(problem):
    ready = problem.atom("ready", "truck1")
    empty_mapping = problem.state(ready, numeric_values={})
    assert empty_mapping == problem.initial_state
    fuel = problem.function_call("fuel", "truck1")
    assert empty_mapping.value(fuel) == 20
    with pytest.raises(ValueError, match="Static"):
        problem.state(ready, numeric_values={fuel: 20})
    other = m.Problem.from_pddl(problem.domain, PROBLEM)
    with pytest.raises(ValueError, match="different problem"):
        problem.state(numeric_values={other.function_call("fuel", "truck1"): 20})
    with pytest.raises(TypeError, match="keys"):
        problem.state(numeric_values={"fuel": 20})
    with pytest.raises(TypeError, match="int or float"):
        problem.state(numeric_values={fuel: True})
    with pytest.raises(ValueError):
        problem.state(numeric_values={fuel: float("inf")})


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
@pytest.mark.parametrize("algorithm", ["bfs", "ucs", "astar"])
def test_numeric_generators_and_planners_use_static_numeric_costs(generator, algorithm):
    domain = m.Domain.from_pddl("""(define (domain steps)
        (:requirements :strips :typing :numeric-fluents :action-costs)
        (:types item) (:functions (fuel ?x - item) (rate ?x - item) (total-cost))
        (:action step :parameters (?x - item)
            :precondition (>= (* (fuel ?x) 2) 2)
            :effect (and (decrease (fuel ?x) 1) (increase (total-cost) (rate ?x)))))""")
    problem = m.Problem.from_pddl(domain, """(define (problem p) (:domain steps)
        (:objects a - item) (:init (= (fuel a) 2) (= (rate a) 1.5) (= (total-cost) 0))
        (:goal (= (fuel a) 0)) (:metric minimize (total-cost)))""", generator=generator)
    condition = domain.action("step").precondition
    assert len(condition.bindings(problem.initial_state)) == 1
    search = getattr(m, algorithm)
    result = search(problem, m.BlindHeuristic(problem)) if algorithm == "astar" else search(problem)
    assert result.is_solved
    assert len(result.solution.plan) == 2
    assert result.solution.cost == 3
    assert result.solution.goal_state.holds(problem.goal)
    assert condition.bindings(result.solution.goal_state) == ()
    assert result.solution.goal_state.applicable_actions() == ()


def test_state_dependent_costs_are_rejected():
    domain = m.Domain.from_pddl("""(define (domain steps)
        (:requirements :strips :numeric-fluents :action-costs)
        (:functions (fuel) (total-cost))
        (:action step :parameters () :precondition (>= (fuel) 1)
            :effect (and (decrease (fuel) 1) (increase (total-cost) (fuel)))))""")
    with pytest.raises(m.PddlError, match="depends on changing numeric fluents"):
        m.Problem.from_pddl(domain, """(define (problem p) (:domain steps)
            (:init (= (fuel) 2) (= (total-cost) 0)) (:goal (= (fuel) 0)) (:metric minimize (total-cost)))""")


def test_unsupported_numeric_heuristics_raise_unsupported_error():
    domain = m.Domain.from_pddl("""(define (domain steps)
        (:requirements :strips :numeric-fluents)
        (:functions (fuel))
        (:action step :parameters () :precondition (>= (fuel) 1) :effect (decrease (fuel) 1)))""")
    problem = m.Problem.from_pddl(domain, "(define (problem p) (:domain steps) (:init (= (fuel) 2)) (:goal (= (fuel) 0)))")
    with pytest.raises(m.UnsupportedError):
        m.iw(problem, max_width=1)
    assert m.gbfs(problem, m.GoalCountHeuristic(problem)).is_solved


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
def test_rich_numeric_builders_and_derived_conditions(generator):
    fuel = m.Numeric.function("fuel", "?t")
    other = m.Numeric.function("fuel", "?u")
    builder = (m.DomainBuilder("rich")
               .requirements().add(":adl").add(":numeric-fluents").add(":derived-predicates").close()
               .predicates().add("available", ("?t", "object")).add("done", ("?t", "object")).close()
               .functions().add("fuel", ("?t", "object")).add("saved").close())
    actions = builder.actions()
    actions.add("drain").add_parameter("?t").decrease(fuel, 2).close()
    choose = actions.add("choose").add_parameter("?t")
    choose.add_precondition(m.Logic.and_(
        m.Logic.atom("available", "?t"),
        m.Logic.forall([("?u", "object")], fuel.greater_than_or_equal(other))))
    choose.add_effect("done", "?t")
    choose.add_conditional_effect().add_condition(m.Logic.or_(
        fuel.greater_than(1), fuel.less_than(5))).increase(m.Numeric.function("saved"), fuel).close()
    choose.close()
    actions.close()
    builder.derived_predicates().define("available", m.Logic.not_(fuel.less_than_or_equal(2))).close()
    domain = builder.build()
    problem_builder = m.ProblemBuilder(domain, "p", generator=generator).objects().add("a").add("b").close()
    problem_builder.initial_state().set_value(m.Numeric.function("fuel", "a"), 4).set_value(
        m.Numeric.function("fuel", "b"), 1).set_value(m.Numeric.function("saved"), 0).close()
    problem = problem_builder.goal().add("done", "a").close().build()
    initial = problem.initial_state
    choose_action = problem.action("choose", "a")
    assert choose_action in initial.applicable_actions()
    assert initial.value(problem.atom("available", "a"))
    assert choose_action.apply(initial).value(problem.function_call("saved")) == 4
    next_state = problem.action("drain", "a").apply(initial)
    assert not next_state.value(problem.atom("available", "a"))
    assert not choose_action.is_applicable(next_state)
    assert choose_action not in next_state.applicable_actions()
    assert initial.value(problem.atom("available", "a"))
    assert len(domain.action("choose").precondition.literals) > 0
    assert choose_action.conditional_effects == ()
    assert len(choose_action.conditional_numeric_effects) == 1


@pytest.mark.parametrize("guard", ["(and)", "(or)", "(or (and) (> (fuel) 0))", "(> (fuel) 0)"])
def test_total_cost_guards_are_rejected_before_simplification(guard):
    with pytest.raises(m.PddlError, match="Conditional action costs"):
        m.Domain.from_pddl(f"""(define (domain guarded-cost)
          (:requirements :adl :numeric-fluents :action-costs)
          (:functions (fuel) (total-cost))
          (:action act :parameters () :effect (when {guard} (increase (total-cost) 1))))""")


def test_effect_strings_include_numeric_parts():
    domain = m.Domain.from_pddl("""(define (domain d)
      (:requirements :strips :numeric-fluents :conditional-effects)
      (:predicates (p))
      (:functions (fuel))
      (:action act :parameters () :precondition (>= (fuel) 1)
        :effect (and (p) (decrease (fuel) 1) (when (>= (fuel) 3) (increase (fuel) 2)))))""")
    problem = m.Problem.from_pddl(domain, "(define (problem q) (:domain d) (:init (= (fuel) 4)) (:goal (p)))")
    for action in (domain.action("act"), problem.action("act")):
        assert str(action.effect) == "(and (p) (decrease (fuel) 1.0))"
        assert str(action.conditional_numeric_effects[0]) == "(when (and (>= (fuel) 3.0)) (and (increase (fuel) 2.0)))"
