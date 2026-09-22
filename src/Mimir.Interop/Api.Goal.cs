using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_create")]
    public static int GoalCreate(
        int problemHandle,
        IntPtr literalHandlesPtr,
        int literalCount, IntPtr comparisonHandles, int comparisonCount)
        => CreateHandle(() =>
        {
            Problem problem = RequireHandle<Problem>(problemHandle);
            Literal<Fact>[] literals = ReadHandleArray<object>(literalHandlesPtr, literalCount, "literals")
                .Select(value => value switch
                {
                    Literal<Fact> item => item,
                    Literal<Fact<Fluent>> item => new Literal<Fact>(item.Value, item.Polarity),
                    Literal<Fact<Static>> item => new Literal<Fact>(item.Value, item.Polarity),
                    Literal<Fact<Derived>> item => new Literal<Fact>(item.Value, item.Polarity),
                    _ => throw new ArgumentException("Goal literals must be ground literals.", "literals"),
                })
                .ToArray();
            return GoalCondition.FromLiterals(problem, literals,
                ReadHandleArray<GroundNumericComparison>(comparisonHandles, comparisonCount, "comparisons"));
        });

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_is_satisfied")]
    public static int GoalIsSatisfied(int goalHandle, int stateHandle)
        => ReadValue(goalHandle, -1, (GoalCondition goal) =>
            goal.IsSatisfied(RequireHandle<ExtendedState>(stateHandle)) ? 1 : 0);

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_literal_count")]
    public static int GoalLiteralCount(int goalHandle)
        => ReadValue(goalHandle, -1, (GoalCondition goal) => goal.GoalLiterals.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_goal_get_literal")]
    public static int GoalGetLiteral(int goalHandle, int index)
    {
        GoalCondition? goal = ObjectRegistry.Get<GoalCondition>(goalHandle);
        if (goal == null || index < 0 || index >= goal.GoalLiterals.Count) return 0;
        return ObjectRegistry.Store(goal.GoalLiterals[index]);
    }
}
