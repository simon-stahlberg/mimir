using System.Runtime.CompilerServices;

namespace Mimir.Core.Schemas;

public abstract class Atom : IEquatable<Atom>
{
    public Predicate Predicate { get; }
    public IReadOnlyList<ITerm> Arguments { get; }

    private protected Atom(Predicate predicate, IReadOnlyList<ITerm> arguments)
    {
        ArgumentNullException.ThrowIfNull(predicate);
        ArgumentNullException.ThrowIfNull(arguments);

        ITerm[] argumentCopy = arguments.ToArray();
        if (argumentCopy.Any(argument => argument is null))
            throw new ArgumentException("The collection cannot contain null values.", nameof(arguments));

        Predicate = predicate;
        Arguments = Array.AsReadOnly(argumentCopy);
    }

    public override string ToString()
    {
        if (Arguments.Count == 0) return $"({Predicate.Name})";
        return $"({Predicate.Name} {string.Join(" ", Arguments)})";
    }

    public bool Equals(Atom? other)
    {
        if (other is null) return false;
        if (GetType() != other.GetType()) return false;
        if (!ReferenceEquals(Predicate, other.Predicate)) return false;
        if (Arguments.Count != other.Arguments.Count) return false;
        for (int i = 0; i < Arguments.Count; i++)
        {
            if (!ReferenceEquals(Arguments[i], other.Arguments[i])) return false;
        }

        return true;
    }

    public override bool Equals(object? obj) => Equals(obj as Atom);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(GetType());
        hash.Add(RuntimeHelpers.GetHashCode(Predicate));
        foreach (var arg in Arguments)
        {
            hash.Add(RuntimeHelpers.GetHashCode(arg));
        }

        return hash.ToHashCode();
    }
}

public sealed class Atom<T> : Atom where T : notnull, IPredicateType
{
    public new Predicate<T> Predicate => (Predicate<T>)base.Predicate;

    public Atom(Predicate<T> predicate, IReadOnlyList<ITerm> arguments)
        : base(predicate, arguments)
    {
    }
}
