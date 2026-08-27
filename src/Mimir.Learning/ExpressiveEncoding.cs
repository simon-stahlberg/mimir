using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Learning;

public static partial class Encoding
{
    public static void EncodeExpressiveState(
        EncodingContext context,
        ExtendedState state,
        string suffix = "")
    {
        ValidateState(context, state);
        ValidateSuffix(suffix);
        ExpressiveFact[] facts = GetExpressiveStateFacts(context, state);
        var missingPairs = new List<(Constant First, Constant Second)>();
        var pendingPairIds = new HashSet<(int First, int Second)>();
        foreach (ExpressiveFact fact in facts)
        {
            QueueMissingFactPairs(
                context,
                fact.Arguments,
                missingPairs,
                pendingPairIds);
        }

        IReadOnlyList<Constant> objects = context.Problem.AllObjects;
        foreach (Constant first in objects)
        {
            foreach (Constant second in objects)
            {
                foreach (Constant third in objects)
                {
                    QueueMissingPair(
                        context,
                        first,
                        second,
                        missingPairs,
                        pendingPairIds);
                    QueueMissingPair(
                        context,
                        second,
                        third,
                        missingPairs,
                        pendingPairIds);
                    QueueMissingPair(
                        context,
                        first,
                        third,
                        missingPairs,
                        pendingPairIds);
                }
            }
        }
        AllocateMissingPairs(context, missingPairs);

        foreach (EncodingContext.TypeRelationGroup group
            in context.CurrentProblemData.TypeRelations)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"expressive_type_relation_{group.TypeName}{suffix}");
            foreach (int localId in group.LocalObjectIds)
                ids.Add(context.ObjectIdValues[localId]);
        }

        if (context.Problem.Domain.UsesEquality)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"expressive_relation_={suffix}");
            foreach (int objectId in context.ObjectIdValues)
            {
                ids.Add(objectId);
                ids.Add(objectId);
                ids.Add(objectId);
                ids.Add(objectId);
            }
        }

        foreach (ExpressiveFact fact in facts)
        {
            List<int> ids = context.GetOrCreateRelation(
                $"expressive_relation_{fact.Predicate.Name}{suffix}");
            AppendFactPairs(context, ids, fact.Arguments);
        }

        List<int> composition = context.GetOrCreateRelation(
            "expressive_composition");
        foreach (Constant first in objects)
        {
            foreach (Constant second in objects)
            {
                foreach (Constant third in objects)
                {
                    composition.Add(context.NewOrExistingObjectPairId(first, second));
                    composition.Add(context.NewOrExistingObjectPairId(second, third));
                    composition.Add(context.NewOrExistingObjectPairId(first, third));
                }
            }
        }
    }

    public static void EncodeExpressiveGoal(
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
        var missingPairs = new List<(Constant First, Constant Second)>();
        var pendingPairIds = new HashSet<(int First, int Second)>();
        foreach (Literal<Fact> literal in literals)
        {
            QueueMissingFactPairs(
                context,
                literal.Value.Arguments,
                missingPairs,
                pendingPairIds);
        }
        AllocateMissingPairs(context, missingPairs);

        foreach (Literal<Fact> literal in literals)
        {
            Fact fact = literal.Value;
            bool isTrue = state.IsTrue(fact);
            List<int> ids = context.GetOrCreateRelation(
                $"expressive_relation_{fact.Predicate.Name}{suffix}_goal_{(isTrue ? "true" : "false")}");
            AppendFactPairs(context, ids, fact.Arguments);
        }
    }

    private sealed record ExpressiveFact(
        Predicate Predicate,
        Constant[] Arguments);

    private static ExpressiveFact[] GetExpressiveStateFacts(
        EncodingContext context,
        ExtendedState state)
    {
        var facts = new List<ExpressiveFact>();
        foreach (EncodingContext.EncodedFact fact
            in context.CurrentProblemData.StaticFacts)
        {
            Constant[] arguments = fact.LocalObjectIds
                .Select(localId => context.Problem.AllObjects[localId])
                .ToArray();
            facts.Add(new ExpressiveFact(fact.Predicate, arguments));
        }
        foreach (Fact<Fluent> fact in state.State.GetTrueFacts())
            AddExpressiveFact(context, facts, fact);
        foreach (Fact<Derived> fact in state.GetTrueDerivedFacts())
            AddExpressiveFact(context, facts, fact);
        return facts.ToArray();
    }

    private static void AddExpressiveFact(
        EncodingContext context,
        List<ExpressiveFact> facts,
        Fact fact)
    {
        if (ReferenceEquals(
            fact.Predicate,
            context.Problem.Domain.EqualityPredicate))
        {
            return;
        }

        Constant[] arguments = fact.Arguments.ToArray();
        foreach (Constant argument in arguments)
            _ = context.GetObjectId(argument);
        facts.Add(new ExpressiveFact(fact.Predicate, arguments));
    }

    private static void QueueMissingFactPairs(
        EncodingContext context,
        IReadOnlyList<Constant> arguments,
        List<(Constant First, Constant Second)> missingPairs,
        HashSet<(int First, int Second)> pendingPairIds)
    {
        foreach (Constant first in arguments)
        {
            foreach (Constant second in arguments)
            {
                QueueMissingPair(
                    context,
                    first,
                    second,
                    missingPairs,
                    pendingPairIds);
            }
        }
    }

    private static void QueueMissingPair(
        EncodingContext context,
        Constant first,
        Constant second,
        List<(Constant First, Constant Second)> missingPairs,
        HashSet<(int First, int Second)> pendingPairIds)
    {
        if (context.TryGetObjectPairId(first, second, out _))
            return;

        var key = (context.GetObjectId(first), context.GetObjectId(second));
        if (!pendingPairIds.Add(key))
            return;
        missingPairs.Add((first, second));
    }

    private static void AllocateMissingPairs(
        EncodingContext context,
        IReadOnlyList<(Constant First, Constant Second)> missingPairs)
    {
        context.EnsureCanAllocate(missingPairs.Count);
        foreach ((Constant first, Constant second) in missingPairs)
            _ = context.NewOrExistingObjectPairId(first, second);
    }

    private static void AppendFactPairs(
        EncodingContext context,
        List<int> ids,
        IReadOnlyList<Constant> arguments)
    {
        if (arguments.Count == 0)
        {
            ids.AddRange(context.ObjectIdValues);
            return;
        }
        foreach (Constant first in arguments)
        {
            foreach (Constant second in arguments)
                ids.Add(context.NewOrExistingObjectPairId(first, second));
        }
    }
}
