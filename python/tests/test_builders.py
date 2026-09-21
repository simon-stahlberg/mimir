import copy
import gc
import pickle
import weakref

import pytest

import pymimir


def test_builder_exports_are_public_and_complete():
    assert "builders" in pymimir.__all__
    assert set(pymimir.builders.__all__) <= set(pymimir.__all__)
    assert pymimir.advanced.builders is not None
    assert all(
        getattr(pymimir, name) is getattr(pymimir.builders, name)
        for name in pymimir.builders.__all__
    )

    internal_construction_only = (
        pymimir.RequirementListBuilder,
        pymimir.TypeListBuilder,
        pymimir.ConstantListBuilder,
        pymimir.PredicateListBuilder,
        pymimir.NumericFunctionListBuilder,
        pymimir.ActionListBuilder,
        pymimir.ActionSchemaBuilder,
        pymimir.ConditionalEffectBuilder,
        pymimir.DerivedPredicateListBuilder,
        pymimir.ProblemObjectListBuilder,
        pymimir.InitialStateBuilder,
        pymimir.GoalBuilder,
        pymimir.LogicalExpressionSpec,
        pymimir.NumericExpressionSpec,
        pymimir.Logic,
        pymimir.Numeric,
    )
    for value_type in internal_construction_only:
        with pytest.raises(TypeError):
            value_type()


def test_builds_relation_instance_without_pddl_text():
    domain_builder = pymimir.DomainBuilder("relations")
    domain_builder.requirements().add(":strips").close()
    predicates = domain_builder.predicates()
    predicates.add("Nullary")
    predicates.add("Unary", ("?value", "object"))
    predicates.add(
        "Ternary",
        ("?first", "object"),
        ("?second", "object"),
        ("?third", "object"),
    )
    domain = predicates.close().build()

    problem_builder = pymimir.ProblemBuilder(domain, "relation-instance")
    problem_builder.objects().add("a").add("b").add("c").close()
    initial = problem_builder.initial_state()
    initial.add_fact("Nullary")
    initial.add_fact("Unary", "a")
    initial.add_fact("Ternary", "a", "b", "c").close()
    problem = problem_builder.build()

    assert problem.generator == "lifted"
    assert [(predicate.name, predicate.arity) for predicate in domain.predicates] == [
        ("Nullary", 0),
        ("Unary", 1),
        ("Ternary", 3),
    ]
    assert [obj.name for obj in problem.all_objects] == ["a", "b", "c"]
    assert {str(atom) for atom in problem.initial_atoms} == {
        "(Nullary)",
        "(Unary a)",
        "(Ternary a b c)",
    }
    assert len(problem.goal) == 0


def _build_complete_domain() -> pymimir.Domain:
    builder = pymimir.DomainBuilder("complete-builder")
    builder.requirements().add(":adl").add(":typing").add(
        ":derived-predicates"
    ).add(":action-costs").close()
    builder.types().add("item").close()
    builder.constants().add("home", "item").close()
    builder.predicates().add("ready", ("?item", "item")).add(
        "selected", ("?item", "item")
    ).add("done", ("?item", "item")).add(
        "eligible", ("?item", "item")
    ).close()
    builder.functions().add("price", ("?item", "item")).close()

    actions = builder.actions()
    finish = actions.add("finish")
    finish.add_parameter("?item", "item")
    finish.add_precondition("ready", "?item")
    finish.add_precondition("done", "?item", positive=False)
    finish.add_precondition("eligible", "?item")
    finish.add_conditional_effect().add_parameter(
        "?other", "item"
    ).add_condition("selected", "?other").add_effect(
        "done", "?other"
    ).close()
    finish.with_cost(
        pymimir.Numeric.add(
            pymimir.Numeric.function("price", "?item"),
            pymimir.Numeric.divide(
                pymimir.Numeric.multiply(
                    pymimir.Numeric.constant(1),
                    pymimir.Numeric.constant(1),
                ),
                pymimir.Numeric.subtract(
                    pymimir.Numeric.constant(2),
                    pymimir.Numeric.constant(1),
                ),
            ),
        )
    ).close()
    actions.add("wait").with_cost(3).close()
    actions.close()

    tautology = pymimir.Logic.forall(
        [("?other", "item")],
        pymimir.Logic.or_(
            pymimir.Logic.atom("selected", "?other"),
            pymimir.Logic.not_(pymimir.Logic.atom("selected", "?other")),
        ),
    )
    eligibility = pymimir.Logic.and_(
        pymimir.Logic.true(),
        pymimir.Logic.not_(pymimir.Logic.false()),
        pymimir.Logic.imply(
            pymimir.Logic.atom("ready", "?item"),
            pymimir.Logic.atom("ready", "?item"),
        ),
        pymimir.Logic.or_(
            pymimir.Logic.atom("selected", "?item"),
            pymimir.Logic.equal("?item", "home"),
        ),
        pymimir.Logic.exists(
            [("?other", "item")],
            pymimir.Logic.atom("selected", "?other"),
        ),
        tautology,
    )
    builder.derived_predicates().define("eligible", eligibility).close()
    return builder.build()


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
def test_complete_builder_surface_produces_working_problem(generator):
    domain = _build_complete_domain()
    builder = pymimir.ProblemBuilder(domain, "complete-problem", generator=generator)
    builder.objects().add("a", "item").add("b", "item").close()
    builder.initial_state().add_fact("ready", "a").add_fact(
        "selected", "a"
    ).add_fact(
        "selected", "b"
    ).add_numeric_initialization("price", 2.5, "a").close()
    builder.goal().add("done", "a").add("done", "b").add(
        "done", "home", positive=False
    ).add("=", "a", "a").close()
    problem = builder.build()

    assert problem.generator == generator
    assert domain.uses_typing
    assert domain.uses_equality
    assert domain.uses_conditional_effects
    assert domain.predicate("ready").is_static
    assert domain.predicate("done").is_fluent
    assert domain.predicate("eligible").is_derived

    actions = problem.initial_state.applicable_actions()
    finish = next(action for action in actions if action.schema.name == "finish")
    assert finish.cost == pytest.approx(3.5)
    successor = finish.apply(problem.initial_state)
    assert successor.holds(problem.atom("done", "a"))
    assert successor.holds(problem.atom("done", "b"))
    assert successor.holds(problem.goal)


def test_builder_lifecycle_is_native_transactional():
    builder = pymimir.DomainBuilder("lifecycle")
    types = builder.types().add("child", "parent")

    with pytest.raises(pymimir.MimirError):
        builder.predicates()
    with pytest.raises(pymimir.MimirError):
        types.close()

    assert types.add("parent").close() is builder
    with pytest.raises(pymimir.MimirError):
        types.add("another")

    predicates = builder.predicates().add("marked", ("?item", "child"))
    with pytest.raises(ValueError):
        predicates.add("MARKED")
    predicates.close()

    domain = builder.build()
    assert domain.is_type_compatible("child", "parent")
    with pytest.raises(pymimir.MimirError):
        builder.build()


@pytest.mark.parametrize("generator", ["grounded", "lifted"])
def test_built_and_parsed_models_have_runtime_parity(generator):
    built_domain = (
        pymimir.DomainBuilder("parity")
        .requirements()
        .add(":strips")
        .add(":typing")
        .add(":negative-preconditions")
        .close()
        .types()
        .add("item")
        .close()
        .predicates()
        .add("ready", ("?item", "item"))
        .add("done", ("?item", "item"))
        .close()
        .actions()
        .add("finish")
        .add_parameter("?item", "item")
        .add_precondition("ready", "?item")
        .add_precondition("done", "?item", positive=False)
        .add_effect("done", "?item")
        .close()
        .close()
        .build()
    )
    built_problem = (
        pymimir.ProblemBuilder(built_domain, "parity-problem", generator=generator)
        .objects()
        .add("a", "item")
        .close()
        .initial_state()
        .add_fact("ready", "a")
        .close()
        .goal()
        .add("done", "a")
        .close()
        .build()
    )

    parsed_domain = pymimir.Domain.from_pddl("""
        (define (domain parity)
          (:requirements :strips :typing :negative-preconditions)
          (:types item)
          (:predicates (ready ?item - item) (done ?item - item))
          (:action finish
            :parameters (?item - item)
            :precondition (and (ready ?item) (not (done ?item)))
            :effect (done ?item)))
    """)
    parsed_problem = pymimir.Problem.from_pddl(
        parsed_domain,
        """
        (define (problem parity-problem)
          (:domain parity)
          (:objects a - item)
          (:init (ready a))
          (:goal (done a)))
        """,
        generator=generator,
    )

    built_action = built_problem.initial_state.applicable_actions()[0]
    parsed_action = parsed_problem.initial_state.applicable_actions()[0]
    assert str(built_action) == str(parsed_action) == "(finish a)"
    assert built_action.cost == parsed_action.cost
    assert built_action.apply(built_problem.initial_state).holds(built_problem.goal)
    assert parsed_action.apply(parsed_problem.initial_state).holds(parsed_problem.goal)


def test_builder_rejects_invalid_bindings_requirements_and_numbers():
    typed_domain = (
        pymimir.DomainBuilder("invalid-bindings")
        .requirements().add(":strips").add(":typing").close()
        .types().add("item").add("other").close()
        .predicates().add("marked", ("?item", "item")).close()
        .build()
    )
    arity_builder = pymimir.ProblemBuilder(typed_domain, "invalid-arity")
    arity_builder.objects().add("a", "item").close()
    with pytest.raises(ValueError):
        arity_builder.initial_state().add_fact("marked")

    type_builder = pymimir.ProblemBuilder(typed_domain, "invalid-type")
    type_builder.objects().add("b", "other").close()
    with pytest.raises(ValueError):
        type_builder.initial_state().add_fact("marked", "b")

    missing_requirement = pymimir.DomainBuilder("missing-requirement")
    action = missing_requirement.actions().add("expensive").with_cost(2)
    with pytest.raises(pymimir.MimirError, match="action-costs"):
        action.close()

    with pytest.raises(ValueError):
        pymimir.Numeric.constant(float("inf"))


def test_builder_owners_and_completed_values_have_independent_lifetimes():
    builder = pymimir.DomainBuilder("ownership")
    section = builder.predicates().add("p", ("?x", "object"))
    builder_reference = weakref.ref(builder)
    del builder
    gc.collect()
    assert builder_reference() is not None

    builder = section.close()
    assert copy.copy(builder) is builder
    assert copy.deepcopy(builder) is builder
    with pytest.raises(TypeError):
        pickle.dumps(builder)
    domain = builder.build()

    problem_builder = pymimir.ProblemBuilder(domain, "ownership-problem")
    problem_builder.objects().add("a").close()
    problem_builder.initial_state().add_fact("p", "a").close()
    problem = problem_builder.build()
    del section, builder, problem_builder
    gc.collect()

    assert domain.predicate("p").arity == 1
    assert str(problem.initial_atoms[0]) == "(p a)"
    assert problem.domain is domain


def test_native_builders_retain_added_expression_values():
    builder = pymimir.DomainBuilder("expression-ownership")
    builder.requirements().add(":strips").add(":derived-predicates").add(
        ":action-costs"
    ).close()
    builder.predicates().add("base").add("derived").close()

    actions = builder.actions()
    action = actions.add("act")
    cost = pymimir.Numeric.constant(2)
    cost_reference = weakref.ref(cost)
    action.with_cost(cost)
    del cost
    gc.collect()
    assert cost_reference() is None
    action.close().close()

    definitions = builder.derived_predicates()
    body = pymimir.Logic.atom("base")
    body_reference = weakref.ref(body)
    definitions.define("derived", body)
    del body
    gc.collect()
    assert body_reference() is None
    domain = definitions.close().build()

    assert domain.predicate("derived").is_derived
    assert domain.action("act").name == "act"


def test_builder_python_type_guards():
    with pytest.raises(TypeError):
        pymimir.DomainBuilder(1)
    with pytest.raises(TypeError):
        pymimir.ProblemBuilder("domain", "problem")
    with pytest.raises(TypeError):
        pymimir.Logic.and_(pymimir.Logic.true(), "not an expression")
    with pytest.raises(TypeError):
        pymimir.Numeric.add(
            pymimir.Numeric.constant(1), "not a cost"
        )

    builder = pymimir.DomainBuilder("guards")
    predicates = builder.predicates()
    with pytest.raises(TypeError):
        predicates.add("p", ["?x", "object"])
    predicates.add("p", ("?x", "object")).close()
    actions = builder.actions()
    with pytest.raises(TypeError):
        actions.add("act").add_effect("p", "?x", positive=1)
