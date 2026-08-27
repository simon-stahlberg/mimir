using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Learning.Tests;

public class EncodingTests
{
    [Fact]
    public void StateAndGoalAppendTypesFactsEqualityAndTruthLabels()
    {
        (_, Problem problem) = CreateRichProblem();
        ExtendedState state = problem.InitialState.Expand();
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeState(context, state, "_state");
        Encoding.EncodeGoal(context, state, problem.Goal, "_goal");
        Literal<Fact> flag = GoalLiteral(problem, "flag");
        Encoding.EncodeGoal(
            context,
            state,
            [flag, flag],
            "_duplicate");
        Encoding.EncodeExpressiveGoal(
            context,
            state,
            [flag, flag],
            "_duplicate");
        context.EndInstance();

        Assert.Equal([0, 1], context.Relations["type_relation_object_state"]);
        Assert.Equal([0, 1], context.Relations["type_relation_vehicle_state"]);
        Assert.Equal([0], context.Relations["type_relation_car_state"]);
        Assert.Equal([0, 0, 1, 1], context.Relations["relation_=_state"]);
        Assert.Equal([0, 1], context.Relations["relation_linked_state"]);
        Assert.Equal([0, 1], context.Relations["relation_flag_state"]);
        Assert.Equal([0], context.Relations["relation_ready_state"]);
        Assert.Equal([0], context.Relations["relation_powered_state"]);
        Assert.Equal(
            [0, 1],
            context.Relations["relation_linked_goal_goal_true"]);
        Assert.Equal(
            [0],
            context.Relations["relation_powered_goal_goal_true"]);
        Assert.Equal(
            [1],
            context.Relations["relation_ready_goal_goal_false"]);
        Assert.Equal(
            [0, 1],
            context.Relations["relation_flag_goal_goal_true"]);
        Assert.Equal(
            [0, 1],
            context.Relations["relation_finished_goal_goal_false"]);
        AssertUnaryRows(
            [0, 1, 0, 1],
            context.Relations["relation_flag_duplicate_goal_true"]);
        AssertUnaryRows(
            [0, 1, 0, 1],
            context.Relations["expressive_relation_flag_duplicate_goal_true"]);
        Assert.Equal([2], context.NodeSizes);
        Assert.Equal([2], context.ObjectSizes);
    }

    [Fact]
    public void StateTypesIncludeConstantsAncestorsAndEmptyMemberships()
    {
        Domain domain = Domain.FromText("""
(define (domain typed-state)
  (:requirements :strips :typing)
  (:types entity unused - object car - entity)
  (:constants shared - car)
  (:predicates (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem typed-state-problem)
  (:domain typed-state)
  (:objects local-car - car local-entity - entity)
  (:init)
  (:goal (done)))
""");
        Problem secondProblem = Problem.FromText(domain, """
(define (problem second-typed-state-problem)
  (:domain typed-state)
  (:objects other-entity - entity)
  (:init)
  (:goal (done)))
""");
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeState(context, problem.InitialState.Expand(), "_typed");
        context.EndInstance();
        context.BeginInstance(secondProblem);
        Encoding.EncodeState(
            context,
            secondProblem.InitialState.Expand(),
            "_typed");
        context.EndInstance();

        Assert.Equal(
            [0, 1, 2, 3, 4],
            context.Relations["type_relation_object_typed"]);
        Assert.Equal(
            [0, 1, 2, 3, 4],
            context.Relations["type_relation_entity_typed"]);
        Assert.Equal(
            [0, 1, 3],
            context.Relations["type_relation_car_typed"]);
        Assert.Empty(context.Relations["type_relation_unused_typed"]);
        Assert.Equal(
            [0, 3],
            context.Relations["constant_relation_shared_typed"]);
        Assert.Equal([3, 2], context.NodeSizes);
    }

    [Fact]
    public void StateObjectTypeAndConstantsUseUnaryRelationsWithoutTyping()
    {
        Domain domain = Domain.FromText("""
(define (domain untyped-constants)
  (:requirements :strips)
  (:constants anchor spare)
  (:predicates (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem untyped-constants-problem)
  (:domain untyped-constants)
  (:objects local)
  (:init)
  (:goal (done)))
""");
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeState(
            context,
            problem.InitialState.Expand(),
            "_identity");
        context.EndInstance();

        Assert.Equal(
            [0],
            context.Relations["constant_relation_anchor_identity"]);
        Assert.Equal(
            [1],
            context.Relations["constant_relation_spare_identity"]);
        Assert.Equal(
            [0, 1, 2],
            context.Relations["type_relation_object_identity"]);
        Assert.Equal([3], context.NodeSizes);
    }

    [Fact]
    public void NullaryRelationsBroadcastObjectsAcrossBatchOffsetsAndEmptyProblems()
    {
        Domain domain = Domain.FromText("""
(define (domain nullary-learning)
  (:requirements :strips :derived-predicates)
  (:predicates (static-on) (fluent-on) (derived-on) (false-static))
  (:derived (derived-on) (fluent-on))
  (:action clear
    :parameters ()
    :precondition (fluent-on)
    :effect (not (fluent-on))))
""");
        Problem first = Problem.FromText(domain, """
(define (problem first-nullary)
  (:domain nullary-learning)
  (:objects a b)
  (:init (static-on) (fluent-on))
  (:goal (and (static-on) (fluent-on) (derived-on) (false-static))))
""");
        Problem second = Problem.FromText(domain, """
(define (problem second-nullary)
  (:domain nullary-learning)
  (:objects c)
  (:init (static-on) (fluent-on))
  (:goal (and (static-on) (fluent-on) (derived-on) (false-static))))
""");
        Problem empty = Problem.FromText(domain, """
(define (problem empty-nullary)
  (:domain nullary-learning)
  (:init (static-on) (fluent-on))
  (:goal (and (static-on) (fluent-on) (derived-on) (false-static))))
""");
        var context = new EncodingContext();

        foreach (Problem problem in new[] { first, second })
        {
            ExtendedState state = problem.InitialState.Expand();
            context.BeginInstance(problem);
            Encoding.EncodeState(context, state, "_broadcast");
            Encoding.EncodeGoal(
                context,
                state,
                problem.Goal,
                "_broadcast");
            Encoding.EncodeExpressiveState(context, state, "_broadcast");
            Encoding.EncodeExpressiveGoal(
                context,
                state,
                problem.Goal,
                "_broadcast");
            context.EndInstance();
        }

        int[] objectIds = [0, 1, 4];
        Assert.Equal([4, 1], context.NodeSizes);
        AssertUnaryRows(
            objectIds,
            context.Relations["type_relation_object_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_type_relation_object_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_static-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_fluent-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_derived-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_static-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_fluent-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_derived-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["relation_false-static_broadcast_goal_false"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_static-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_fluent-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_derived-on_broadcast"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_static-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_fluent-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_derived-on_broadcast_goal_true"]);
        AssertUnaryRows(
            objectIds,
            context.Relations["expressive_relation_false-static_broadcast_goal_false"]);

        var emptyContext = new EncodingContext();
        ExtendedState emptyState = empty.InitialState.Expand();
        emptyContext.BeginInstance(empty);
        Encoding.EncodeState(emptyContext, emptyState, "_empty");
        Encoding.EncodeGoal(
            emptyContext,
            emptyState,
            empty.Goal,
            "_empty");
        Encoding.EncodeExpressiveState(emptyContext, emptyState, "_empty");
        Encoding.EncodeExpressiveGoal(
            emptyContext,
            emptyState,
            empty.Goal,
            "_empty");
        emptyContext.EndInstance();

        Assert.Equal([0], emptyContext.NodeSizes);
        string[] emptyRelations =
        [
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
        ];
        Assert.All(
            emptyRelations,
            name => Assert.Empty(emptyContext.Relations[name]));
    }

    [Fact]
    public void ActionListsAllocateOccurrencesInEncoderOrder()
    {
        (_, Problem problem) = CreateRichProblem();
        ExtendedState state = problem.InitialState.Expand();
        GroundAction action = Assert.Single(GetApplicableActions(problem, state));
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeVirtualNode(context);
        Encoding.EncodeActionList(
            context,
            state,
            [action, action],
            "_available");
        Encoding.EncodeActionList(context, state, [action], "_second");
        context.EndInstance();

        Assert.Equal(
            [3, 0, 4, 0],
            context.Relations["action_consume_available"]);
        Assert.Equal([5, 0], context.Relations["action_consume_second"]);
        Assert.Equal([3, 4, 5], context.ActionIndices);
        Assert.Equal([2], context.VirtualIndices);
        Assert.Equal([6], context.NodeSizes);
        Assert.Equal([3], context.ActionSizes);
        Assert.Equal([1], context.VirtualSizes);
    }

    [Fact]
    public void TransitionEffectsEncodePolarityGoalMembershipNullariesAndLinks()
    {
        Domain domain = Domain.FromText("""
(define (domain transition-learning)
  (:requirements :strips :derived-predicates)
  (:predicates (p ?value) (q) (r ?value) (derived-p ?value))
  (:derived (derived-p ?value) (p ?value))
  (:action change
    :parameters (?value)
    :precondition (q)
    :effect (and (p ?value) (r ?value) (not (q)))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem transition-problem)
  (:domain transition-learning)
  (:objects a)
  (:init (q))
  (:goal (and (p a) (derived-p a) (q))))
""");
        ExtendedState source = problem.InitialState.Expand();
        GroundAction action = Assert.Single(GetApplicableActions(problem, source));
        ExtendedState successor = source.Apply(action).Expand();
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeTransitionEffects(
            context,
            source,
            [successor, source, successor],
            [(0, 1), (1, 2), (0, 1)],
            problem.Goal,
            "_transition");
        context.EndInstance();

        Assert.Equal(
            [1, 0, 3, 0],
            context.Relations["p_transition_pos"]);
        Assert.Equal(
            [1, 0, 3, 0],
            context.Relations["p_transition_pos_goal"]);
        Assert.Equal(
            [1, 0, 3, 0],
            context.Relations["r_transition_pos"]);
        Assert.DoesNotContain(
            "r_transition_pos_goal",
            context.Relations.Keys);
        AssertUnaryRows(
            [1, 3],
            context.Relations["q_transition_neg"]);
        AssertUnaryRows(
            [1, 3],
            context.Relations["q_transition_neg_goal"]);
        Assert.Equal(
            [1, 0, 3, 0],
            context.Relations["derived-p_transition_pos"]);
        Assert.Equal(
            [1, 0, 3, 0],
            context.Relations["derived-p_transition_pos_goal"]);
        Assert.Equal(
            [1, 2, 2, 3, 1, 2],
            context.Relations["effect_relation_transition"]);
        Assert.Equal([1, 2, 3], context.ActionIndices);
        Assert.Equal([3], context.ActionSizes);
        Assert.Equal([4], context.NodeSizes);
    }

    [Fact]
    public void NullaryTransitionEffectsUseTransitionNodesAcrossOffsetsAndEmptyProblems()
    {
        Domain domain = Domain.FromText("""
(define (domain nullary-transition)
  (:requirements :strips)
  (:predicates (enabled))
  (:action disable
    :parameters ()
    :precondition (enabled)
    :effect (not (enabled))))
""");
        Problem first = Problem.FromText(domain, """
(define (problem first-nullary-transition)
  (:domain nullary-transition)
  (:objects a b)
  (:init (enabled))
  (:goal (enabled)))
""");
        Problem second = Problem.FromText(domain, """
(define (problem second-nullary-transition)
  (:domain nullary-transition)
  (:objects c)
  (:init (enabled))
  (:goal (enabled)))
""");
        Problem empty = Problem.FromText(domain, """
(define (problem empty-nullary-transition)
  (:domain nullary-transition)
  (:init (enabled))
  (:goal (enabled)))
""");
        var context = new EncodingContext();

        foreach (Problem problem in new[] { first, second })
        {
            ExtendedState state = problem.InitialState.Expand();
            GroundAction action = Assert.Single(GetApplicableActions(problem, state));
            ExtendedState disabled = state.Apply(action).Expand();
            context.BeginInstance(problem);
            Encoding.EncodeTransitionEffects(
                context,
                state,
                [disabled],
                [],
                problem.Goal,
                "_broadcast");
            Encoding.EncodeTransitionEffects(
                context,
                disabled,
                [state],
                [],
                problem.Goal,
                "_broadcast");
            context.EndInstance();
        }

        int[] expectedPositive = [3, 6];
        int[] expectedNegative = [2, 5];
        Assert.Equal([4, 3], context.NodeSizes);
        AssertUnaryRows(
            expectedPositive,
            context.Relations["enabled_broadcast_pos"]);
        AssertUnaryRows(
            expectedNegative,
            context.Relations["enabled_broadcast_neg"]);
        AssertUnaryRows(
            expectedPositive,
            context.Relations["enabled_broadcast_pos_goal"]);
        AssertUnaryRows(
            expectedNegative,
            context.Relations["enabled_broadcast_neg_goal"]);

        var emptyContext = new EncodingContext();
        ExtendedState emptyState = empty.InitialState.Expand();
        GroundAction emptyAction = Assert.Single(GetApplicableActions(empty, emptyState));
        ExtendedState emptyDisabled = emptyState.Apply(emptyAction).Expand();
        emptyContext.BeginInstance(empty);
        Encoding.EncodeTransitionEffects(
            emptyContext,
            emptyState,
            [emptyDisabled],
            [],
            empty.Goal,
            "_empty");
        Encoding.EncodeTransitionEffects(
            emptyContext,
            emptyDisabled,
            [emptyState],
            [],
            empty.Goal,
            "_empty");
        emptyContext.EndInstance();

        Assert.Equal([2], emptyContext.NodeSizes);
        AssertUnaryRows(
            [1],
            emptyContext.Relations["enabled_empty_pos"]);
        AssertUnaryRows(
            [0],
            emptyContext.Relations["enabled_empty_neg"]);
        AssertUnaryRows(
            [1],
            emptyContext.Relations["enabled_empty_pos_goal"]);
        AssertUnaryRows(
            [0],
            emptyContext.Relations["enabled_empty_neg_goal"]);
    }

    [Fact]
    public void VirtualNodeLinksOnlyDeclaredObjectsAndStillExistsWithoutThem()
    {
        string examples = Path.Combine(
            AppContext.BaseDirectory,
            "../../../../../Tests/Examples/gripper");
        Domain domain = Domain.FromFile(Path.Combine(examples, "domain.pddl"));
        Problem problem = Problem.FromFile(domain, Path.Combine(examples, "p01.pddl"));
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeVirtualNode(context);
        context.EndInstance();

        Assert.Equal([6], context.VirtualIndices);
        Assert.Equal(
            [6, 2, 6, 3, 6, 4, 6, 5],
            context.Relations["virtual_node_link"]);

        Domain emptyDomain = Domain.FromText("""
(define (domain empty-virtual)
  (:requirements :strips)
  (:predicates (flag)))
""");
        Problem emptyProblem = Problem.FromText(emptyDomain, """
(define (problem empty-virtual-problem)
  (:domain empty-virtual)
  (:init (flag))
  (:goal (flag)))
""");
        var emptyContext = new EncodingContext();
        emptyContext.BeginInstance(emptyProblem);
        Encoding.EncodeVirtualNode(emptyContext);
        emptyContext.EndInstance();

        Assert.Equal([0], emptyContext.VirtualIndices);
        Assert.Equal([1], emptyContext.NodeSizes);
        Assert.DoesNotContain("virtual_node_link", emptyContext.Relations.Keys);
    }

    [Fact]
    public void ExpressiveStateAndGoalShareOrderedPairNodes()
    {
        (_, Problem problem) = CreateRichProblem();
        ExtendedState state = problem.InitialState.Expand();
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeExpressiveState(context, state, "_state");
        Encoding.EncodeExpressiveGoal(context, state, problem.Goal, "_goal");
        context.EndInstance();

        Assert.Equal([2, 3], context.AuxiliaryIndices);
        Assert.Equal([4], context.NodeSizes);
        Assert.Equal(
            [0, 2, 3, 1],
            context.Relations["expressive_relation_linked_state"]);
        Assert.Equal(
            [0, 2, 3, 1],
            context.Relations["expressive_relation_linked_goal_goal_true"]);
        Assert.Equal(
            [0, 0, 0, 0, 1, 1, 1, 1],
            context.Relations["expressive_relation_=_state"]);
        Assert.Equal(
            [0, 1],
            context.Relations["expressive_type_relation_object_state"]);
        Assert.Equal(
            [0, 1],
            context.Relations["expressive_relation_flag_state"]);
        Assert.Equal(
            [0, 1],
            context.Relations["expressive_relation_flag_goal_goal_true"]);
        Assert.Equal(
            [
                0, 0, 0,
                0, 2, 2,
                2, 3, 0,
                2, 1, 2,
                3, 0, 3,
                3, 2, 1,
                1, 3, 3,
                1, 1, 1,
            ],
            context.Relations["expressive_composition"]);
        Assert.DoesNotContain(
            "expressive_composition_state",
            context.Relations.Keys);
    }

    [Fact]
    public void ExpressiveGoalCanAllocatePairsBeforeStateAndPreservesDuplicates()
    {
        (_, Problem problem) = CreateRichProblem();
        ExtendedState state = problem.InitialState.Expand();
        Literal<Fact> linked = GoalLiteral(problem, "linked");
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeExpressiveGoal(
            context,
            state,
            [linked, linked],
            "_goal");
        Assert.Equal([2, 3], context.AuxiliaryIds);
        Assert.Equal(4, context.NewAuxiliaryId());
        Encoding.EncodeExpressiveState(context, state);
        context.EndInstance();

        Assert.Equal(
            [0, 2, 3, 1, 0, 2, 3, 1],
            context.Relations["expressive_relation_linked_goal_goal_true"]);
        Assert.Equal([2, 3, 4], context.AuxiliaryIndices);
        Assert.Equal([5], context.NodeSizes);
    }

    [Fact]
    public void RepeatedInstancesUseContiguousOffsetsAndAccumulateRelations()
    {
        string examples = Path.Combine(
            AppContext.BaseDirectory,
            "../../../../../Tests/Examples/gripper");
        Domain domain = Domain.FromFile(Path.Combine(examples, "domain.pddl"));
        Problem problem = Problem.FromFile(domain, Path.Combine(examples, "p01.pddl"));
        ExtendedState state = problem.InitialState.Expand();
        GroundAction[] actions = GetApplicableActions(problem, state);
        var context = new EncodingContext();

        for (int instance = 0; instance < 2; instance++)
        {
            context.BeginInstance(problem);
            Encoding.EncodeState(context, state);
            Encoding.EncodeActionList(context, state, actions);
            context.EndInstance();
        }

        Assert.Equal(24, context.NodeCount);
        Assert.Equal([12, 12], context.NodeSizes);
        Assert.Equal([6, 6], context.ObjectSizes);
        Assert.Equal(
            [0, 1, 2, 3, 4, 5, 12, 13, 14, 15, 16, 17],
            context.ObjectIndices);
        Assert.Equal([6, 6], context.ActionSizes);
        Assert.Equal(
            [6, 7, 8, 9, 10, 11, 18, 19, 20, 21, 22, 23],
            context.ActionIndices);
        Assert.Equal([0, 1, 12, 13], context.Relations["relation_room"]);
    }

    [Fact]
    public void DerivedRelationsFollowTheEncodedState()
    {
        (_, Problem problem) = CreateRichProblem();
        ExtendedState initial = problem.InitialState.Expand();
        GroundAction action = Assert.Single(GetApplicableActions(problem, initial));
        ExtendedState successor = initial.Apply(action).Expand();
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Encoding.EncodeState(context, successor);
        Encoding.EncodeExpressiveState(context, successor);
        context.EndInstance();

        Assert.DoesNotContain("relation_ready", context.Relations.Keys);
        Assert.DoesNotContain("relation_powered", context.Relations.Keys);
        Assert.DoesNotContain(
            "expressive_relation_ready",
            context.Relations.Keys);
        Assert.DoesNotContain(
            "expressive_relation_powered",
            context.Relations.Keys);
        Assert.Equal([0, 1], context.Relations["relation_linked"]);
    }

    [Fact]
    public void InvalidInputsAreRejectedBeforeNodeAllocation()
    {
        (Domain domain, Problem problem) = CreateRichProblem();
        (_, Problem foreign) = CreateRichProblem(domain, "foreign-rich");
        ExtendedState state = problem.InitialState.Expand();
        ExtendedState foreignState = foreign.InitialState.Expand();
        GroundAction foreignAction = Assert.Single(
            GetApplicableActions(foreign, foreignState));
        Literal<Fact> powered = GoalLiteral(problem, "powered");
        var negative = new Literal<Fact>(powered.Value, Polarity.Negative);
        var context = new EncodingContext();
        context.BeginInstance(problem);

        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeState(context, foreignState));
        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeGoal(context, state, [negative]));
        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeActionList(context, state, [foreignAction]));
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            Encoding.EncodeTransitionEffects(
                context,
                state,
                [state],
                [(0, -1)],
                [powered]));
        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeTransitionEffects(
                context,
                state,
                [state],
                [],
                [negative]));
        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeTransitionEffects(
                context,
                state,
                [foreignState],
                [],
                [powered]));
        Assert.Throws<ArgumentException>(() =>
            Encoding.EncodeExpressiveGoal(context, state, [negative]));
        Assert.Throws<ArgumentNullException>(() =>
            Encoding.EncodeState(context, state, null!));

        Assert.Empty(context.ActionIds);
        Assert.Empty(context.VirtualIds);
        Assert.Empty(context.AuxiliaryIds);
        Assert.Equal(2, context.NodeCount);
        Assert.Empty(context.Relations);
    }

    private static Literal<Fact> GoalLiteral(Problem problem, string predicateName)
        => problem.Goal.Single(literal =>
            literal.Value.Predicate.Name == predicateName);

    private static (Domain Domain, Problem Problem) CreateRichProblem(
        Domain? existingDomain = null,
        string problemName = "rich-problem")
    {
        Domain domain = existingDomain ?? Domain.FromText("""
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
""");
        Problem problem = Problem.FromText(domain, $$"""
(define (problem {{problemName}})
  (:domain rich-learning)
  (:objects a - car b - vehicle)
  (:init (linked a b) (flag) (ready a))
  (:goal (and
    (linked a b)
    (powered a)
    (ready b)
    (flag)
    (finished))))
""");
        return (domain, problem);
    }

    private static GroundAction[] GetApplicableActions(
        Problem problem,
        ExtendedState state)
        => problem.GetApplicableActionGenerator(state.State)
            .GetApplicableActions(state)
            .ToArray();

    private static void AssertUnaryRows(
        IEnumerable<int> expected,
        IEnumerable<int> actual)
        => Assert.Equal(expected.Order(), actual.Order());

    private static void AssertBinaryRows(
        IReadOnlyList<int> expected,
        IReadOnlyList<int> actual)
        => Assert.Equal(BinaryRows(expected), BinaryRows(actual));

    private static IReadOnlyList<(int First, int Second)> BinaryRows(
        IReadOnlyList<int> values)
    {
        Assert.Equal(0, values.Count % 2);
        return values
            .Chunk(2)
            .Select(row => (row[0], row[1]))
            .OrderBy(row => row.Item1)
            .ThenBy(row => row.Item2)
            .ToArray();
    }
}
