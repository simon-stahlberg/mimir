using Mimir.Core.Grounding;
using System.Runtime.CompilerServices;
using Mimir.Core;

namespace Mimir.Core.Schemas;

/// <summary>
/// A lifted conjunctive condition: a set of parameter variables and a set of
/// literals over those variables (and optionally over object constants from
/// the problem).
///
/// Literals are typed by their predicate type (static / fluent / derived);
/// the constructor accepts pre-partitioned lists. See <see cref="Of"/> for a
/// helper that splits a heterogeneous list by inspecting each literal.
/// </summary>
public sealed class ConjunctiveCondition : IEquatable<ConjunctiveCondition>
{
    public IReadOnlyList<Literal> Literals => Array.AsReadOnly(StaticLiterals.Cast<Literal>().Concat(FluentLiterals).Concat(DerivedLiterals).ToArray());
    public IReadOnlyList<NumericComparison> Comparisons => Array.Empty<NumericComparison>();
    public Problem Problem { get; }
    public IReadOnlyList<Variable> Parameters { get; }
    public IReadOnlyList<Literal<Atom<Static>>> StaticLiterals { get; }
    public IReadOnlyList<Literal<Atom<Fluent>>> FluentLiterals { get; }
    public IReadOnlyList<Literal<Atom<Derived>>> DerivedLiterals { get; }
    internal ConjunctiveCondition(
        Problem problem,
        IReadOnlyList<Variable> parameters,
        IReadOnlyList<Literal<Atom<Static>>> staticLiterals,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentLiterals,
        IReadOnlyList<Literal<Atom<Derived>>> derivedLiterals)
    {
        Problem = problem;
        Parameters = Array.AsReadOnly(parameters.ToArray());
        StaticLiterals = Array.AsReadOnly(staticLiterals.ToArray());
        FluentLiterals = Array.AsReadOnly(fluentLiterals.ToArray());
        DerivedLiterals = Array.AsReadOnly(derivedLiterals.ToArray());
    }

    /// <summary>
    /// Build a ConjunctiveCondition from a heterogeneous literal list. Each
    /// literal must wrap an Atom&lt;Static&gt;, Atom&lt;Fluent&gt;, or Atom&lt;Derived&gt;.
    /// </summary>
    internal static ConjunctiveCondition Of(
        Problem problem,
        IReadOnlyList<Variable> parameters,
        IEnumerable<Literal> literals)
    {
        var statics = new List<Literal<Atom<Static>>>();
        var fluents = new List<Literal<Atom<Fluent>>>();
        var deriveds = new List<Literal<Atom<Derived>>>();
        foreach (var lit in literals)
        {
            switch (lit)
            {
                case Literal<Atom<Static>> ls: statics.Add(ls); break;
                case Literal<Atom<Fluent>> lf: fluents.Add(lf); break;
                case Literal<Atom<Derived>> ld: deriveds.Add(ld); break;
                default:
                    throw new ArgumentException(
                        $"ConjunctiveCondition.Of: unexpected literal type {lit?.GetType().FullName ?? "null"}.");
            }
        }
        return new ConjunctiveCondition(problem, parameters, statics, fluents, deriveds);
    }

    public bool Equals(ConjunctiveCondition? other)
        => other is not null
        && ReferenceEquals(Problem, other.Problem)
        && ValueSequence.Equals(Parameters, other.Parameters)
        && ValueSequence.Equals(StaticLiterals, other.StaticLiterals)
        && ValueSequence.Equals(FluentLiterals, other.FluentLiterals)
        && ValueSequence.Equals(DerivedLiterals, other.DerivedLiterals);

    public override bool Equals(object? obj) => Equals(obj as ConjunctiveCondition);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(Problem));
        ValueSequence.AddToHash(ref hash, Parameters);
        ValueSequence.AddToHash(ref hash, StaticLiterals);
        ValueSequence.AddToHash(ref hash, FluentLiterals);
        ValueSequence.AddToHash(ref hash, DerivedLiterals);
        return hash.ToHashCode();
    }
}
