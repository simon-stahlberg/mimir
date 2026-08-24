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
        int literalCount)
    {
        var problem = ObjectRegistry.Get<Problem>(problemHandle);
        if (problem == null || literalCount < 0) return 0;
        if (literalCount > 0 && literalHandlesPtr == IntPtr.Zero) return 0;

        var literals = new List<Literal<Fact>>(literalCount);
        unsafe
        {
            int* handles = (int*)literalHandlesPtr;
            for (int i = 0; i < literalCount; i++)
            {
                object? value = ObjectRegistry.GetRaw(handles[i]);
                Literal<Fact>? literal = value switch
                {
                    Literal<Fact> item => item,
                    Literal<Fact<Fluent>> item => new Literal<Fact>(item.Value, item.Polarity),
                    Literal<Fact<Static>> item => new Literal<Fact>(item.Value, item.Polarity),
                    Literal<Fact<Derived>> item => new Literal<Fact>(item.Value, item.Polarity),
                    _ => null,
                };
                if (literal == null) return 0;
                literals.Add(literal);
            }
        }

        return CreateHandle(() => GoalCondition.FromLiterals(problem, literals));
    }

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
