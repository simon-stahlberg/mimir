using System.Runtime.CompilerServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed class GhostStep : IEquatable<GhostStep>
{
    private readonly Constant?[] _binding;

    public ActionSchema Schema { get; }
    public IReadOnlyList<Constant?> Binding { get; }

    internal GhostStep(ActionSchema schema, Constant?[] binding)
    {
        Schema = schema;
        _binding = binding;
        Binding = Array.AsReadOnly(binding);
    }

    internal Constant?[] CopyBinding() => (Constant?[])_binding.Clone();

    public bool Equals(GhostStep? other)
    {
        if (other is null || !ReferenceEquals(Schema, other.Schema)) return false;
        for (int i = 0; i < _binding.Length; i++)
        {
            if (!ReferenceEquals(_binding[i], other._binding[i])) return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => Equals(obj as GhostStep);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(Schema));
        foreach (Constant? constant in _binding)
            hash.Add(constant is null ? 0 : RuntimeHelpers.GetHashCode(constant));
        return hash.ToHashCode();
    }

    public override string ToString()
        => $"({Schema.Name} {string.Join(" ", _binding.Select(constant => constant?.Name ?? "?"))})";
}

internal static class SchemaTerms
{
    public static int ParameterIndex(ActionSchema schema, Variable variable)
    {
        for (int i = 0; i < schema.Parameters.Count; i++)
        {
            if (ReferenceEquals(schema.Parameters[i], variable)) return i;
        }

        throw new InvalidOperationException($"Variable {variable} is not a parameter of action {schema.Name}.");
    }

    public static Constant?[] Resolve(ActionSchema schema, IReadOnlyList<ITerm> terms, IReadOnlyList<Constant?> binding)
    {
        var arguments = new Constant?[terms.Count];
        for (int i = 0; i < terms.Count; i++)
        {
            arguments[i] = terms[i] switch
            {
                Variable variable => binding[ParameterIndex(schema, variable)],
                Constant constant => constant,
                _ => throw new NotSupportedException($"Unsupported term type {terms[i].GetType().Name}.")
            };
        }

        return arguments;
    }

    public static Constant[] ResolveBound(ActionSchema schema, IReadOnlyList<ITerm> terms, IReadOnlyList<Constant?> binding)
    {
        Constant?[] resolved = Resolve(schema, terms, binding);
        var arguments = new Constant[resolved.Length];
        for (int i = 0; i < resolved.Length; i++)
            arguments[i] = resolved[i] ?? throw new InvalidOperationException($"Term {terms[i]} of action {schema.Name} is unbound.");
        return arguments;
    }
}

public sealed class AchieverIndex
{
    private readonly Domain _domain;
    private readonly Dictionary<Predicate, List<(ActionSchema Schema, Atom<Fluent> Effect)>> _addEffectsByPredicate = new();

    public AchieverIndex(Domain domain)
    {
        ArgumentNullException.ThrowIfNull(domain);
        _domain = domain;
        foreach (ActionSchema schema in domain.Actions)
        {
            foreach (ConditionalEffect effect in schema.Effects)
            {
                if (!effect.IsUnconditional || effect.Effect.IsNegative) continue;
                if (!_addEffectsByPredicate.TryGetValue(effect.Effect.Value.Predicate, out List<(ActionSchema, Atom<Fluent>)>? achievers))
                {
                    achievers = new List<(ActionSchema, Atom<Fluent>)>();
                    _addEffectsByPredicate.Add(effect.Effect.Value.Predicate, achievers);
                }

                achievers.Add((schema, effect.Effect.Value));
            }
        }
    }

    public IReadOnlyList<GhostStep> Achievers(Fact<Fluent> fact)
    {
        ArgumentNullException.ThrowIfNull(fact);
        if (!_addEffectsByPredicate.TryGetValue(fact.Predicate, out List<(ActionSchema Schema, Atom<Fluent> Effect)>? candidates)) return [];

        var ghosts = new List<GhostStep>();
        foreach ((ActionSchema schema, Atom<Fluent> effect) in candidates)
        {
            Constant?[]? binding = Unify(schema, effect, fact.Arguments);
            if (binding is null) continue;

            var ghost = new GhostStep(schema, binding);
            if (!ghosts.Contains(ghost)) ghosts.Add(ghost);
        }

        return ghosts;
    }

    private Constant?[]? Unify(ActionSchema schema, Atom<Fluent> effect, IReadOnlyList<Constant> arguments)
    {
        var binding = new Constant?[schema.Parameters.Count];
        for (int position = 0; position < arguments.Count; position++)
        {
            Constant argument = arguments[position];
            if (effect.Arguments[position] is Constant constant)
            {
                if (!ReferenceEquals(constant, argument)) return null;
                continue;
            }

            var variable = (Variable)effect.Arguments[position];
            int index = SchemaTerms.ParameterIndex(schema, variable);
            if (binding[index] is null)
            {
                if (!_domain.IsCompatible(argument.Type, variable.Type)) return null;
                binding[index] = argument;
                continue;
            }

            if (!ReferenceEquals(binding[index], argument)) return null;
        }

        return binding;
    }
}
