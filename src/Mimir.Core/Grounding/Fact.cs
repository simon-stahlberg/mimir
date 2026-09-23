namespace Mimir.Core.Grounding;

using Schemas;
using System.Runtime.CompilerServices;

public abstract class Fact : IEquatable<Fact>
{
    internal FactIndex Index { get; }
    public InstanceContext Context { get; }
    public Predicate Predicate { get; }
    public IReadOnlyList<Constant> Arguments { get; }

    private protected Fact(
        InstanceContext context,
        FactIndex index,
        Predicate predicate,
        IReadOnlyList<Constant> arguments)
    {
        Context = context;
        Index = index;
        Predicate = predicate;
        Arguments = Array.AsReadOnly(arguments.ToArray());
    }

    public override string ToString()
    {
        if (Arguments.Count == 0) return $"({Predicate.Name})";
        return $"({Predicate.Name} {string.Join(" ", Arguments.Select(a => a.Name))})";
    }

    public bool Equals(Fact? other)
        => other is not null
        && ReferenceEquals(Context, other.Context)
        && Index == other.Index;
    public override bool Equals(object? obj) => Equals(obj as Fact);
    public override int GetHashCode() => HashCode.Combine(RuntimeHelpers.GetHashCode(Context), Index);
}

public class Fact<T> : Fact where T : notnull, IPredicateType
{
    internal int LocalIndex { get; }
    public new Predicate<T> Predicate => (Predicate<T>)base.Predicate;

    internal Fact(
        InstanceContext context,
        FactIndex index,
        int localIndex,
        Predicate<T> predicate,
        IReadOnlyList<Constant> arguments)
        : base(context, index, predicate, arguments)
    {
        LocalIndex = localIndex;
    }
}
