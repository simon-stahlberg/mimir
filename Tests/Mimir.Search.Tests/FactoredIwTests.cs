using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class FactoredIwTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private static Problem Load(string domainDirectory, ApplicableActionGeneratorType generatorType)
    {
        var domain = Domain.FromFile(Path.Combine(ExamplesPath, domainDirectory, "domain.pddl"));
        return Problem.FromFile(domain, Path.Combine(ExamplesPath, domainDirectory, "p01.pddl"), generatorType);
    }

    /// <summary>
    /// childsnack's make_sandwich adds only at_kitchen_sandwich(?s), so IW(1) can tell its members apart by the
    /// sandwich alone. Projecting onto that parameter has to keep one member per sandwich and no more.
    /// </summary>
    [Fact]
    public void ProjectingAGroupKeepsOneMemberPerCombination()
    {
        Problem problem = Load("childsnack", ApplicableActionGeneratorType.Factored);
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        ExtendedState state = problem.InitialState.Expand();

        foreach (FactoredAction group in generator.GetFactoredApplicableActions(state))
        {
            if (!group.Schema.Name.Equals("make_sandwich", StringComparison.OrdinalIgnoreCase)) continue;

            GroundAction[] projected = group.ExpandProjection([0]).ToArray();
            Assert.Equal(group.GetCandidates(0).Length, projected.Length);
            Assert.Equal(projected.Length, projected.Select(action => action.Arguments[0]).Distinct().Count());
            Assert.All(projected, action => Assert.True(action.IsApplicable(state)));
        }
    }

    [Fact]
    public void ProjectingOntoEveryParameterYieldsTheWholeGroup()
    {
        Problem problem = Load("gripper", ApplicableActionGeneratorType.Factored);
        var generator = (IFactoredApplicableActionGenerator)
            problem.GetApplicableActionGenerator(problem.InitialState);
        ExtendedState state = problem.InitialState.Expand();

        foreach (FactoredAction group in generator.GetFactoredApplicableActions(state))
        {
            int[] all = Enumerable.Range(0, group.ParameterCount).ToArray();
            Assert.Equal(
                group.Expand().Select(action => action.ToString()).Order(StringComparer.Ordinal).ToList(),
                group.ExpandProjection(all).Select(action => action.ToString()).Order(StringComparer.Ordinal).ToList());
        }
    }

    /// <summary>
    /// The factored path picks a different representative among members IW(1) cannot distinguish, so its plan
    /// need not match the lifted one. It must still be a plan.
    /// </summary>
    [Theory]
    [InlineData("gripper")]
    [InlineData("blocks_4")]
    [InlineData("childsnack")]
    [InlineData("delivery")]
    [InlineData("miconic")]
    [InlineData("ferry")]
    public void FactoredIw1AgreesWithTheLiftedPathOnTheOutcome(string domainDirectory)
    {
        PlanResult lifted = Solve(Load(domainDirectory, ApplicableActionGeneratorType.Lifted));
        Problem factoredProblem = Load(domainDirectory, ApplicableActionGeneratorType.Factored);
        PlanResult factored = Solve(factoredProblem);

        Assert.Equal(lifted.Status, factored.Status);
        if (!factored.IsSuccess) return;

        var goal = GoalCondition.FromProblem(factoredProblem);
        State state = factoredProblem.InitialState;
        foreach (GroundAction action in factored.Plan)
        {
            Assert.True(action.IsApplicable(state.Expand()), $"'{action}' is not applicable in '{domainDirectory}'.");
            state = state.Expand().Apply(action);
        }

        Assert.True(goal.IsSatisfied(state.Expand()));
    }

    private static PlanResult Solve(Problem problem)
        => new IteratedWidthPlanner(1).Solve(problem);
}
