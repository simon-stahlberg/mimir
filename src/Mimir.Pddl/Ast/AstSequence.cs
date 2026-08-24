using System.Collections.Immutable;

namespace Mimir.Pddl.Ast;

internal static class AstSequence
{
    public static bool Equals<T>(ImmutableArray<T> left, ImmutableArray<T> right)
    {
        if (left.Length != right.Length)
            return false;

        EqualityComparer<T> comparer = EqualityComparer<T>.Default;
        for (int i = 0; i < left.Length; i++)
        {
            if (!comparer.Equals(left[i], right[i]))
                return false;
        }

        return true;
    }

    public static void AddHashCode<T>(ref HashCode hash, ImmutableArray<T> values)
    {
        hash.Add(values.Length);
        foreach (T value in values)
            hash.Add(value);
    }
}
