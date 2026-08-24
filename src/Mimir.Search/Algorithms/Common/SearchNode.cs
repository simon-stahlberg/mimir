using Mimir.Core.Grounding;
using Action = Mimir.Core.Grounding.Action;


namespace Mimir.Search;

public readonly record struct SearchTransition(
    State State,
    Action Action,
    double Cost,
    State SuccessorState);

public class SearchNode
{
    public State State { get; }
    public double Cost { get; }
    public int Depth { get; }
    public Action? Action { get; }
    public SearchNode? Parent { get; }

    public SearchNode(State state, Action? action = null, SearchNode? parent = null, double cost = 0, int depth = 0)
    {
        State = state;
        Action = action;
        Parent = parent;
        Cost = cost;
        Depth = depth;
    }

    public List<Action> ExtractPlan()
    {
        var plan = new List<Action>();
        var current = this;
        while (current.Action != null)
        {
            plan.Add(current.Action);
            current = current.Parent!;
        }
        plan.Reverse();
        return plan;
    }
}
