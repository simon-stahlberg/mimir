using Mimir.Core.Schemas;
using Mimir.PlanSpace;

namespace Mimir.Learning.Tests;

public class PlanSpaceEncodingTests
{
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private static Problem LoadBlocks()
    {
        Domain domain = Domain.FromFile(Path.Combine(ExamplesPath, "blocks_4", "domain.pddl"));
        return Problem.FromFile(domain, Path.Combine(ExamplesPath, "blocks_4", "p01.pddl"));
    }

    // Three rounds from the empty plan leave one threat (pickup b2 deletes the linked (clear b2)),
    // one unbound parameter (unstack ? b1) and open conditions with ghost and link resolvers.
    private static PartialOrderPlan BuildPlanWithEveryFlawKind(Problem problem)
    {
        var plan = PartialOrderPlan.Empty(problem, new PlanSpaceOptions(TombstoneCapacity: 4, TombstoneLifespan: 4));
        FlawSet flaws = plan.ComputeFlaws();
        int[] chosen = new int[flaws.Flaws.Count];
        Choose(flaws, chosen, "(clear b2)", resolver => resolver.Kind == ResolverKind.LinkExisting && resolver.StepId == 0);
        Choose(flaws, chosen, "(on b2 b3)", resolver => IsGhost(flaws, resolver, "(stack b2 b3)"));
        Choose(flaws, chosen, "(clear b1)", resolver => IsGhost(flaws, resolver, "(unstack ? b1)"));
        plan.Apply(flaws, chosen, []);

        flaws = plan.ComputeFlaws();
        chosen = new int[flaws.Flaws.Count];
        Choose(flaws, chosen, "(holding b2)", resolver => IsGhost(flaws, resolver, "(pickup b2)"));
        plan.Apply(flaws, chosen, []);
        return plan;
    }

    private static bool IsGhost(FlawSet flaws, Resolver resolver, string ghost)
        => resolver.Kind == ResolverKind.AdoptGhost && flaws.Ghosts[resolver.GhostIndex].ToString() == ghost;

    private static void Choose(FlawSet flaws, int[] chosen, string fact, Func<Resolver, bool> predicate)
    {
        int flawIndex = flaws.Flaws.ToList().FindIndex(flaw => flaw.Kind == FlawKind.OpenCondition && flaw.Fact!.ToString() == fact);
        Assert.True(flawIndex >= 0, $"No open condition for {fact}.");
        Flaw flaw = flaws.Flaws[flawIndex];
        int resolverIndex = flaw.Resolvers.ToList().FindIndex(resolver => predicate(resolver));
        Assert.True(resolverIndex > 0, $"No matching resolver in {flaw}: {string.Join(", ", flaw.Resolvers)}");
        chosen[flawIndex] = resolverIndex;
    }

    private static IEnumerable<int[]> Rows(IReadOnlyList<int> values, int arity)
    {
        Assert.Equal(0, values.Count % arity);
        for (int offset = 0; offset < values.Count; offset += arity)
            yield return values.Skip(offset).Take(arity).ToArray();
    }

    private static void AssertRow(EncodingContext context, string relation, params int[] expected)
        => Assert.Contains(Rows(context.Relations[relation], expected.Length), row => row.SequenceEqual(expected));

    [Fact]
    public void PartialPlanEncodesStepsLinksFlawsAndResolversMatchingTheLayout()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = BuildPlanWithEveryFlawKind(problem);
        FlawSet flaws = plan.ComputeFlaws();
        var context = new EncodingContext();
        context.BeginInstance(problem);
        Dictionary<string, int> objects = problem.AllObjects.ToDictionary(constant => constant.Name, context.GetObjectId);
        PlanEncodingLayout layout = Encoding.EncodePartialPlan(context, plan, flaws, "_p");
        context.EndInstance();

        List<PlanStep> steps = plan.Steps.ToList();
        List<CausalLink> links = plan.Links.ToList();
        List<Flaw> flawList = flaws.Flaws.ToList();
        int Node(PlanStep step) => layout.StepNodeIds[steps.IndexOf(step)];
        int Parameter(PlanStep step, int index) => layout.ParameterNodeIds[steps.IndexOf(step)][index];
        PlanStep stack = steps.Single(step => step.Schema?.Name == "stack");
        PlanStep unstack = steps.Single(step => step.Schema?.Name == "unstack");
        PlanStep pickup = steps.Single(step => step.Schema?.Name == "pickup");

        Assert.Equal([layout.StepNodeIds[0]], context.Relations["plan_initial_p"]);
        Assert.Equal([layout.StepNodeIds[1]], context.Relations["plan_goal_p"]);
        Assert.Equal([Node(stack), Node(unstack), Node(pickup)], context.Relations["plan_step_p"]);
        Assert.Equal(context.Relations["plan_step_p"], context.ActionIndices);
        Assert.Empty(layout.ParameterNodeIds[0]);
        Assert.Empty(layout.ParameterNodeIds[1]);
        AssertRow(context, "plan_action_stack_p", Node(stack), Parameter(stack, 0), Parameter(stack, 1));
        AssertRow(context, "plan_action_unstack_p", Node(unstack), Parameter(unstack, 0), Parameter(unstack, 1));
        AssertRow(context, "plan_action_pickup_p", Node(pickup), Parameter(pickup, 0));
        AssertRow(context, "plan_bound_p", Parameter(stack, 0), objects["b2"]);
        AssertRow(context, "plan_bound_p", Parameter(stack, 1), objects["b3"]);
        AssertRow(context, "plan_bound_p", Parameter(unstack, 1), objects["b1"]);
        AssertRow(context, "plan_bound_p", Parameter(pickup, 0), objects["b2"]);
        Assert.Contains(Parameter(unstack, 0), context.Relations["plan_unbound_p"]);
        Assert.DoesNotContain(Parameter(unstack, 1), context.Relations["plan_unbound_p"]);

        int[] expectedBefore = plan.OrderingClosure.SelectMany(pair => new[] { Node(plan.GetStep(pair.Before)), Node(plan.GetStep(pair.After)) }).ToArray();
        Assert.Equal(expectedBefore, context.Relations["plan_before_p"]);

        CausalLink clearLink = links.Single(link => link.Fact.ToString() == "(clear b2)");
        int clearLinkNode = layout.LinkNodeIds[links.IndexOf(clearLink)];
        Assert.Equal(links.Count, layout.LinkNodeIds.Length);
        AssertRow(context, "plan_link_clear_p", clearLinkNode, objects["b2"]);
        AssertRow(context, "plan_produces_p", layout.StepNodeIds[0], clearLinkNode);
        AssertRow(context, "plan_consumes_p", clearLinkNode, layout.StepNodeIds[1]);
        Assert.Equal(links.Count * 2, context.Relations["plan_produces_p"].Count);
        Assert.Equal(links.Count * 2, context.Relations["plan_consumes_p"].Count);

        Flaw threat = Assert.Single(flawList, flaw => flaw.Kind == FlawKind.Threat);
        int threatNode = layout.FlawNodeIds[flawList.IndexOf(threat)];
        Assert.Equal([threatNode], context.Relations["plan_flaw_threat_p"]);
        Assert.Equal([threatNode, Node(pickup), clearLinkNode], context.Relations["plan_threat_p"]);
        Assert.Equal([ResolverKind.NoOp, ResolverKind.DeleteStep], threat.Resolvers.Select(resolver => resolver.Kind));
        Assert.Equal([layout.ResolverNodeIds[flawList.IndexOf(threat)][1], Node(pickup)], context.Relations["plan_res_delete_p"]);
        Assert.False(context.Relations.ContainsKey("plan_res_promote_p"));
        Assert.False(context.Relations.ContainsKey("plan_res_demote_p"));

        Flaw unbound = Assert.Single(flawList, flaw => flaw.Kind == FlawKind.UnboundParameter);
        int unboundIndex = flawList.IndexOf(unbound);
        Assert.Equal([layout.FlawNodeIds[unboundIndex]], context.Relations["plan_flaw_unbound_p"]);
        Assert.Equal([layout.FlawNodeIds[unboundIndex], Parameter(unstack, 0)], context.Relations["plan_unbound_flaw_p"]);
        int[] expectedBinds = unbound.Resolvers
            .Select((resolver, index) => (resolver, index))
            .Where(pair => pair.resolver.Kind == ResolverKind.Bind)
            .SelectMany(pair => new[] { layout.ResolverNodeIds[unboundIndex][pair.index], objects[pair.resolver.Object!.Name] })
            .ToArray();
        Assert.NotEmpty(expectedBinds);
        Assert.Equal(expectedBinds, context.Relations["plan_res_bind_p"]);

        Flaw clearB3 = flawList.Single(flaw => flaw.Kind == FlawKind.OpenCondition && flaw.Fact!.ToString() == "(clear b3)");
        int clearB3Index = flawList.IndexOf(clearB3);
        int clearB3Node = layout.FlawNodeIds[clearB3Index];
        Assert.Contains(clearB3Node, context.Relations["plan_flaw_open_p"]);
        AssertRow(context, "plan_open_clear_p", clearB3Node, objects["b3"]);
        AssertRow(context, "plan_open_consumer_p", clearB3Node, Node(stack));
        Resolver adoptPutdown = clearB3.Resolvers.Single(resolver => IsGhost(flaws, resolver, "(putdown b3)"));
        Resolver adoptUnstack = clearB3.Resolvers.Single(resolver => IsGhost(flaws, resolver, "(unstack ? b3)"));
        int putdownGhost = layout.GhostNodeIds[adoptPutdown.GhostIndex];
        int unstackGhost = layout.GhostNodeIds[adoptUnstack.GhostIndex];
        AssertRow(context, "plan_res_adopt_p", layout.ResolverNodeIds[clearB3Index][clearB3.Resolvers.ToList().IndexOf(adoptPutdown)], putdownGhost);
        Assert.Contains(putdownGhost, context.Relations["plan_ghost_p"]);
        AssertRow(context, "plan_action_putdown_p", putdownGhost, layout.GhostParameterNodeIds[adoptPutdown.GhostIndex][0]);
        AssertRow(context, "plan_bound_p", layout.GhostParameterNodeIds[adoptPutdown.GhostIndex][0], objects["b3"]);
        AssertRow(context, "plan_action_unstack_p", unstackGhost, layout.GhostParameterNodeIds[adoptUnstack.GhostIndex][0], layout.GhostParameterNodeIds[adoptUnstack.GhostIndex][1]);
        Assert.Contains(layout.GhostParameterNodeIds[adoptUnstack.GhostIndex][0], context.Relations["plan_unbound_p"]);
        AssertRow(context, "plan_bound_p", layout.GhostParameterNodeIds[adoptUnstack.GhostIndex][1], objects["b3"]);

        Flaw clearB2 = flawList.Single(flaw => flaw.Kind == FlawKind.OpenCondition && flaw.Fact!.ToString() == "(clear b2)" && flaw.StepId == pickup.Id);
        int linkExisting = clearB2.Resolvers.ToList().FindIndex(resolver => resolver.Kind == ResolverKind.LinkExisting && resolver.StepId == 0);
        Assert.True(linkExisting > 0);
        AssertRow(context, "plan_res_producer_p", layout.ResolverNodeIds[flawList.IndexOf(clearB2)][linkExisting], layout.StepNodeIds[0]);

        int[] expectedResolves = layout.ResolverNodeIds
            .SelectMany((resolvers, flawIndex) => resolvers.SelectMany(resolver => new[] { resolver, layout.FlawNodeIds[flawIndex] }))
            .ToArray();
        Assert.Equal(expectedResolves, context.Relations["plan_resolves_p"]);
        Assert.Equal(layout.ResolverNodeIds.Select(resolvers => resolvers[0]), context.Relations["plan_res_noop_p"]);
        Assert.Empty(layout.TombstoneNodeIds);
        Assert.False(context.Relations.ContainsKey("plan_tombstone_p"));

        int auxiliaryCount = 2
            + layout.ParameterNodeIds.Sum(ids => ids.Length)
            + layout.LinkNodeIds.Length
            + layout.GhostNodeIds.Length
            + layout.GhostParameterNodeIds.Sum(ids => ids.Length)
            + layout.FlawNodeIds.Length
            + layout.ResolverNodeIds.Sum(ids => ids.Length);
        Assert.Equal([3], context.ActionSizes);
        Assert.Equal([auxiliaryCount], context.AuxiliarySizes);
        Assert.Equal([3 + 3 + auxiliaryCount], context.NodeSizes);

        Dictionary<string, int> arities = Encoding.PartialPlanRelations(problem.Domain, "_p").ToDictionary(relation => relation.Name, relation => relation.Arity);
        Assert.All(context.Relations, relation =>
        {
            Assert.True(arities.ContainsKey(relation.Key), $"Relation {relation.Key} is not declared.");
            Assert.NotEmpty(relation.Value);
            Assert.Equal(0, relation.Value.Count % arities[relation.Key]);
        });
    }

    [Fact]
    public void DeletedStepsAreEncodedAsTombstones()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = BuildPlanWithEveryFlawKind(problem);
        FlawSet flaws = plan.ComputeFlaws();
        int[] chosen = new int[flaws.Flaws.Count];
        Flaw threat = Assert.Single(flaws.Flaws, flaw => flaw.Kind == FlawKind.Threat);
        chosen[flaws.Flaws.ToList().IndexOf(threat)] = 1;
        plan.Apply(flaws, chosen, []);
        Tombstone tombstone = Assert.Single(plan.Tombstones);
        Assert.Equal("pickup", tombstone.Schema.Name);

        flaws = plan.ComputeFlaws();
        var context = new EncodingContext();
        context.BeginInstance(problem);
        int b2 = context.GetObjectId(problem.AllObjects.Single(constant => constant.Name == "b2"));
        PlanEncodingLayout layout = Encoding.EncodePartialPlan(context, plan, flaws);
        context.EndInstance();

        int tombstoneNode = Assert.Single(layout.TombstoneNodeIds);
        Assert.Equal([tombstoneNode], context.Relations["plan_tombstone"]);
        Assert.Equal([tombstoneNode, b2], context.Relations["plan_tombstone_action_pickup"]);
        Assert.Equal(2, context.Relations["plan_step"].Count);
        Assert.False(context.Relations.ContainsKey("plan_flaw_threat"));
    }

    [Fact]
    public void RelationListCoversFixedSchemaAndFluentRelations()
    {
        Problem problem = LoadBlocks();
        Dictionary<string, int> relations = Encoding.PartialPlanRelations(problem.Domain, "_s").ToDictionary(relation => relation.Name, relation => relation.Arity);

        Assert.Equal(1, relations["plan_initial_s"]);
        Assert.Equal(3, relations["plan_threat_s"]);
        Assert.Equal(2, relations["plan_res_bind_s"]);
        Assert.Equal(3, relations["plan_action_stack_s"]);
        Assert.Equal(2, relations["plan_tombstone_action_pickup_s"]);
        Assert.Equal(3, relations["plan_link_on_s"]);
        Assert.Equal(1, relations["plan_open_arm-empty_s"]);
        Assert.Equal(24 + 2 * problem.Domain.Actions.Count + 2 * problem.Domain.Fluents.Count, relations.Count);
    }

    [Fact]
    public void EncodingRejectsForeignPlansAndFlawSets()
    {
        Problem problem = LoadBlocks();
        var plan = PartialOrderPlan.Empty(problem, new PlanSpaceOptions());
        FlawSet foreignFlaws = plan.Clone().ComputeFlaws();
        var context = new EncodingContext();

        Assert.Throws<InvalidOperationException>(() => Encoding.EncodePartialPlan(context, plan, plan.ComputeFlaws()));
        context.BeginInstance(problem);
        Assert.Throws<ArgumentException>(() => Encoding.EncodePartialPlan(context, plan, foreignFlaws));
        context.EndInstance();

        Problem other = Problem.FromFile(problem.Domain, Path.Combine(ExamplesPath, "blocks_4", "p01.pddl"));
        context.BeginInstance(other);
        Assert.Throws<ArgumentException>(() => Encoding.EncodePartialPlan(context, plan, plan.ComputeFlaws()));
        context.EndInstance();
    }
}
