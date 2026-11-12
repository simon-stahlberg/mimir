using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed record SerialInsert(int Point, ActionSchema Schema, IReadOnlyList<Constant> Binding);

public sealed record SerialMove(int StepId, int Point);

public sealed record SerialRebind(int StepId, IReadOnlyList<Constant> Binding);

// The edits of one round. All of them refer to the plan as it is before the round: a point is the insertion
// point before the step at that index, and a step is named by its id.
public sealed class SerialEdits
{
    public IReadOnlyList<SerialInsert> Inserts { get; init; } = [];
    public IReadOnlyList<int> DeletedStepIds { get; init; } = [];
    public IReadOnlyList<SerialMove> Moves { get; init; } = [];
    public IReadOnlyList<SerialRebind> Rebinds { get; init; } = [];
}

public sealed partial class SerialPlan
{
    // The new plan is read off point by point in the old order: the steps that enter at the point, inserted
    // ones in the order given and then moved ones in their old order, followed by the old step after the point
    // unless it was deleted or moved away. A rebound step keeps its id and gets its new binding wherever it ends up.
    public void Apply(SerialEdits edits)
    {
        ArgumentNullException.ThrowIfNull(edits);
        int stepCount = _steps.Count;
        var indexById = new Dictionary<int, int>();
        for (int index = 0; index < stepCount; index++) indexById.Add(_steps[index].Id, index);

        int IndexOf(int stepId)
            => indexById.TryGetValue(stepId, out int index) ? index : throw new ArgumentException($"Step {stepId} is not part of the plan.", nameof(edits));

        int Checked(int point)
            => point >= 0 && point <= stepCount ? point : throw new ArgumentOutOfRangeException(nameof(edits), point, $"The plan has {stepCount + 1} insertion points.");

        SerialStep[] steps = _steps.ToArray();
        var rebound = new HashSet<int>();
        foreach (SerialRebind rebind in edits.Rebinds)
        {
            int index = IndexOf(rebind.StepId);
            if (!rebound.Add(index)) throw new ArgumentException($"Step {rebind.StepId} is rebound twice.", nameof(edits));
            steps[index] = steps[index].WithBinding(rebind.Binding, Problem);
        }

        var entering = new List<SerialStep>[stepCount + 1];
        for (int point = 0; point <= stepCount; point++) entering[point] = new List<SerialStep>();
        int nextStepId = _nextStepId;
        foreach (SerialInsert insert in edits.Inserts)
            entering[Checked(insert.Point)].Add(new SerialStep(nextStepId++, insert.Schema, insert.Binding, Problem));

        var leaves = new bool[stepCount];
        foreach (int stepId in edits.DeletedStepIds)
        {
            int index = IndexOf(stepId);
            if (leaves[index]) throw new ArgumentException($"Step {stepId} is deleted twice.", nameof(edits));
            leaves[index] = true;
        }

        foreach (SerialMove move in edits.Moves.OrderBy(move => IndexOf(move.StepId)))
        {
            int index = IndexOf(move.StepId);
            if (leaves[index]) throw new ArgumentException($"Step {move.StepId} is moved twice or also deleted.", nameof(edits));
            leaves[index] = true;
            entering[Checked(move.Point)].Add(steps[index]);
        }

        _steps.Clear();
        for (int point = 0; point <= stepCount; point++)
        {
            _steps.AddRange(entering[point]);
            if (point < stepCount && !leaves[point]) _steps.Add(steps[point]);
        }

        _nextStepId = nextStepId;
        _diagnosis = null;
    }
}
