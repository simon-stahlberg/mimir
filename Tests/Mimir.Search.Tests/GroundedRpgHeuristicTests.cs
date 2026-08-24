using System.Collections.Immutable;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Xunit;

namespace Mimir.Search.Tests;

public class GroundedRpgHeuristicTests
{
    [Fact]
    public void WeightedChain_ReturnsExactValuesAtEachState()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        AssertValues(problem, problem.InitialState, new HeuristicValues(7d, 7d, 7d, 7d, 7d));

        State afterReachMid = problem.InitialState.Expand().Apply(
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "reach-mid"));
        AssertValues(problem, afterReachMid, new HeuristicValues(5d, 5d, 5d, 5d, 5d));

        State afterReachGoal = afterReachMid.Expand().Apply(
            SearchTestHelpers.GetApplicableAction(generator, afterReachMid, "reach-goal"));
        AssertValues(problem, afterReachGoal, new HeuristicValues(0d, 0d, 0d, 0d, 0d));
    }

    [Fact]
    public void SharedSupport_ReturnsDistinctExactValuesForEachHeuristic()
    {
        Problem problem = CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic();

        AssertValues(problem, problem.InitialState, new HeuristicValues(12d, 7d, 10d, 10d, 10d));
    }

    [Fact]
    public void SingleActionAchievesTwoGoalFacts_CountsThatActionExactly()
    {
        Problem problem = CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly();

        AssertValues(problem, problem.InitialState, new HeuristicValues(8d, 4d, 4d, 4d, 4d));
    }

    [Fact]
    public void SetAdd_SelectsSharedSupportThatAddAndFfRejectLocally()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally();

        AssertValues(problem, problem.InitialState, new HeuristicValues(8d, 6d, 8d, 6d, 6d));
    }

    [Fact]
    public void CheapestAchieverDeclaredLater_IsUsedAndPreferredByFf()
    {
        Problem problem = CreateProblemForCheapestAchieverDeclaredLater_IsUsedAndPreferredByFf();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        FFHeuristic heuristic = new(generator);

        AssertValues(problem, problem.InitialState, new HeuristicValues(1d, 1d, 1d, 1d, 1d));

        HeuristicEvaluation evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        Mimir.Core.Grounding.Action expensiveGoal =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "expensive-goal");
        Mimir.Core.Grounding.Action cheapGoal =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "cheap-goal");

        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(cheapGoal));
        Assert.False(evaluation.IsPreferredAction(expensiveGoal));
    }

    [Fact]
    public void UnreachableGoal_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForUnreachableGoal_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void MissingPositiveStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForMissingPositiveStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void BlockedByNegativeStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForBlockedByNegativeStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void NegativeFluentPrecondition_IsIgnoredByDeleteRelaxation()
    {
        Problem problem = CreateProblemForNegativeFluentPrecondition_IsIgnoredByDeleteRelaxation();

        AssertValues(problem, problem.InitialState, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
    }

    [Fact]
    public void DeleteEffects_AreIgnoredByDeleteRelaxation()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState0();

        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void FfPreferredActions_AreOnlyCurrentFirstLayerRelaxedPlanChoices()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState1();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        FFHeuristic heuristic = new(generator);

        HeuristicEvaluation evaluation = heuristic.Evaluate(problem.InitialState.Expand());
        Mimir.Core.Grounding.Action distract =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "distract");
        Mimir.Core.Grounding.Action reachMid =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "reach-mid");
        State afterReachMid = problem.InitialState.Expand().Apply(reachMid);
        Mimir.Core.Grounding.Action reachGoal =
            SearchTestHelpers.GetApplicableAction(generator, afterReachMid, "reach-goal");

        Assert.Equal(2d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(reachMid));
        Assert.False(evaluation.IsPreferredAction(distract));
        Assert.False(evaluation.IsPreferredAction(reachGoal));
    }

    [Fact]
    public void FfPartialUnreachableMultiGoal_ReturnsPositiveInfinity()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState2();

        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Equal(double.PositiveInfinity, new FFHeuristic(generator).Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void SetAddPartialUnreachableMultiGoal_ReturnsPositiveInfinity()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState3();

        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Equal(double.PositiveInfinity, new SetAddHeuristic(generator).Evaluate(problem.InitialState.Expand()).Value);
    }

    [Fact]
    public void OneGoalAlreadyTrueAndOneUnreachable_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState4();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void AllGoalsAlreadyTrue_ReturnsZeroForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState5();

        AssertValues(problem, problem.InitialState, new HeuristicValues(0d, 0d, 0d, 0d, 0d));
    }

    [Fact]
    public void FfPreferredActions_RecomputeForSuccessorState()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState6();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        FFHeuristic heuristic = new(generator);
        Mimir.Core.Grounding.Action reachMid =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "reach-mid");
        State afterReachMid = problem.InitialState.Expand().Apply(reachMid);
        Mimir.Core.Grounding.Action reachGoal =
            SearchTestHelpers.GetApplicableAction(generator, afterReachMid, "reach-goal");

        HeuristicEvaluation initialEvaluation = heuristic.Evaluate(problem.InitialState.Expand());
        HeuristicEvaluation successorEvaluation = heuristic.Evaluate(afterReachMid.Expand());

        Assert.Equal(2d, initialEvaluation.Value);
        Assert.NotNull(initialEvaluation.IsPreferredAction);
        Assert.True(initialEvaluation.IsPreferredAction!(reachMid));
        Assert.False(initialEvaluation.IsPreferredAction(reachGoal));

        Assert.Equal(1d, successorEvaluation.Value);
        Assert.NotNull(successorEvaluation.IsPreferredAction);
        Assert.True(successorEvaluation.IsPreferredAction!(reachGoal));
        Assert.False(successorEvaluation.IsPreferredAction(reachMid));
    }

    [Fact]
    public void FfPreferredActions_ExcludeApplicableActionsOutsideRelaxedPlan()
    {
        Problem problem = CreateProblemForFfPreferredActions_ExcludeApplicableActionsOutsideRelaxedPlan();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        FFHeuristic heuristic = new(generator);
        Mimir.Core.Grounding.Action good =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "good");
        Mimir.Core.Grounding.Action side =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "side");

        HeuristicEvaluation evaluation = heuristic.Evaluate(problem.InitialState.Expand());

        Assert.Equal(1d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(good));
        Assert.False(evaluation.IsPreferredAction(side));
    }

    [Fact]
    public void WeightedSharedSupportWithUnevenCosts_ReturnsExactValuesForEveryGroundedRpgHeuristic()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState8();

        AssertValues(problem, problem.InitialState, new HeuristicValues(23d, 12d, 13d, 13d, 13d));
    }

    [Fact]
    public void ConjunctivePreconditionWithWeightedSupport_ReturnsExactPairCosts()
    {
        Problem problem = CreateProblemForWeightedChain_ReturnsExactValuesAtEachState9();

        AssertValues(problem, problem.InitialState, new HeuristicValues(10d, 8d, 10d, 10d, 10d));
    }

    [Fact]
    public void ZeroCostActions_PropagateExactZeroCostSupport()
    {
        Problem problem = CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic0();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        AssertValues(problem, problem.InitialState, new HeuristicValues(4d, 4d, 4d, 4d, 4d));

        State afterReachMid = problem.InitialState.Expand().Apply(
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "reach-mid"));
        AssertValues(problem, afterReachMid, new HeuristicValues(4d, 4d, 4d, 4d, 4d));

        State afterReachGoal = afterReachMid.Expand().Apply(
            SearchTestHelpers.GetApplicableAction(generator, afterReachMid, "reach-goal"));
        AssertValues(problem, afterReachGoal, new HeuristicValues(0d, 0d, 0d, 0d, 0d));
    }

    [Fact]
    public void StaticPreconditionEnablesExactlyOneBinding()
    {
        Problem problem = CreateProblemForStaticPreconditionEnablesExactlyOneBinding();
        GoalCondition doneO1 = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(
                problem.AllPredicates["done"],
                new ITerm[] { problem.Objects.Single(o => o.Name == "o1") }));

        AssertValues(problem, problem.InitialState, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
        AssertValues(problem, problem.InitialState, doneO1, new HeuristicValues(
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity));
    }

    [Fact]
    public void EvaluateWithGoalOverride_DoesNotMutateDefaultGoal()
    {
        Problem problem = CreateProblemForEvaluateWithGoalOverride_DoesNotMutateDefaultGoal();
        GoalCondition g1Goal = GoalFor(problem, "g1");

        AssertValues(problem, problem.InitialState, g1Goal, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void ConstructorDefaultGoal_IsUsedWhenEvaluateOmitsGoal()
    {
        Problem problem = CreateProblemForConstructorDefaultGoal_IsUsedWhenEvaluateOmitsGoal();
        GoalCondition g1Goal = GoalFor(problem, "g1");
        GoalCondition problemGoal = GoalCondition.FromProblem(problem);

        AssertValuesWithConstructorGoal(problem, problem.InitialState, g1Goal, null, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
        AssertValuesWithConstructorGoal(problem, problem.InitialState, g1Goal, problemGoal, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void NegativeGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape()
    {
        Problem problem = CreateProblemForNegativeGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void DerivedGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape()
    {
        Problem problem = CreateProblemForDerivedGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void ActionWithDerivedPrecondition_ThrowsEvenWhenCurrentlyTrue()
    {
        Problem problem = CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic6();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void ConditionalPositiveEffect_ContributesWhenConditionIsTrueInCurrentState()
    {
        Problem problem = CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic7();

        AssertValues(problem, problem.InitialState, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
    }

    [Fact]
    public void MultipleCurrentStateTrueFactsSatisfyConjunctivePreconditionAtActionCost()
    {
        Problem problem = CreateProblemForMultipleCurrentStateTrueFactsSatisfyConjunctivePreconditionAtActionCost();

        AssertValues(problem, problem.InitialState, new HeuristicValues(5d, 5d, 5d, 5d, 5d));
    }

    [Fact]
    public void RepeatedVariableBinding_DoesNotCrossContaminateObjects()
    {
        Problem problem = CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic9();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void ConstantsInPreconditionsAndEffects_AreGroundedExactly()
    {
        Problem problem = CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly0();
        Constant package1 = problem.Objects.Single(o => o.Name == "package1");
        Constant otherPlace = problem.Objects.Single(o => o.Name == "other-place");
        GoalCondition wrongLocationGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates["delivered"], new ITerm[] { package1, otherPlace }));

        AssertValues(problem, problem.InitialState, new HeuristicValues(3d, 3d, 3d, 3d, 3d));
        AssertValues(problem, problem.InitialState, wrongLocationGoal, new HeuristicValues(
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity));
    }

    [Fact]
    public void TrueGoalReturnsZeroEvenWithUnreachableDistractorActions()
    {
        Problem problem = CreateProblemForTrueGoalReturnsZeroEvenWithUnreachableDistractorActions();

        AssertValues(problem, problem.InitialState, new HeuristicValues(0d, 0d, 0d, 0d, 0d));
    }

    [Fact]
    public void ZeroPreconditionAction_FiresInGroundedRpg()
    {
        Problem problem = CreateProblemForZeroPreconditionAction_FiresInGroundedRpg();

        AssertValues(problem, problem.InitialState, new HeuristicValues(3d, 3d, 3d, 3d, 3d));
    }

    [Fact]
    public void StaticOnlyPreconditionAction_FiresInGroundedRpg()
    {
        Problem problem = CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly3();

        AssertValues(problem, problem.InitialState, new HeuristicValues(4d, 4d, 4d, 4d, 4d));
    }

    [Fact]
    public void UnsupportedNegativeOnlyGoal_ThrowsInsteadOfReturningZero()
    {
        Problem problem = CreateProblemForUnsupportedNegativeOnlyGoal_ThrowsInsteadOfReturningZero();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void UnusedDerivedDefinition_DoesNotBlockGroundedRpg()
    {
        Problem problem = CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly5();

        AssertValues(problem, problem.InitialState, new HeuristicValues(5d, 5d, 5d, 5d, 5d));
    }

    [Fact]
    public void ConditionalPositiveEffect_ContributesWhenConditionIsReachable()
    {
        Problem problem = CreateProblemForConditionalPositiveEffect_ContributesWhenConditionIsReachable();

        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void LateRegisteredReachableGoal_ReturnsExactValueForAlreadyConstructedHeuristics()
    {
        Problem problem = CreateProblemForLateRegisteredReachableGoal_ReturnsExactValueForAlreadyConstructedHeuristics();
        Heuristics heuristics = CreateHeuristics(problem);
        GoalCondition lateGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(
                problem.AllPredicates["done"],
                new ITerm[] { problem.Objects.Single(o => o.Name == "a") }));

        AssertValues(heuristics, problem.InitialState, lateGoal, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
    }

    [Fact]
    public void LateRegisteredUnreachableGoal_ReturnsInfinityForAlreadyConstructedHeuristics()
    {
        Problem problem = CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly8();
        Heuristics heuristics = CreateHeuristics(problem);
        GoalCondition lateGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAtom(
                problem.AllPredicates["done"],
                new ITerm[] { problem.Objects.Single(o => o.Name == "b") }));

        AssertValues(heuristics, problem.InitialState, lateGoal, new HeuristicValues(
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity,
            double.PositiveInfinity));
    }

    [Fact]
    public void LateRegisteredTrueGoal_IsSkippedWhileCompiledGoalIsStillEvaluated()
    {
        Problem problem = CreateProblemForLateRegisteredTrueGoal_IsSkippedWhileCompiledGoalIsStillEvaluated();
        Heuristics heuristics = CreateHeuristics(problem);
        Constant a = problem.Objects.Single(constant => constant.Name == "a");
        Constant b = problem.Objects.Single(constant => constant.Name == "b");
        Predicate done = problem.AllPredicates["done"];
        GoalCondition lateGoal = GoalCondition.FromExpression(
            problem,
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
                new GroundedAtom(done, new ITerm[] { a }),
                new GroundedAtom(done, new ITerm[] { b }))));
        Fact<Fluent> lateFact = problem.Context.Fluents.Single(fact =>
            ReferenceEquals(fact.Predicate, done)
            && ReferenceEquals(fact.Arguments.Single(), b));
        State state = problem.InitialState.WithAdditionalFluentFacts([lateFact]);

        AssertValues(heuristics, state, lateGoal, new HeuristicValues(1d, 1d, 1d, 1d, 1d));
    }

    [Fact]
    public void RpgGrounder_ExcludesEffectBehindFalseStaticCondition()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally0();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        Assert.Single(actions);
        Assert.Contains(actions, action => action.Schema.Name == "maybe-goal");
        Assert.DoesNotContain(actions, action => action.Schema.Name == "finish-final");
    }

    [Fact]
    public void RpgGrounder_ConservativelyIncludesEffectBehindFalseDerivedCondition()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally1();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        Assert.Contains(actions, action => action.Schema.Name == "finish-final");
    }

    [Fact]
    public void UnsupportedDisjunctiveGoal_ThrowsInsteadOfReturningZero()
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => CreateProblem(
            """
(define (domain unsupported-disjunctive-goal)
  (:requirements :strips :adl)
  (:predicates (g1) (g2) (side))

  (:action make-side
    :parameters ()
    :precondition ()
    :effect (side)))
""",
            """
(define (problem unsupported-disjunctive-goal-problem)
  (:domain unsupported-disjunctive-goal)
  (:init)
  (:goal (or (g1) (g2))))
"""));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Fact]
    public void ConditionalEffectSupport_IncludesConditionCost()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally2();

        AssertValues(problem, problem.InitialState, new HeuristicValues(5d, 5d, 5d, 5d, 5d));
    }

    [Fact]
    public void ConditionalEffectsFromSameAction_CountSharedActionCostOnceForRelaxedPlanHeuristics()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally3();

        AssertValues(problem, problem.InitialState, new HeuristicValues(8d, 4d, 4d, 4d, 4d));
    }

    [Fact]
    public void ConditionalEffectSupport_IncludesActionPreconditionAndConditionCosts()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally4();

        AssertValues(problem, problem.InitialState, new HeuristicValues(10d, 8d, 10d, 10d, 10d));
    }

    [Fact]
    public void ConditionalEffectWithFalseStaticCondition_RemainsUnreachable()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally5();

        AssertAllPositiveInfinity(problem, problem.InitialState);
    }

    [Fact]
    public void ConditionalEffectWithTrueStaticCondition_ContributesActionCost()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally6();

        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void ConditionalEffectWithFalseDerivedCondition_Throws()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally7();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void ConditionalEffectWithTrueDerivedCondition_Throws()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally8();

        AssertThrowsNotSupportedForAll(problem, problem.InitialState);
    }

    [Fact]
    public void NegativeConditionalFluentCondition_IsIgnoredByDeleteRelaxation()
    {
        Problem problem = CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally9();

        AssertValues(problem, problem.InitialState, new HeuristicValues(2d, 2d, 2d, 2d, 2d));
    }

    [Fact]
    public void FfPreferredAction_IncludesApplicableConditionalEffectAction()
    {
        Problem problem = CreateProblemForFfPreferredAction_IncludesApplicableConditionalEffectAction();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        FFHeuristic heuristic = new(generator);
        Mimir.Core.Grounding.Action finish =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "finish");
        Mimir.Core.Grounding.Action distract =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "distract");

        HeuristicEvaluation evaluation = heuristic.Evaluate(problem.InitialState.Expand());

        Assert.Equal(2d, evaluation.Value);
        Assert.NotNull(evaluation.IsPreferredAction);
        Assert.True(evaluation.IsPreferredAction!(finish));
        Assert.False(evaluation.IsPreferredAction(distract));
    }

    [Fact]
    public void FfAndSetAdd_KeepConditionalEffectSupportPreconditions()
    {
        Problem problem = CreateProblemForFfAndSetAdd_KeepConditionalEffectSupportPreconditions();

        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Equal(5d, new FFHeuristic(generator).Evaluate(problem.InitialState.Expand()).Value);
        Assert.Equal(5d, new SetAddHeuristic(generator).Evaluate(problem.InitialState.Expand()).Value);
    }

    private static Problem CreateProblem(string domainText, string problemText)
    {
        return SearchTestHelpers.CreateProblemFromText(domainText, problemText);
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState()
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(5d);
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

    private static Problem CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic()
    {
        DomainBuilder domainBuilder = new DomainBuilder("shared-support");
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("finish-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("finish-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(5d);
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

    private static Problem CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly()
    {
        DomainBuilder domainBuilder = new DomainBuilder("multi-achiever");
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("g1");
        action.AddEffect("g2");
        action.WithCost(4d);
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

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally()
    {
        DomainBuilder domainBuilder = new DomainBuilder("set-add-shared-support");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("left");
        predicates.Add("right");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("prepare-both");
        action.AddPrecondition("seed");
        action.AddEffect("left");
        action.AddEffect("right");
        action.WithCost(5d);
        action.Close();
        action = actions.Add("finish-shared");
        action.AddPrecondition("left");
        action.AddPrecondition("right");
        action.AddEffect("goal");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("finish-direct");
        action.AddPrecondition("seed");
        action.AddEffect("goal");
        action.WithCost(8d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "set-add-shared-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCheapestAchieverDeclaredLater_IsUsedAndPreferredByFf()
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("goal");
        action.WithCost(100d);
        action.Close();
        action = actions.Add("cheap-goal");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("goal");
        action.WithCost(1d);
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

    private static Problem CreateProblemForUnreachableGoal_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unreachable-goal");
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
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "unreachable-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForMissingPositiveStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
    {
        DomainBuilder domainBuilder = new DomainBuilder("missing-positive-static");
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
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "missing-positive-static-problem");
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

    private static Problem CreateProblemForBlockedByNegativeStaticPrecondition_ReturnsPositiveInfinityForEveryGroundedRpgHeuristic()
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

    private static Problem CreateProblemForNegativeFluentPrecondition_IsIgnoredByDeleteRelaxation()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-fluent-relaxation");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Add("side");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-blocked-fluent");
        action.AddEffect("blocked");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddPrecondition("blocked", Polarity.Negative);
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-fluent-relaxation-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState0()
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
        action.AddPrecondition("start");
        action.AddPrecondition("mid");
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

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState1()
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("junk");
        action.Close();
        action = actions.Add("reach-mid");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
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

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("partial-unreachable");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("blocked");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-g1");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("g1");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("blocked");
        action.AddEffect("g2");
        action.WithCost(7d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "partial-unreachable-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("partial-unreachable-set-add");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("blocked");
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-g1");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("g1");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("blocked");
        action.AddEffect("g2");
        action.WithCost(7d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "partial-unreachable-set-add-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("true-and-unreachable");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("g1");
        predicates.Add("blocked");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-g1");
        action.AddPrecondition("g1");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("blocked");
        action.AddEffect("g2");
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "true-and-unreachable-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("g1");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("goals-already-true");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("g1");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-g1");
        action.AddPrecondition("g1");
        action.AddEffect("g1");
        action.Close();
        action = actions.Add("keep-g2");
        action.AddPrecondition("g2");
        action.AddEffect("g2");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "goals-already-true-problem");
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

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-actions-recompute");
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-actions-recompute-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFfPreferredActions_ExcludeApplicableActionsOutsideRelaxedPlan()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-actions-exclude-side-path");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("goal");
        predicates.Add("junk");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("good");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("goal");
        action.Close();
        action = actions.Add("side");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("junk");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-actions-exclude-side-path-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("uneven-shared-support");
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(10d);
        action.Close();
        action = actions.Add("finish-g1");
        action.AddPrecondition("mid");
        action.AddEffect("g1");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("finish-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "uneven-shared-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForWeightedChain_ReturnsExactValuesAtEachState9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conjunctive-precondition-costs");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("p");
        predicates.Add("q");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-p");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("p");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-q");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("q");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("make-goal");
        action.AddPrecondition("p");
        action.AddPrecondition("q");
        action.AddEffect("goal");
        action.WithCost(5d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conjunctive-precondition-costs-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("zero-cost-support");
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
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(0d);
        action.Close();
        action = actions.Add("reach-goal");
        action.AddPrecondition("mid");
        action.AddEffect("goal");
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "zero-cost-support-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionEnablesExactlyOneBinding()
    {
        DomainBuilder domainBuilder = new DomainBuilder("static-precondition-binding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("allowed", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "object");
        action.AddPrecondition("seed");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("allowed", "?x");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "static-precondition-binding-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("ready", "o1");
        initialState.AddFact("ready", "o2");
        initialState.AddFact("allowed", "o2");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "o2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEvaluateWithGoalOverride_DoesNotMutateDefaultGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("goal-override");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("g1");
        predicates.Add("mid");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-g1");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("g1");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("make-mid");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "goal-override-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForConstructorDefaultGoal_IsUsedWhenEvaluateOmitsGoal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constructor-default-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("g1");
        predicates.Add("mid");
        predicates.Add("g2");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-g1");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("g1");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("make-mid");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("mid");
        action.WithCost(1d);
        action.Close();
        action = actions.Add("make-g2");
        action.AddPrecondition("mid");
        action.AddEffect("g2");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "constructor-default-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNegativeGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-goal-literal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear-p");
        action.AddPrecondition("p");
        action.AddEffect("p", Polarity.Negative);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-goal-literal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("p", Polarity.Negative);
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForDerivedGoalLiteral_ThrowsForUnsupportedGroundedRpgGoalShape()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-goal-literal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("missing");
        predicates.Add("reachable");
        predicates.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("missing"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-goal-literal-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("reachable");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-precondition");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("goal");
        predicates.Add("reachable");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddPrecondition("reachable");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("goal");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("seed"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-precondition-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-effect");
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
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("trigger");
        conditional0.AddEffect("goal");
        conditional0.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-effect-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("trigger");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForMultipleCurrentStateTrueFactsSatisfyConjunctivePreconditionAtActionCost()
    {
        DomainBuilder domainBuilder = new DomainBuilder("current-fact-conjunction");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Add("q");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("p");
        action.AddPrecondition("q");
        action.AddEffect("p", Polarity.Negative);
        action.AddEffect("q", Polarity.Negative);
        action.AddEffect("goal");
        action.WithCost(5d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "current-fact-conjunction-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p");
        initialState.AddFact("q");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSharedSupport_ReturnsDistinctExactValuesForEachHeuristic9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("repeated-variable-binding");
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
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "repeated-variable-binding-problem");
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

    private static Problem CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constants-grounding");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("depot", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("at", ("?pkg", "object"), ("?loc", "object"));
        predicates.Add("delivered", ("?pkg", "object"), ("?loc", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("deliver");
        action.AddParameter("?pkg", "object");
        action.AddPrecondition("seed");
        action.AddPrecondition("at", "?pkg", "depot");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("delivered", "?pkg", "depot");
        action.WithCost(3d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "constants-grounding-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("package1", "object");
        objects.Add("other-place", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("at", "package1", "depot");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("delivered", "package1", "depot");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForTrueGoalReturnsZeroEvenWithUnreachableDistractorActions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("true-goal-with-distractors");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("goal");
        predicates.Add("blocked");
        predicates.Add("side");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-goal");
        action.AddPrecondition("goal");
        action.AddEffect("goal");
        action.Close();
        action = actions.Add("unreachable-side");
        action.AddPrecondition("blocked");
        action.AddEffect("side");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "true-goal-with-distractors-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("goal");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForZeroPreconditionAction_FiresInGroundedRpg()
    {
        DomainBuilder domainBuilder = new DomainBuilder("zero-precondition-action");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddEffect("goal");
        action.WithCost(3d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "zero-precondition-action-problem");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("static-only-precondition");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("enabled");
        action.AddEffect("goal");
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "static-only-precondition-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForUnsupportedNegativeOnlyGoal_ThrowsInsteadOfReturningZero()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unsupported-negative-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("clear-p");
        action.AddPrecondition("p");
        action.AddEffect("p", Polarity.Negative);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "unsupported-negative-goal-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("p", Polarity.Negative);
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("derived-precondition-supported");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("goal");
        predicates.Add("reachable");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("goal");
        action.WithCost(5d);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("seed"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "derived-precondition-supported-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForConditionalPositiveEffect_ContributesWhenConditionIsReachable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-positive-effect-supported");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("trigger");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("trigger");
        conditional1.AddEffect("goal");
        conditional1.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-positive-effect-supported-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("trigger");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLateRegisteredReachableGoal_ReturnsExactValueForAlreadyConstructedHeuristics()
    {
        DomainBuilder domainBuilder = new DomainBuilder("late-reachable-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish-a");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("done", "a");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "late-reachable-goal-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSingleActionAchievesTwoGoalFacts_CountsThatActionExactly8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("late-unreachable-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish-a");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("done", "a");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "late-unreachable-goal-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForLateRegisteredTrueGoal_IsSkippedWhileCompiledGoalIsStillEvaluated()
    {
        DomainBuilder domainBuilder = new DomainBuilder("late-true-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish-a");
        action.AddPrecondition("seed");
        action.AddEffect("done", "a");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "late-true-goal-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-static-conditional-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("enabled");
        predicates.Add("goal");
        predicates.Add("final");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("maybe-goal");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional2 = action.AddConditionalEffect();
        conditional2.AddCondition("enabled");
        conditional2.AddEffect("goal");
        conditional2.Close();
        action.Close();
        action = actions.Add("finish-final");
        action.AddPrecondition("goal");
        action.AddEffect("final");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "false-static-conditional-effect-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("final");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-derived-conditional-effect");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("missing");
        predicates.Add("goal");
        predicates.Add("final");
        predicates.Add("reachable");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("maybe-goal");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional3 = action.AddConditionalEffect();
        conditional3.AddCondition("reachable");
        conditional3.AddEffect("goal");
        conditional3.Close();
        action.Close();
        action = actions.Add("finish-final");
        action.AddPrecondition("goal");
        action.AddEffect("final");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("missing"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "false-derived-conditional-effect-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("final");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-effect-condition-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
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
        action.WithCost(3d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional4 = action.AddConditionalEffect();
        conditional4.AddCondition("trigger");
        conditional4.AddEffect("goal");
        conditional4.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-effect-condition-cost-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("shared-conditional-action");
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
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional5 = action.AddConditionalEffect();
        conditional5.AddCondition("c1");
        conditional5.AddEffect("g1");
        conditional5.Close();
        ConditionalEffectBuilder conditional6 = action.AddConditionalEffect();
        conditional6.AddCondition("c2");
        conditional6.AddEffect("g2");
        conditional6.Close();
        action.WithCost(4d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "shared-conditional-action-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("c1");
        initialState.AddFact("c2");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("g1");
        goals.Add("g2");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-action-and-condition-costs");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("ready");
        predicates.Add("trigger");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-ready");
        action.AddPrecondition("start");
        action.AddEffect("ready");
        action.WithCost(2d);
        action.Close();
        action = actions.Add("make-trigger");
        action.AddPrecondition("start");
        action.AddEffect("trigger");
        action.WithCost(3d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("ready");
        ConditionalEffectBuilder conditional7 = action.AddConditionalEffect();
        conditional7.AddCondition("trigger");
        conditional7.AddEffect("goal");
        conditional7.Close();
        action.WithCost(5d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-action-and-condition-costs-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-static-conditional-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("enabled");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional8 = action.AddConditionalEffect();
        conditional8.AddCondition("enabled");
        conditional8.AddEffect("goal");
        conditional8.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "false-static-conditional-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("true-static-conditional-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("enabled");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional9 = action.AddConditionalEffect();
        conditional9.AddCondition("enabled");
        conditional9.AddEffect("goal");
        conditional9.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "true-static-conditional-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("enabled");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("false-derived-conditional-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("missing");
        predicates.Add("goal");
        predicates.Add("reachable");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional10 = action.AddConditionalEffect();
        conditional10.AddCondition("reachable");
        conditional10.AddEffect("goal");
        conditional10.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("missing"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "false-derived-conditional-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("true-derived-conditional-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("goal");
        predicates.Add("reachable");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional11 = action.AddConditionalEffect();
        conditional11.AddCondition("reachable");
        conditional11.AddEffect("goal");
        conditional11.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("reachable", Logic.Atom("seed"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "true-derived-conditional-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForSetAdd_SelectsSharedSupportThatAddAndFfRejectLocally9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-conditional-fluent-rpg");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("blocked");
        predicates.Add("goal");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("keep-blocked-fluent");
        action.AddPrecondition("seed");
        action.AddEffect("blocked");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional12 = action.AddConditionalEffect();
        conditional12.AddCondition("blocked", Polarity.Negative);
        conditional12.AddEffect("goal");
        conditional12.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "negative-conditional-fluent-rpg-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFfPreferredAction_IncludesApplicableConditionalEffectAction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("preferred-conditional-action");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("seed");
        predicates.Add("trigger");
        predicates.Add("goal");
        predicates.Add("junk");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional13 = action.AddConditionalEffect();
        conditional13.AddCondition("trigger");
        conditional13.AddEffect("goal");
        conditional13.Close();
        action.WithCost(2d);
        action.Close();
        action = actions.Add("distract");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        action.AddEffect("junk");
        action.WithCost(1d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "preferred-conditional-action-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.AddFact("trigger");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFfAndSetAdd_KeepConditionalEffectSupportPreconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-support-extraction");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
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
        action.WithCost(3d);
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("seed");
        action.AddEffect("seed", Polarity.Negative);
        ConditionalEffectBuilder conditional14 = action.AddConditionalEffect();
        conditional14.AddCondition("trigger");
        conditional14.AddEffect("goal");
        conditional14.Close();
        action.WithCost(2d);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "conditional-support-extraction-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("seed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("goal");
        goals.Close();
        return problemBuilder.Build();
    }

    private static GoalCondition GoalFor(Problem problem, string predicateName)
    {
        return GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates[predicateName], Array.Empty<ITerm>()));
    }

    private static void AssertValues(Problem problem, State state, HeuristicValues expected)
    {
        HeuristicValues actual = Evaluate(problem, state);

        Assert.Equal(expected.Add, actual.Add);
        Assert.Equal(expected.Max, actual.Max);
        Assert.Equal(expected.FF, actual.FF);
        Assert.Equal(expected.SetAdd, actual.SetAdd);
        Assert.Equal(expected.H2, actual.H2);
    }

    private static void AssertValues(Problem problem, State state, GoalCondition goal, HeuristicValues expected)
    {
        HeuristicValues actual = Evaluate(problem, state, goal);

        Assert.Equal(expected.Add, actual.Add);
        Assert.Equal(expected.Max, actual.Max);
        Assert.Equal(expected.FF, actual.FF);
        Assert.Equal(expected.SetAdd, actual.SetAdd);
        Assert.Equal(expected.H2, actual.H2);
    }

    private static void AssertValues(Heuristics heuristics, State state, GoalCondition goal, HeuristicValues expected)
    {
        HeuristicValues actual = Evaluate(heuristics, state, goal);

        Assert.Equal(expected.Add, actual.Add);
        Assert.Equal(expected.Max, actual.Max);
        Assert.Equal(expected.FF, actual.FF);
        Assert.Equal(expected.SetAdd, actual.SetAdd);
        Assert.Equal(expected.H2, actual.H2);
    }

    private static void AssertAllPositiveInfinity(Problem problem, State state)
    {
        AssertValues(
            problem,
            state,
            new HeuristicValues(
                double.PositiveInfinity,
                double.PositiveInfinity,
                double.PositiveInfinity,
                double.PositiveInfinity,
                double.PositiveInfinity));
    }

    private static void AssertThrowsNotSupportedForAll(Problem problem, State state)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Assert.Throws<NotSupportedException>(() => new AddHeuristic(generator).Evaluate(state.Expand()));
        Assert.Throws<NotSupportedException>(() => new MaxHeuristic(generator).Evaluate(state.Expand()));
        Assert.Throws<NotSupportedException>(() => new FFHeuristic(generator).Evaluate(state.Expand()));
        Assert.Throws<NotSupportedException>(() => new SetAddHeuristic(generator).Evaluate(state.Expand()));
        Assert.Throws<NotSupportedException>(() => new H2Heuristic(generator).Evaluate(state.Expand()));
    }

    private static void AssertValuesWithConstructorGoal(
        Problem problem,
        State state,
        GoalCondition constructorGoal,
        GoalCondition? evaluationGoal,
        HeuristicValues expected)
    {
        HeuristicValues actual = EvaluateWithConstructorGoal(problem, state, constructorGoal, evaluationGoal);

        Assert.Equal(expected.Add, actual.Add);
        Assert.Equal(expected.Max, actual.Max);
        Assert.Equal(expected.FF, actual.FF);
        Assert.Equal(expected.SetAdd, actual.SetAdd);
        Assert.Equal(expected.H2, actual.H2);
    }

    private static HeuristicValues Evaluate(Problem problem, State state)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new HeuristicValues(
            new AddHeuristic(generator).Evaluate(state.Expand()).Value,
            new MaxHeuristic(generator).Evaluate(state.Expand()).Value,
            new FFHeuristic(generator).Evaluate(state.Expand()).Value,
            new SetAddHeuristic(generator).Evaluate(state.Expand()).Value,
            new H2Heuristic(generator).Evaluate(state.Expand()).Value);
    }

    private static HeuristicValues Evaluate(Problem problem, State state, GoalCondition goal)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new HeuristicValues(
            new AddHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new MaxHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new FFHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new SetAddHeuristic(generator).Evaluate(state.Expand(), goal).Value,
            new H2Heuristic(generator).Evaluate(state.Expand(), goal).Value);
    }

    private static Heuristics CreateHeuristics(Problem problem)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new Heuristics(
            new AddHeuristic(generator),
            new MaxHeuristic(generator),
            new FFHeuristic(generator),
            new SetAddHeuristic(generator),
            new H2Heuristic(generator));
    }

    private static HeuristicValues Evaluate(Heuristics heuristics, State state, GoalCondition goal)
    {
        return new HeuristicValues(
            heuristics.Add.Evaluate(state.Expand(), goal).Value,
            heuristics.Max.Evaluate(state.Expand(), goal).Value,
            heuristics.FF.Evaluate(state.Expand(), goal).Value,
            heuristics.SetAdd.Evaluate(state.Expand(), goal).Value,
            heuristics.H2.Evaluate(state.Expand(), goal).Value);
    }

    private static HeuristicValues EvaluateWithConstructorGoal(
        Problem problem,
        State state,
        GoalCondition constructorGoal,
        GoalCondition? evaluationGoal)
    {
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        return new HeuristicValues(
            new AddHeuristic(generator, constructorGoal).Evaluate(state.Expand(), evaluationGoal).Value,
            new MaxHeuristic(generator, constructorGoal).Evaluate(state.Expand(), evaluationGoal).Value,
            new FFHeuristic(generator, constructorGoal).Evaluate(state.Expand(), evaluationGoal).Value,
            new SetAddHeuristic(generator, constructorGoal).Evaluate(state.Expand(), evaluationGoal).Value,
            new H2Heuristic(generator, constructorGoal).Evaluate(state.Expand(), evaluationGoal).Value);
    }

    private sealed record HeuristicValues(double Add, double Max, double FF, double SetAdd, double H2);

    private sealed record Heuristics(
        AddHeuristic Add,
        MaxHeuristic Max,
        FFHeuristic FF,
        SetAddHeuristic SetAdd,
        H2Heuristic H2);
}
