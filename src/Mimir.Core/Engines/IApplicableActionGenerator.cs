namespace Mimir.Core.Engines;

using Grounding;
using Schemas;

public enum ApplicableActionGeneratorType
{
    Grounded,
    Lifted,
}

public interface IApplicableActionGenerator
{
    Problem Problem { get; }

    IEnumerable<Action> GetApplicableActions(ExtendedState state);

    IEnumerable<Action> GetApplicableActions(ExtendedState state, int maxActions);
}
