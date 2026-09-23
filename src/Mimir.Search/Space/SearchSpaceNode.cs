using Mimir.Core.Grounding;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Space;

/// <summary>
/// Internal node representing a state within the fully-expanded search space.
/// Stores all graph connectivity and metadata needed for queries.
/// </summary>
internal class SearchSpaceNode
{
    private bool _isFrozen;

    public State State { get; }
    public int Depth { get; }
    public double CostFromRoot { get; set; } = double.PositiveInfinity;

    // Graph connectivity
    public List<(SearchSpaceNode Parent, Action Action)> Parents { get; } = new();
    public List<(SearchSpaceNode Child, Action Action)> Children { get; } = new();
    public List<Action> ApplicableActions { get; } = new();
    public List<(State Successor, Action Action)> Successors { get; } = new();
    public IReadOnlyList<(State Successor, Action Action)> SuccessorView { get; private set; }
        = Array.Empty<(State, Action)>();
    public IReadOnlyList<(State Predecessor, Action Action)> PredecessorView { get; private set; }
        = Array.Empty<(State, Action)>();
    public IReadOnlyList<Action> ApplicableActionView { get; private set; } = Array.Empty<Action>();

    // Computed properties (set during analysis phase)
    public bool IsGoal { get; set; }
    public bool IsDeadEnd { get; set; }
    public int DistanceToGoal { get; set; } = -1;
    public double CostToGoal { get; set; } = double.PositiveInfinity;

    public SearchSpaceNode(State state, int depth)
    {
        State = state;
        Depth = depth;
    }

    public void Freeze()
    {
        if (_isFrozen)
            throw new InvalidOperationException("Search-space node query views have already been frozen.");

        SuccessorView = Array.AsReadOnly(Successors.ToArray());

        var predecessors = new (State Predecessor, Action Action)[Parents.Count];
        for (int i = 0; i < Parents.Count; i++)
        {
            (SearchSpaceNode parent, Action action) = Parents[i];
            predecessors[i] = (parent.State, action);
        }

        PredecessorView = Array.AsReadOnly(predecessors);
        ApplicableActionView = Array.AsReadOnly(ApplicableActions.ToArray());
        _isFrozen = true;
    }
}
