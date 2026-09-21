namespace Mimir.Core.Engines;

using Mimir.Core.Schemas;
using Mimir.Core.Grounding;

public class GroundedApplicableActionGenerator : IApplicableActionGenerator
{
    private const int UnexpandedNode = -1;

    private readonly SuccessorGeneratorNode _root = new();
    private readonly PriorityQueue<TraversalItem, int> _orderedFrontier = new();
    private readonly Stack<SuccessorGeneratorNode> _satisfiedNodes = new();
    private readonly uint[] _emittedTransitionGroups;

    public Problem Problem { get; }
    public State GroundingStartState { get; }
    internal IReadOnlyList<Action> GroundActions { get; }

    private readonly record struct FluentPreconditionLiteral(FluentIndex FluentIndex, bool IsPositive);

    private readonly record struct IndexedGroundAction(
        Action Action,
        int GroundingIndex,
        int TransitionEquivalenceGroupIndex);

    private sealed class SuccessorGeneratorNode
    {
        private Dictionary<FluentPreconditionLiteral, SuccessorGeneratorNode>? _children = new();

        public int EarliestGroundingIndex { get; set; } = int.MaxValue;
        public List<IndexedGroundAction> GroundActionsAtNode { get; } = new();
        public Branch[] Branches { get; private set; } = Array.Empty<Branch>();

        public SuccessorGeneratorNode GetOrAddChild(FluentPreconditionLiteral precondition)
        {
            if (_children is null)
                throw new InvalidOperationException();

            if (_children.TryGetValue(precondition, out SuccessorGeneratorNode? child))
                return child;

            child = new SuccessorGeneratorNode();
            _children.Add(precondition, child);
            return child;
        }

        public void Freeze()
        {
            if (_children is null)
                throw new InvalidOperationException();

            Branches = new Branch[_children.Count];
            int index = 0;
            foreach ((FluentPreconditionLiteral precondition, SuccessorGeneratorNode child) in _children)
            {
                child.Freeze();
                Branches[index++] = new Branch(precondition, child);
            }

            _children = null;
        }
    }

    private readonly record struct Branch(
        FluentPreconditionLiteral Precondition,
        SuccessorGeneratorNode Node);

    private readonly record struct TraversalItem(SuccessorGeneratorNode Node, int NextActionIndex);

    internal GroundedApplicableActionGenerator(Problem problem, State startState, IGrounder grounder)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(startState);
        ArgumentNullException.ThrowIfNull(grounder);
        if (!ReferenceEquals(startState.Context, problem.Context))
            throw new ArgumentException("Start state belongs to a different problem.", nameof(startState));

        Problem = problem;
        GroundingStartState = startState;
        IEnumerable<Action> groundedActions = grounder.Ground(problem, startState)
            ?? throw new InvalidOperationException("Grounder returned a null action sequence.");
        var validatedActions = groundedActions.TryGetNonEnumeratedCount(out int actionCount)
            ? new List<Action>(actionCount)
            : new List<Action>();
        foreach (Action? action in groundedActions)
        {
            if (action is null)
                throw new InvalidOperationException("Grounder returned a null action.");
            if (!ReferenceEquals(action.Context, problem.Context))
                throw new InvalidOperationException("Grounder returned an action from a different problem context.");

            validatedActions.Add(action);
        }

        GroundActions = validatedActions.AsReadOnly();

        int[] transitionEquivalenceGroupIndices = AssignTransitionRepresentativeGroups(
            GroundActions,
            problem.Domain.Actions,
            problem.Context,
            out int transitionEquivalenceGroupCount);
        _emittedTransitionGroups = new uint[(transitionEquivalenceGroupCount + 31) / 32];
        for (int index = 0; index < GroundActions.Count; index++)
        {
            Action action = GroundActions[index];
            if (!BitboardOps.StaticPreconditionHolds(
                    action.PositiveStaticPreconditions,
                    action.NegativeStaticPreconditions,
                    problem.Context.StaticBitboardWords))
            {
                continue;
            }
            if (!action.AreStaticDerivedPreconditionsSatisfied())
                continue;

            IndexGroundAction(new IndexedGroundAction(
                action,
                index,
                transitionEquivalenceGroupIndices[index]));
        }

        _root.Freeze();
    }

    internal void CollectApplicableActions(
        ExtendedState state,
        List<Action> destination)
    {
        ValidateState(state);
        ArgumentNullException.ThrowIfNull(destination);

        destination.Clear();
        if (_root.EarliestGroundingIndex == int.MaxValue)
            return;

        CollectApplicableActions(state, int.MaxValue, destination);
    }

    private void CollectApplicableActions(
        ExtendedState state,
        int maxActions,
        List<Action> actions)
    {
        _orderedFrontier.Clear();
        _satisfiedNodes.Clear();
        Array.Clear(_emittedTransitionGroups);

        try
        {
            // Full traversal batches satisfied nodes for throughput; bounded traversal keeps
            // nodes in grounding order so an exact prefix does not expand unnecessary branches.
            if (maxActions == int.MaxValue)
            {
                _satisfiedNodes.Push(_root);
                while (_satisfiedNodes.TryPop(out SuccessorGeneratorNode? node))
                {
                    if (node.GroundActionsAtNode.Count > 0)
                    {
                        IndexedGroundAction first = node.GroundActionsAtNode[0];
                        _orderedFrontier.Enqueue(
                            new TraversalItem(node, NextActionIndex: 0),
                            first.GroundingIndex);
                    }

                    foreach (Branch branch in node.Branches)
                    {
                        if (IsLiteralSatisfied(state.State, branch.Precondition))
                            _satisfiedNodes.Push(branch.Node);
                    }
                }
            }
            else
            {
                _orderedFrontier.Enqueue(
                    new TraversalItem(_root, NextActionIndex: UnexpandedNode),
                    _root.EarliestGroundingIndex);
            }

            while (actions.Count < maxActions
                && _orderedFrontier.TryDequeue(out TraversalItem item, out _))
            {
                SuccessorGeneratorNode node = item.Node;
                if (item.NextActionIndex == UnexpandedNode)
                {
                    if (node.GroundActionsAtNode.Count > 0)
                    {
                        IndexedGroundAction first = node.GroundActionsAtNode[0];
                        _orderedFrontier.Enqueue(
                            new TraversalItem(node, NextActionIndex: 0),
                            first.GroundingIndex);
                    }

                    foreach (Branch branch in node.Branches)
                    {
                        if (IsLiteralSatisfied(state.State, branch.Precondition))
                        {
                            _orderedFrontier.Enqueue(
                                new TraversalItem(branch.Node, NextActionIndex: UnexpandedNode),
                                branch.Node.EarliestGroundingIndex);
                        }
                    }

                    continue;
                }

                IndexedGroundAction candidate = node.GroundActionsAtNode[item.NextActionIndex];
                int nextActionIndex = item.NextActionIndex + 1;
                if (nextActionIndex < node.GroundActionsAtNode.Count)
                {
                    IndexedGroundAction next = node.GroundActionsAtNode[nextActionIndex];
                    _orderedFrontier.Enqueue(
                        new TraversalItem(node, nextActionIndex),
                        next.GroundingIndex);
                }

                int groupIndex = candidate.TransitionEquivalenceGroupIndex;
                int wordIndex = 0;
                uint groupMask = 0;
                if (groupIndex >= 0)
                {
                    wordIndex = groupIndex >> 5;
                    groupMask = 1u << (groupIndex & 31);
                    if ((_emittedTransitionGroups[wordIndex] & groupMask) != 0)
                        continue;
                }

                if (!candidate.Action.AreStateDependentDerivedPreconditionsSatisfied(state))
                    continue;

                if (groupIndex >= 0)
                    _emittedTransitionGroups[wordIndex] |= groupMask;

                actions.Add(candidate.Action);
            }
        }
        finally
        {
            _orderedFrontier.Clear();
            _satisfiedNodes.Clear();
            Array.Clear(_emittedTransitionGroups);
        }
    }

    private static int[] AssignTransitionRepresentativeGroups(
        IReadOnlyList<Action> actions,
        IReadOnlyList<ActionSchema> schemas,
        InstanceContext context,
        out int transitionEquivalenceGroupCount)
    {
        var schemasWithExistentialWitnesses = new Dictionary<ActionSchema, bool>(
            schemas.Count,
            ReferenceEqualityComparer.Instance);
        foreach (ActionSchema schema in schemas)
        {
            bool[] parameters = context
                .GetActionGroundingPlan(schema)
                .TransitionRelevantParameters;
            schemasWithExistentialWitnesses.Add(
                schema,
                parameters.Any(parameter => !parameter));
        }

        // Shared ActionEffects instances identify ground actions with the same transition.
        var transitionEquivalenceGroups = new Dictionary<ActionEffects, int>(
            ReferenceEqualityComparer.Instance);
        var transitionEquivalenceGroupCounts = new List<int>();
        var actionTransitionEquivalenceGroups = new int[actions.Count];
        Array.Fill(actionTransitionEquivalenceGroups, -1);

        for (int actionIndex = 0; actionIndex < actions.Count; actionIndex++)
        {
            Action action = actions[actionIndex];
            if (!schemasWithExistentialWitnesses.TryGetValue(
                    action.Schema,
                    out bool hasExistentialWitnessParameters))
            {
                throw new InvalidOperationException(
                    $"Grounder returned action '{action.Schema.Name}' whose schema does not belong to the problem domain.");
            }

            if (!hasExistentialWitnessParameters)
                continue;

            ActionEffects key = action.EffectsComponent;
            if (!transitionEquivalenceGroups.TryGetValue(key, out int groupIndex))
            {
                groupIndex = transitionEquivalenceGroups.Count;
                transitionEquivalenceGroups.Add(key, groupIndex);
                transitionEquivalenceGroupCounts.Add(0);
            }

            actionTransitionEquivalenceGroups[actionIndex] = groupIndex;
            transitionEquivalenceGroupCounts[groupIndex]++;
        }

        var representativeGroupIndices = new int[transitionEquivalenceGroupCounts.Count];
        Array.Fill(representativeGroupIndices, -1);
        transitionEquivalenceGroupCount = 0;
        for (int groupIndex = 0; groupIndex < transitionEquivalenceGroupCounts.Count; groupIndex++)
        {
            if (transitionEquivalenceGroupCounts[groupIndex] > 1)
                representativeGroupIndices[groupIndex] = transitionEquivalenceGroupCount++;
        }

        for (int actionIndex = 0; actionIndex < actions.Count; actionIndex++)
        {
            int transitionGroupIndex = actionTransitionEquivalenceGroups[actionIndex];
            actionTransitionEquivalenceGroups[actionIndex] = transitionGroupIndex >= 0
                ? representativeGroupIndices[transitionGroupIndex]
                : -1;
        }

        return actionTransitionEquivalenceGroups;
    }

    public IEnumerable<Action> GetApplicableActions(ExtendedState state)
        => GetApplicableActions(state, int.MaxValue);

    public IEnumerable<Action> GetApplicableActions(ExtendedState state, int maxActions)
    {
        ValidateState(state);
        if (maxActions < 0)
            throw new ArgumentOutOfRangeException(nameof(maxActions));
        if (maxActions == 0 || _root.EarliestGroundingIndex == int.MaxValue)
            return Array.Empty<Action>();

        List<Action> actions = maxActions == int.MaxValue
            ? new List<Action>()
            : new List<Action>(Math.Min(maxActions, GroundActions.Count));
        CollectApplicableActions(state, maxActions, actions);
        return actions;
    }

    private void IndexGroundAction(IndexedGroundAction indexedAction)
    {
        List<FluentPreconditionLiteral> preconditions = ExtractFluentPreconditions(
            indexedAction.Action);
        SuccessorGeneratorNode current = _root;
        current.EarliestGroundingIndex = Math.Min(
            current.EarliestGroundingIndex,
            indexedAction.GroundingIndex);
        foreach (FluentPreconditionLiteral precondition in preconditions)
        {
            current = current.GetOrAddChild(precondition);
            current.EarliestGroundingIndex = Math.Min(
                current.EarliestGroundingIndex,
                indexedAction.GroundingIndex);
        }

        current.GroundActionsAtNode.Add(indexedAction);
    }

    private static List<FluentPreconditionLiteral> ExtractFluentPreconditions(Action action)
    {
        var preconditions = new List<FluentPreconditionLiteral>();
        foreach (int localFluentIndex in BitboardOps.EnumerateSetBits(
            action.PositiveFluentPreconditions))
        {
            preconditions.Add(new FluentPreconditionLiteral(
                new FluentIndex(localFluentIndex),
                IsPositive: true));
        }

        foreach (int localFluentIndex in BitboardOps.EnumerateSetBits(
            action.NegativeFluentPreconditions))
        {
            preconditions.Add(new FluentPreconditionLiteral(
                new FluentIndex(localFluentIndex),
                IsPositive: false));
        }

        preconditions.Sort(static (left, right) =>
        {
            int factComparison = left.FluentIndex.Value.CompareTo(right.FluentIndex.Value);
            if (factComparison != 0)
                return factComparison;
            if (left.IsPositive == right.IsPositive)
                return 0;

            return left.IsPositive ? -1 : 1;
        });
        return preconditions;
    }

    private static bool IsLiteralSatisfied(State state, FluentPreconditionLiteral literal)
    {
        bool isTrue = state.IsTrueUnchecked(literal.FluentIndex);
        return literal.IsPositive ? isTrue : !isTrue;
    }

    private void ValidateState(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.State.Context, Problem.Context))
            throw new InvalidOperationException("State and applicable-action generator belong to different problem contexts.");
    }
}
