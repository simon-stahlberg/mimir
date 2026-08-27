using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using System.Numerics;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Learning;

public static partial class Encoding
{
    public static void EncodeState(
        EncodingContext context,
        ExtendedState state,
        string suffix = "")
    {
        ValidateState(context, state);
        ValidateSuffix(suffix);
        EncodingContext.ProblemData data = context.CurrentProblemData;

        foreach (EncodingContext.TypeRelationGroup group in data.TypeRelations)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"type_relation_{group.TypeName}{suffix}");
            foreach (int localId in group.LocalObjectIds)
                ids.Add(context.ObjectIdValues[localId]);
        }

        foreach (Constant constant in context.Problem.Domain.Constants)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"constant_relation_{constant.Name}{suffix}");
            ids.Add(context.ObjectIds[constant]);
        }

        if (context.Problem.Domain.UsesEquality)
        {
            List<int> ids = context.GetOrCreateRelation($"relation_={suffix}");
            foreach (int objectId in context.ObjectIdValues)
            {
                ids.Add(objectId);
                ids.Add(objectId);
            }
        }

        foreach (EncodingContext.StaticRelationGroup group in data.StaticRelations)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"relation_{group.Predicate.Name}{suffix}");
            if (group.Predicate.Parameters.Count == 0)
            {
                ids.AddRange(context.ObjectIdValues);
                continue;
            }
            foreach (int localId in group.LocalObjectIds)
                ids.Add(context.ObjectIdValues[localId]);
        }

        foreach (Fact<Fluent> fact in state.State.GetTrueFacts())
            AppendFact(context, fact, $"relation_{fact.Predicate.Name}{suffix}");
        foreach (Fact<Derived> fact in state.GetTrueDerivedFacts())
            AppendFact(context, fact, $"relation_{fact.Predicate.Name}{suffix}");
    }

    public static void EncodeGoal(
        EncodingContext context,
        ExtendedState state,
        IReadOnlyList<Literal<Fact>> goal,
        string suffix = "")
    {
        ValidateState(context, state);
        ValidateSuffix(suffix);
        Literal<Fact>[] literals = SnapshotGoal(
            context,
            state.State.Context,
            goal,
            nameof(goal));

        foreach (Literal<Fact> literal in literals)
        {
            Fact fact = literal.Value;
            bool isTrue = state.IsTrue(fact);
            string relationName =
                $"relation_{fact.Predicate.Name}{suffix}_goal_{(isTrue ? "true" : "false")}";
            AppendFact(context, fact, relationName);
        }
    }

    public static void EncodeActionList(
        EncodingContext context,
        ExtendedState state,
        IReadOnlyList<GroundAction> actions,
        string suffix = "")
    {
        ValidateState(context, state);
        ValidateSuffix(suffix);
        GroundAction[] actionValues = Snapshot(actions, nameof(actions));
        var argumentIds = new int[actionValues.Length][];
        for (int position = 0; position < actionValues.Length; position++)
        {
            GroundAction action = actionValues[position];
            if (!ReferenceEquals(action.Context, state.State.Context))
            {
                throw new ArgumentException(
                    $"Action {position} belongs to a different instance context.",
                    nameof(actions));
            }
            argumentIds[position] = GetObjectIds(context, action.Arguments);
        }

        context.EnsureCanAllocate(actionValues.Length);
        for (int position = 0; position < actionValues.Length; position++)
        {
            GroundAction action = actionValues[position];
            List<int> ids = context.GetOrCreateRelation(
                $"action_{action.Schema.Name}{suffix}");
            ids.Add(context.NewActionId());
            ids.AddRange(argumentIds[position]);
        }
    }

    public static void EncodeTransitionEffects(
        EncodingContext context,
        ExtendedState source,
        IReadOnlyList<ExtendedState> successors,
        IReadOnlyList<(int FromIndex, int ToIndex)> effectRelations,
        IReadOnlyList<Literal<Fact>> goal,
        string suffix = "")
    {
        ValidateState(context, source);
        ArgumentNullException.ThrowIfNull(successors);
        ArgumentNullException.ThrowIfNull(effectRelations);
        ValidateSuffix(suffix);

        ExtendedState[] successorValues = Snapshot(successors, nameof(successors));
        for (int index = 0; index < successorValues.Length; index++)
        {
            if (!ReferenceEquals(
                    successorValues[index].State.Context,
                    source.State.Context))
            {
                throw new ArgumentException(
                    $"Successor {index} belongs to a different instance context.",
                    nameof(successors));
            }
        }

        Literal<Fact>[] goalLiterals = SnapshotGoal(
            context,
            source.State.Context,
            goal,
            nameof(goal));
        var goalFacts = new HashSet<Fact>(ReferenceEqualityComparer.Instance);
        foreach (Literal<Fact> literal in goalLiterals)
            goalFacts.Add(literal.Value);

        var validatedEffects = new ValidatedEffect[successorValues.Length][];
        if (successorValues.Length > 0)
        {
            var sourceDerivedFacts = new HashSet<Fact<Derived>>(
                source.GetTrueDerivedFacts(),
                ReferenceEqualityComparer.Instance);
            for (int transitionIndex = 0;
                 transitionIndex < successorValues.Length;
                 transitionIndex++)
            {
                validatedEffects[transitionIndex] = GetTransitionEffects(
                    context,
                    source,
                    successorValues[transitionIndex],
                    sourceDerivedFacts);
            }
        }

        int relationCount = effectRelations.Count;
        (int FromIndex, int ToIndex)[] relationValues = effectRelations.ToArray();
        if (relationValues.Length != relationCount)
        {
            throw new InvalidOperationException(
                "The effect relations changed while they were being encoded.");
        }
        foreach ((int fromIndex, int toIndex) in relationValues)
        {
            if (fromIndex < 0 || fromIndex >= successorValues.Length)
            {
                throw new ArgumentOutOfRangeException(
                    nameof(effectRelations),
                    fromIndex,
                    "An effect-relation source index is outside the transition list.");
            }
            if (toIndex < 0 || toIndex >= successorValues.Length)
            {
                throw new ArgumentOutOfRangeException(
                    nameof(effectRelations),
                    toIndex,
                    "An effect-relation destination index is outside the transition list.");
            }
        }

        context.EnsureCanAllocate(successorValues.Length);
        var transitionIds = new int[successorValues.Length];
        for (int index = 0; index < transitionIds.Length; index++)
            transitionIds[index] = context.NewActionId();

        for (int transitionIndex = 0; transitionIndex < validatedEffects.Length; transitionIndex++)
        {
            int transitionId = transitionIds[transitionIndex];
            foreach (ValidatedEffect effect in validatedEffects[transitionIndex])
            {
                string polarity = effect.Polarity == Polarity.Positive ? "pos" : "neg";
                string relationName =
                    $"{effect.Fact.Predicate.Name}{suffix}_{polarity}";
                AppendTransitionEffect(
                    context,
                    relationName,
                    transitionId,
                    effect.ObjectIds);

                if (!goalFacts.Contains(effect.Fact))
                    continue;
                AppendTransitionEffect(
                    context,
                    $"{relationName}_goal",
                    transitionId,
                    effect.ObjectIds);
            }
        }

        foreach ((int fromIndex, int toIndex) in relationValues)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"effect_relation{suffix}");
            ids.Add(transitionIds[fromIndex]);
            ids.Add(transitionIds[toIndex]);
        }
    }

    public static void EncodeVirtualNode(EncodingContext context)
    {
        ArgumentNullException.ThrowIfNull(context);
        Problem problem = context.Problem;
        int[] objectIds = GetObjectIds(context, problem.DeclaredObjects);
        context.EnsureCanAllocate(1);
        int virtualId = context.NewVirtualId();
        if (objectIds.Length == 0)
            return;

        List<int> ids = context.GetOrCreateRelation("virtual_node_link");
        foreach (int objectId in objectIds)
        {
            ids.Add(virtualId);
            ids.Add(objectId);
        }
    }

    private readonly record struct ValidatedEffect(
        Fact Fact,
        Polarity Polarity,
        int[] ObjectIds);

    private static void AppendFact(
        EncodingContext context,
        Fact fact,
        string relationName)
    {
        List<int> ids = context.GetOrCreateRelation(relationName);
        if (fact.Arguments.Count == 0)
        {
            ids.AddRange(context.ObjectIdValues);
            return;
        }
        foreach (Constant argument in fact.Arguments)
            ids.Add(context.GetObjectId(argument));
    }

    private static void AppendTransitionEffect(
        EncodingContext context,
        string relationName,
        int transitionId,
        IReadOnlyList<int> objectIds)
    {
        List<int> ids = context.GetOrCreateRelation(relationName);
        ids.Add(transitionId);
        ids.AddRange(objectIds);
    }

    private static ValidatedEffect[] GetTransitionEffects(
        EncodingContext context,
        ExtendedState source,
        ExtendedState successor,
        IReadOnlySet<Fact<Derived>> sourceDerivedFacts)
    {
        var result = new List<ValidatedEffect>();
        ReadOnlySpan<ulong> sourceBits = source.State.Bitboard;
        ReadOnlySpan<ulong> successorBits = successor.State.Bitboard;
        int wordCount = Math.Max(sourceBits.Length, successorBits.Length);

        for (int wordIndex = 0; wordIndex < wordCount; wordIndex++)
        {
            ulong sourceWord = wordIndex < sourceBits.Length
                ? sourceBits[wordIndex]
                : 0UL;
            ulong successorWord = wordIndex < successorBits.Length
                ? successorBits[wordIndex]
                : 0UL;
            AppendChangedFluentEffects(
                context,
                source.State.Context,
                result,
                wordIndex,
                successorWord & ~sourceWord,
                Polarity.Positive);
            AppendChangedFluentEffects(
                context,
                source.State.Context,
                result,
                wordIndex,
                sourceWord & ~successorWord,
                Polarity.Negative);
        }

        var successorDerivedFacts = new HashSet<Fact<Derived>>(
            successor.GetTrueDerivedFacts(),
            ReferenceEqualityComparer.Instance);
        foreach (Fact<Derived> fact in successorDerivedFacts)
        {
            if (!sourceDerivedFacts.Contains(fact))
                result.Add(CreateValidatedEffect(context, fact, Polarity.Positive));
        }
        foreach (Fact<Derived> fact in sourceDerivedFacts)
        {
            if (!successorDerivedFacts.Contains(fact))
                result.Add(CreateValidatedEffect(context, fact, Polarity.Negative));
        }

        return result.ToArray();
    }

    private static void AppendChangedFluentEffects(
        EncodingContext context,
        InstanceContext instanceContext,
        List<ValidatedEffect> effects,
        int wordIndex,
        ulong changedBits,
        Polarity polarity)
    {
        while (changedBits != 0)
        {
            int bitIndex = BitOperations.TrailingZeroCount(changedBits);
            Fact<Fluent> fact = instanceContext.GetFact(
                new FluentIndex(checked(wordIndex * 64 + bitIndex)));
            effects.Add(CreateValidatedEffect(context, fact, polarity));
            changedBits &= changedBits - 1;
        }
    }

    private static ValidatedEffect CreateValidatedEffect(
        EncodingContext context,
        Fact fact,
        Polarity polarity)
        => new(fact, polarity, GetObjectIds(context, fact.Arguments));

    private static void ValidateLiteral(
        EncodingContext context,
        InstanceContext instanceContext,
        Literal<Fact> literal,
        string parameterName)
    {
        if (!ReferenceEquals(literal.Value.Context, instanceContext))
        {
            throw new ArgumentException(
                "A literal belongs to a different instance context.",
                parameterName);
        }
        _ = GetObjectIds(context, literal.Value.Arguments);
    }

    private static Literal<Fact>[] SnapshotGoal(
        EncodingContext context,
        InstanceContext instanceContext,
        IReadOnlyList<Literal<Fact>> goal,
        string parameterName)
    {
        Literal<Fact>[] literals = Snapshot(goal, parameterName);
        foreach (Literal<Fact> literal in literals)
        {
            if (literal.IsNegative)
            {
                throw new ArgumentException(
                    "Only positive goal literals can be encoded.",
                    parameterName);
            }
            ValidateLiteral(
                context,
                instanceContext,
                literal,
                parameterName);
        }
        return literals;
    }

    private static T[] Snapshot<T>(
        IReadOnlyList<T> values,
        string parameterName)
        where T : class
    {
        ArgumentNullException.ThrowIfNull(values, parameterName);
        int count = values.Count;
        T[] result = values.ToArray();
        if (result.Length != count)
        {
            throw new InvalidOperationException(
                $"{parameterName} changed while it was being encoded.");
        }
        if (result.Any(value => value is null))
        {
            throw new ArgumentException(
                $"{parameterName} cannot contain null values.",
                parameterName);
        }
        return result;
    }

    private static int[] GetObjectIds(
        EncodingContext context,
        IReadOnlyList<Constant> objects)
    {
        if (objects.Count == 0)
            return Array.Empty<int>();

        var result = new int[objects.Count];
        for (int index = 0; index < objects.Count; index++)
            result[index] = context.GetObjectId(objects[index]);
        return result;
    }

    private static void ValidateState(
        EncodingContext context,
        ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(context);
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(context.Problem, state.State.Context.Problem))
        {
            throw new ArgumentException(
                "State belongs to a different problem than the active encoding instance.",
                nameof(state));
        }
    }

    private static void ValidateSuffix(string suffix)
        => ArgumentNullException.ThrowIfNull(suffix);
}
