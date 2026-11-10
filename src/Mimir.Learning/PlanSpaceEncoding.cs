using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.PlanSpace;

namespace Mimir.Learning;

public sealed class PlanEncodingLayout
{
    public required int[] StepNodeIds { get; init; }
    public required int[][] ParameterNodeIds { get; init; }
    public required int[] LinkNodeIds { get; init; }
    public required int[] GhostNodeIds { get; init; }
    public required int[][] GhostParameterNodeIds { get; init; }
    public required int[] FlawNodeIds { get; init; }
    public required int[][] ResolverNodeIds { get; init; }
    public required int[] TombstoneNodeIds { get; init; }
}

public static partial class Encoding
{
    private static readonly (string Name, int Arity)[] FixedPlanRelations =
    [
        ("plan_initial", 1),
        ("plan_goal", 1),
        ("plan_step", 1),
        ("plan_ghost", 1),
        ("plan_tombstone", 1),
        ("plan_flaw_open", 1),
        ("plan_flaw_threat", 1),
        ("plan_flaw_unbound", 1),
        ("plan_unbound", 1),
        ("plan_res_noop", 1),
        ("plan_bound", 2),
        ("plan_before", 2),
        ("plan_produces", 2),
        ("plan_consumes", 2),
        ("plan_open_consumer", 2),
        ("plan_threat", 3),
        ("plan_unbound_flaw", 2),
        ("plan_resolves", 2),
        ("plan_res_producer", 2),
        ("plan_res_adopt", 2),
        ("plan_res_promote", 2),
        ("plan_res_demote", 2),
        ("plan_res_delete", 2),
        ("plan_res_bind", 2),
    ];

    public static IReadOnlyList<(string Name, int Arity)> PartialPlanRelations(Domain domain, string suffix = "")
    {
        ArgumentNullException.ThrowIfNull(domain);
        ValidateSuffix(suffix);

        var relations = new List<(string Name, int Arity)>();
        foreach ((string name, int arity) in FixedPlanRelations)
            relations.Add(($"{name}{suffix}", arity));
        foreach (ActionSchema schema in domain.Actions)
        {
            relations.Add(($"plan_action_{schema.Name}{suffix}", 1 + schema.Parameters.Count));
            relations.Add(($"plan_tombstone_action_{schema.Name}{suffix}", 1 + schema.Parameters.Count));
        }
        foreach (Mimir.Core.Schemas.Predicate<Fluent> predicate in domain.Fluents)
        {
            relations.Add(($"plan_link_{predicate.Name}{suffix}", 1 + predicate.Parameters.Count));
            relations.Add(($"plan_open_{predicate.Name}{suffix}", 1 + predicate.Parameters.Count));
        }
        return relations;
    }

    public static PlanEncodingLayout EncodePartialPlan(
        EncodingContext context,
        PartialOrderPlan plan,
        FlawSet flaws,
        string suffix = "")
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(plan);
        ArgumentNullException.ThrowIfNull(flaws);
        ValidateSuffix(suffix);
        if (!ReferenceEquals(context.Problem, plan.Problem))
        {
            throw new ArgumentException(
                "Plan belongs to a different problem than the active encoding instance.",
                nameof(plan));
        }
        if (!ReferenceEquals(flaws.Plan, plan))
            throw new ArgumentException("The flaw set was not computed from the plan.", nameof(flaws));

        PlanStep[] steps = plan.Steps.ToArray();
        CausalLink[] links = plan.Links.ToArray();
        Tombstone[] tombstones = plan.Tombstones.ToArray();
        (int Before, int After)[] ordering = plan.OrderingClosure.ToArray();
        IReadOnlyList<Flaw> flawValues = flaws.Flaws;
        IReadOnlyList<GhostStep> ghosts = flaws.Ghosts;

        int nodeCount = steps.Length
            + steps.Sum(step => step.Binding.Count)
            + links.Length
            + ghosts.Count
            + ghosts.Sum(ghost => ghost.Binding.Count)
            + flawValues.Count
            + flawValues.Sum(flaw => flaw.Resolvers.Count)
            + tombstones.Length;
        context.EnsureCanAllocate(nodeCount);

        var layout = new PlanEncodingLayout
        {
            StepNodeIds = steps.Select(step => step.Kind == StepKind.Step ? context.NewActionId() : context.NewAuxiliaryId()).ToArray(),
            ParameterNodeIds = steps.Select(step => AllocateAuxiliary(context, step.Binding.Count)).ToArray(),
            LinkNodeIds = AllocateAuxiliary(context, links.Length),
            GhostNodeIds = AllocateAuxiliary(context, ghosts.Count),
            GhostParameterNodeIds = ghosts.Select(ghost => AllocateAuxiliary(context, ghost.Binding.Count)).ToArray(),
            FlawNodeIds = AllocateAuxiliary(context, flawValues.Count),
            ResolverNodeIds = flawValues.Select(flaw => AllocateAuxiliary(context, flaw.Resolvers.Count)).ToArray(),
            TombstoneNodeIds = AllocateAuxiliary(context, tombstones.Length),
        };

        var stepIndexById = new Dictionary<int, int>();
        for (int index = 0; index < steps.Length; index++) stepIndexById.Add(steps[index].Id, index);
        var linkIndexById = new Dictionary<int, int>();
        for (int index = 0; index < links.Length; index++) linkIndexById.Add(links[index].Id, index);

        int StepNode(int stepId)
        {
            if (!stepIndexById.TryGetValue(stepId, out int index))
                throw new InvalidOperationException($"Step {stepId} is not part of the plan.");
            return layout.StepNodeIds[index];
        }

        int LinkNode(int linkId)
        {
            if (!linkIndexById.TryGetValue(linkId, out int index))
                throw new InvalidOperationException($"Link {linkId} is not part of the plan.");
            return layout.LinkNodeIds[index];
        }

        for (int index = 0; index < steps.Length; index++)
        {
            PlanStep step = steps[index];
            int node = layout.StepNodeIds[index];
            switch (step.Kind)
            {
                case StepKind.Initial:
                    context.GetOrCreateRelation($"plan_initial{suffix}").Add(node);
                    break;
                case StepKind.Goal:
                    context.GetOrCreateRelation($"plan_goal{suffix}").Add(node);
                    break;
                default:
                    context.GetOrCreateRelation($"plan_step{suffix}").Add(node);
                    AppendSchemaInstance(context, suffix, node, step.Schema!, step.Binding, layout.ParameterNodeIds[index]);
                    break;
            }
        }

        for (int index = 0; index < ghosts.Count; index++)
        {
            int node = layout.GhostNodeIds[index];
            context.GetOrCreateRelation($"plan_ghost{suffix}").Add(node);
            AppendSchemaInstance(context, suffix, node, ghosts[index].Schema, ghosts[index].Binding, layout.GhostParameterNodeIds[index]);
        }

        foreach ((int before, int after) in ordering)
        {
            List<int> ids = context.GetOrCreateRelation($"plan_before{suffix}");
            ids.Add(StepNode(before));
            ids.Add(StepNode(after));
        }

        for (int index = 0; index < links.Length; index++)
        {
            CausalLink link = links[index];
            int node = layout.LinkNodeIds[index];
            AppendFactRow(context, $"plan_link_{link.Fact.Predicate.Name}{suffix}", node, link.Fact);
            List<int> produces = context.GetOrCreateRelation($"plan_produces{suffix}");
            produces.Add(StepNode(link.ProducerId));
            produces.Add(node);
            List<int> consumes = context.GetOrCreateRelation($"plan_consumes{suffix}");
            consumes.Add(node);
            consumes.Add(StepNode(link.ConsumerId));
        }

        for (int index = 0; index < flawValues.Count; index++)
        {
            Flaw flaw = flawValues[index];
            int node = layout.FlawNodeIds[index];
            switch (flaw.Kind)
            {
                case FlawKind.OpenCondition:
                    context.GetOrCreateRelation($"plan_flaw_open{suffix}").Add(node);
                    AppendFactRow(context, $"plan_open_{flaw.Fact!.Predicate.Name}{suffix}", node, flaw.Fact);
                    List<int> consumer = context.GetOrCreateRelation($"plan_open_consumer{suffix}");
                    consumer.Add(node);
                    consumer.Add(StepNode(flaw.StepId));
                    break;
                case FlawKind.Threat:
                    context.GetOrCreateRelation($"plan_flaw_threat{suffix}").Add(node);
                    List<int> threat = context.GetOrCreateRelation($"plan_threat{suffix}");
                    threat.Add(node);
                    threat.Add(StepNode(flaw.StepId));
                    threat.Add(LinkNode(flaw.LinkId));
                    break;
                case FlawKind.UnboundParameter:
                    context.GetOrCreateRelation($"plan_flaw_unbound{suffix}").Add(node);
                    List<int> unbound = context.GetOrCreateRelation($"plan_unbound_flaw{suffix}");
                    unbound.Add(node);
                    unbound.Add(layout.ParameterNodeIds[stepIndexById[flaw.StepId]][flaw.ParameterIndex]);
                    break;
                default:
                    throw new NotSupportedException(flaw.Kind.ToString());
            }

            for (int resolverIndex = 0; resolverIndex < flaw.Resolvers.Count; resolverIndex++)
            {
                Resolver resolver = flaw.Resolvers[resolverIndex];
                int resolverNode = layout.ResolverNodeIds[index][resolverIndex];
                List<int> resolves = context.GetOrCreateRelation($"plan_resolves{suffix}");
                resolves.Add(resolverNode);
                resolves.Add(node);

                switch (resolver.Kind)
                {
                    case ResolverKind.NoOp:
                        context.GetOrCreateRelation($"plan_res_noop{suffix}").Add(resolverNode);
                        break;
                    case ResolverKind.LinkExisting:
                        AppendPair(context, $"plan_res_producer{suffix}", resolverNode, StepNode(resolver.StepId));
                        break;
                    case ResolverKind.AdoptGhost:
                        AppendPair(context, $"plan_res_adopt{suffix}", resolverNode, layout.GhostNodeIds[resolver.GhostIndex]);
                        break;
                    case ResolverKind.Promote:
                        AppendPair(context, $"plan_res_promote{suffix}", resolverNode, StepNode(flaw.StepId));
                        break;
                    case ResolverKind.Demote:
                        AppendPair(context, $"plan_res_demote{suffix}", resolverNode, StepNode(flaw.StepId));
                        break;
                    case ResolverKind.DeleteStep:
                        AppendPair(context, $"plan_res_delete{suffix}", resolverNode, StepNode(resolver.StepId));
                        break;
                    case ResolverKind.Bind:
                        AppendPair(context, $"plan_res_bind{suffix}", resolverNode, context.GetObjectId(resolver.Object!));
                        break;
                    default:
                        throw new NotSupportedException(resolver.Kind.ToString());
                }
            }
        }

        for (int index = 0; index < tombstones.Length; index++)
        {
            Tombstone tombstone = tombstones[index];
            int node = layout.TombstoneNodeIds[index];
            context.GetOrCreateRelation($"plan_tombstone{suffix}").Add(node);
            List<int> ids = context.GetOrCreateRelation($"plan_tombstone_action_{tombstone.Schema.Name}{suffix}");
            ids.Add(node);
            foreach (Constant constant in tombstone.Binding)
                ids.Add(context.GetObjectId(constant));
        }

        return layout;
    }

    private static int[] AllocateAuxiliary(EncodingContext context, int count)
    {
        var ids = new int[count];
        for (int index = 0; index < count; index++)
            ids[index] = context.NewAuxiliaryId();
        return ids;
    }

    private static void AppendSchemaInstance(
        EncodingContext context,
        string suffix,
        int node,
        ActionSchema schema,
        IReadOnlyList<Constant?> binding,
        int[] parameterNodes)
    {
        List<int> action = context.GetOrCreateRelation($"plan_action_{schema.Name}{suffix}");
        action.Add(node);
        action.AddRange(parameterNodes);
        for (int index = 0; index < binding.Count; index++)
        {
            Constant? value = binding[index];
            if (value is null)
            {
                context.GetOrCreateRelation($"plan_unbound{suffix}").Add(parameterNodes[index]);
                continue;
            }
            AppendPair(context, $"plan_bound{suffix}", parameterNodes[index], context.GetObjectId(value));
        }
    }

    private static void AppendFactRow(EncodingContext context, string relationName, int node, Fact fact)
    {
        List<int> ids = context.GetOrCreateRelation(relationName);
        ids.Add(node);
        foreach (Constant argument in fact.Arguments)
            ids.Add(context.GetObjectId(argument));
    }

    private static void AppendPair(EncodingContext context, string relationName, int first, int second)
    {
        List<int> ids = context.GetOrCreateRelation(relationName);
        ids.Add(first);
        ids.Add(second);
    }
}
