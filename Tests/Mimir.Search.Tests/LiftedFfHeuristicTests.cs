using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class LiftedFfHeuristicTests
{
    [Fact]
    public void LiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);

        Assert.Equal(2, heuristic.Evaluate(problem.InitialState.Expand()).Value);

        var afterReachMid = problem.InitialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "reach-mid"));
        Assert.Equal(1, heuristic.Evaluate(afterReachMid.Expand()).Value);

        var afterReachGoal = afterReachMid.Expand().Apply(SearchTestHelpers.GetApplicableAction(groundedGenerator, afterReachMid, "reach-goal"));
        Assert.Equal(0, heuristic.Evaluate(afterReachGoal.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);

        Assert.Equal(7d, heuristic.Evaluate(problem.InitialState.Expand()).Value);

        var afterReachMid = problem.InitialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "reach-mid"));
        Assert.Equal(5d, heuristic.Evaluate(afterReachMid.Expand()).Value);

        var afterReachGoal = afterReachMid.Expand().Apply(SearchTestHelpers.GetApplicableAction(groundedGenerator, afterReachMid, "reach-goal"));
        Assert.Equal(0d, heuristic.Evaluate(afterReachGoal.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsWhenActionCostIsNegative()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ThrowsWhenActionCostIsNegative();

        var heuristic = new LiftedFfHeuristic(problem);
        var error = Assert.Throws<InvalidOperationException>(() => heuristic.Evaluate(problem.InitialState.Expand()));

        Assert.Contains("negative cost", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsWhenActionCostDivisionByZeroOccurs()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ThrowsWhenActionCostDivisionByZeroOccurs();

        var heuristic = new LiftedFfHeuristic(problem);
        var error = Assert.Throws<InvalidOperationException>(() => heuristic.Evaluate(problem.InitialState.Expand()));

        Assert.Contains("Failed to evaluate lifted action 'finish()'", error.Message);
        Assert.NotNull(error.InnerException);
        Assert.Contains("division by zero", error.InnerException!.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(4d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable()
    {
        var problem = CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable();

        var heuristic = new LiftedFfHeuristic(problem);

        InvalidOperationException error = Assert.Throws<InvalidOperationException>(
            () => heuristic.Evaluate(problem.InitialState.Expand()));

        Assert.Contains("Failed to evaluate lifted action 'finish(bad)'", error.Message);
        Assert.NotNull(error.InnerException);
        Assert.Contains("missing an initialization", error.InnerException!.Message);
    }

    [Fact]
    public void LiftedFfHeuristic_DoesNotRegisterEffectProjectionForStaticallyImpossibleAction()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotRegisterEffectProjectionForStaticallyImpossibleAction();
        Mimir.Core.Schemas.Predicate<Fluent> done = Assert.Single(problem.Domain.Fluents);

        _ = new LiftedFfHeuristic(problem);

        Assert.DoesNotContain(
            problem.Context.Fluents,
            fact => ReferenceEquals(fact.Predicate, done));
    }

    [Fact]
    public void LiftedFfHeuristic_RegistersOnlyReachedHighArityEffectBindings()
    {
        var problem = CreateProblemForLiftedFfHeuristic_RegistersOnlyReachedHighArityEffectBindings();
        Mimir.Core.Schemas.Predicate<Fluent> done = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "done");

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.DoesNotContain(
            problem.Context.Fluents,
            fact => ReferenceEquals(fact.Predicate, done));
        Assert.Equal(2d, heuristic.Evaluate(problem.InitialState.Expand()).Value);

        Fact<Fluent> reachedEffect = Assert.Single(
            problem.Context.Fluents,
            fact => ReferenceEquals(fact.Predicate, done));
        Assert.Equal(
            new[] { "a", "b", "c" },
            reachedEffect.Arguments.Select(argument => argument.Name));
    }

    [Fact]
    public void LiftedFfHeuristic_DeduplicatesPositivePreconditionFacts_WhenComputingSupportCost()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DeduplicatesPositivePreconditionFacts_WhenComputingSupportCost();

        var lifted = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var grounded = new FFHeuristic(groundedGenerator);
        var evaluation = lifted.Evaluate(problem.InitialState.Expand());
        var directGoal = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "direct-goal");
        var makeMid = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "make-mid");

        Assert.Equal(grounded.Evaluate(problem.InitialState.Expand()).Value, evaluation.Value);
        Assert.Equal(2d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(makeMid));
        Assert.False(evaluation.IsPreferredAction!(directGoal));
    }

    [Fact]
    public void LiftedFfHeuristic_Evaluate_ReturnsPreferredActionsForRelaxedFirstStep()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain0();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        var distract = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "distract");
        var reachMid = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "reach-mid");

        Assert.Equal(2, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(reachMid));
        Assert.False(evaluation.IsPreferredAction!(distract));
    }

    [Fact]
    public void LiftedFfHeuristic_PreferredActions_ExcludeRelaxedActionBlockedByNegativeFluentPrecondition()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain1();

        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var finish = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "finish");
        var blockedState = problem.InitialState.WithAdditionalFluentFacts(
            new[] { SearchTestHelpers.GetFluentFact(problem, "blocked") });
        var heuristic = new LiftedFfHeuristic(problem);
        var evaluation = heuristic.Evaluate(blockedState.Expand());

        Assert.Equal(1d, evaluation.Value);
        Assert.False(finish.IsApplicable(blockedState.Expand()));
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.False(evaluation.IsPreferredAction!(finish));
    }

    [Fact]
    public void LiftedFfHeuristic_UsesLastExplicitGoalWhenGoalArgumentIsOmitted()
    {
        var problem = CreateProblemForLiftedFfHeuristic_UsesLastExplicitGoalWhenGoalArgumentIsOmitted();

        var heuristic = new LiftedFfHeuristic(problem);
        var midGoal = GoalCondition.FromExpression(problem, new GroundedAtom(problem.AllPredicates["mid"], Array.Empty<ITerm>()));

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand(), midGoal).Value);
        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_RepeatedUnsupportedGoalDoesNotReplaceLastSuccessfulGoal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain3();
        var heuristic = new LiftedFfHeuristic(problem);
        var midGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates["mid"], Array.Empty<ITerm>()));
        var unsupportedGoal = GoalCondition.FromExpression(
            problem,
            new GroundedNot(new GroundedAtom(problem.AllPredicates["start"], Array.Empty<ITerm>())));

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand(), midGoal).Value);
        Assert.Throws<NotSupportedException>(() => heuristic.Evaluate(problem.InitialState.Expand(), unsupportedGoal));
        Assert.Throws<NotSupportedException>(() => heuristic.Evaluate(problem.InitialState.Expand(), unsupportedGoal));
        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_StateValidationFailureDoesNotReplaceLastSuccessfulGoal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain4();
        var heuristic = new LiftedFfHeuristic(problem);
        var midGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates["mid"], Array.Empty<ITerm>()));
        var problemGoal = GoalCondition.FromProblem(problem);
        var otherProblem = SearchTestHelpers.LoadProblem("blocks_4");

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand(), midGoal).Value);
        Assert.Throws<ArgumentException>(() => heuristic.Evaluate(otherProblem.InitialState.Expand(), problemGoal));
        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_EvaluationFailureDoesNotReplaceLastSuccessfulGoal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_EvaluationFailureDoesNotReplaceLastSuccessfulGoal();
        var heuristic = new LiftedFfHeuristic(problem);
        var badGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates["bad"], Array.Empty<ITerm>()));

        Assert.Equal(0, heuristic.Evaluate(problem.InitialState.Expand()).Value);
        Assert.Throws<InvalidOperationException>(() => heuristic.Evaluate(problem.InitialState.Expand(), badGoal));
        Assert.Equal(0, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_Constructor_ThrowsWhenGoalBelongsToDifferentProblem()
    {
        Problem first = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain6();
        Problem second = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain7();

        ArgumentException error = Assert.Throws<ArgumentException>(() => { _ = new LiftedFfHeuristic(first, GoalCondition.FromProblem(second)); });

        Assert.Contains("different problem instance", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_Evaluate_ThrowsWhenGoalBelongsToDifferentProblem()
    {
        Problem first = CreateProblemForLiftedFfHeuristic_Evaluate_ThrowsWhenGoalBelongsToDifferentProblem();
        Problem second = CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain9();
        var heuristic = new LiftedFfHeuristic(first);

        ArgumentException error = Assert.Throws<ArgumentException>(() => heuristic.Evaluate(first.InitialState.Expand(), GoalCondition.FromProblem(second)));

        Assert.Contains("different problem instance", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_IgnoresNegativeAndDerivedPreconditions()
    {
      var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain0();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ReturnsPositiveInfinity_WhenMissingGoalHasNoPossibleAchiever()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain1();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ReturnsPositiveInfinity_WhenOnlyAchieversFailStaticPreconditions()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain2();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_RespectsNegativeStaticPreconditions()
    {
        var problem = CreateProblemForLiftedFfHeuristic_RespectsNegativeStaticPreconditions();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ReturnsZero_WhenAllGoalFactsAlreadyTrue()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain4();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(0, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ReturnsZero_ForGroundedTrueGoal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsZero_ForGroundedTrueGoal();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(0, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_IgnoresActionsWithOnlyNegativeEffects()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain6();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_IgnoresDeleteEffects_WhenPositiveAddAlsoPresent()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain7();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(2, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsForQuantifiedEffects()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain8();

        var error = Assert.Throws<NotSupportedException>(() => { _ = new LiftedFfHeuristic(problem); });

        Assert.Contains("quantified effects", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsForNegativeGoalLiteral()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain9();

        var error = Assert.Throws<NotSupportedException>(() => { _ = new LiftedFfHeuristic(problem); });

        Assert.Contains("positive conjunctive fluent goals", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsForDerivedGoalLiteral()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ThrowsForDerivedGoalLiteral();

        var error = Assert.Throws<NotSupportedException>(() => { _ = new LiftedFfHeuristic(problem); });

        Assert.Contains("positive conjunctive fluent goals", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void ProblemLoadingThrowsForNonConjunctiveGoal()
    {
        PddlLoadException error = Assert.Throws<PddlLoadException>(() => CreateProblem(
            """
(define (domain disjunctive-goal)
  (:requirements :strips :adl)
  (:predicates (p) (q)))
""",
            """
(define (problem disjunctive-goal-problem)
  (:domain disjunctive-goal)
  (:init)
  (:goal (or (p) (q))))
"""));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, error.ErrorCode);
    }

    [Fact]
    public void LiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenChoosingBindings()
    {
        var problem = CreateProblemForLiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenChoosingBindings();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_KeepsBindingsObjectSpecific_WithoutCrossObjectContamination()
    {
        var problem = CreateProblemForLiftedFfHeuristic_KeepsBindingsObjectSpecific_WithoutCrossObjectContamination();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_RespectsRepeatedVariableEqualityConstraints()
    {
        var problem = CreateProblemForLiftedFfHeuristic_RespectsRepeatedVariableEqualityConstraints();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(double.PositiveInfinity, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_HandlesConstantsInPreconditionsAndEffects()
    {
        var problem = CreateProblemForLiftedFfHeuristic_HandlesConstantsInPreconditionsAndEffects();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_HandlesZeroArityPredicatesAndZeroParameterActions()
    {
        var problem = CreateProblemForLiftedFfHeuristic_HandlesZeroArityPredicatesAndZeroParameterActions();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_CountsSingleActionOnce_WhenItAchievesMultipleGoalFacts()
    {
        var problem = CreateProblemForLiftedFfHeuristic_CountsSingleActionOnce_WhenItAchievesMultipleGoalFacts();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_DoesNotDoubleCountSharedSupportChain()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotDoubleCountSharedSupportChain();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(3, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_CountsDifferentGroundBindingsSeparately_WhenBothAreNeeded()
    {
        var problem = CreateProblemForLiftedFfHeuristic_CountsDifferentGroundBindingsSeparately_WhenBothAreNeeded();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(2, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_CanReachFactsThatWereOnlyPreregisteredForStaticallyFeasibleBindings()
    {
        var problem = CreateProblemForLiftedFfHeuristic_CanReachFactsThatWereOnlyPreregisteredForStaticallyFeasibleBindings();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(2, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_IgnoresNegativeAndDerivedPreconditions_InMultiStepParameterizedCase()
    {
        var problem = CreateProblemForLiftedFfHeuristic_IgnoresNegativeAndDerivedPreconditions_InMultiStepParameterizedCase();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(2, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ThrowsForConditionalEffects()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ThrowsForConditionalEffects();

        var error = Assert.Throws<NotSupportedException>(() => { _ = new LiftedFfHeuristic(problem); });

        Assert.Contains("conditional effects", error.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void PlannerFactory_CreatesLiftedFfHeuristic()
    {
    var problem = CreateProblemForPlannerFactory_CreatesLiftedFfHeuristic();

        var heuristic = PlannerFactory.CreateHeuristic("lifted-ff", problem);

        Assert.IsType<LiftedFfHeuristic>(heuristic);
        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void PlannerFactory_CreatesLiftedFfHeuristic_ForAliasName()
    {
        var problem = CreateProblemForPlannerFactory_CreatesLiftedFfHeuristic_ForAliasName();

        var heuristic = PlannerFactory.CreateHeuristic("liftedff", problem);

        Assert.IsType<LiftedFfHeuristic>(heuristic);
        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_UsesFirstAchieverForAlreadySharedSubgoalOnlyOnce()
    {
        var problem = CreateProblemForLiftedFfHeuristic_UsesFirstAchieverForAlreadySharedSubgoalOnlyOnce();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(3, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_PrefersCheapestFirstLayerAchiever()
    {
        var problem = CreateProblemForLiftedFfHeuristic_PrefersCheapestFirstLayerAchiever();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        var expensiveGoal = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "expensive-goal");
        var cheapGoal = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "cheap-goal");

        Assert.Equal(1d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(cheapGoal));
        Assert.False(evaluation.IsPreferredAction!(expensiveGoal));
    }

    [Fact]
    public void LiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenOneBindingIsLegal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenOneBindingIsLegal();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(1, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ChoosesCheapestAccumulatedGoalAchiever()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ChoosesCheapestAccumulatedGoalAchiever();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(51d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_PreferredActionFollowsCheapestAccumulatedPlan()
    {
        var problem = CreateProblemForLiftedFfHeuristic_PreferredActionFollowsCheapestAccumulatedPlan();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        var prepareExpensive = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "prepare-expensive");
        var prepareCheap = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "prepare-cheap");

        Assert.Equal(51d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(prepareCheap));
        Assert.False(evaluation.IsPreferredAction!(prepareExpensive));
    }

    [Fact]
    public void LiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceEarlierReachableGoal()
    {
        var problem = CreateProblemForLiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceEarlierReachableGoal();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(2d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceAlreadyReachedIntermediateFact()
    {
        var problem = CreateProblemForLiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceAlreadyReachedIntermediateFact();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(3d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_ChoosesCheapestAccumulatedParameterizedBinding()
    {
        var problem = CreateProblemForLiftedFfHeuristic_ChoosesCheapestAccumulatedParameterizedBinding();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(51d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_DoesNotPreferCheapMultiEffectActionWithExpensiveSupport()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotPreferCheapMultiEffectActionWithExpensiveSupport();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(41d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_SharedSubgoalUsesBestAccumulatedAchiever()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings3();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(16d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_WeightedThreeLevelBranchChoosesLowerTotalNotLowerLastStep()
    {
        var problem = CreateProblemForLiftedFfHeuristic_WeightedThreeLevelBranchChoosesLowerTotalNotLowerLastStep();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(52d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_NumericActionCostExpressionsUseAccumulatedSupportCost()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings5();

        var heuristic = new LiftedFfHeuristic(problem);

        Assert.Equal(51d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void LiftedFfHeuristic_PreferredActionUpdatesWhenIntermediateFactGetsCheaper()
    {
        var problem = CreateProblemForLiftedFfHeuristic_PreferredActionUpdatesWhenIntermediateFactGetsCheaper();

        var heuristic = new LiftedFfHeuristic(problem);
        var groundedGenerator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        var reachMidExpensive = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "reach-mid-expensive");
        var reachStep = SearchTestHelpers.GetApplicableAction(groundedGenerator, problem.InitialState, "reach-step");

        Assert.Equal(3d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(reachStep));
        Assert.False(evaluation.IsPreferredAction!(reachMidExpensive));
    }

    [Fact]
    public void LiftedFfHeuristic_PreferredActions_ReturnFalseForActionFromDifferentProblemContext()
    {
        Domain domain = new DomainBuilder("preferred-context")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("start").Add("goal").Close()
            .Actions().Add("finish").AddPrecondition("start").AddEffect("goal")
                .Close().Close().Build();
        Problem first = new ProblemBuilder(domain, "preferred-context-first")
            .InitialState().AddFact("start").Close().Goal().Add("goal").Close().Build();
        Problem second = new ProblemBuilder(domain, "preferred-context-second")
            .InitialState().AddFact("start").Close().Goal().Add("goal").Close().Build();
        var heuristic = new LiftedFfHeuristic(first);
        HeuristicEvaluation evaluation = heuristic.Evaluate(first.InitialState.Expand());
        Mimir.Core.Grounding.Action secondAction = SearchTestHelpers.GetApplicableAction(
            SearchTestHelpers.CreateGroundedGenerator(second),
            second.InitialState,
            "finish");

        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.False(evaluation.IsPreferredAction!(secondAction));
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForSimpleUnitCostChain()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForSimpleUnitCostChain();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForWeightedChain()
    {
        var problem = CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings8();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForCompetingGoalAchievers()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForCompetingGoalAchievers();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForLaterCheaperGoalRoute()
    {
        var problem = CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable0();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForSharedSubgoalTwoGoals()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForSharedSubgoalTwoGoals();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForSingleActionAchievesMultipleGoals()
    {
        var problem = CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable2();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForParameterizedObjectsWithStaticPrecondition()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForParameterizedObjectsWithStaticPrecondition();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForRepeatedVariableEqualityConstraint()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForRepeatedVariableEqualityConstraint();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForZeroArityPredicateAndZeroParameterAction()
    {
        var problem = CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable5();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    [Fact]
    public void LiftedFfHeuristic_MatchesGroundedFf_ForNumericFunctionActionCosts()
    {
        var problem = CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForNumericFunctionActionCosts();

        AssertMatchesGroundedFf(problem, problem.InitialState);
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain()
    {
        DomainBuilder domainBuilder = new DomainBuilder("chain");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
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
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "chain-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain()
    {
        DomainBuilder domainBuilder = new DomainBuilder("weighted-chain");
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
        action.WithCost(Numeric.Constant(2d));
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(5d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "weighted-chain-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ThrowsWhenActionCostIsNegative()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Subtract(Numeric.Constant(0d), Numeric.Constant(1d)));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-cost-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ThrowsWhenActionCostDivisionByZeroOccurs()
    {
        DomainBuilder domainBuilder = new DomainBuilder("divide-by-zero-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Divide(Numeric.Constant(1d), Numeric.Constant(0d)));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "divide-by-zero-cost-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unreachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "unreachable-cost-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled", "good");
        initialState.AddNumericInitialization("item-cost", 4d, "good");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "good");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("later-reachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("enable");
        action.AddParameter("?x", "item");
        action.AddPrecondition("start");
        action.AddEffect("enabled", "?x");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "later-reachable-cost-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("enabled", "good");
        initialState.AddNumericInitialization("item-cost", 4d, "good");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "bad");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotRegisterEffectProjectionForStaticallyImpossibleAction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("impossible-projection");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("empty", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?x", "item"), ("?y", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("impossible");
        action.AddParameter("?x", "item");
        action.AddParameter("?y", "item");
        action.AddParameter("?unused", "empty");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "impossible-projection-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_RegistersOnlyReachedHighArityEffectBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("reached-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("allowed", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("combine");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("allowed", "?x", "?y", "?z");
        action.AddEffect("done", "?x", "?y", "?z");
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("done", "?x", "?y", "?z");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "reached-effect-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Add("d", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("allowed", "a", "b", "c");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DeduplicatesPositivePreconditionFacts_WhenComputingSupportCost()
    {
        DomainBuilder domainBuilder = new DomainBuilder("duplicate-preconditions");
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
        ActionSchemaBuilder action = actions.Add("direct-goal");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(3d));
        action.Close();
        action = actions.Add("make-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "duplicate-preconditions-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-actions");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("junk");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("distract");
        action.AddPrecondition("start");
        action.AddEffect("junk");
        action.Close();
        action = actions.Add("reach-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-actions-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-negative-precondition");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("mark-blocked");
        action.AddPrecondition("start");
        action.AddEffect("blocked");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("start");
        action.AddPrecondition("blocked", Polarity.Negative);
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-negative-precondition-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_UsesLastExplicitGoalWhenGoalArgumentIsOmitted()
    {
        DomainBuilder domainBuilder = new DomainBuilder("goal-rebinding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
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
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "goal-rebinding-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unsupported-goal-rebinding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
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
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "unsupported-goal-rebinding-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("invalid-state-goal-rebinding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
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
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "invalid-state-goal-rebinding-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_EvaluationFailureDoesNotReplaceLastSuccessfulGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("failed-evaluation-goal-rebinding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("safe");
        predicates.Add("bad");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-bad");
        action.AddPrecondition("ready");
        action.AddEffect("bad");
        action.AddEffect("safe", Polarity.Negative);
        action.WithCost(Numeric.Subtract(Numeric.Constant(0d), Numeric.Constant(1d)));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "failed-evaluation-goal-rebinding-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.AddFact("safe");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("safe");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constructor-goal-problem-a");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "constructor-goal-problem-a-instance");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constructor-goal-problem-b");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "constructor-goal-problem-b-instance");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_Evaluate_ThrowsWhenGoalBelongsToDifferentProblem()
    {
        DomainBuilder domainBuilder = new DomainBuilder("evaluate-goal-problem-a");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "evaluate-goal-problem-a-instance");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsExpectedRelaxedPlanLength_ForSimpleChain9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("evaluate-goal-problem-b");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "evaluate-goal-problem-b-instance");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("ignored-preconditions");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("reachable", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("blocked", Polarity.Negative, "?x");
        action.AddPrecondition("reachable", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("ready", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "ignored-preconditions-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("missing-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("side");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("distract");
        action.AddPrecondition("start");
        action.AddEffect("side");
        action.Close();
        action = actions.Add("unreachable-goal");
        action.AddPrecondition("blocked");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "missing-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("blocked-by-static");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("allowed", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("allowed", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "blocked-by-static-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_RespectsNegativeStaticPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("blocked-by-negative-static");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("blocked", Polarity.Negative, "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "blocked-by-negative-static-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("blocked", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("satisfied-goals");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("reinforce-g1");
        action.AddPrecondition("g1");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("reinforce-g2");
        action.AddPrecondition("g2");
        action.AddEffect("g2");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "satisfied-goals-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("g1");
        initialState.AddFact("g2");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsZero_ForGroundedTrueGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("true-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "true-goal-problem");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("delete-only");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("erase-start");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.Close();
        action = actions.Add("unreachable-goal");
        action.AddPrecondition("blocked");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "delete-only-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("delete-relaxation");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
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
        action.AddEffect("start", Polarity.Negative);
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "delete-relaxation-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("quantified-effects");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("fan-out");
        action.AddPrecondition("seed");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddParameter("?x", "object");
        conditional0.AddEffect("done", "?x");
        conditional0.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "quantified-effects-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ReturnsRelaxedPlanCost_ForWeightedChain9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-goal-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("p", Polarity.Negative);
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ThrowsForDerivedGoalLiteral()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Add("reachable");
        predicates.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("p"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("reachable");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenChoosingBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("static-binding-choice");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("allowed", ("?x", "object"));
        predicates.Add("owner", ("?x", "object"), ("?item", "object"));
        predicates.Add("done", ("?item", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddParameter("?item", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("allowed", "?x");
        action.AddPrecondition("owner", "?x", "?item");
        action.AddEffect("done", "?item");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "static-binding-choice-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("item1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.AddFact("allowed", "o1");
        initialState.AddFact("owner", "o2", "item1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "item1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_KeepsBindingsObjectSpecific_WithoutCrossObjectContamination()
    {
        DomainBuilder domainBuilder = new DomainBuilder("object-specific");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "object-specific-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_RespectsRepeatedVariableEqualityConstraints()
    {
        DomainBuilder domainBuilder = new DomainBuilder("repeated-variable");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("edge", ("?x", "object"), ("?y", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("close-loop");
        action.AddParameter("?x", "object");
        action.AddPrecondition("edge", "?x", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "repeated-variable-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("edge", "a", "b");
        initialState.AddFact("edge", "b", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_HandlesConstantsInPreconditionsAndEffects()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constants");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("depot", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("at", ("?pkg", "object"), ("?loc", "object"));
        predicates.Add("delivered", ("?pkg", "object"), ("?loc", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("deliver");
        action.AddParameter("?pkg", "object");
        action.AddPrecondition("at", "?pkg", "depot");
        action.AddEffect("delivered", "?pkg", "depot");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "constants-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("package1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("at", "package1", "depot");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("delivered", "package1", "depot");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_HandlesZeroArityPredicatesAndZeroParameterActions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("zero-arity");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("armed");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("fire");
        action.AddPrecondition("armed");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "zero-arity-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("armed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_CountsSingleActionOnce_WhenItAchievesMultipleGoalFacts()
    {
        DomainBuilder domainBuilder = new DomainBuilder("multi-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish-both");
        action.AddPrecondition("start");
        action.AddEffect("g1");
        action.AddEffect("g2");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "multi-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotDoubleCountSharedSupportChain()
    {
        DomainBuilder domainBuilder = new DomainBuilder("shared-support");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.Close();
        action = actions.Add("finish-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("finish-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "shared-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_CountsDifferentGroundBindingsSeparately_WhenBothAreNeeded()
    {
        DomainBuilder domainBuilder = new DomainBuilder("multiple-bindings");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "multiple-bindings-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Add("done", "o2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_CanReachFactsThatWereOnlyPreregisteredForStaticallyFeasibleBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preregistered-facts");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed", ("?x", "object"));
        predicates.Add("enabled", ("?x", "object"));
        predicates.Add("mid", ("?x", "object"));
        predicates.Add("goal", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("build-mid");
        action.AddParameter("?x", "object");
        action.AddPrecondition("seed", "?x");
        action.AddEffect("mid", "?x");
        action.Close();
        action = actions.Add("build-goal");
        action.AddParameter("?x", "object");
        action.AddPrecondition("mid", "?x");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("goal", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preregistered-facts-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed", "o1");
        initialState.AddFact("enabled", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_IgnoresNegativeAndDerivedPreconditions_InMultiStepParameterizedCase()
    {
        DomainBuilder domainBuilder = new DomainBuilder("ignored-preconditions-multistep");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("mid", ("?x", "object"));
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Add("reachable", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("mid", "?x");
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("mid", "?x");
        action.AddPrecondition("blocked", Polarity.Negative, "?x");
        action.AddPrecondition("reachable", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("mid", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "ignored-preconditions-multistep-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ThrowsForConditionalEffects()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-effects");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Add("q");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("advance");
        action.AddPrecondition("p");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("p");
        conditional1.AddEffect("q");
        conditional1.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-effects-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("q");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForPlannerFactory_CreatesLiftedFfHeuristic()
    {
        DomainBuilder domainBuilder = new DomainBuilder("planner-factory");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("advance");
        action.AddEffect("p");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "planner-factory-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("p");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForPlannerFactory_CreatesLiftedFfHeuristic_ForAliasName()
    {
        DomainBuilder domainBuilder = new DomainBuilder("planner-factory-alias");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("advance");
        action.AddEffect("p");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "planner-factory-alias-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("p");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_UsesFirstAchieverForAlreadySharedSubgoalOnlyOnce()
    {
        DomainBuilder domainBuilder = new DomainBuilder("first-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.Close();
        action = actions.Add("make-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "first-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_PrefersCheapestFirstLayerAchiever()
    {
        DomainBuilder domainBuilder = new DomainBuilder("cheapest-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("expensive-goal");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("cheap-goal");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "cheapest-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_RespectsPositiveStaticPreconditions_WhenOneBindingIsLegal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("legal-static-binding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("allowed", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("allowed", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "legal-static-binding-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.AddFact("allowed", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ChoosesCheapestAccumulatedGoalAchiever()
    {
        DomainBuilder domainBuilder = new DomainBuilder("accumulated-goal-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("expensive-ready");
        predicates.Add("cheap-ready");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddPrecondition("start");
        action.AddEffect("expensive-ready");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddPrecondition("start");
        action.AddEffect("cheap-ready");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-cheap");
        action.AddPrecondition("expensive-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-expensive");
        action.AddPrecondition("cheap-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(50d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "accumulated-goal-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_PreferredActionFollowsCheapestAccumulatedPlan()
    {
        DomainBuilder domainBuilder = new DomainBuilder("accumulated-preferred-action");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("expensive-ready");
        predicates.Add("cheap-ready");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddPrecondition("start");
        action.AddEffect("expensive-ready");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddPrecondition("start");
        action.AddEffect("cheap-ready");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-cheap");
        action.AddPrecondition("expensive-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-expensive");
        action.AddPrecondition("cheap-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(50d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "accumulated-preferred-action-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceEarlierReachableGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("later-cheaper-goal");
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
        ActionSchemaBuilder action = actions.Add("direct-goal");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("reach-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "later-cheaper-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_AllowsLaterCheaperSupportToReplaceAlreadyReachedIntermediateFact()
    {
        DomainBuilder domainBuilder = new DomainBuilder("later-cheaper-intermediate");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("step");
        predicates.Add("mid");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("reach-mid-expensive");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("reach-step");
        action.AddPrecondition("start");
        action.AddEffect("step");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("reach-mid-cheap");
        action.AddPrecondition("step");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "later-cheaper-intermediate-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_ChoosesCheapestAccumulatedParameterizedBinding()
    {
        DomainBuilder domainBuilder = new DomainBuilder("accumulated-parameterized-binding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("expensive-ready", ("?x", "item"));
        predicates.Add("cheap-ready", ("?x", "item"));
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddParameter("?x", "item");
        action.AddPrecondition("start");
        action.AddEffect("expensive-ready", "?x");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddParameter("?x", "item");
        action.AddPrecondition("start");
        action.AddEffect("cheap-ready", "?x");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-cheap");
        action.AddParameter("?x", "item");
        action.AddPrecondition("expensive-ready", "?x");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-expensive");
        action.AddParameter("?x", "item");
        action.AddPrecondition("cheap-ready", "?x");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(50d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "accumulated-parameterized-binding-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "item");
        objects.Add("o2", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotPreferCheapMultiEffectActionWithExpensiveSupport()
    {
        DomainBuilder domainBuilder = new DomainBuilder("multi-effect-expensive-support");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("expensive-ready");
        predicates.Add("cheap-ready");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddPrecondition("start");
        action.AddEffect("expensive-ready");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddPrecondition("start");
        action.AddEffect("cheap-ready");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-both-cheap");
        action.AddPrecondition("expensive-ready");
        action.AddEffect("g1");
        action.AddEffect("g2");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-g1-expensive");
        action.AddPrecondition("cheap-ready");
        action.AddEffect("g1");
        action.WithCost(Numeric.Constant(20d));
        action.Close();
        action = actions.Add("finish-g2-expensive");
        action.AddPrecondition("cheap-ready");
        action.AddEffect("g2");
        action.WithCost(Numeric.Constant(20d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "multi-effect-expensive-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("shared-subgoal-best-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("slow-support");
        predicates.Add("fast-support");
        predicates.Add("mid");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-slow-support");
        action.AddPrecondition("start");
        action.AddEffect("slow-support");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("make-fast-support");
        action.AddPrecondition("start");
        action.AddEffect("fast-support");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("make-mid-cheap");
        action.AddPrecondition("slow-support");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("make-mid-expensive");
        action.AddPrecondition("fast-support");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(10d));
        action.Close();
        action = actions.Add("make-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.WithCost(Numeric.Constant(2d));
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(Numeric.Constant(3d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "shared-subgoal-best-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_WeightedThreeLevelBranchChoosesLowerTotalNotLowerLastStep()
    {
        DomainBuilder domainBuilder = new DomainBuilder("weighted-three-level-branch");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("a1");
        predicates.Add("a2");
        predicates.Add("b1");
        predicates.Add("b2");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-a1");
        action.AddPrecondition("start");
        action.AddEffect("a1");
        action.WithCost(Numeric.Constant(40d));
        action.Close();
        action = actions.Add("make-a2");
        action.AddPrecondition("a1");
        action.AddEffect("a2");
        action.WithCost(Numeric.Constant(40d));
        action.Close();
        action = actions.Add("finish-a");
        action.AddPrecondition("a2");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("make-b1");
        action.AddPrecondition("start");
        action.AddEffect("b1");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("make-b2");
        action.AddPrecondition("b1");
        action.AddEffect("b2");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-b");
        action.AddPrecondition("b2");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(50d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "weighted-three-level-branch-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("numeric-accumulated-support");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("route", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("slow-route", ("?r", "route"));
        predicates.Add("fast-route", ("?r", "route"));
        predicates.Add("expensive-ready");
        predicates.Add("cheap-ready");
        predicates.Add("goal");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("setup-cost", ("?r", "route"));
        functions.Add("finish-cost", ("?r", "route"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddParameter("?r", "route");
        action.AddPrecondition("start");
        action.AddPrecondition("slow-route", "?r");
        action.AddEffect("expensive-ready");
        action.WithCost(Numeric.Function("setup-cost", "?r"));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddParameter("?r", "route");
        action.AddPrecondition("start");
        action.AddPrecondition("fast-route", "?r");
        action.AddEffect("cheap-ready");
        action.WithCost(Numeric.Function("setup-cost", "?r"));
        action.Close();
        action = actions.Add("finish-cheap");
        action.AddParameter("?r", "route");
        action.AddPrecondition("expensive-ready");
        action.AddPrecondition("slow-route", "?r");
        action.AddEffect("goal");
        action.WithCost(Numeric.Function("finish-cost", "?r"));
        action.Close();
        action = actions.Add("finish-expensive");
        action.AddParameter("?r", "route");
        action.AddPrecondition("cheap-ready");
        action.AddPrecondition("fast-route", "?r");
        action.AddEffect("goal");
        action.WithCost(Numeric.Function("finish-cost", "?r"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "numeric-accumulated-support-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("slow", "route");
        objects.Add("fast", "route");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("slow-route", "slow");
        initialState.AddFact("fast-route", "fast");
        initialState.AddNumericInitialization("setup-cost", 100d, "slow");
        initialState.AddNumericInitialization("finish-cost", 1d, "slow");
        initialState.AddNumericInitialization("setup-cost", 1d, "fast");
        initialState.AddNumericInitialization("finish-cost", 50d, "fast");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_PreferredActionUpdatesWhenIntermediateFactGetsCheaper()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-later-cheaper-intermediate");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("step");
        predicates.Add("mid");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("reach-mid-expensive");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("reach-step");
        action.AddPrecondition("start");
        action.AddEffect("step");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("reach-mid-cheap");
        action.AddPrecondition("step");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-later-cheaper-intermediate-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForSimpleUnitCostChain()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-chain");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
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
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-chain-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_DoesNotEvaluateCostsForRelaxedUnreachableBindings8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-weighted-chain");
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
        action.WithCost(Numeric.Constant(2d));
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(5d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-weighted-chain-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForCompetingGoalAchievers()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-competing-achievers");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("expensive-ready");
        predicates.Add("cheap-ready");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-expensive");
        action.AddPrecondition("start");
        action.AddEffect("expensive-ready");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("prepare-cheap");
        action.AddPrecondition("start");
        action.AddEffect("cheap-ready");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-cheap");
        action.AddPrecondition("expensive-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish-expensive");
        action.AddPrecondition("cheap-ready");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(50d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-competing-achievers-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-later-cheaper-goal");
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
        ActionSchemaBuilder action = actions.Add("direct-goal");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(100d));
        action.Close();
        action = actions.Add("reach-mid");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-later-cheaper-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForSharedSubgoalTwoGoals()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-shared-subgoal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        action.WithCost(Numeric.Constant(2d));
        action.Close();
        action = actions.Add("finish-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.WithCost(Numeric.Constant(3d));
        action.Close();
        action = actions.Add("finish-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(Numeric.Constant(5d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-shared-subgoal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-multi-achiever");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish-both");
        action.AddPrecondition("start");
        action.AddEffect("g1");
        action.AddEffect("g2");
        action.WithCost(Numeric.Constant(4d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-multi-achiever-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForParameterizedObjectsWithStaticPrecondition()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-static-parameterized");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "item"));
        predicates.Add("allowed", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("allowed", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-static-parameterized-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "item");
        objects.Add("o2", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.AddFact("allowed", "o1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForRepeatedVariableEqualityConstraint()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-repeated-variable");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "item"), ("?y", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("ready", "?x", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-repeated-variable-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "item");
        objects.Add("o2", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "o1", "o1");
        initialState.AddFact("ready", "o1", "o2");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_EvaluatesCostWhenBindingBecomesRelaxedReachable5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-zero-arity");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("start");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-zero-arity-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLiftedFfHeuristic_MatchesGroundedFf_ForNumericFunctionActionCosts()
    {
        DomainBuilder domainBuilder = new DomainBuilder("parity-numeric-action-costs");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("at", ("?x", "location"));
        predicates.Add("connected", ("?from", "location"), ("?to", "location"));
        predicates.Add("goal");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("move-cost", ("?from", "location"), ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?from", "location");
        action.AddParameter("?to", "location");
        action.AddPrecondition("at", "?from");
        action.AddPrecondition("connected", "?from", "?to");
        action.AddEffect("at", "?to");
        action.AddEffect("goal");
        action.WithCost(Numeric.Function("move-cost", "?from", "?to"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "parity-numeric-action-costs-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "location");
        objects.Add("b", "location");
        objects.Add("c", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("at", "a");
        initialState.AddFact("connected", "a", "b");
        initialState.AddFact("connected", "a", "c");
        initialState.AddNumericInitialization("move-cost", 7d, "a", "b");
        initialState.AddNumericInitialization("move-cost", 3d, "a", "c");
        initialState.AddNumericInitialization("move-cost", 9d, "b", "a");
        initialState.AddNumericInitialization("move-cost", 11d, "b", "c");
        initialState.AddNumericInitialization("move-cost", 13d, "c", "a");
        initialState.AddNumericInitialization("move-cost", 15d, "c", "b");
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

    private static void AssertMatchesGroundedFf(Problem problem, State state)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Equal(
            new FFHeuristic(generator).Evaluate(state.Expand()).Value,
            new LiftedFfHeuristic(problem).Evaluate(state.Expand()).Value);
    }
}
