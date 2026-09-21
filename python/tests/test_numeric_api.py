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
        assert value.precondition.comparisons == ()
        assert len(value.effect.literals) == 2
        assert value.effect.numeric_updates == ()
        assert len(value.conditional_effects) == 1
        conditional = value.conditional_effects[0]
        assert len(conditional.condition.literals) == 1
        assert conditional.condition.comparisons == ()
        assert len(conditional.effect.literals) == 1
        assert conditional.effect.numeric_updates == ()
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


def test_numeric_planning_fails_explicitly_at_builder_boundaries(problem):
    fuel = m.Numeric.function("fuel", "?t")
    builder = m.DomainBuilder("unsupported").actions().add("drive")
    with pytest.raises(NotImplementedError, match="conditions"):
        builder.add_precondition(fuel.greater_than_or_equal(5))
    for method in (builder.assign, builder.increase, builder.decrease):
        with pytest.raises(NotImplementedError, match="updates"):
            method(fuel, 5)
        with pytest.raises(TypeError, match="targets"):
            method(fuel + 1, 5)
    conditional = builder.add_conditional_effect()
    with pytest.raises(NotImplementedError):
        conditional.add_condition(fuel.less_than(5))
    with pytest.raises(NotImplementedError):
        conditional.increase(fuel, 1)
    with pytest.raises(NotImplementedError):
        m.ProblemBuilder(problem.domain, "p").goal().add(fuel.equal_to(2))
    ground_fuel = problem.function_call("fuel", "truck1")
    with pytest.raises(NotImplementedError):
        problem.state(numeric_values={ground_fuel: 4})
    with pytest.raises(NotImplementedError):
        problem.ground_condition(comparisons=[ground_fuel.equal_to(4)])


@pytest.mark.parametrize("body", [
    ":precondition (> (fuel) 0) :effect (done)",
    ":precondition () :effect (increase (fuel) 1)",
])
def test_native_missing_numeric_planning_maps_to_not_implemented(body):
    with pytest.raises(NotImplementedError):
        m.Domain.from_pddl(f"""(define (domain numerical)
            (:requirements :strips :numeric-fluents)
            (:predicates (done)) (:functions (fuel))
            (:action act :parameters () {body}))""")


def test_missing_numeric_initialization_is_not_zero(problem):
    uninitialized = m.Problem.from_pddl(problem.domain, PROBLEM.replace("(= (fuel truck1) 20)", ""))
    fuel = uninitialized.function_call("fuel", "truck1")
    with pytest.raises(m.MimirError, match="missing an initialization"):
        uninitialized.initial_state.value(fuel)


def test_nested_numeric_goals_fail_explicitly():
    domain = m.Domain.from_pddl(DOMAIN.replace(":action-costs)", ":action-costs :numeric-fluents)"))
    with pytest.raises(NotImplementedError):
        m.Problem.from_pddl(domain, PROBLEM.replace("(:goal (done truck1))", "(:goal (not (> (fuel truck1) 0)))"))
