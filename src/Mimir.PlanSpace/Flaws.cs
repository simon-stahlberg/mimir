using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public enum FlawKind
{
    OpenCondition,
    Threat,
    UnboundParameter
}

public enum ResolverKind
{
    NoOp,
    LinkExisting,
    AdoptGhost,
    Promote,
    Demote,
    DeleteStep,
    Bind
}

public sealed class Resolver
{
    public static readonly Resolver NoOp = new(ResolverKind.NoOp, -1, -1, null);

    public ResolverKind Kind { get; }
    public int StepId { get; }
    public int GhostIndex { get; }
    public Constant? Object { get; }

    private Resolver(ResolverKind kind, int stepId, int ghostIndex, Constant? value)
    {
        Kind = kind;
        StepId = stepId;
        GhostIndex = ghostIndex;
        Object = value;
    }

    internal static Resolver LinkExisting(int producerId) => new(ResolverKind.LinkExisting, producerId, -1, null);
    internal static Resolver AdoptGhost(int ghostIndex) => new(ResolverKind.AdoptGhost, -1, ghostIndex, null);
    internal static Resolver Promote() => new(ResolverKind.Promote, -1, -1, null);
    internal static Resolver Demote() => new(ResolverKind.Demote, -1, -1, null);
    internal static Resolver DeleteStep(int threatenerId) => new(ResolverKind.DeleteStep, threatenerId, -1, null);
    internal static Resolver Bind(Constant value) => new(ResolverKind.Bind, -1, -1, value);

    public override string ToString() => Kind switch
    {
        ResolverKind.LinkExisting => $"LinkExisting({StepId})",
        ResolverKind.AdoptGhost => $"AdoptGhost({GhostIndex})",
        ResolverKind.DeleteStep => $"DeleteStep({StepId})",
        ResolverKind.Bind => $"Bind({Object})",
        _ => Kind.ToString()
    };
}

public sealed class Flaw
{
    public FlawKind Kind { get; }
    public int StepId { get; }
    public Fact<Fluent>? Fact { get; }
    public int LinkId { get; }
    public int ParameterIndex { get; }
    public IReadOnlyList<Resolver> Resolvers { get; }

    internal Flaw(FlawKind kind, int stepId, Fact<Fluent>? fact, int linkId, int parameterIndex, IReadOnlyList<Resolver> resolvers)
    {
        if (resolvers.Count == 0 || resolvers[0].Kind != ResolverKind.NoOp)
            throw new ArgumentException("The first resolver of a flaw must be NoOp.", nameof(resolvers));

        Kind = kind;
        StepId = stepId;
        Fact = fact;
        LinkId = linkId;
        ParameterIndex = parameterIndex;
        Resolvers = resolvers;
    }

    public override string ToString() => Kind switch
    {
        FlawKind.OpenCondition => $"OpenCondition(step {StepId}, {Fact})",
        FlawKind.Threat => $"Threat(step {StepId} on link {LinkId}, {Fact})",
        _ => $"UnboundParameter(step {StepId}, parameter {ParameterIndex})"
    };
}

public sealed class FlawSet
{
    public IReadOnlyList<Flaw> Flaws { get; }
    public IReadOnlyList<GhostStep> Ghosts { get; }
    internal PartialOrderPlan Plan { get; }
    internal int Version { get; }

    internal FlawSet(IReadOnlyList<Flaw> flaws, IReadOnlyList<GhostStep> ghosts, PartialOrderPlan plan, int version)
    {
        Flaws = flaws;
        Ghosts = ghosts;
        Plan = plan;
        Version = version;
    }
}

internal static class StaticFilter
{
    // Static preconditions whose arguments are all bound must hold initially; the others cannot be judged yet.
    public static bool Passes(Problem problem, ActionSchema schema, IReadOnlyList<Constant?> binding)
    {
        foreach (Literal<Atom<Static>> literal in schema.StaticPreconditions)
        {
            Constant?[] arguments = SchemaTerms.Resolve(schema, literal.Value.Arguments, binding);
            if (arguments.Any(argument => argument is null)) continue;
            if (Holds(problem, literal.Value.Predicate, arguments.Select(argument => argument!).ToArray()) != literal.IsPositive) return false;
        }

        return true;
    }

    public static bool Holds(Problem problem, Mimir.Core.Schemas.Predicate<Static> predicate, Constant[] arguments)
    {
        if (problem.Domain.IsEqualityPredicate(predicate)) return ReferenceEquals(arguments[0], arguments[1]);
        for (int i = 0; i < arguments.Length; i++)
        {
            if (!problem.Domain.IsCompatible(arguments[i].Type, predicate.Parameters[i].Type)) return false;
        }

        Fact<Static>? fact = problem.Context.FindFact(predicate, arguments);
        return fact is not null && problem.Context.IsTrue(new StaticIndex(fact.LocalIndex));
    }
}

public sealed partial class PartialOrderPlan
{
    public FlawSet ComputeFlaws()
    {
        var flaws = new List<Flaw>();
        var ghosts = new List<GhostStep>();
        var ghostIndices = new Dictionary<GhostStep, int>();

        foreach (PlanStep step in _steps)
        {
            if (!step.IsFullyBound)
            {
                AddUnboundParameterFlaws(step, flaws);
                continue;
            }

            foreach (Fact<Fluent> fact in step.Preconditions)
            {
                if (_supported.Contains((step.Id, fact))) continue;
                flaws.Add(OpenCondition(step, fact, ghosts, ghostIndices));
            }
        }

        foreach (CausalLink link in _links)
        {
            foreach (PlanStep threatener in _steps)
            {
                if (IsThreat(link, threatener)) flaws.Add(Threat(link, threatener));
            }
        }

        return new FlawSet(flaws, ghosts, this, _version);
    }

    private Flaw OpenCondition(PlanStep consumer, Fact<Fluent> fact, List<GhostStep> ghosts, Dictionary<GhostStep, int> ghostIndices)
    {
        var resolvers = new List<Resolver> { Resolver.NoOp };
        foreach (PlanStep producer in _steps)
        {
            if (producer.Id == consumer.Id || !producer.IsFullyBound || !producer.AddEffects.Contains(fact)) continue;
            if (IsBefore(consumer.Id, producer.Id)) continue;
            resolvers.Add(Resolver.LinkExisting(producer.Id));
        }

        foreach (GhostStep ghost in Achievers.Achievers(fact))
        {
            if (!StaticFilter.Passes(Problem, ghost.Schema, ghost.Binding)) continue;
            if (!ghostIndices.TryGetValue(ghost, out int index))
            {
                index = ghosts.Count;
                ghosts.Add(ghost);
                ghostIndices.Add(ghost, index);
            }

            resolvers.Add(Resolver.AdoptGhost(index));
        }

        return new Flaw(FlawKind.OpenCondition, consumer.Id, fact, -1, -1, resolvers);
    }

    private Flaw Threat(CausalLink link, PlanStep threatener)
    {
        var resolvers = new List<Resolver> { Resolver.NoOp };
        if (!IsBefore(threatener.Id, link.ConsumerId)) resolvers.Add(Resolver.Promote());
        if (!IsBefore(link.ProducerId, threatener.Id)) resolvers.Add(Resolver.Demote());
        resolvers.Add(Resolver.DeleteStep(threatener.Id));
        return new Flaw(FlawKind.Threat, threatener.Id, link.Fact, link.Id, -1, resolvers);
    }

    private void AddUnboundParameterFlaws(PlanStep step, List<Flaw> flaws)
    {
        ActionSchema schema = step.Schema!;
        for (int index = 0; index < step.Binding.Count; index++)
        {
            if (step.Binding[index] is not null) continue;

            var resolvers = new List<Resolver> { Resolver.NoOp };
            Constant?[] trial = step.Binding.ToArray();
            foreach (Constant candidate in Problem.AllObjects)
            {
                if (!Problem.Domain.IsCompatible(candidate.Type, schema.Parameters[index].Type)) continue;
                trial[index] = candidate;
                if (!StaticFilter.Passes(Problem, schema, trial)) continue;
                resolvers.Add(Resolver.Bind(candidate));
            }

            flaws.Add(new Flaw(FlawKind.UnboundParameter, step.Id, null, -1, index, resolvers));
        }
    }
}
