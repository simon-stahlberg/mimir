using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Mimir.Search.Heuristics;
using System.Collections.Immutable;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

internal static class SearchTestHelpers
{
    public static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    public static Problem LoadProblem(string domainDir, string problemFile = "p01.pddl",
        ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        var domainPath = Path.Combine(BasePath, domainDir, "domain.pddl");
        var problemPath = Path.Combine(BasePath, domainDir, problemFile);

        var domain = Domain.FromFile(domainPath);
        return Problem.FromFile(domain, problemPath, generatorType);
    }

    public static Problem LoadUnsolvableProblem(string problemPrefix)
    {
        var basePath = Path.Combine(BasePath, "Unsolvable");
        var domainPath = Path.Combine(basePath, $"{problemPrefix}_domain.pddl");
        var problemPath = Path.Combine(basePath, $"{problemPrefix}_unsolvable.pddl");

        var domain = Domain.FromFile(domainPath);
        return Problem.FromFile(domain, problemPath);
    }

    public static Problem CreateProblemFromText(string domainText, string problemText,
        ApplicableActionGeneratorType generatorType = ApplicableActionGeneratorType.Grounded)
    {
        Domain domain = Domain.FromText(domainText);
        return Problem.FromText(domain, problemText, generatorType);
    }

    public static GroundedApplicableActionGenerator CreateGroundedGenerator(Problem problem) =>
        CreateGroundedGenerator(problem, problem.InitialState);

    public static GroundedApplicableActionGenerator CreateGroundedGenerator(Problem problem, State startState) =>
        (GroundedApplicableActionGenerator)problem.GetApplicableActionGenerator(startState);

    public static State ApplyPlan(State state, IReadOnlyList<Action> plan)
    {
        foreach (var action in plan)
        {
            ExtendedState extendedState = state.Expand();
            if (!action.IsApplicable(extendedState))
                throw new InvalidOperationException("Search result contained an inapplicable action.");

            state = extendedState.Apply(action);
        }

        return state;
    }

    public static Action GetApplicableAction(IApplicableActionGenerator generator, State state, string actionName)
    {
        var matches = generator.GetApplicableActions(state.Expand())
            .Where(action => string.Equals(action.Schema.Name, actionName, StringComparison.OrdinalIgnoreCase))
            .ToList();

        return matches.Count switch
        {
            1 => matches[0],
            0 => throw new InvalidOperationException($"No applicable action named '{actionName}' was found."),
            _ => throw new InvalidOperationException($"Multiple applicable actions named '{actionName}' were found.")
        };
    }

    public static Fact<Fluent> GetFluentFact(Problem problem, string predicateName)
    {
        var matches = problem.Context.Fluents
            .Where(fact => string.Equals(fact.Predicate.Name, predicateName, StringComparison.OrdinalIgnoreCase))
            .ToList();

        return matches.Count switch
        {
            1 => matches[0],
            0 => throw new InvalidOperationException($"No fluent fact named '{predicateName}' was registered."),
            _ => throw new InvalidOperationException($"Multiple fluent facts named '{predicateName}' were registered.")
        };
    }

    public static GoalCondition ContradictoryGoal(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        _ = problem.InitialState;

        Fact fact = problem.Goal.FirstOrDefault()?.Value
            ?? problem.Context.AllFacts.FirstOrDefault()
            ?? throw new InvalidOperationException("The problem has no ground fact that can form a contradictory goal.");
        var atom = new GroundedAtom(fact.Predicate, fact.Arguments);
        var expression = new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
            atom,
            new GroundedNot(atom)));
        return GoalCondition.FromExpression(problem, expression);
    }

    public static string[] PlanSteps(IReadOnlyList<Action> plan) => plan.Select(action => action.ToString()).ToArray();
}

internal sealed class DelegateHeuristic : IHeuristic
{
    private readonly Func<ExtendedState, GoalCondition?, HeuristicEvaluation> _evaluate;

    public DelegateHeuristic(Func<ExtendedState, double> estimate)
    {
        _evaluate = (state, _) => new(estimate(state));
    }

    public DelegateHeuristic(Func<ExtendedState, HeuristicEvaluation> evaluate)
    {
        _evaluate = (state, _) => evaluate(state);
    }

    public DelegateHeuristic(Func<ExtendedState, GoalCondition?, HeuristicEvaluation> evaluate)
    {
        _evaluate = evaluate;
    }

    public HeuristicEvaluation Evaluate(ExtendedState state, GoalCondition? goal = null) => _evaluate(state, goal);
}
