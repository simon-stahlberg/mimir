using System.Reflection;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public class PublicSearchSurfaceTests
{
    private static readonly Func<Problem, GoalCondition?, IHeuristic>[] GroundedHeuristics =
    [
        (problem, goal) => new AddHeuristic(problem, goal),
        (problem, goal) => new MaxHeuristic(problem, goal),
        (problem, goal) => new FFHeuristic(problem, goal),
        (problem, goal) => new SetAddHeuristic(problem, goal),
        (problem, goal) => new H2Heuristic(problem, goal)
    ];

    [Fact]
    public void PublicConstructorsCannotBypassGeneratorCache()
    {
        // Test assemblies can call internal constructors, so inspect public metadata.
        Assert.Empty(typeof(GroundedApplicableActionGenerator).GetConstructors());
        Assert.Empty(typeof(CliqueApplicableActionGenerator).GetConstructors());
        Type[] heuristicTypes = [typeof(AddHeuristic), typeof(MaxHeuristic), typeof(FFHeuristic), typeof(SetAddHeuristic), typeof(H2Heuristic)];
        foreach (Type type in heuristicTypes)
        {
            ConstructorInfo constructor = Assert.Single(type.GetConstructors());
            Assert.Equal([typeof(Problem), typeof(GoalCondition)],
                constructor.GetParameters().Select(parameter => parameter.ParameterType));
        }
    }

    [Fact]
    public void GroundedHeuristicsShareInitialGeneratorAndHonorExplicitGoal()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3");
        GoalCondition goal = GoalCondition.Always(problem);
        foreach (Func<Problem, GoalCondition?, IHeuristic> create in GroundedHeuristics)
        {
            IHeuristic heuristic = create(problem, goal);
            IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
            Assert.Same(generator, ((IGroundedHeuristic)heuristic).ActionGenerator);
            Assert.Equal(0d, heuristic.Evaluate(problem.InitialState.Expand()).Value);
            Assert.True(heuristic.Evaluate(problem.InitialState.Expand(), GoalCondition.FromProblem(problem)).Value > 0);
            Assert.True(new SearchBuilder().WithInitialState(problem.InitialState).WithGoal(problem)
                .WithHeuristic(heuristic).BuildAStar().Search().IsSuccess);
            Assert.Same(generator, problem.GetApplicableActionGenerator(problem.InitialState));
        }
    }

    [Fact]
    public void GroundedHeuristicsRejectInvalidProblemsBeforeGeneratorCreation()
    {
        Problem problem = SearchTestHelpers.LoadProblem("blocks_3", generatorType: ApplicableActionGeneratorType.Lifted);
        foreach (Func<Problem, GoalCondition?, IHeuristic> create in GroundedHeuristics)
        {
            Assert.Throws<ArgumentNullException>(() => create(null!, null));
            Assert.Equal("problem", Assert.Throws<ArgumentException>(() => create(problem, null)).ParamName);
        }

        // Inspect the lazy value without triggering construction or adding a production test hook.
        FieldInfo field = typeof(Problem).GetField("_initialActionGenerator", BindingFlags.Instance | BindingFlags.NonPublic)!;
        var lazy = (Lazy<IApplicableActionGenerator>)field.GetValue(problem)!;
        Assert.False(lazy.IsValueCreated);
    }

    [Theory]
    [InlineData(-1)]
    [InlineData(4)]
    public void IteratedWidthPlannerRejectsUnsupportedWidthAtConstruction(int width)
    {
        Assert.Throws<ArgumentOutOfRangeException>(
            () => new IteratedWidthPlanner(width));
    }

    [Fact]
    public void SearchBuilder_RejectsGoalFromAnotherProblemDuringBuild()
    {
        var first = SearchTestHelpers.LoadProblem("ferry");
        var second = SearchTestHelpers.LoadProblem("ferry");

        var builder = new SearchBuilder()
            .WithInitialState(first.InitialState)
            .WithGoal(second);

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(() => builder.BuildBfs());

        Assert.Contains("goal condition", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void PlanResult_DefensivelyCopiesPlan()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        var mutablePlan = new List<Mimir.Core.Grounding.Action>
        {
            generator.GetApplicableActions(problem.InitialState.Expand()).First()
        };
        var result = new PlanResult(
            SearchStatus.Succeeded,
            mutablePlan,
            new SearchStatistics(1, 1, TimeSpan.Zero, 0),
            TimeSpan.Zero,
            TimeSpan.Zero);

        mutablePlan.Clear();

        Assert.Single(result.Plan);
        Assert.Throws<NotSupportedException>(() => ((IList<Mimir.Core.Grounding.Action>)result.Plan).Clear());
    }

    [Fact]
    public void SearchResult_DefensivelyCopiesPlanAndKeepsCachedMetadataConsistent()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var generator = SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action action = generator.GetApplicableActions(problem.InitialState.Expand()).First();
        var mutablePlan = new List<Mimir.Core.Grounding.Action> { action };
        var result = SearchResult.Success(
            mutablePlan,
            new SearchStatistics(1, 1, TimeSpan.Zero, 1));

        mutablePlan.Clear();

        Assert.Single(result.Plan);
        Assert.Equal(1, result.PlanLength);
        Assert.Equal(action.Cost, result.PlanCost);
        Assert.Throws<NotSupportedException>(() => ((IList<Mimir.Core.Grounding.Action>)result.Plan).Clear());
    }

}
