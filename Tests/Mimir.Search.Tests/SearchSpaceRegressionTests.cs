using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Space;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class SearchSpaceRegressionTests
{
    [Fact]
    public void ReverseMetadata_UsesEveryPredecessorEdge()
    {
        Problem problem = CreateNonTreeGoalProblem();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace searchSpace = BuildSpace(problem, generator);
        Action takeBranch = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "take-branch");
        State branch = problem.InitialState.Expand().Apply(takeBranch);

        Assert.False(searchSpace.Query.IsDeadEnd(branch));
        Assert.Equal(1, searchSpace.Query.GetDistanceToGoal(branch));
        Assert.Equal(1.0, searchSpace.Query.GetCostToGoal(branch));
        Assert.Equal(0, searchSpace.DeadEndStateCount);

        State goal = searchSpace.GoalStates.Single();
        Assert.Equal(2, searchSpace.Query.GetPredecessors(goal).Count);
    }

    [Fact]
    public void CostFromRoot_PropagatesCheaperPathsThroughExistingNodes()
    {
        Problem problem = CreateWeightedMergeProblem();
        GroundedApplicableActionGenerator generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        SearchSpace searchSpace = BuildSpace(problem, generator);
        Action directToX = SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "start-to-x-expensive");
        State x = problem.InitialState.Expand().Apply(directToX);
        State goal = x.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, x, "x-to-goal"));

        Assert.Equal(2.0, searchSpace.Query.GetCost(x));
        Assert.Equal(3.0, searchSpace.Query.GetCost(goal));
        Assert.Equal(3.0, searchSpace.Query.GetCostToGoal(problem.InitialState));
    }

    [Fact]
    public void MaxDistanceToGoal_GoalOnlySpace_IsZero()
    {
        Problem problem = CreateGoalOnlyProblem();
        SearchSpace searchSpace = BuildSpace(problem, SearchTestHelpers.CreateGroundedGenerator(problem));

        Assert.Equal(1, searchSpace.TotalStates);
        Assert.Equal(0, searchSpace.MaxDistanceToGoal);
    }

    private static SearchSpace BuildSpace(Problem problem, IApplicableActionGenerator generator)
    {
        return new SearchSpaceBuilder()
            .WithInitialState(problem.InitialState)
            .WithGoal(problem)
            .Build();
    }

    private static Problem CreateNonTreeGoalProblem()
    {
        DomainBuilder builder = new DomainBuilder("non-tree-goal")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("start").Add("branch").Add("goal").Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("direct").AddPrecondition("start")
            .AddEffect("start", Polarity.Negative).AddEffect("goal").Close();
        actions.Add("take-branch").AddPrecondition("start")
            .AddEffect("start", Polarity.Negative).AddEffect("branch").Close();
        actions.Add("finish-branch").AddPrecondition("branch")
            .AddEffect("branch", Polarity.Negative).AddEffect("goal").Close();
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "non-tree-goal-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add("goal").Close()
            .Build();
    }

    private static Problem CreateWeightedMergeProblem()
    {
        DomainBuilder builder = new DomainBuilder("weighted-merge")
            .Requirements().Add(":strips").Add(":action-costs").Close()
            .Predicates().Add("start").Add("b").Add("x").Add("goal").Close();
        ActionListBuilder actions = builder.Actions();
        AddWeightedTransition(actions, "start-to-x-expensive", "start", "x", 10d);
        AddWeightedTransition(actions, "start-to-b", "start", "b", 1d);
        AddWeightedTransition(actions, "b-to-x", "b", "x", 1d);
        AddWeightedTransition(actions, "x-to-goal", "x", "goal", 1d);
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "weighted-merge-problem")
            .InitialState().AddFact("start").Close()
            .Goal().Add("goal").Close()
            .Build();
    }

    private static void AddWeightedTransition(
        ActionListBuilder actions,
        string name,
        string from,
        string to,
        double cost)
        => actions.Add(name).AddPrecondition(from)
            .AddEffect(from, Polarity.Negative).AddEffect(to).WithCost(cost).Close();

    private static Problem CreateGoalOnlyProblem()
    {
        Domain domain = new DomainBuilder("goal-only")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("done").Close()
            .Build();
        return new ProblemBuilder(domain, "goal-only-problem")
            .InitialState().AddFact("done").Close()
            .Goal().Add("done").Close()
            .Build();
    }
}
