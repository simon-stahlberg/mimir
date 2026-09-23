namespace Mimir.Core.Grounding;

public class StateEqualityComparer : IEqualityComparer<State>
{
    public static StateEqualityComparer Instance { get; } = new();

    public bool Equals(State? x, State? y)
    {
        if (ReferenceEquals(x, y)) return true;
        if (x is null || y is null) return false;

        return x.Equals(y);
    }

    public int GetHashCode(State obj) => obj.GetHashCode();
}
