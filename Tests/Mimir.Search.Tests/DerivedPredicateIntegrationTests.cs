using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class DerivedPredicateIntegrationTests
{
    [Fact]
    public void DerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan()
    {
        Problem problem = CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan();
        var goal = GoalCondition.FromProblem(problem);

        var result = new BreadthFirstPlanner().Solve(problem);

        Assert.True(goal.IsSatisfied(problem.InitialState.Expand()));
        Assert.True(result.IsSuccess);
        Assert.Empty(result.Plan);
        Assert.Equal(0, result.PlanLength);
    }

    [Fact]
    public void DerivedGoalBecomesTrueAfterFluentEffect_SearchFindsOneStepPlan()
    {
        Problem problem = CreateProblemForDerivedGoalBecomesTrueAfterFluentEffect_SearchFindsOneStepPlan();
        var goal = GoalCondition.FromProblem(problem);

        var result = new BreadthFirstPlanner().Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { "(make-base)" }, SearchTestHelpers.PlanSteps(result.Plan));
        Assert.True(goal.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Fact]
    public void GroundedGenerator_FiltersActionsWithFalsePositiveDerivedPreconditions()
    {
        Problem problem = CreateProblemForGroundedGenerator_FiltersActionsWithFalsePositiveDerivedPreconditions();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var finishObjects = ApplicableActionArguments(generator, problem.InitialState, "finish");

        Assert.Equal(new[] { "ready-object" }, finishObjects);
    }

    [Fact]
    public void ApplicabilityChangesAfterBaseFluentInvalidatesDerivedPrecondition()
    {
        Problem problem = CreateProblemForApplicabilityChangesAfterBaseFluentInvalidatesDerivedPrecondition();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var finish = ApplicableActions(generator, problem.InitialState, "finish").Single();
        var clear = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "clear-p");

        var afterClear = problem.InitialState.Expand().Apply(clear);

        Assert.True(finish.IsApplicable(problem.InitialState.Expand()));
        Assert.False(finish.IsApplicable(afterClear.Expand()));
        Assert.Empty(ApplicableActions(generator, afterClear, "finish"));
    }

    [Fact]
    public void NegativeDerivedPrecondition_GatesActionApplicability()
    {
        Problem problem = CreateProblemForNegativeDerivedPrecondition_GatesActionApplicability();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var addHazardSafe = ApplicableActions(generator, problem.InitialState, "add-hazard")
            .Single(action => action.Arguments[0].Name == "safe");

        var afterHazard = problem.InitialState.Expand().Apply(addHazardSafe);

        Assert.Equal(new[] { "safe" }, ApplicableActionArguments(generator, problem.InitialState, "protect"));
        Assert.Empty(ApplicableActions(generator, afterHazard, "protect"));
    }

    [Fact]
    public void QuantifiedDerivedPrecondition_SurvivesGroundingAndSearch()
    {
        Problem problem = CreateProblemForQuantifiedDerivedPrecondition_SurvivesGroundingAndSearch();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var result = new BreadthFirstPlanner().Solve(problem);

        Assert.Empty(ApplicableActions(generator, problem.InitialState, "finish"));
        Assert.True(result.IsSuccess);
        Assert.Equal(3, result.PlanLength);
        Assert.Equal("finish", result.Plan[^1].Schema.Name);
        AssertPlanReachesGoal(problem, result.Plan);
    }

    [Fact]
    public void MergedDerivedDefinitions_EnableAlternativeActionBranches()
    {
        Problem problem = CreateProblemForMergedDerivedDefinitions_EnableAlternativeActionBranches();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var result = new BreadthFirstPlanner().Solve(problem);

        Assert.Equal(new[] { "left", "right" }, ApplicableActionArguments(generator, problem.InitialState, "collect"));
        Assert.True(result.IsSuccess);
        Assert.Equal(2, result.PlanLength);
        AssertPlanReachesGoal(problem, result.Plan);
    }

    [Fact]
    public void DerivedConditionOnConditionalEffect_IsEvaluatedDuringStateApply()
    {
        Problem problem = CreateProblemForDerivedConditionOnConditionalEffect_IsEvaluatedDuringStateApply();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var trigger = ApplicableActions(generator, problem.InitialState, "trigger").Single();
        var clearBase = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "clear-base");
        var goalId = SearchTestHelpers.GetFluentFact(problem, "goal");

        var trueResult = problem.InitialState.Expand().Apply(trigger);
        var falseResult = problem.InitialState.Expand().Apply(clearBase).Expand().Apply(trigger);

        Assert.True(trueResult.IsTrue(goalId));
        Assert.False(falseResult.IsTrue(goalId));
    }

    [Fact]
    public void GroundedRpgHeuristics_RejectDerivedPreconditions()
    {
        Problem problem = CreateProblemForGroundedRpgHeuristics_RejectDerivedPreconditions();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Throws<NotSupportedException>(() => new AddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new FFHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new SetAddHeuristic(generator));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(generator.Problem));
    }

    [Fact]
    public void GbfsWithGroundedFf_RejectsDerivedPreconditionGate()
    {
        Problem problem = CreateProblemForGbfsWithGroundedFf_RejectsDerivedPreconditionGate();

        Assert.Throws<NotSupportedException>(() =>
            new GreedyBestFirstPlanner((start, goal) =>
                    new FFHeuristic(SearchTestHelpers.CreateGroundedGenerator(start.Context.Problem, start), goal)).Solve(problem));
    }

    [Fact]
    public void CliqueGenerator_MatchesGroundedGeneratorForPositiveDerivedUnaryPreconditions()
    {
        Problem problem = CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan1();

        var grounded = SearchTestHelpers.CreateGroundedGenerator(problem);
        var clique = new CliqueApplicableActionGenerator(problem);

        Assert.Equal(
            ApplicableActionStrings(grounded, problem.InitialState, "finish"),
            ApplicableActionStrings(clique, problem.InitialState, "finish"));
    }

    [Fact]
    public void CliqueGenerator_MatchesGroundedGeneratorForNegativeDerivedUnaryPreconditions()
    {
        Problem problem = CreateProblemForCliqueGenerator_MatchesGroundedGeneratorForNegativeDerivedUnaryPreconditions();

        var grounded = SearchTestHelpers.CreateGroundedGenerator(problem);
        var clique = new CliqueApplicableActionGenerator(problem);

        Assert.Equal(new[] { "(protect safe)" }, ApplicableActionStrings(grounded, problem.InitialState, "protect"));
        Assert.Equal(
            ApplicableActionStrings(grounded, problem.InitialState, "protect"),
            ApplicableActionStrings(clique, problem.InitialState, "protect"));
    }

    [Fact]
    public void CliqueGenerator_FiltersBinaryDerivedPreconditionPairs()
    {
        Problem problem = CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan3();
        var generator = new CliqueApplicableActionGenerator(problem);

        var actions = ApplicableActionStrings(generator, problem.InitialState, "move");

        Assert.Equal(new[] { "(move a b)", "(move b c)" }, actions);
    }

    [Fact]
    public void CliqueGenerator_FiltersHigherArityDerivedPreconditions()
    {
        Problem problem = CreateProblemForCliqueGenerator_FiltersHigherArityDerivedPreconditions();
        var generator = new CliqueApplicableActionGenerator(problem);

        var actions = ApplicableActionStrings(generator, problem.InitialState, "pick");

        Assert.Equal(new[] { "(pick a b c)" }, actions);
    }

    [Fact]
    public void DerivedPreconditionWithConstantArgument_BindsCorrectly()
    {
        Problem problem = CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan5();
        var generator = new CliqueApplicableActionGenerator(problem);

        var actions = ApplicableActionStrings(generator, problem.InitialState, "finish");

        Assert.Equal(new[] { "(finish package-a)" }, actions);
    }

    [Fact]
    public void CliqueGenerator_ReevaluatesCachedActionAfterDerivedPreconditionChanges()
    {
        Problem problem = CreateProblemForCliqueGenerator_ReevaluatesCachedActionAfterDerivedPreconditionChanges();
        var generator = new CliqueApplicableActionGenerator(problem);

        var firstQuery = ApplicableActions(generator, problem.InitialState, "finish");
        var clear = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "clear-p");
        var afterClear = problem.InitialState.Expand().Apply(clear);

        Assert.Equal(new[] { "(finish a)" }, firstQuery.Select(action => action.ToString()).ToArray());
        Assert.Empty(ApplicableActions(generator, afterClear, "finish"));
    }

    [Fact]
    public void Search_DoesNotSolveThroughShortcutWithFalseDerivedPrecondition()
    {
        Problem problem = CreateProblemForSearch_DoesNotSolveThroughShortcutWithFalseDerivedPrecondition();

        var result = new BreadthFirstPlanner().Solve(problem);

        Assert.True(result.IsSuccess);
        Assert.Equal(new[] { "(advance)", "(finish)" }, SearchTestHelpers.PlanSteps(result.Plan));
        AssertPlanReachesGoal(problem, result.Plan);
    }

    [Fact]
    public void NegativeDerivedConditionOnConditionalEffect_IsRespectedDuringStateApply()
    {
        Problem problem = CreateProblemForNegativeDerivedConditionOnConditionalEffect_IsRespectedDuringStateApply();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var trigger = ApplicableActions(generator, problem.InitialState, "trigger").Single();
        var block = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "block");
        var goalId = SearchTestHelpers.GetFluentFact(problem, "goal");

        var unblockedResult = problem.InitialState.Expand().Apply(trigger);
        var blockedResult = problem.InitialState.Expand().Apply(block).Expand().Apply(trigger);

        Assert.True(unblockedResult.IsTrue(goalId));
        Assert.False(blockedResult.IsTrue(goalId));
    }

    [Fact]
    public void DerivedConditionalDeleteEffect_IsAppliedDuringStateApply()
    {
        Problem problem = CreateProblemForDerivedConditionalDeleteEffect_IsAppliedDuringStateApply();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var clearGoal = ApplicableActions(generator, problem.InitialState, "clear-goal").Single();
        var clearBase = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "clear-base");
        var goalId = SearchTestHelpers.GetFluentFact(problem, "goal");

        var enabledResult = problem.InitialState.Expand().Apply(clearGoal);
        var disabledResult = problem.InitialState.Expand().Apply(clearBase).Expand().Apply(clearGoal);

        Assert.False(enabledResult.IsTrue(goalId));
        Assert.True(disabledResult.IsTrue(goalId));
    }

    [Fact]
    public void MixedFluentAndDerivedGoal_CountsUnsatisfiedGoalsPerLiteral()
    {
        Problem problem = CreateProblemForDerivedGoalBecomesTrueAfterFluentEffect_SearchFindsOneStepPlan0();
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var goal = GoalCondition.FromProblem(problem);
        var makeBase = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "make-base");
        var makeDone = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "make-done");

        var afterBase = problem.InitialState.Expand().Apply(makeBase);
        var afterBaseAndDone = afterBase.Expand().Apply(makeDone);

        Assert.Equal(2, goal.CountUnsatisfiedGoals(problem.InitialState.Expand()));
        Assert.Equal(1, goal.CountUnsatisfiedGoals(afterBase.Expand()));
        Assert.Equal(0, goal.CountUnsatisfiedGoals(afterBaseAndDone.Expand()));
    }

    private static Problem CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-root-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("done");
        predicates.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("done", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-root-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalBecomesTrueAfterFluentEffect_SearchFindsOneStepPlan()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-goal-after-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-base");
        action.AddEffect("base");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("done", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-goal-after-effect-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedGenerator_FiltersActionsWithFalsePositiveDerivedPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-positive-filter");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p", ("?x", "object"));
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("ready", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.And(Logic.Atom("p", "?x"), Logic.Not(Logic.Atom("blocked", "?x"))));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-positive-filter-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("ready-object", "object");
        objects.Add("blocked-object", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p", "ready-object");
        initialState.AddFact("p", "blocked-object");
        initialState.AddFact("blocked", "blocked-object");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "ready-object");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForApplicabilityChangesAfterBaseFluentInvalidatesDerivedPrecondition()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-applicability-change");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("ready", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        action = actions.Add("clear-p");
        action.AddParameter("?x", "object");
        action.AddPrecondition("p", "?x");
        action.AddEffect("p", Polarity.Negative, "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("p", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-applicability-change-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNegativeDerivedPrecondition_GatesActionApplicability()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-derived-gate");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("hazard", ("?x", "object"));
        predicates.Add("protected", ("?x", "object"));
        predicates.Add("unsafe", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("protect");
        action.AddParameter("?x", "object");
        action.AddPrecondition("unsafe", Polarity.Negative, "?x");
        action.AddEffect("protected", "?x");
        action.Close();
        action = actions.Add("add-hazard");
        action.AddParameter("?x", "object");
        action.AddPrecondition("hazard", Polarity.Negative, "?x");
        action.AddEffect("hazard", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("unsafe", Logic.Atom("hazard", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-derived-gate-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("safe", "object");
        objects.Add("bad", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("hazard", "bad");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("protected", "safe");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForQuantifiedDerivedPrecondition_SurvivesGroundingAndSearch()
    {
        DomainBuilder domainBuilder = new DomainBuilder("quantified-derived-precondition");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":typing");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("checked", ("?x", "item"));
        predicates.Add("done");
        predicates.Add("all-checked");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("check");
        action.AddParameter("?x", "item");
        action.AddPrecondition("checked", Polarity.Negative, "?x");
        action.AddEffect("checked", "?x");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("all-checked");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("all-checked", Logic.Forall(new[] { ("?x", "item") }, Logic.Atom("checked", "?x")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "quantified-derived-precondition-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("i2", "item");
        objects.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForMergedDerivedDefinitions_EnableAlternativeActionBranches()
    {
        DomainBuilder domainBuilder = new DomainBuilder("merged-derived-branches");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("via-a", ("?x", "object"));
        predicates.Add("via-b", ("?x", "object"));
        predicates.Add("visited", ("?x", "object"));
        predicates.Add("reachable", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("collect");
        action.AddParameter("?x", "object");
        action.AddPrecondition("reachable", "?x");
        action.AddEffect("visited", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Or(Logic.Atom("via-a", "?x"), Logic.Atom("via-b", "?x")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "merged-derived-branches-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("left", "object");
        objects.Add("right", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("via-a", "left");
        initialState.AddFact("via-b", "right");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "left");
        goals.Add("visited", "right");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedConditionOnConditionalEffect_IsEvaluatedDuringStateApply()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-conditional-effect-apply");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("goal");
        predicates.Add("enabled-derived");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("trigger");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("enabled-derived");
        conditional0.AddEffect("goal");
        conditional0.Close();
        action.Close();
        action = actions.Add("clear-base");
        action.AddPrecondition("base");
        action.AddEffect("base", Polarity.Negative);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("enabled-derived", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-conditional-effect-apply-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedRpgHeuristics_RejectDerivedPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-rpg-current-state");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("goal");
        predicates.Add("ready");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("goal");
        action.Close();
        action = actions.Add("clear-seed");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("seed"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-rpg-current-state-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGbfsWithGroundedFf_RejectsDerivedPreconditionGate()
    {
        DomainBuilder domainBuilder = new DomainBuilder("gbfs-derived-gate");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("key");
        predicates.Add("mid");
        predicates.Add("missing");
        predicates.Add("done");
        predicates.Add("can-finish");
        predicates.Add("impossible");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("tempting");
        action.AddPrecondition("start");
        action.AddPrecondition("impossible");
        action.AddEffect("done");
        action.Close();
        action = actions.Add("make-key");
        action.AddPrecondition("start");
        action.AddEffect("key");
        action.AddEffect("mid");
        action.AddEffect("start", Polarity.Negative);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddPrecondition("can-finish");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("can-finish", Logic.Atom("key"));
        derivedPredicates.Define("impossible", Logic.Atom("missing"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "gbfs-derived-gate-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("clique-positive-derived-unary");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p", ("?x", "object"));
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("ready", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.And(Logic.Atom("p", "?x"), Logic.Not(Logic.Atom("blocked", "?x"))));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "clique-positive-derived-unary-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("ready", "object");
        objects.Add("blocked", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p", "ready");
        initialState.AddFact("p", "blocked");
        initialState.AddFact("blocked", "blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "ready");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCliqueGenerator_MatchesGroundedGeneratorForNegativeDerivedUnaryPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("clique-negative-derived-unary");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("hazard", ("?x", "object"));
        predicates.Add("protected", ("?x", "object"));
        predicates.Add("unsafe", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("protect");
        action.AddParameter("?x", "object");
        action.AddPrecondition("unsafe", Polarity.Negative, "?x");
        action.AddEffect("protected", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("unsafe", Logic.Atom("hazard", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "clique-negative-derived-unary-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("safe", "object");
        objects.Add("bad", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("hazard", "bad");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("protected", "safe");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("clique-binary-derived");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("edge", ("?from", "object"), ("?to", "object"));
        predicates.Add("moved", ("?from", "object"), ("?to", "object"));
        predicates.Add("connected", ("?from", "object"), ("?to", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?from", "object");
        action.AddParameter("?to", "object");
        action.AddPrecondition("connected", "?from", "?to");
        action.AddEffect("moved", "?from", "?to");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("connected", Logic.Atom("edge", "?from", "?to"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "clique-binary-derived-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("edge", "a", "b");
        initialState.AddFact("edge", "b", "c");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("moved", "a", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCliqueGenerator_FiltersHigherArityDerivedPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("clique-higher-arity-derived");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("first", ("?x", "object"));
        predicates.Add("second", ("?x", "object"));
        predicates.Add("third", ("?x", "object"));
        predicates.Add("picked", ("?a", "object"), ("?b", "object"), ("?c", "object"));
        predicates.Add("allowed", ("?a", "object"), ("?b", "object"), ("?c", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("pick");
        action.AddParameter("?a", "object");
        action.AddParameter("?b", "object");
        action.AddParameter("?c", "object");
        action.AddPrecondition("allowed", "?a", "?b", "?c");
        action.AddEffect("picked", "?a", "?b", "?c");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("allowed", Logic.And(Logic.Atom("first", "?a"), Logic.Atom("second", "?b"), Logic.Atom("third", "?c")));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "clique-higher-arity-derived-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Add("distractor", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("first", "a");
        initialState.AddFact("second", "b");
        initialState.AddFact("third", "c");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("picked", "a", "b", "c");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalSatisfiedInInitialState_ReturnsZeroLengthPlan5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-constant-argument");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("home", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("link", ("?x", "object"), ("?zone", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("same-zone", ("?x", "object"), ("?zone", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("same-zone", "?x", "home");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("same-zone", Logic.Atom("link", "?x", "?zone"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-constant-argument-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("remote", "object");
        objects.Add("package-a", "object");
        objects.Add("package-b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("link", "package-a", "home");
        initialState.AddFact("link", "package-b", "remote");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "package-a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCliqueGenerator_ReevaluatesCachedActionAfterDerivedPreconditionChanges()
    {
        DomainBuilder domainBuilder = new DomainBuilder("clique-cached-derived-state");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("ready", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        action = actions.Add("clear-p");
        action.AddParameter("?x", "object");
        action.AddPrecondition("p", "?x");
        action.AddEffect("p", Polarity.Negative, "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("ready", Logic.Atom("p", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "clique-cached-derived-state-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSearch_DoesNotSolveThroughShortcutWithFalseDerivedPrecondition()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-derived-shortcut");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("done");
        predicates.Add("missing");
        predicates.Add("shortcut-open");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("shortcut");
        action.AddPrecondition("start");
        action.AddPrecondition("shortcut-open");
        action.AddEffect("done");
        action.Close();
        action = actions.Add("advance");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.AddEffect("start", Polarity.Negative);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("shortcut-open", Logic.Atom("missing"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "false-derived-shortcut-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNegativeDerivedConditionOnConditionalEffect_IsRespectedDuringStateApply()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-derived-conditional-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":negative-preconditions");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("blocked-base");
        predicates.Add("goal");
        predicates.Add("blocked-derived");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("trigger");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("blocked-derived", Polarity.Negative);
        conditional1.AddEffect("goal");
        conditional1.Close();
        action.Close();
        action = actions.Add("block");
        action.AddEffect("blocked-base");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("blocked-derived", Logic.Atom("blocked-base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-derived-conditional-effect-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedConditionalDeleteEffect_IsAppliedDuringStateApply()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-conditional-delete-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("goal");
        predicates.Add("enabled-derived");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear-goal");
        ConditionalEffectBuilder conditional2 = action.AddConditionalEffect();
        conditional2.AddCondition("enabled-derived");
        conditional2.AddEffect("goal", Polarity.Negative);
        conditional2.Close();
        action.Close();
        action = actions.Add("clear-base");
        action.AddPrecondition("base");
        action.AddEffect("base", Polarity.Negative);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("enabled-derived", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-conditional-delete-effect-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("base");
        initialState.AddFact("goal");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalBecomesTrueAfterFluentEffect_SearchFindsOneStepPlan0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("mixed-derived-goal-count");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base");
        predicates.Add("done");
        predicates.Add("derived-done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-base");
        action.AddEffect("base");
        action.Close();
        action = actions.Add("make-done");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("derived-done", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "mixed-derived-goal-count-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Add("derived-done");
        goals.Close();
        return problemBuilder.Build();
    }


    private static List<GroundAction> ApplicableActions(
        IApplicableActionGenerator generator,
        State state,
        string actionName)
    {
        return generator.GetApplicableActions(state.Expand())
            .Where(action => action.Schema.Name.Equals(actionName, StringComparison.OrdinalIgnoreCase))
            .ToList();
    }

    private static string[] ApplicableActionArguments(
        IApplicableActionGenerator generator,
        State state,
        string actionName)
    {
        return ApplicableActions(generator, state, actionName)
            .Select(action => action.Arguments.Single().Name)
            .OrderBy(name => name, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private static string[] ApplicableActionStrings(
        IApplicableActionGenerator generator,
        State state,
        string actionName)
    {
        return ApplicableActions(generator, state, actionName)
            .Select(action => action.ToString())
            .OrderBy(text => text, StringComparer.OrdinalIgnoreCase)
            .ToArray();
    }

    private static void AssertPlanReachesGoal(Problem problem, IReadOnlyList<GroundAction> plan)
    {
        var goal = GoalCondition.FromProblem(problem);
        State state = problem.InitialState;
        foreach (var action in plan)
        {
            Assert.True(action.IsApplicable(state.Expand()));
            state = state.Expand().Apply(action);
        }

        Assert.True(goal.IsSatisfied(state.Expand()));
    }

    private static HeuristicValues EvaluateHeuristics(Problem problem, State state)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new HeuristicValues(
            new AddHeuristic(generator).Evaluate(state.Expand()).Value,
            new MaxHeuristic(generator).Evaluate(state.Expand()).Value,
            new FFHeuristic(generator).Evaluate(state.Expand()).Value,
            new SetAddHeuristic(generator).Evaluate(state.Expand()).Value,
            new H2Heuristic(generator.Problem).Evaluate(state.Expand()).Value);
    }

    private static HeuristicValues InfiniteHeuristicValues { get; } = new(
        double.PositiveInfinity,
        double.PositiveInfinity,
        double.PositiveInfinity,
        double.PositiveInfinity,
        double.PositiveInfinity);

    private sealed record HeuristicValues(double Add, double Max, double FF, double SetAdd, double H2);
}
