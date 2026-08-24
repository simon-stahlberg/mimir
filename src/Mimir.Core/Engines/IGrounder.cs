namespace Mimir.Core.Engines;

using Mimir.Core.Schemas;
using Mimir.Core.Grounding;


public interface IGrounder
{
    IEnumerable<Action> Ground(Problem problem, State startState);
}
