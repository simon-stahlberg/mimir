namespace Mimir.Core;

using System.Runtime.CompilerServices;

internal static class ValueSequence
{
    public static bool Equals<T>(IReadOnlyList<T> left, IReadOnlyList<T> right)
    {
        if (left.Count != right.Count) return false;

        EqualityComparer<T> comparer = EqualityComparer<T>.Default;
        for (int i = 0; i < left.Count; i++)
        {
            if (!comparer.Equals(left[i], right[i])) return false;
        }

        return true;
    }

    public static void AddToHash<T>(ref HashCode hash, IReadOnlyList<T> values)
    {
        hash.Add(values.Count);
        EqualityComparer<T> comparer = EqualityComparer<T>.Default;
        foreach (T value in values)
            hash.Add(value, comparer);
    }

    public static bool ReferenceEqualsItems<T>(IReadOnlyList<T> left, IReadOnlyList<T> right)
        where T : class
    {
        if (left.Count != right.Count) return false;

        for (int i = 0; i < left.Count; i++)
        {
            if (!ReferenceEquals(left[i], right[i])) return false;
        }

        return true;
    }

    public static void AddReferencesToHash<T>(ref HashCode hash, IReadOnlyList<T> values)
        where T : class
    {
        hash.Add(values.Count);
        foreach (T value in values)
            hash.Add(value is null ? 0 : RuntimeHelpers.GetHashCode(value));
    }
}
