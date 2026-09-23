using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Xunit;

namespace Mimir.Search.Tests;

public class GoalCountHeuristicTests
{
    [Fact]
    public void GoalCountHeuristicReturnsZeroForGoalState()
    {
        Problem problem = CreateMixedGoalProblem(weighted: false);
        var heuristic = new GoalCountHeuristic(problem);
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var state = problem.InitialState;
        state = state.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, state, "make-p"));
        state = state.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, state, "clear-q"));

        Assert.Equal(0, heuristic.Evaluate(state.Expand()).Value);
    }

    [Fact]
    public void GoalCountHeuristicCountsUnsatisfiedPositiveAndNegativeGoalLiteralsCorrectly()
    {
        Problem problem = CreateMixedGoalProblem(weighted: false);
        var heuristic = new GoalCountHeuristic(problem);
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var initialState = problem.InitialState;
        var afterMakeP = initialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, initialState, "make-p"));
        var afterClearQ = initialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, initialState, "clear-q"));

        Assert.Equal(2, heuristic.Evaluate(initialState.Expand()).Value);
        Assert.Equal(1, heuristic.Evaluate(afterMakeP.Expand()).Value);
        Assert.Equal(1, heuristic.Evaluate(afterClearQ.Expand()).Value);
    }

    [Fact]
    public void GoalCountHeuristic_DoesNotScaleUnsatisfiedGoalsByActionCost()
    {
        Problem problem = CreateMixedGoalProblem(weighted: true);
        var heuristic = new GoalCountHeuristic(problem);
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);

        var initialState = problem.InitialState;
        var afterMakeP = initialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, initialState, "make-p"));
        var afterClearQ = initialState.Expand().Apply(SearchTestHelpers.GetApplicableAction(generator, initialState, "clear-q"));

        Assert.Equal(2d, heuristic.Evaluate(initialState.Expand()).Value);
        Assert.Equal(1d, heuristic.Evaluate(afterMakeP.Expand()).Value);
        Assert.Equal(1d, heuristic.Evaluate(afterClearQ.Expand()).Value);
    }

    private static Problem CreateMixedGoalProblem(bool weighted)
    {
        var domainBuilder = new DomainBuilder(weighted ? "weighted-mixed-goal" : "mixed-goal");
        RequirementListBuilder requirements = domainBuilder.Requirements().Add(":strips");
        if (weighted) requirements.Add(":action-costs");
        requirements.Close();
        domainBuilder.Predicates()
            .Add("ready")
            .Add("p")
            .Add("q")
            .Close();

        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder makeP = actions.Add("make-p")
            .AddPrecondition("ready")
            .AddEffect("p");
        if (weighted) makeP.WithCost(2d);
        makeP.Close();

        ActionSchemaBuilder clearQ = actions.Add("clear-q")
            .AddPrecondition("ready")
            .AddPrecondition("q")
            .AddEffect("q", Polarity.Negative);
        if (weighted) clearQ.WithCost(3d);
        clearQ.Close();
        actions.Close();

        Domain domain = domainBuilder.Build();
        var problemBuilder = new ProblemBuilder(domain, $"{domain.Name}-problem");
        problemBuilder.InitialState()
            .AddFact("ready")
            .AddFact("q")
            .Close();
        problemBuilder.Goal()
            .Add("p")
            .Add("q", Polarity.Negative)
            .Close();
        return problemBuilder.Build();
    }
}
