using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed partial class PartialOrderPlan
{
    public PartialOrderPlan WithGoal(IReadOnlyList<Literal<Fact>> goal)
    {
        ArgumentNullException.ThrowIfNull(goal);
        ValidateGoal(Problem, goal);
        List<Fact<Fluent>> goalFacts = GoalFacts(Problem, goal);

        PartialOrderPlan result = Clone(goal);
        PlanStep goalStep = PlanStep.CreateGoal(goalFacts);
        result._steps[result._steps.IndexOf(result.GetStep(1))] = goalStep;
        result._stepById[1] = goalStep;
        foreach (CausalLink link in result._links.Where(link => link.ConsumerId == 1 && !goalFacts.Contains(link.Fact)).ToArray())
            result.RemoveLink(link);
        result._ordering.Rebuild(result._steps.Select(step => step.Id), result._orderings);
        return result;
    }

    // The sub-plan keeps the explicit orderings among its steps, so threats are judged against that
    // induced closure rather than the full one, which may order kept steps through dropped ones.
    public PartialOrderPlan PruneToAchievedGoal()
    {
        var closures = new Dictionary<Fact<Fluent>, (HashSet<int> Steps, HashSet<int> Links)>();
        foreach (Fact<Fluent> fact in GetStep(1).Preconditions)
        {
            if (_supported.Contains((1, fact))) closures.Add(fact, BackwardClosure(fact));
        }

        var induced = new OrderingGraph();
        while (true)
        {
            var keptSteps = new HashSet<int> { 0, 1 };
            var keptLinks = new HashSet<int>();
            foreach ((HashSet<int> steps, HashSet<int> links) in closures.Values)
            {
                keptSteps.UnionWith(steps);
                keptLinks.UnionWith(links);
            }

            induced.Rebuild(keptSteps, KeptConstraints(keptSteps, keptLinks));
            var offendingSteps = new HashSet<int>();
            foreach (int id in keptSteps)
            {
                PlanStep step = GetStep(id);
                if (step.Kind != StepKind.Step) continue;
                if (!step.IsFullyBound || step.Preconditions.Any(fact => !_supported.Contains((id, fact)))) offendingSteps.Add(id);
            }

            var offendingLinks = new HashSet<int>();
            foreach (int linkId in keptLinks)
            {
                CausalLink link = GetLink(linkId);
                if (keptSteps.Any(id => IsThreat(link, GetStep(id), induced))) offendingLinks.Add(linkId);
            }

            if (offendingSteps.Count == 0 && offendingLinks.Count == 0)
                return BuildSubPlan(closures.Keys.ToHashSet(), keptSteps, keptLinks);

            foreach ((Fact<Fluent> fact, (HashSet<int> steps, HashSet<int> links)) in closures.ToArray())
            {
                if (steps.Overlaps(offendingSteps) || links.Overlaps(offendingLinks)) closures.Remove(fact);
            }
        }
    }

    private (HashSet<int> Steps, HashSet<int> Links) BackwardClosure(Fact<Fluent> goalFact)
    {
        var steps = new HashSet<int> { 1 };
        var links = new HashSet<int>();
        var frontier = new Stack<int>();
        CausalLink goalLink = _links.Single(link => link.ConsumerId == 1 && link.Fact.Equals(goalFact));
        links.Add(goalLink.Id);
        frontier.Push(goalLink.ProducerId);
        while (frontier.Count > 0)
        {
            int id = frontier.Pop();
            if (!steps.Add(id)) continue;
            foreach (CausalLink link in _links)
            {
                if (link.ConsumerId != id) continue;
                links.Add(link.Id);
                frontier.Push(link.ProducerId);
            }
        }

        return (steps, links);
    }

    private IEnumerable<OrderingConstraint> KeptConstraints(HashSet<int> keptSteps, HashSet<int> keptLinks)
        => _orderings.Where(constraint => keptSteps.Contains(constraint.Before)
            && keptSteps.Contains(constraint.After)
            && (constraint.LinkId < 0 || keptLinks.Contains(constraint.LinkId)));

    private PartialOrderPlan BuildSubPlan(HashSet<Fact<Fluent>> goalFacts, HashSet<int> keptSteps, HashSet<int> keptLinks)
    {
        Literal<Fact>[] goal = Goal.Where(literal => literal.Value is not Fact<Fluent> fluent || goalFacts.Contains(fluent)).ToArray();
        var result = new PartialOrderPlan(Problem, goal, Options) { Round = Round };
        result.RegisterStep(PlanStep.CreateInitial(Problem));
        result.RegisterStep(PlanStep.CreateGoal(GoalFacts(Problem, goal)));
        result.AddOrdering(0, 1, -1);

        var stepIds = new Dictionary<int, int> { [0] = 0, [1] = 1 };
        foreach (PlanStep step in _steps)
        {
            if (step.Kind != StepKind.Step || !keptSteps.Contains(step.Id)) continue;
            stepIds.Add(step.Id, result.CreateStep(step.Schema!, step.Binding.ToArray()).Id);
        }

        var linkIds = new Dictionary<int, int>();
        foreach (CausalLink link in _links)
        {
            if (!keptLinks.Contains(link.Id)) continue;
            CausalLink copy = result.TryAddLink(stepIds[link.ProducerId], stepIds[link.ConsumerId], link.Fact)
                ?? throw new InvalidOperationException($"Copying link {link} into the pruned plan created a cycle.");
            linkIds.Add(link.Id, copy.Id);
        }

        // CreateStep and TryAddLink already added the step and link orderings; only threat resolutions remain.
        foreach (OrderingConstraint constraint in KeptConstraints(keptSteps, keptLinks))
        {
            if (constraint.LinkId < 0) continue;
            CausalLink link = GetLink(constraint.LinkId);
            if (constraint.Before == link.ProducerId && constraint.After == link.ConsumerId) continue;
            if (!result.AddOrdering(stepIds[constraint.Before], stepIds[constraint.After], linkIds[constraint.LinkId]))
                throw new InvalidOperationException($"Ordering {constraint.Before} < {constraint.After} created a cycle in the pruned plan.");
        }

        if (!result.IsSolution())
            throw new InvalidOperationException($"The pruned plan still has flaws; the first is {result.ComputeFlaws().Flaws[0]}.");
        return result;
    }
}
