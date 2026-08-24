using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Mimir.Search.Space;
using Xunit;

namespace Mimir.Search.Tests;

public class PerfectHeuristicTests
{
    [Fact]
    public void CustomGoal_BindsConstructionAndGuidesGbfs()
    {
        Problem problem = CreateProblem();
        GoalCondition customGoal = CreateCustomGoal(problem);
        var heuristic = new PerfectHeuristic(problem, customGoal);
        var planner = new GreedyBestFirstPlanner(
            SearchTestHelpers.CreateGroundedGenerator,
            (_, _, _) => heuristic);

        PlanResult result = planner.Solve(problem, goal: customGoal);

        Assert.Equal(1.0, heuristic.Evaluate(problem.InitialState.Expand()).Value);
        Assert.True(result.IsSuccess);
        Assert.Single(result.Plan);
        Assert.True(customGoal.IsSatisfied(SearchTestHelpers.ApplyPlan(problem.InitialState, result.Plan).Expand()));
    }

    [Fact]
    public void Evaluate_SemanticallyEquivalentGoal_IsAccepted()
    {
        Problem problem = CreateProblem();
        GoalCondition boundGoal = CreateCustomGoal(problem);
        GoalCondition equivalentGoal = CreateCustomGoal(problem);
        var heuristic = new PerfectHeuristic(problem, boundGoal);

        HeuristicEvaluation evaluation = heuristic.Evaluate(problem.InitialState.Expand(), equivalentGoal);

        Assert.Equal(1.0, evaluation.Value);
    }

    [Fact]
    public void Evaluate_DifferentGoal_Throws()
    {
        Problem problem = CreateProblem();
        var heuristic = new PerfectHeuristic(problem, CreateCustomGoal(problem));

        Assert.Throws<ArgumentException>(
            () => heuristic.Evaluate(problem.InitialState.Expand(), GoalCondition.FromProblem(problem)));
    }

    [Fact]
    public void Evaluate_SameProblemStateOutsideReachableSpace_Throws()
    {
        Problem problem = CreateProblem();
        GoalCondition customGoal = CreateCustomGoal(problem);
        var heuristic = new PerfectHeuristic(problem, customGoal);
        Fact<Fluent> unreachableFact = SearchTestHelpers.GetFluentFact(problem, "custom");
        State unreachableState = problem.InitialState.WithAdditionalFluentFacts(new[] { unreachableFact });

        Assert.Throws<ArgumentException>(() => heuristic.Evaluate(unreachableState.Expand()));
    }

    [Fact]
    public void SearchSpaceConstructor_BindsTheSpacesGoal()
    {
        Problem problem = CreateProblem();
        GoalCondition customGoal = CreateCustomGoal(problem);
        SearchSpace searchSpace = new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(customGoal)
            .WithActionGenerator(SearchTestHelpers.CreateGroundedGenerator(problem))
            .Build();
        var heuristic = new PerfectHeuristic(searchSpace);

        Assert.Equal(1.0, heuristic.Evaluate(problem.InitialState.Expand(), CreateCustomGoal(problem)).Value);
        Assert.Throws<ArgumentException>(
            () => heuristic.Evaluate(problem.InitialState.Expand(), GoalCondition.FromProblem(problem)));
    }

    private static Problem CreateProblem()
    {
        Domain domain = new DomainBuilder("perfect-custom-goal")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("start").Add("custom").Add("original").Close()
            .Actions()
                .Add("reach-custom")
                    .AddPrecondition("start")
                    .AddEffect("start", Polarity.Negative)
                    .AddEffect("custom")
                    .Close()
                .Close()
            .Build();
        return new ProblemBuilder(domain, "perfect-custom-goal-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add("original").Close()
            .Build();
    }

    private static GoalCondition CreateCustomGoal(Problem problem) =>
        GoalCondition.FromExpression(
            problem,
            new GroundedAtom(problem.AllPredicates["custom"], Array.Empty<ITerm>()));

}
