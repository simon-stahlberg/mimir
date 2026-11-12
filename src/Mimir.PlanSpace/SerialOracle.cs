using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.PlanSpace;

public sealed class SerialOracleTargets
{
    public IReadOnlyList<int> RepairIndexPerError { get; }
    public IReadOnlyList<int> StepIdsToDelete { get; }
    public int MatchedStepCount { get; }

    internal SerialOracleTargets(IReadOnlyList<int> repairIndexPerError, IReadOnlyList<int> stepIdsToDelete, int matchedStepCount)
    {
        RepairIndexPerError = repairIndexPerError;
        StepIdsToDelete = stepIdsToDelete;
        MatchedStepCount = matchedStepCount;
    }
}

// The reference is one witness among the plans that differ from it only in how symmetric objects are chosen,
// so a step stands for a reference step by the role it plays and is never corrected for its binding alone.
public static class SerialOracle
{
    public static SerialOracleTargets Targets(SerialPlan current, SerialDiagnosis diagnosis, SerialPlan reference)
    {
        ArgumentNullException.ThrowIfNull(current);
        ArgumentNullException.ThrowIfNull(diagnosis);
        ArgumentNullException.ThrowIfNull(reference);
        if (!ReferenceEquals(diagnosis.Plan, current) || diagnosis.Version != current.Version)
            throw new ArgumentException("The diagnosis was not computed from the current plan in its current state.", nameof(diagnosis));
        if (!ReferenceEquals(current.Problem, reference.Problem))
            throw new ArgumentException("The reference plan belongs to a different problem.", nameof(reference));
        if (!reference.IsSolution())
            throw new ArgumentException("The reference plan is not a solution.", nameof(reference));

        var matching = new Matching(current, reference);
        var targets = new int[diagnosis.Errors.Count];
        var requests = new List<Request>();
        var startingOver = new HashSet<int>();
        for (int i = 0; i < targets.Length; i++)
        {
            SerialError error = diagnosis.Errors[i];
            if (error.StepId != -1 && !matching.ReferenceByCurrent.ContainsKey(error.StepId)) continue;

            if (error.Kind == SerialErrorKind.UnboundParameter)
            {
                targets[i] = BindTarget(error, matching.ReferenceByCurrent[error.StepId]);
                continue;
            }

            int slot = matching.Slot(current, error.StepId, error.Literal!);
            Literal<Fact> referenceLiteral = matching.ReferenceLiteral(error.StepId, slot, error.Literal!);
            ReferenceChild? missingChild = matching.MissingChildren[error.StepId].FirstOrDefault(child => child.Slot == slot);
            if (missingChild is not null)
            {
                requests.Add(new Request(i, missingChild.Step, referenceLiteral));
                continue;
            }

            // The reference inserts nothing for this literal, so there it holds because of other steps or from
            // the start. Here it does so too once those steps are in place, unless the flagged step or the step
            // that makes the literal true in the reference is bound to objects for which it never will.
            if (error.Kind == SerialErrorKind.UnsatisfiedPrecondition) targets[i] = RebindTarget(error, current, matching.ReferenceByCurrent[error.StepId]);
            if (targets[i] != 0) continue;

            if (!referenceLiteral.Equals(error.Literal) && HoldsBefore(current, error.StepId, referenceLiteral))
            {
                startingOver.Add(matching.StepToStartOver(error.StepId));
                continue;
            }

            SerialStep? referenceProducer = matching.ReferenceProducer(error.StepId, referenceLiteral);
            if (referenceProducer is null) continue;

            if (matching.CurrentByReference.TryGetValue(referenceProducer.Id, out SerialStep? producer))
            {
                // A producer with errors of its own is still being repaired.
                bool isSettled = diagnosis.Errors.All(other => other.StepId != producer.Id);
                if (isSettled && !Produces(producer, referenceLiteral, current.Problem.Domain)) startingOver.Add(matching.StepToStartOver(producer.Id));
                continue;
            }

            // A step with a role is asked for through it. The others are asked for where the reference depends
            // on them, by the first error that does.
            bool isRequested = requests.Any(request => request.ReferenceStep == referenceProducer);
            if (!matching.HasRole(referenceProducer.Id) && !isRequested) requests.Add(new Request(i, referenceProducer, referenceLiteral));
        }

        foreach (IGrouping<int, Request> stepRequests in requests.GroupBy(request => diagnosis.Errors[request.ErrorIndex].StepId))
            GrantInOrder(stepRequests, diagnosis, matching, reference, targets);

        // A step goes together with the steps inserted for it, which come before it.
        var deletions = new List<int>();
        foreach (SerialStep step in current.Steps.Reverse())
        {
            bool isMatched = matching.ReferenceByCurrent.ContainsKey(step.Id);
            if (!isMatched || startingOver.Contains(step.Id) || deletions.Contains(step.PurposeStepId)) deletions.Add(step.Id);
        }

        deletions.Reverse();
        for (int i = 0; i < targets.Length; i++)
        {
            if (deletions.Contains(diagnosis.Errors[i].StepId)) targets[i] = 0;
        }

        return new SerialOracleTargets(targets, deletions, matching.ReferenceByCurrent.Count);
    }

    // An error that asks for a step standing for the reference step.
    private sealed record Request(int ErrorIndex, SerialStep ReferenceStep, Literal<Fact> ReferenceLiteral);

    // The steps inserted for one flagged step land right before it in the order of its errors, after the steps
    // inserted for it earlier. So they keep the order of the reference only when they are asked for in that
    // order, the others waiting for their turn in a later round.
    private static void GrantInOrder(IGrouping<int, Request> stepRequests, SerialDiagnosis diagnosis, Matching matching, SerialPlan reference, int[] targets)
    {
        // A step with an unbound parameter cannot be judged on every precondition yet, and those that are
        // judged later may ask for earlier steps of the reference.
        if (diagnosis.Errors.Any(error => error.StepId == stepRequests.Key && error.Kind == SerialErrorKind.UnboundParameter)) return;

        List<SerialStep> referenceOrder = reference.Steps.ToList();
        Queue<ReferenceChild> missing = matching.MissingChildren[stepRequests.Key];
        int lastErrorIndex = -1;
        foreach (Request request in stepRequests.OrderBy(request => referenceOrder.IndexOf(request.ReferenceStep)))
        {
            bool hasRole = matching.HasRole(request.ReferenceStep.Id);
            if (hasRole && missing.Peek().Step != request.ReferenceStep) continue;
            if (request.ErrorIndex < lastErrorIndex) return;

            int target = InsertTarget(diagnosis.Errors[request.ErrorIndex], diagnosis, request.ReferenceStep, request.ReferenceLiteral);
            if (target == 0) continue;

            targets[request.ErrorIndex] = target;
            lastErrorIndex = request.ErrorIndex;
            if (hasRole) missing.Dequeue();
        }
    }

    // The object of the reference is not offered only when another parameter of the step is bound differently
    // from the reference, and then any object that is offered completes the step.
    private static int BindTarget(SerialError error, SerialStep referenceStep)
    {
        int target = IndexOf(error, repair => ReferenceEquals(repair.Object, referenceStep.Binding[error.ParameterIndex]));
        if (target == 0 && error.Repairs.Count > 1) return 1;
        return target;
    }

    private static int RebindTarget(SerialError error, SerialPlan current, SerialStep referenceStep)
    {
        SerialStep step = current.Steps.First(step => step.Id == error.StepId);
        return IndexOf(error, repair => repair.Kind == SerialRepairKind.Rebind
            && ReferenceEquals(repair.Object, referenceStep.Binding[repair.ParameterIndex])
            && Produces(step.WithBinding(repair.ParameterIndex, repair.Object!, current.Problem), step.Purpose, current.Problem.Domain));
    }

    private static int InsertTarget(SerialError error, SerialDiagnosis diagnosis, SerialStep referenceStep, Literal<Fact> referenceLiteral)
    {
        int target = 0;
        int targetBoundCount = -1;
        for (int i = 1; i < error.Repairs.Count; i++)
        {
            if (error.Repairs[i].Kind != SerialRepairKind.Insert) continue;

            GhostStep candidate = diagnosis.Candidates[error.Repairs[i].CandidateIndex];
            if (!IsConsistent(candidate, referenceStep, error.Literal!.Value, referenceLiteral.Value)) continue;

            int boundCount = candidate.Binding.Count(constant => constant is not null);
            if (boundCount <= targetBoundCount) continue;
            target = i;
            targetBoundCount = boundCount;
        }

        return target;
    }

    // The candidate is bound to the arguments of the literal it is inserted for, so it may differ from the
    // reference step exactly where that literal differs from the one the reference step was inserted for.
    private static bool IsConsistent(GhostStep candidate, SerialStep referenceStep, Fact literal, Fact referenceLiteral)
    {
        if (!ReferenceEquals(candidate.Schema, referenceStep.Schema)) return false;
        for (int i = 0; i < candidate.Binding.Count; i++)
        {
            Constant? value = candidate.Binding[i];
            Constant referenceValue = referenceStep.Binding[i]!;
            if (value is null || ReferenceEquals(value, referenceValue)) continue;

            bool followsLiteral = Enumerable.Range(0, literal.Arguments.Count).Any(position =>
                ReferenceEquals(literal.Arguments[position], value) && ReferenceEquals(referenceLiteral.Arguments[position], referenceValue));
            if (!followsLiteral) return false;
        }

        return true;
    }

    private static bool Produces(SerialStep step, Literal<Fact> literal, Domain domain)
    {
        var fact = (Fact<Fluent>)literal.Value;
        return step.Effects.Any(effect => effect.IsPositive == literal.IsPositive && effect.Fact.Equals(fact))
            || step.OpenEffects.Any(effect => effect.IsPositive == literal.IsPositive && effect.Matches(fact, domain));
    }

    private static bool HoldsBefore(SerialPlan plan, int stepId, Literal<Fact> literal)
    {
        var state = new SerialState(plan.Problem);
        foreach (SerialStep step in plan.Steps.TakeWhile(step => step.Id != stepId)) state.Apply(step);
        return state.Holds((Fact<Fluent>)literal.Value, literal.IsPositive) == Truth.True;
    }

    // Null while an argument of the precondition is unbound.
    private static Literal<Fact>? Precondition(SerialStep step, int slot, Problem problem)
    {
        Literal<Atom<Fluent>> precondition = step.Schema.FluentPreconditions[slot];
        Constant?[] arguments = SchemaTerms.Resolve(step.Schema, precondition.Value.Arguments, step.Binding);
        if (arguments.Any(argument => argument is null)) return null;

        Fact<Fluent> fact = problem.Context.RegisterFact(precondition.Value.Predicate, arguments.Select(argument => argument!).ToArray());
        return new Literal<Fact>(fact, precondition.Polarity);
    }

    private static int IndexOf(SerialError error, Func<SerialRepair, bool> predicate)
    {
        for (int i = 1; i < error.Repairs.Count; i++)
        {
            if (predicate(error.Repairs[i])) return i;
        }

        return 0;
    }

    // Slot is the role of the step: the precondition of its parent, or the goal literal, that it serves.
    private sealed record ReferenceChild(SerialStep Step, int ParentStepId, int Slot);

    // A step stands for the reference step that serves the same precondition of the same, already matched, step
    // (or the same goal literal), so the matching follows the roles down from the goal. The steps serving one
    // step are matched in the order of the reference, because that order cannot be repaired by later inserts.
    private sealed class Matching
    {
        private readonly SerialPlan _current;
        private readonly SerialPlan _reference;
        private readonly List<ReferenceChild> _referenceChildren = new();

        public Dictionary<int, SerialStep> ReferenceByCurrent { get; } = new();
        public Dictionary<int, SerialStep> CurrentByReference { get; } = new();
        public Dictionary<int, Queue<ReferenceChild>> MissingChildren { get; } = new();

        // A step of a hindsight reference can have a purpose the reference no longer has: its parent was pruned
        // or rebound, its goal literal given up, or the step itself rebound. Such a step, and the steps inserted
        // for it, have no role.
        public Matching(SerialPlan current, SerialPlan reference)
        {
            _current = current;
            _reference = reference;
            // Steps are inserted before their parent, so going backwards the parent of a step is seen first.
            foreach (SerialStep step in reference.Steps.Reverse())
            {
                int slot = Slot(reference, step.PurposeStepId, step.Purpose);
                bool hasParent = step.PurposeStepId == -1 || HasRole(step.PurposeStepId);
                if (slot >= 0 && hasParent && Produces(step, step.Purpose, reference.Problem.Domain))
                    _referenceChildren.Add(new ReferenceChild(step, step.PurposeStepId, slot));
            }

            _referenceChildren.Reverse();
            MatchChildren(-1);
        }

        public bool HasRole(int referenceStepId) => _referenceChildren.Any(child => child.Step.Id == referenceStepId);

        // The index of the precondition of the parent that names the literal under the binding of the parent,
        // or of the literal in the goal of the reference; -1 when the literal is neither.
        public int Slot(SerialPlan plan, int parentStepId, Literal<Fact> literal)
        {
            if (parentStepId == -1) return _reference.Goal.ToList().IndexOf(literal);

            SerialStep? parent = plan.Steps.FirstOrDefault(step => step.Id == parentStepId);
            if (parent is null) return -1;
            for (int slot = 0; slot < parent.Schema.FluentPreconditions.Count; slot++)
            {
                if (literal.Equals(Precondition(parent, slot, plan.Problem))) return slot;
            }

            return -1;
        }

        // The literal the reference has where the matched parent has the given one at the slot.
        public Literal<Fact> ReferenceLiteral(int currentParent, int slot, Literal<Fact> literal)
            => currentParent == -1 ? literal : Precondition(ReferenceByCurrent[currentParent], slot, _reference.Problem)!;

        // The reference step that makes the literal true for the step the consumer stands for, null when the
        // literal holds from the start.
        public SerialStep? ReferenceProducer(int currentConsumer, Literal<Fact> referenceLiteral)
        {
            int referenceConsumer = currentConsumer == -1 ? -1 : ReferenceByCurrent[currentConsumer].Id;
            SerialSupport? support = _reference.Diagnose().Supports.FirstOrDefault(candidate =>
                candidate.ConsumerStepId == referenceConsumer && candidate.Literal.Equals(referenceLiteral));
            return support is null ? null : _reference.Steps.FirstOrDefault(step => step.Id == support.ProducerStepId);
        }

        // A step that was inserted for a literal its reference step does not produce differs from it because
        // its parent does, and would be bound the same way again. So the step to delete for a binding that
        // does not work is the nearest one, going up from the given step, that can be bound as in the reference.
        public int StepToStartOver(int currentStepId)
        {
            SerialStep step = _current.Steps.First(candidate => candidate.Id == currentStepId);
            while (step.PurposeStepId != -1 && !Produces(ReferenceByCurrent[step.Id], step.Purpose, _current.Problem.Domain))
                step = _current.Steps.First(parent => parent.Id == step.PurposeStepId);
            return step.Id;
        }

        private void MatchChildren(int currentParent)
        {
            int referenceParent = currentParent == -1 ? -1 : ReferenceByCurrent[currentParent].Id;
            var missing = new Queue<ReferenceChild>(_referenceChildren.Where(child => child.ParentStepId == referenceParent));
            MissingChildren.Add(currentParent, missing);
            foreach (SerialStep step in _current.Steps)
            {
                if (step.PurposeStepId != currentParent) continue;

                SerialStep? referenceStep = ReferenceStep(step, missing);
                if (referenceStep is null) continue;

                ReferenceByCurrent.Add(step.Id, referenceStep);
                CurrentByReference.Add(referenceStep.Id, step);
                MatchChildren(step.Id);
            }
        }

        private SerialStep? ReferenceStep(SerialStep step, Queue<ReferenceChild> missing)
        {
            // A rebind can leave a step unable to produce the literal it was inserted for.
            int slot = Slot(_current, step.PurposeStepId, step.Purpose);
            if (slot < 0 || !Produces(step, step.Purpose, _current.Problem.Domain)) return null;

            if (missing.Count > 0 && missing.Peek().Slot == slot && ReferenceEquals(missing.Peek().Step.Schema, step.Schema))
                return missing.Dequeue().Step;

            // A step without a role is matched where the reference depends on it, which is where it is asked for.
            SerialStep? producer = ReferenceProducer(step.PurposeStepId, ReferenceLiteral(step.PurposeStepId, slot, step.Purpose));
            if (producer is null || HasRole(producer.Id) || CurrentByReference.ContainsKey(producer.Id)) return null;
            return ReferenceEquals(producer.Schema, step.Schema) ? producer : null;
        }
    }
}
