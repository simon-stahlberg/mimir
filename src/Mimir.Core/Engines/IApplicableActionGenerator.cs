namespace Mimir.Core.Engines;

using Grounding;
using Schemas;

public enum ApplicableActionGeneratorType
{
    Grounded,
    Lifted,
    Factored,
}

public interface IApplicableActionGenerator
{
    Problem Problem { get; }

    IEnumerable<Action> GetApplicableActions(ExtendedState state);

    IEnumerable<Action> GetApplicableActions(ExtendedState state, int maxActions);

    // Allocation-free alternative to GetApplicableActions for callers that expand many states.
    void CollectApplicableActions(ExtendedState state, List<Action> destination);
}
