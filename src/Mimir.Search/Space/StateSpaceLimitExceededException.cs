namespace Mimir.Search.Space;

public sealed class StateSpaceLimitExceededException : Exception
{
    public StateSpaceLimitExceededException(int maxStates)
        : base($"Search-space expansion exceeded the maximum of {maxStates} states.")
    {
    }
}
