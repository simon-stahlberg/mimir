using System.Collections.Immutable;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Mimir.Search.Space;
using Xunit;
using XunitFact = Xunit.FactAttribute;

namespace Mimir.Search.Tests;

public sealed class GroundedRpgRegressionTests
{
    [XunitFact]
    public void MixedUnsupportedProblemGoal_ThrowsDuringConstruction()
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => CreateProblem(
            """
(define (domain mixed-goal)
  (:requirements :strips :adl)
  (:predicates (g1) (g2) (g3))
  (:action make-g1 :parameters () :precondition () :effect (g1))
  (:action make-g2 :parameters () :precondition () :effect (g2))
  (:action make-g3 :parameters () :precondition () :effect (g3)))
""",
            """
(define (problem mixed-goal-problem)
  (:domain mixed-goal)
  (:init (g1))
  (:goal (and (g1) (or (g2) (g3)))))
"""));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [XunitFact]
    public void MixedUnsupportedGoalOverride_IsRejectedByGoalCondition()
    {
        Problem problem = CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition();
        ArgumentException exception = Assert.Throws<ArgumentException>(() => GoalCondition.FromExpression(
            problem,
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
                Atom(problem, "g1"),
                new GroundedOr(ImmutableArray.Create<IGroundedExpression>(
                    Atom(problem, "g2"),
                    Atom(problem, "g3")))))));

        Assert.Equal("expression", exception.ParamName);
    }

    [XunitFact]
    public void PositiveStaticGoal_ThrowsDuringConstruction()
    {
        Problem problem = CreateProblemForPositiveStaticGoal_ThrowsDuringConstruction();

        AssertConstructorsThrow(problem);
    }

    [XunitFact]
    public void SelfEnablingConditionalEffect_DistinguishesSetSupportFromActionOccurrences()
    {
        Problem problem = CreateProblemForSelfEnablingConditionalEffect_DistinguishesSetSupportFromActionOccurrences();

        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 1d, 2d));
    }

    [XunitFact]
    public void SharedConditionalAction_UsesBothSupportsAndOneSameLayerOccurrence()
    {
        Problem problem = CreateProblemForSharedConditionalAction_UsesBothSupportsAndOneSameLayerOccurrence();
        HeuristicValues expected = new(13d, 7d, 9d, 9d, 9d);
        GoalCondition reversedGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
                Atom(problem, "g2"),
                Atom(problem, "g1"))));

        AssertValues(problem, problem.InitialState, expected);
        AssertValues(problem, problem.InitialState, reversedGoal, expected);
    }

    [XunitFact]
    public void FfPreferredActions_RequireSelectedConditionalGuardToHold()
    {
        Problem problem = CreateProblemForFfPreferredActions_RequireSelectedConditionalGuardToHold();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action makeTrigger =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "make-trigger");
        Mimir.Core.Grounding.Action finish =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "finish");
        FFHeuristic heuristic = new(generator);

        HeuristicEvaluation initial = heuristic.Evaluate(problem.InitialState.Expand());
        Assert.Equal(2d, initial.Value);
        Assert.NotNull(initial.IsPreferredAction);
        Assert.True(initial.IsPreferredAction!(makeTrigger));
        Assert.False(initial.IsPreferredAction(finish));

        State afterTrigger = problem.InitialState.Expand().Apply(makeTrigger);
        HeuristicEvaluation successor = heuristic.Evaluate(afterTrigger.Expand());
        Assert.Equal(1d, successor.Value);
        Assert.NotNull(successor.IsPreferredAction);
        Assert.True(successor.IsPreferredAction!(finish));
    }

    [XunitFact]
    public void FfPreferredActions_RejectViolatedNegativeConditionalGuard()
    {
        Problem problem = CreateProblemForFfPreferredActions_RejectViolatedNegativeConditionalGuard();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action finish =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "finish");

        HeuristicEvaluation evaluation = new FFHeuristic(generator).Evaluate(problem.InitialState.Expand());

        Assert.Equal(1d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.False(evaluation.IsPreferredAction!(finish));
    }

    [XunitFact]
    public void ConservativeGrounding_KeepsPlanEnabledByLaterNegativeDerivedGuard()
    {
        Problem problem = CreateProblemForConservativeGrounding_KeepsPlanEnabledByLaterNegativeDerivedGuard();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        PlanResult result = new BreadthFirstPlanner((_, _) => generator).Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(
            new[] { "clear-base", "enable", "finish" },
            result.Plan.Select(action => action.Schema.Name));
    }

    [XunitFact]
    public void GroundedRpgHeuristics_RejectSatisfiedStateAndGoalFromAnotherProblem()
    {
        Problem first = CreateForeignStateProblem();
        Problem second = CreateForeignStateProblem();
        GoalCondition secondGoal = GoalCondition.FromProblem(second);
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(first);
        IHeuristic[] heuristics =
        [
            new AddHeuristic(generator),
            new MaxHeuristic(generator),
            new FFHeuristic(generator),
            new SetAddHeuristic(generator),
            new H2Heuristic(generator),
        ];

        foreach (IHeuristic heuristic in heuristics)
        {
            ArgumentException exception = Assert.Throws<ArgumentException>(
                () => heuristic.Evaluate(second.InitialState.Expand(), secondGoal));
            Assert.Equal("state", exception.ParamName);
        }
    }

    [XunitFact]
    public void FfPreferredAction_ReturnsFalseForActionFromAnotherContext()
    {
        Problem first = CreateSingleStepProblem("foreign-preferred-action");
        Problem second = CreateSingleStepProblem("foreign-preferred-action");
        GroundedApplicableActionGenerator firstGenerator = SearchTestHelpers.CreateGroundedGenerator(first);
        GroundedApplicableActionGenerator secondGenerator = SearchTestHelpers.CreateGroundedGenerator(second);
        Mimir.Core.Grounding.Action firstFinish =
            SearchTestHelpers.GetApplicableAction(firstGenerator, first.InitialState, "finish");
        Mimir.Core.Grounding.Action secondFinish =
            SearchTestHelpers.GetApplicableAction(secondGenerator, second.InitialState, "finish");

        HeuristicEvaluation evaluation = new FFHeuristic(firstGenerator).Evaluate(first.InitialState.Expand());

        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(firstFinish));
        Assert.False(evaluation.IsPreferredAction(secondFinish));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void SearchBuilder_GroundedRpgHeuristicAcceptsSharedGenerator(bool buildAStar)
    {
        Problem problem = CreateSingleStepProblem();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchBuilder builder = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .WithHeuristic(new FFHeuristic(generator));

        var search = buildAStar ? builder.BuildAStar() : builder.BuildGbfs();

        Assert.True(search.Search().IsSuccess);
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void SearchBuilder_RejectsGroundedRpgHeuristicsUsingAnotherGroundedGenerator(bool buildAStar)
    {
        Problem problem = CreateSingleStepProblem();
        GroundedApplicableActionGenerator heuristicGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        GroundedApplicableActionGenerator searchGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        IHeuristic[] heuristics =
        [
            new AddHeuristic(heuristicGenerator),
            new MaxHeuristic(heuristicGenerator),
            new FFHeuristic(heuristicGenerator),
            new SetAddHeuristic(heuristicGenerator),
            new H2Heuristic(heuristicGenerator),
        ];

        foreach (IHeuristic heuristic in heuristics)
        {
            SearchBuilder builder = new SearchBuilder()
                .WithInitialState(problem.InitialState)
                .WithGoal(problem)
                .WithActionGenerator(searchGenerator)
                .WithHeuristic(heuristic);

            InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
            {
                if (buildAStar)
                {
                    builder.BuildAStar();
                    return;
                }

                builder.BuildGbfs();
            });

            Assert.Contains("same GroundedApplicableActionGenerator instance", exception.Message);
        }
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void SearchBuilder_RejectsGroundedRpgHeuristicWithCliqueGenerator(bool buildAStar)
    {
        Problem problem = CreateSingleStepProblem();
        GroundedApplicableActionGenerator groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchBuilder builder = new SearchBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(new CliqueApplicableActionGenerator(problem))
            .WithHeuristic(new FFHeuristic(groundedGenerator));

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() =>
        {
            if (buildAStar)
            {
                builder.BuildAStar();
                return;
            }

            builder.BuildGbfs();
        });

        Assert.Contains("same GroundedApplicableActionGenerator instance", exception.Message);
    }

    [XunitFact]
    public void DuplicateGoalLiteral_IsCountedOnce()
    {
        Problem problem = CreateProblemForDuplicateGoalLiteral_IsCountedOnce();

        AssertValues(problem, problem.InitialState, new HeuristicValues(3d, 3d, 3d, 3d, 3d));
    }

    [XunitFact]
    public void CheaperQueuedFactLabel_PropagatesToConsumers()
    {
        Problem problem = CreateProblemForCheaperQueuedFactLabel_PropagatesToConsumers();

        AssertValues(problem, problem.InitialState, new HeuristicValues(5d, 5d, 5d, 5d, 5d));
    }

    [XunitFact]
    public void FractionalActionCosts_PropagateExactly()
    {
        Problem problem = CreateProblemForFractionalActionCosts_PropagateExactly();

        AssertValues(problem, problem.InitialState, new HeuristicValues(3.75, 3.75, 3.75, 3.75, 3.75));
    }

    [XunitFact]
    public void EmptyConjunctiveGoal_ReturnsZero()
    {
        Problem problem = CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition1();

        AssertValues(problem, problem.InitialState, new HeuristicValues(0d, 0d, 0d, 0d, 0d));
    }

    [XunitFact]
    public void QuantifiedPositiveEffect_AchievesAllGroundGoalsWithOneOccurrence()
    {
        Problem problem = CreateProblemForQuantifiedPositiveEffect_AchievesAllGroundGoalsWithOneOccurrence();

        AssertValues(problem, problem.InitialState, new HeuristicValues(8d, 4d, 4d, 4d, 4d));
    }

    [XunitFact]
    public void Max_IsAdmissibleForEverySolvableReachableStateInWeightedStripsProblem()
    {
        Problem problem = CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition3();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .WithActionGenerator(generator)
            .Build();
        MaxHeuristic heuristic = new(generator);

        foreach (State state in searchSpace.AllStates)
        {
            double optimalCost = searchSpace.Query.GetCostToGoal(state);
            if (double.IsPositiveInfinity(optimalCost))
                continue;

            double estimate = heuristic.Evaluate(state.Expand()).Value;
            Assert.True(
                estimate <= optimalCost,
                $"hmax={estimate} exceeds optimal cost {optimalCost} for state {state}.");
        }
    }

    private static Problem CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition()
    {
        DomainBuilder domainBuilder = new DomainBuilder("mixed-goal-override");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Add("g3");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-g1");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("make-g2");
        action.AddEffect("g2");
        action.Close();
        action = actions.Add("make-g3");
        action.AddEffect("g3");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "mixed-goal-override-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("g1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForPositiveStaticGoal_ThrowsDuringConstruction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("static-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled");
        predicates.Add("side");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-side");
        action.AddPrecondition("enabled");
        action.AddEffect("side");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "static-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("enabled");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSelfEnablingConditionalEffect_DistinguishesSetSupportFromActionOccurrences()
    {
        DomainBuilder domainBuilder = new DomainBuilder("self-enabling-conditional");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("trigger");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("advance");
        action.AddPrecondition("seed");
        action.AddEffect("trigger");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("trigger");
        conditional0.AddEffect("goal");
        conditional0.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "self-enabling-conditional-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSharedConditionalAction_UsesBothSupportsAndOneSameLayerOccurrence()
    {
        DomainBuilder domainBuilder = new DomainBuilder("shared-conditional-support");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("c1");
        predicates.Add("c2");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-c1");
        action.AddPrecondition("seed");
        action.AddEffect("c1");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-c2");
        action.AddPrecondition("seed");
        action.AddEffect("c2");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("c1");
        conditional1.AddEffect("g1");
        conditional1.Close();
        ConditionalEffectBuilder conditional2 = action.AddConditionalEffect();
        conditional2.AddCondition("c2");
        conditional2.AddEffect("g2");
        conditional2.Close();
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "shared-conditional-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFfPreferredActions_RequireSelectedConditionalGuardToHold()
    {
        DomainBuilder domainBuilder = new DomainBuilder("delayed-preferred-conditional");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("trigger");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-trigger");
        action.AddPrecondition("seed");
        action.AddEffect("trigger");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        ConditionalEffectBuilder conditional3 = action.AddConditionalEffect();
        conditional3.AddCondition("trigger");
        conditional3.AddEffect("goal");
        conditional3.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "delayed-preferred-conditional-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFfPreferredActions_RejectViolatedNegativeConditionalGuard()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-guard-preferred");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-blocked");
        action.AddPrecondition("seed");
        action.AddEffect("blocked");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        ConditionalEffectBuilder conditional4 = action.AddConditionalEffect();
        conditional4.AddCondition("blocked", Polarity.Negative);
        conditional4.AddEffect("goal");
        conditional4.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-guard-preferred-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForConservativeGrounding_KeepsPlanEnabledByLaterNegativeDerivedGuard()
    {
        DomainBuilder domainBuilder = new DomainBuilder("later-negative-derived-guard");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("stage");
        predicates.Add("enabled");
        predicates.Add("goal");
        predicates.Add("blocked");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear-base");
        action.AddPrecondition("base");
        action.AddEffect("base", Polarity.Negative);
        action.AddEffect("stage");
        action.Close();
        action = actions.Add("enable");
        action.AddPrecondition("stage");
        ConditionalEffectBuilder conditional5 = action.AddConditionalEffect();
        conditional5.AddCondition("blocked", Polarity.Negative);
        conditional5.AddEffect("enabled");
        conditional5.Close();
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("enabled");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("blocked", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "later-negative-derived-guard-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDuplicateGoalLiteral_IsCountedOnce()
    {
        DomainBuilder domainBuilder = new DomainBuilder("duplicate-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("goal");
        action.WithCost(3d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "duplicate-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCheaperQueuedFactLabel_PropagatesToConsumers()
    {
        DomainBuilder domainBuilder = new DomainBuilder("queued-cost-improvement");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("expensive-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(10d);
        action.Close();
        action = actions.Add("cheap-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(3d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "queued-cost-improvement-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFractionalActionCosts_PropagateExactly()
    {
        DomainBuilder domainBuilder = new DomainBuilder("fractional-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("reach-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(1.25d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(2.5d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "fractional-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("empty-rpg-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("side");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-side");
        action.AddEffect("side");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "empty-rpg-goal-problem");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForQuantifiedPositiveEffect_AchievesAllGroundGoalsWithOneOccurrence()
    {
        DomainBuilder domainBuilder = new DomainBuilder("quantified-rpg-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        ConditionalEffectBuilder conditional6 = action.AddConditionalEffect();
        conditional6.AddParameter("?x", "item");
        conditional6.AddEffect("done", "?x");
        conditional6.Close();
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "quantified-rpg-effect-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Add("done", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForMixedUnsupportedGoalOverride_IsRejectedByGoalCondition3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("max-admissibility");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("a");
        predicates.Add("b");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("direct");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(9d);
        action.Close();
        action = actions.Add("make-a");
        action.AddPrecondition("start");
        action.AddEffect("a");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-b");
        action.AddPrecondition("start");
        action.AddEffect("b");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("combine");
        action.AddPrecondition("a");
        action.AddPrecondition("b");
        action.AddEffect("goal");
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "max-admissibility-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblem(string domainText, string problemText)
    {
        return SearchTestHelpers.CreateProblemFromText(domainText, problemText);
    }

    private static Problem CreateSingleStepProblem()
        => CreateSingleStepProblem("single-step");

    private static Problem CreateSingleStepProblem(string domainName)
    {
        Domain domain = new DomainBuilder(domainName)
            .Requirements().Add(":strips").Close()
            .Predicates().Add("seed").Add("goal").Close()
            .Actions().Add("finish").AddPrecondition("seed").AddEffect("goal")
                .Close().Close().Build();
        return new ProblemBuilder(domain, $"{domainName}-problem")
            .InitialState().AddFact("seed").Close()
            .Goal().Add("goal").Close().Build();
    }

    private static Problem CreateForeignStateProblem()
    {
        Domain domain = new DomainBuilder("foreign-state")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("goal").Close()
            .Actions().Add("keep-goal").AddPrecondition("goal").AddEffect("goal")
                .Close().Close().Build();
        return new ProblemBuilder(domain, "foreign-state-problem")
            .InitialState().AddFact("goal").Close()
            .Goal().Add("goal").Close().Build();
    }

    private static GroundedAtom Atom(Problem problem, string name)
    {
        return new GroundedAtom(problem.AllPredicates[name], Array.Empty<ITerm>());
    }

    private static void AssertValues(Problem problem, State state, HeuristicValues expected)
    {
        AssertValues(problem, state, GoalCondition.FromProblem(problem), expected);
    }

    private static void AssertValues(
        Problem problem,
        State state,
        GoalCondition goal,
        HeuristicValues expected)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        HeuristicValues actual = new(
            new AddHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new MaxHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new FFHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new SetAddHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new H2Heuristic(generator).Evaluate(state.Expand(), goal).Value);

        Assert.Equal(expected, actual);
    }

    private static void AssertConstructorsThrow(Problem problem)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Throws<NotSupportedException>(() => new AddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new FFHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new SetAddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(generator));
    }

    private static void AssertOverridesThrow(Problem problem, GoalCondition goal)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Throws<NotSupportedException>(() => new AddHeuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
        Assert.Throws<NotSupportedException>(() => new FFHeuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
        Assert.Throws<NotSupportedException>(() => new SetAddHeuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(generator).Evaluate(problem.InitialState.Expand(), goal));
    }

    private sealed record HeuristicValues(double Add, double Max, double FF, double SetAdd, double H2);
}
