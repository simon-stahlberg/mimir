using System.Runtime.InteropServices;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Learning;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Interop;

public static partial class Exports
{
    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_create")]
    public static int LearningEncodingContextCreate()
        => CreateHandle(() => new EncodingContext());

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_begin_instance")]
    public static byte LearningEncodingContextBeginInstance(
        int contextHandle,
        int problemHandle)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        Problem? problem = GetLearningHandle<Problem>(problemHandle);
        if (problem is null)
            return 0;

        return RunLearningOperation(() => context.BeginInstance(problem));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_end_instance")]
    public static byte LearningEncodingContextEndInstance(int handle)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(handle);
        if (context is null)
            return 0;

        return RunLearningOperation(context.EndInstance);
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_batch_count")]
    public static int LearningEncodingContextGetBatchCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.BatchCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_node_count")]
    public static int LearningEncodingContextGetNodeCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.NodeCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_node_sizes")]
    public static int LearningEncodingContextCopyNodeSizes(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.NodeSizes);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_object_sizes")]
    public static int LearningEncodingContextCopyObjectSizes(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ObjectSizes);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_object_indices")]
    public static int LearningEncodingContextCopyObjectIndices(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ObjectIndices);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_action_sizes")]
    public static int LearningEncodingContextCopyActionSizes(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ActionSizes);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_action_indices")]
    public static int LearningEncodingContextCopyActionIndices(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ActionIndices);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_virtual_sizes")]
    public static int LearningEncodingContextCopyVirtualSizes(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.VirtualSizes);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_virtual_indices")]
    public static int LearningEncodingContextCopyVirtualIndices(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.VirtualIndices);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_auxiliary_sizes")]
    public static int LearningEncodingContextCopyAuxiliarySizes(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.AuxiliarySizes);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_auxiliary_indices")]
    public static int LearningEncodingContextCopyAuxiliaryIndices(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.AuxiliaryIndices);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_object_count")]
    public static int LearningEncodingContextGetCurrentObjectCount(int handle)
        => ReadValue(
            handle,
            -1,
            (EncodingContext context) => context.ObjectIdValues.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_node_offset")]
    public static int LearningEncodingContextGetCurrentNodeOffset(int handle)
        => ReadValue(
            handle,
            -1,
            (EncodingContext context) => context.CurrentNodeOffset);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_node_count")]
    public static int LearningEncodingContextGetCurrentNodeCount(int handle)
        => ReadValue(
            handle,
            -1,
            (EncodingContext context) => context.CurrentNodeCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_action_count")]
    public static int LearningEncodingContextGetCurrentActionCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.ActionIds.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_virtual_count")]
    public static int LearningEncodingContextGetCurrentVirtualCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.VirtualIds.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_current_auxiliary_count")]
    public static int LearningEncodingContextGetCurrentAuxiliaryCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.AuxiliaryIds.Count);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_current_object_ids")]
    public static int LearningEncodingContextCopyCurrentObjectIds(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ObjectIdValues);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_current_action_ids")]
    public static int LearningEncodingContextCopyCurrentActionIds(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.ActionIds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_current_virtual_ids")]
    public static int LearningEncodingContextCopyCurrentVirtualIds(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.VirtualIds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_current_auxiliary_ids")]
    public static int LearningEncodingContextCopyCurrentAuxiliaryIds(
        int handle,
        IntPtr destination,
        int capacity)
        => CopyEncodingContextValues(
            handle,
            destination,
            capacity,
            context => context.AuxiliaryIds);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_object_id")]
    public static int LearningEncodingContextGetObjectId(
        int contextHandle,
        int objectHandle)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return -1;
        Constant? value = GetLearningHandle<Constant>(objectHandle);
        if (value is null)
            return -1;

        return RunLearningValue(() => context.GetObjectId(value));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_new_action_id")]
    public static int LearningEncodingContextNewActionId(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.NewActionId());

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_new_virtual_id")]
    public static int LearningEncodingContextNewVirtualId(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.NewVirtualId());

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_new_or_existing_virtual_id")]
    public static int LearningEncodingContextNewOrExistingVirtualId(int handle)
        => ReadValue(
            handle,
            -1,
            (EncodingContext context) => context.NewOrExistingVirtualId());

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_new_auxiliary_id")]
    public static int LearningEncodingContextNewAuxiliaryId(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.NewAuxiliaryId());

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_new_or_existing_object_pair_id")]
    public static int LearningEncodingContextNewOrExistingObjectPairId(
        int contextHandle,
        int firstHandle,
        int secondHandle)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return -1;
        Constant? first = GetLearningHandle<Constant>(firstHandle);
        if (first is null)
            return -1;
        Constant? second = GetLearningHandle<Constant>(secondHandle);
        if (second is null)
            return -1;

        return RunLearningValue(
            () => context.NewOrExistingObjectPairId(first, second));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_try_get_object_pair_id")]
    public static int LearningEncodingContextTryGetObjectPairId(
        int contextHandle,
        int firstHandle,
        int secondHandle)
    {
        EncodingContext? context = ObjectRegistry.Get<EncodingContext>(contextHandle);
        if (context is null)
        {
            RecordInvalidHandle(nameof(EncodingContext), contextHandle);
            return -2;
        }
        Constant? first = ObjectRegistry.Get<Constant>(firstHandle);
        if (first is null)
        {
            RecordInvalidHandle(nameof(Constant), firstHandle);
            return -2;
        }
        Constant? second = ObjectRegistry.Get<Constant>(secondHandle);
        if (second is null)
        {
            RecordInvalidHandle(nameof(Constant), secondHandle);
            return -2;
        }

        try
        {
            return context.TryGetObjectPairId(first, second, out int pairId)
                ? pairId
                : -1;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return -2;
        }
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_relation_count")]
    public static int LearningEncodingContextGetRelationCount(int handle)
        => ReadValue(handle, -1, (EncodingContext context) => context.RelationCount);

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_relation_name")]
    public static IntPtr LearningEncodingContextGetRelationName(
        int handle,
        int relationIndex)
        => ReadValue(handle, IntPtr.Zero, (EncodingContext context) =>
        {
            KeyValuePair<string, IReadOnlyList<int>> relation = context.GetRelation(
                relationIndex);
            return AllocUtf8(relation.Key);
        });

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_get_relation_value_count")]
    public static int LearningEncodingContextGetRelationValueCount(
        int handle,
        int relationIndex)
        => ReadValue(handle, -1, (EncodingContext context) =>
        {
            KeyValuePair<string, IReadOnlyList<int>> relation = context.GetRelation(
                relationIndex);
            return relation.Value.Count;
        });

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_relation_values")]
    public static int LearningEncodingContextCopyRelationValues(
        int handle,
        int relationIndex,
        IntPtr destination,
        int capacity)
        => ReadValue(handle, -1, (EncodingContext context) =>
        {
            KeyValuePair<string, IReadOnlyList<int>> relation = context.GetRelation(
                relationIndex);
            return CopyInt32Values(
                relation.Value,
                destination,
                capacity,
                "relation values");
        });

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encoding_context_copy_all_relation_values")]
    public static int LearningEncodingContextCopyAllRelationValues(
        int handle,
        IntPtr destination,
        int capacity)
        => ReadValue(handle, -1, (EncodingContext context) =>
            CopyAllRelationValues(context, destination, capacity));

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_state")]
    public static byte LearningEncodeState(
        int contextHandle,
        int stateHandle,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? state = GetLearningHandle<ExtendedState>(stateHandle);
        if (state is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(() => Encoding.EncodeState(context, state, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_goal")]
    public static byte LearningEncodeGoal(
        int contextHandle,
        int stateHandle,
        IntPtr goalHandlesPtr,
        int goalCount,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? state = GetLearningHandle<ExtendedState>(stateHandle);
        if (state is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(
            goalHandlesPtr,
            goalCount,
            "goal");
        if (goal is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(
            () => Encoding.EncodeGoal(context, state, goal, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_action_list")]
    public static byte LearningEncodeActionList(
        int contextHandle,
        int stateHandle,
        IntPtr actionHandlesPtr,
        int actionCount,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? state = GetLearningHandle<ExtendedState>(stateHandle);
        if (state is null)
            return 0;
        List<GroundAction>? actions = ReadLearningHandleArray<GroundAction>(
            actionHandlesPtr,
            actionCount,
            "actions");
        if (actions is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(
            () => Encoding.EncodeActionList(context, state, actions, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_transition_effects")]
    public static byte LearningEncodeTransitionEffects(
        int contextHandle,
        int sourceHandle,
        IntPtr successorHandlesPtr,
        int successorCount,
        IntPtr actionHandlesPtr,
        IntPtr effectRelationIndicesPtr,
        int effectRelationCount,
        IntPtr goalHandlesPtr,
        int goalCount,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? source = GetLearningHandle<ExtendedState>(sourceHandle);
        if (source is null)
            return 0;
        List<ExtendedState>? successors = ReadLearningHandleArray<ExtendedState>(
            successorHandlesPtr,
            successorCount,
            "successors");
        if (successors is null)
            return 0;
        List<GroundAction>? actions = ReadLearningHandleArray<GroundAction>(
            actionHandlesPtr,
            successorCount,
            "actions");
        if (actions is null)
            return 0;
        (int FromIndex, int ToIndex)[]? effectRelations = ReadEffectRelations(
            effectRelationIndicesPtr,
            effectRelationCount);
        if (effectRelations is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(
            goalHandlesPtr,
            goalCount,
            "goal");
        if (goal is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(() => Encoding.EncodeTransitionEffects(
            context,
            source,
            successors,
            actions,
            effectRelations,
            goal,
            suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_virtual_node")]
    public static byte LearningEncodeVirtualNode(int contextHandle)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;

        return RunLearningOperation(() => Encoding.EncodeVirtualNode(context));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_expressive_state")]
    public static byte LearningEncodeExpressiveState(
        int contextHandle,
        int stateHandle,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? state = GetLearningHandle<ExtendedState>(stateHandle);
        if (state is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(
            () => Encoding.EncodeExpressiveState(context, state, suffix));
    }

    [UnmanagedCallersOnly(EntryPoint = "mimir_learning_encode_expressive_goal")]
    public static byte LearningEncodeExpressiveGoal(
        int contextHandle,
        int stateHandle,
        IntPtr goalHandlesPtr,
        int goalCount,
        IntPtr suffixPtr)
    {
        EncodingContext? context = GetLearningHandle<EncodingContext>(contextHandle);
        if (context is null)
            return 0;
        ExtendedState? state = GetLearningHandle<ExtendedState>(stateHandle);
        if (state is null)
            return 0;
        List<Literal<Fact>>? goal = ReadGroundLiteralArray(
            goalHandlesPtr,
            goalCount,
            "goal");
        if (goal is null)
            return 0;
        string? suffix = GetLearningSuffix(suffixPtr);
        if (suffix is null)
            return 0;

        return RunLearningOperation(
            () => Encoding.EncodeExpressiveGoal(context, state, goal, suffix));
    }

    private static T? GetLearningHandle<T>(int handle)
        where T : class
    {
        T? value = ObjectRegistry.Get<T>(handle);
        if (value is not null)
            return value;

        RecordInvalidHandle(typeof(T).Name, handle);
        return null;
    }

    private static byte RunLearningOperation(System.Action operation)
    {
        try
        {
            operation();
            return 1;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return 0;
        }
    }

    private static int RunLearningValue(Func<int> operation)
    {
        try
        {
            return operation();
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return -1;
        }
    }

    private static string? GetLearningSuffix(IntPtr suffixPtr)
    {
        try
        {
            string? suffix = ReadUtf8(suffixPtr);
            if (suffix is not null)
                return suffix;

            RecordError(new ArgumentNullException("suffix"));
            return null;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return null;
        }
    }

    private static List<T>? ReadLearningHandleArray<T>(
        IntPtr handlesPtr,
        int count,
        string parameterName)
        where T : class
    {
        if (!ValidateLearningArray(handlesPtr, count, parameterName))
            return null;

        try
        {
            var result = new List<T>(count);
            unsafe
            {
                int* handles = (int*)handlesPtr;
                for (int index = 0; index < count; index++)
                {
                    T? value = ObjectRegistry.Get<T>(handles[index]);
                    if (value is null)
                    {
                        RecordInvalidHandle(typeof(T).Name, handles[index]);
                        return null;
                    }
                    result.Add(value);
                }
            }
            return result;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return null;
        }
    }

    private static List<Literal<Fact>>? ReadGroundLiteralArray(
        IntPtr handlesPtr,
        int count,
        string parameterName)
    {
        if (!ValidateLearningArray(handlesPtr, count, parameterName))
            return null;

        try
        {
            var result = new List<Literal<Fact>>(count);
            unsafe
            {
                int* handles = (int*)handlesPtr;
                for (int index = 0; index < count; index++)
                {
                    int handle = handles[index];
                    object? value = ObjectRegistry.GetRaw(handle);
                    if (value is null)
                    {
                        RecordInvalidHandle(nameof(Literal<Fact>), handle);
                        return null;
                    }

                    Literal<Fact>? literal = ConvertGroundLiteral(value);
                    if (literal is null)
                    {
                        RecordError(new ArgumentException(
                            $"Handle {handle} is not a ground literal.",
                            parameterName));
                        return null;
                    }
                    result.Add(literal);
                }
            }
            return result;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return null;
        }
    }

    private static Literal<Fact>? ConvertGroundLiteral(object value)
        => value switch
        {
            Literal<Fact> literal => literal,
            Literal<Fact<Static>> literal => new Literal<Fact>(
                literal.Value,
                literal.Polarity),
            Literal<Fact<Fluent>> literal => new Literal<Fact>(
                literal.Value,
                literal.Polarity),
            Literal<Fact<Derived>> literal => new Literal<Fact>(
                literal.Value,
                literal.Polarity),
            _ => null,
        };

    private static bool ValidateLearningArray(
        IntPtr valuesPtr,
        int count,
        string parameterName)
    {
        if (count < 0)
        {
            RecordError(new ArgumentOutOfRangeException(
                parameterName,
                count,
                "Array count cannot be negative."));
            return false;
        }
        if (count > 0 && valuesPtr == IntPtr.Zero)
        {
            RecordError(new ArgumentNullException(parameterName));
            return false;
        }
        return true;
    }

    private static int[]? ReadLearningInt32Array(
        IntPtr valuesPtr,
        int count,
        string parameterName)
    {
        if (!ValidateLearningArray(valuesPtr, count, parameterName))
            return null;

        try
        {
            var result = new int[count];
            if (count > 0)
                Marshal.Copy(valuesPtr, result, 0, count);
            return result;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return null;
        }
    }

    private static (int FromIndex, int ToIndex)[]? ReadEffectRelations(
        IntPtr valuesPtr,
        int relationCount)
    {
        if (relationCount < 0)
        {
            RecordError(new ArgumentOutOfRangeException(
                nameof(relationCount),
                relationCount,
                "Effect-relation count cannot be negative."));
            return null;
        }

        try
        {
            int valueCount = checked(relationCount * 2);

            int[]? values = ReadLearningInt32Array(
                valuesPtr,
                valueCount,
                "effectRelationIndices");
            if (values is null)
                return null;

            var result = new (int FromIndex, int ToIndex)[relationCount];
            for (int index = 0; index < relationCount; index++)
                result[index] = (values[index * 2], values[index * 2 + 1]);
            return result;
        }
        catch (Exception exception)
        {
            RecordError(exception);
            return null;
        }
    }

    private static int CopyEncodingContextValues(
        int handle,
        IntPtr destination,
        int capacity,
        Func<EncodingContext, IReadOnlyList<int>> select)
        => ReadValue(handle, -1, (EncodingContext context) => CopyInt32Values(
            select(context),
            destination,
            capacity,
            "encoding context values"));

    private static unsafe int CopyInt32Values(
        IReadOnlyList<int> values,
        IntPtr destination,
        int capacity,
        string parameterName)
    {
        int count = values.Count;
        if (capacity < count)
        {
            throw new ArgumentException(
                $"The destination capacity {capacity} is smaller than the required {count} values.",
                parameterName);
        }
        if (count > 0 && destination == IntPtr.Zero)
            throw new ArgumentNullException(parameterName);

        if (count == 0)
            return 0;
        int* target = (int*)destination;
        for (int index = 0; index < count; index++)
            target[index] = values[index];
        return count;
    }

    private static unsafe int CopyAllRelationValues(
        EncodingContext context,
        IntPtr destination,
        int capacity)
    {
        int required = context.RelationValueCount;
        if (capacity < required)
        {
            throw new ArgumentException(
                $"The destination capacity {capacity} is smaller than the required {required} values.",
                "relation values");
        }
        if (required > 0 && destination == IntPtr.Zero)
            throw new ArgumentNullException("relation values");

        Span<int> values = required == 0
            ? Span<int>.Empty
            : new Span<int>((void*)destination, required);
        return context.CopyRelationValues(values);
    }
}
