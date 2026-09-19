using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

/// <summary>
/// Deliberately naive h^2, written straight from the Haslum &amp; Geffner equations and
/// kept independent of the production compilation so that the two can be compared.
/// Every fluent is doubled into a positive and a negative literal, and tasks with
/// conditional effects are rejected rather than approximated.
/// </summary>
internal sealed class NaiveH2
{
    private readonly IReadOnlyList<Fact<Fluent>> _fluents;
    private readonly List<NaiveAction> _actions = [];
    private readonly int _fluentCount;
    private readonly int _factCount;

    public NaiveH2(GroundedApplicableActionGenerator generator)
    {
        _fluents = generator.Problem.Context.Fluents;
        _fluentCount = _fluents.Count;
        _factCount = 2 * _fluentCount;

        ulong[] staticBitboard = generator.Problem.Context.StaticBitboardWords.ToArray();
        foreach (GroundAction action in generator.GroundActions)
        {
            if (action.ConditionalEffects.Count > 0)
                throw new NotSupportedException("The naive oracle only covers tasks without conditional effects.");
            if (!BitboardOps.StaticPreconditionHolds(
                    action.PositiveStaticPreconditions,
                    action.NegativeStaticPreconditions,
                    staticBitboard))
                continue;

            int[] added = BitboardOps.DecodeSetBits(action.AddEffects);
            int[] deleted = BitboardOps.DecodeSetBits(action.DeleteEffects)
                .Where(fluent => !added.Contains(fluent))
                .ToArray();

            List<int> preconditions = [.. BitboardOps.DecodeSetBits(action.PositiveFluentPreconditions)];
            preconditions.AddRange(BitboardOps.DecodeSetBits(action.NegativeFluentPreconditions).Select(Negate));

            List<int> adds = [.. added];
            adds.AddRange(deleted.Select(Negate));

            List<int> deletes = [.. deleted];
            deletes.AddRange(added.Select(Negate));

            _actions.Add(new NaiveAction(preconditions, adds, deletes, action.Cost));
        }
    }

    public double Evaluate(State state, IReadOnlyList<int> goalFluents)
    {
        var costs = new double[_factCount, _factCount];
        for (int first = 0; first < _factCount; first++)
        {
            for (int second = 0; second < _factCount; second++)
                costs[first, second] = double.PositiveInfinity;
        }

        List<int> trueLiterals = [];
        for (int fluent = 0; fluent < _fluentCount; fluent++)
            trueLiterals.Add(state.IsTrue(_fluents[fluent]) ? fluent : Negate(fluent));
        foreach (int first in trueLiterals)
        {
            foreach (int second in trueLiterals)
                costs[first, second] = 0d;
        }

        bool changed;
        do
        {
            changed = false;
            foreach (NaiveAction action in _actions)
            {
                double preconditionCost = SetCost(costs, action.Preconditions);
                if (double.IsPositiveInfinity(preconditionCost))
                    continue;

                foreach (int add in action.Adds)
                {
                    foreach (int other in action.Adds)
                        changed |= Update(costs, add, other, action.Cost + preconditionCost);

                    for (int persisted = 0; persisted < _factCount; persisted++)
                    {
                        if (persisted == add || action.Deletes.Contains(persisted))
                            continue;

                        List<int> regressed = [.. action.Preconditions, persisted];
                        double regressedCost = SetCost(costs, regressed);
                        if (double.IsPositiveInfinity(regressedCost))
                            continue;

                        changed |= Update(costs, add, persisted, action.Cost + regressedCost);
                    }
                }
            }
        }
        while (changed);

        return SetCost(costs, goalFluents);
    }

    private int Negate(int fluent) => _fluentCount + fluent;

    private static double SetCost(double[,] costs, IReadOnlyList<int> facts)
    {
        double max = 0d;
        foreach (int first in facts)
        {
            foreach (int second in facts)
                max = Math.Max(max, costs[first, second]);
        }

        return max;
    }

    private static bool Update(double[,] costs, int first, int second, double candidate)
    {
        if (candidate >= costs[first, second])
            return false;

        costs[first, second] = candidate;
        costs[second, first] = candidate;
        return true;
    }

    private sealed record NaiveAction(List<int> Preconditions, List<int> Adds, List<int> Deletes, double Cost);
}
