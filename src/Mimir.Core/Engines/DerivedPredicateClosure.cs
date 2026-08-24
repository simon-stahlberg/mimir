namespace Mimir.Core.Engines;

using Grounding;
using Schemas;

internal sealed partial class DerivedPredicateClosure
{
    private readonly InstanceContext _context;
    private readonly DerivedPredicatePlan _plan;
    private readonly Dictionary<Predicate<Derived>, CompiledRule> _rulesByPredicate =
        new(ReferenceEqualityComparer.Instance);
    private readonly Dictionary<DerivedPredicateComponent, CompiledRule[]> _rulesByComponent =
        new(ReferenceEqualityComparer.Instance);
    private readonly HashSet<DerivedEvaluationSlot> _staticTruth = new();
    private readonly bool _hasStateDependentComponents;

    internal DerivedPredicateClosure(
        InstanceContext context,
        DerivedPredicatePlan plan)
    {
        _context = context;
        _plan = plan;

        CompileRules();
        DerivedEvaluationWorkspace workspace =
            _context.RentDerivedEvaluationWorkspace();
        try
        {
            foreach (DerivedPredicateComponent component in plan.Components)
            {
                if (component.IsStateDependent)
                {
                    _hasStateDependentComponents = true;
                    continue;
                }

                EvaluateComponent(
                    state: null,
                    component,
                    evaluation: null,
                    workspace);
            }
        }
        finally
        {
            _context.ReturnDerivedEvaluationWorkspace(workspace);
        }
    }

    internal ExtendedState Expand(State state)
    {
        if (!_hasStateDependentComponents)
            return new ExtendedState(state, derivedEvaluation: null);

        var evaluation = new DerivedPredicateEvaluation(
            this,
            state,
            _context.DerivedEvaluationSlotCount);
        evaluation.CompleteRecursiveComponents();
        return new ExtendedState(state, evaluation);
    }

    internal bool IsStatic(Predicate<Derived> predicate)
        => !_plan.IsStateDependent(predicate);

    internal bool IsStaticTrue(
        Predicate<Derived> predicate,
        IReadOnlyList<Constant> arguments)
        => _context.TryGetDerivedEvaluationSlot(
                new DerivedFactKey(predicate, arguments),
                out DerivedEvaluationSlot slot)
            && _staticTruth.Contains(slot);

    internal bool IsStaticTrue(DerivedEvaluationSlot slot)
        => _staticTruth.Contains(slot);

    internal void CompleteRecursiveComponents(DerivedPredicateEvaluation evaluation)
    {
        DerivedEvaluationWorkspace workspace =
            _context.RentDerivedEvaluationWorkspace();
        try
        {
            foreach (DerivedPredicateComponent component in _plan.Components)
            {
                if (!component.IsStateDependent || !component.RequiresFixedPoint)
                    continue;

                EvaluateComponent(
                    evaluation.State,
                    component,
                    evaluation,
                    workspace);
            }
        }
        finally
        {
            _context.ReturnDerivedEvaluationWorkspace(workspace);
        }
    }

    internal bool IsStateDependentTrue(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedFactKey key)
    {
        DerivedPredicateComponent component = _plan.GetComponent(predicate);
        if (!component.IsStateDependent)
            throw new InvalidOperationException(predicate.Name);

        if (component.RequiresFixedPoint)
        {
            return _context.TryGetDerivedEvaluationSlot(key, out DerivedEvaluationSlot slot)
                && evaluation.IsRecursiveTrue(slot);
        }

        DerivedEvaluationSlot acyclicSlot =
            _context.GetOrAddDerivedEvaluationSlot(key);
        if (evaluation.TryGetAcyclicTruth(acyclicSlot, out bool isTrue))
            return isTrue;

        DerivedEvaluationWorkspace workspace =
            _context.RentDerivedEvaluationWorkspace();
        try
        {
            return EvaluateUncachedAcyclicFact(
                evaluation,
                predicate,
                acyclicSlot,
                key,
                workspace);
        }
        finally
        {
            _context.ReturnDerivedEvaluationWorkspace(workspace);
        }
    }

    internal bool IsStateDependentTrue(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedEvaluationSlot slot)
    {
        DerivedPredicateComponent component = _plan.GetComponent(predicate);
        if (!component.IsStateDependent)
            throw new InvalidOperationException(predicate.Name);

        if (component.RequiresFixedPoint)
            return evaluation.IsRecursiveTrue(slot);

        if (evaluation.TryGetAcyclicTruth(slot, out bool isTrue))
            return isTrue;

        DerivedEvaluationWorkspace workspace =
            _context.RentDerivedEvaluationWorkspace();
        try
        {
            return EvaluateUncachedAcyclicFact(
                evaluation,
                predicate,
                slot,
                _context.GetDerivedEvaluationKey(slot),
                workspace);
        }
        finally
        {
            _context.ReturnDerivedEvaluationWorkspace(workspace);
        }
    }

    private bool IsAcyclicTrue(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedFactKey key,
        DerivedEvaluationWorkspace workspace)
    {
        DerivedEvaluationSlot slot = _context.GetOrAddDerivedEvaluationSlot(key);
        return IsAcyclicTrue(evaluation, predicate, slot, key, workspace);
    }

    private bool IsAcyclicTrue(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedEvaluationSlot slot,
        DerivedFactKey key,
        DerivedEvaluationWorkspace workspace)
    {
        if (evaluation.TryGetAcyclicTruth(slot, out bool isTrue))
            return isTrue;

        return EvaluateUncachedAcyclicFact(
            evaluation,
            predicate,
            slot,
            key,
            workspace);
    }

    private bool EvaluateUncachedAcyclicFact(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedEvaluationSlot slot,
        DerivedFactKey key,
        DerivedEvaluationWorkspace workspace)
    {
        bool isTrue = EvaluateAcyclicFact(
            evaluation,
            predicate,
            key,
            workspace);
        evaluation.SetAcyclicTruth(slot, isTrue);
        return isTrue;
    }

    private bool EvaluateAcyclicFact(
        DerivedPredicateEvaluation evaluation,
        Predicate<Derived> predicate,
        DerivedFactKey key,
        DerivedEvaluationWorkspace workspace)
    {
        CompiledRule rule = _rulesByPredicate[predicate];
        DerivedEvaluationFrame frame = workspace.RentFrame(
            rule.SlotCount,
            headArgumentCount: 0);
        try
        {
            for (int i = 0; i < rule.HeadSlots.Length; i++)
                frame.Bindings[rule.HeadSlots[i]] = key.GetArgument(i);

            return Evaluate(
                evaluation.State,
                rule.Body,
                frame.Bindings,
                evaluation,
                workspace);
        }
        finally
        {
            workspace.ReturnFrame(frame);
        }
    }

    private void EvaluateComponent(
        State? state,
        DerivedPredicateComponent component,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        CompiledRule[] rules = _rulesByComponent[component];
        if (component.RequiresFixedPoint && rules.Length == 1)
        {
            CompiledRule rule = rules[0];
            bool changed;
            do
            {
                changed = EvaluateRule(
                    state,
                    rule,
                    evaluation,
                    workspace);
            }
            while (changed);
            return;
        }

        if (!component.RequiresFixedPoint)
        {
            for (int i = 0; i < rules.Length; i++)
                EvaluateRule(state, rules[i], evaluation, workspace);
            return;
        }

        var pending = new Queue<int>(rules.Length);
        var isPending = new bool[rules.Length];
        for (int i = 0; i < rules.Length; i++)
        {
            pending.Enqueue(i);
            isPending[i] = true;
        }

        while (pending.TryDequeue(out int ruleIndex))
        {
            isPending[ruleIndex] = false;
            if (!EvaluateRule(
                    state,
                    rules[ruleIndex],
                    evaluation,
                    workspace))
            {
                continue;
            }

            IReadOnlyList<int> dependents =
                component.PositiveDependentRuleIndicesByPredicate[ruleIndex];
            for (int i = 0; i < dependents.Count; i++)
            {
                int dependentIndex = dependents[i];
                if (isPending[dependentIndex])
                    continue;

                pending.Enqueue(dependentIndex);
                isPending[dependentIndex] = true;
            }
        }
    }

    private bool EvaluateRule(
        State? state,
        CompiledRule rule,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        DerivedEvaluationFrame frame = workspace.RentFrame(
            rule.SlotCount,
            rule.HeadSlots.Length);
        try
        {
            bool changed = false;
            EvaluateHeadBindings(
                state,
                rule,
                parameterIndex: 0,
                frame,
                evaluation,
                workspace,
                ref changed);
            return changed;
        }
        finally
        {
            workspace.ReturnFrame(frame);
        }
    }

    private void EvaluateHeadBindings(
        State? state,
        CompiledRule rule,
        int parameterIndex,
        DerivedEvaluationFrame frame,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace,
        ref bool changed)
    {
        Predicate<Derived> predicate = rule.Predicate;
        if (parameterIndex == predicate.Parameters.Count)
        {
            var key = new DerivedFactKey(
                predicate,
                frame.HeadArguments,
                rule.HeadSlots.Length);
            if (IsTargetTrue(key, evaluation))
                return;

            if (Evaluate(
                    state,
                    rule.Body,
                    frame.Bindings,
                    evaluation,
                    workspace))
            {
                changed |= AddTargetTruth(key, evaluation);
            }
            return;
        }

        Constant[] candidates = rule.HeadCandidates[parameterIndex];
        for (int i = 0; i < candidates.Length; i++)
        {
            Constant candidate = candidates[i];
            frame.HeadArguments[parameterIndex] = candidate;
            frame.Bindings[rule.HeadSlots[parameterIndex]] = candidate;
            EvaluateHeadBindings(
                state,
                rule,
                parameterIndex + 1,
                frame,
                evaluation,
                workspace,
                ref changed);
        }
    }

    private bool IsTargetTrue(
        DerivedFactKey key,
        DerivedPredicateEvaluation? evaluation)
    {
        if (!_context.TryGetDerivedEvaluationSlot(key, out DerivedEvaluationSlot slot))
            return false;

        return evaluation is null
            ? _staticTruth.Contains(slot)
            : evaluation.IsRecursiveTrue(slot);
    }

    private bool AddTargetTruth(
        DerivedFactKey key,
        DerivedPredicateEvaluation? evaluation)
    {
        DerivedEvaluationSlot slot = _context.GetOrAddDerivedEvaluationSlot(key);
        return evaluation is null
            ? _staticTruth.Add(slot)
            : evaluation.AddRecursiveTruth(slot);
    }

    private bool Evaluate(
        State? state,
        CompiledExpression expression,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        switch (expression)
        {
            case CompiledTrueExpression:
                return true;
            case CompiledAtomExpression atom:
                return EvaluateAtom(
                    state,
                    atom,
                    bindings,
                    evaluation,
                    workspace);
            case CompiledNotExpression not:
                return !Evaluate(
                    state,
                    not.Expression,
                    bindings,
                    evaluation,
                    workspace);
            case CompiledAndExpression and:
                for (int i = 0; i < and.Expressions.Length; i++)
                {
                    if (!Evaluate(
                            state,
                            and.Expressions[i],
                            bindings,
                            evaluation,
                            workspace))
                    {
                        return false;
                    }
                }
                return true;
            case CompiledOrExpression or:
                for (int i = 0; i < or.Expressions.Length; i++)
                {
                    if (Evaluate(
                            state,
                            or.Expressions[i],
                            bindings,
                            evaluation,
                            workspace))
                    {
                        return true;
                    }
                }
                return false;
            case CompiledImplyExpression imply:
                return !Evaluate(
                        state,
                        imply.Antecedent,
                        bindings,
                        evaluation,
                        workspace)
                    || Evaluate(
                        state,
                        imply.Consequent,
                        bindings,
                        evaluation,
                        workspace);
            case CompiledExistentialSchedule schedule:
                return EvaluateExistentialSchedule(
                    state,
                    schedule,
                    bindings,
                    evaluation,
                    workspace);
            case CompiledQuantifierExpression quantifier:
                return EvaluateQuantifier(
                    state,
                    quantifier,
                    variableIndex: 0,
                    bindings,
                    evaluation,
                    workspace);
            default:
                throw new InvalidOperationException(expression.GetType().Name);
        }
    }

    private bool EvaluateAtom(
        State? state,
        CompiledAtomExpression atom,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        int arity = atom.Arguments.Length;
        Constant? arg1 = arity > 0 ? atom.Arguments[0].Resolve(bindings) : null;
        Constant? arg2 = arity > 1 ? atom.Arguments[1].Resolve(bindings) : null;
        Constant? arg3 = arity > 2 ? atom.Arguments[2].Resolve(bindings) : null;
        Constant? arg4 = arity > 3 ? atom.Arguments[3].Resolve(bindings) : null;
        Constant[]? arguments = null;
        if (arity > 4)
        {
            arguments = new Constant[arity];
            arguments[0] = arg1!;
            arguments[1] = arg2!;
            arguments[2] = arg3!;
            arguments[3] = arg4!;
            for (int i = 4; i < arity; i++)
                arguments[i] = atom.Arguments[i].Resolve(bindings);
        }

        if (atom.Predicate is Predicate<Derived> derived)
        {
            DerivedFactKey key = arity switch
            {
                0 => new DerivedFactKey(derived),
                1 => new DerivedFactKey(derived, arg1!),
                2 => new DerivedFactKey(derived, arg1!, arg2!),
                3 => new DerivedFactKey(derived, arg1!, arg2!, arg3!),
                4 => new DerivedFactKey(derived, arg1!, arg2!, arg3!, arg4!),
                _ => new DerivedFactKey(derived, arguments!),
            };
            return IsDerivedTrue(evaluation, derived, key, workspace);
        }

        if (atom.Predicate is Predicate<Static> equality
            && _context.Problem.Domain.IsEqualityPredicate(equality))
        {
            return ReferenceEquals(arg1, arg2);
        }

        FactIndex? factIndex = arity switch
        {
            0 => _context.GetPrevalidatedFactIndex(atom.Predicate),
            1 => _context.GetPrevalidatedFactIndex(atom.Predicate, arg1!),
            2 => _context.GetPrevalidatedFactIndex(atom.Predicate, arg1!, arg2!),
            3 => _context.GetPrevalidatedFactIndex(atom.Predicate, arg1!, arg2!, arg3!),
            4 => _context.GetPrevalidatedFactIndex(atom.Predicate, arg1!, arg2!, arg3!, arg4!),
            _ => _context.GetPrevalidatedFactIndex(atom.Predicate, arguments!),
        };
        if (factIndex is null)
            return false;

        Fact fact = _context.GetFact(factIndex.Value);
        if (fact is Fact<Static> staticFact)
            return _context.IsTrue(new StaticIndex(staticFact.LocalIndex));

        if (fact is Fact<Fluent> fluentFact)
        {
            if (state is null)
                throw new InvalidOperationException(nameof(state));
            return state.IsTrueUnchecked(new FluentIndex(fluentFact.LocalIndex));
        }

        throw new InvalidOperationException(fact.GetType().Name);
    }

    private bool IsDerivedTrue(
        DerivedPredicateEvaluation? evaluation,
        Predicate<Derived> predicate,
        DerivedFactKey key,
        DerivedEvaluationWorkspace workspace)
    {
        DerivedPredicateComponent component = _plan.GetComponent(predicate);
        if (!component.IsStateDependent)
        {
            return _context.TryGetDerivedEvaluationSlot(key, out DerivedEvaluationSlot slot)
                && _staticTruth.Contains(slot);
        }

        if (evaluation is null)
            throw new InvalidOperationException(nameof(evaluation));

        if (component.RequiresFixedPoint)
        {
            return _context.TryGetDerivedEvaluationSlot(key, out DerivedEvaluationSlot slot)
                && evaluation.IsRecursiveTrue(slot);
        }

        return IsAcyclicTrue(evaluation, predicate, key, workspace);
    }

    private bool EvaluateExistentialSchedule(
        State? state,
        CompiledExistentialSchedule schedule,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        if (schedule.HasEmptyCandidates)
            return false;

        if (!EvaluateGuardGroup(
                state,
                schedule.GuardsByDepth[0],
                bindings,
                evaluation,
                workspace))
        {
            return false;
        }

        return EvaluateExistentialBindings(
            state,
            schedule,
            variableIndex: 0,
            bindings,
            evaluation,
            workspace);
    }

    private bool EvaluateExistentialBindings(
        State? state,
        CompiledExistentialSchedule schedule,
        int variableIndex,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        if (variableIndex == schedule.VariableSlots.Length)
            return true;

        int slot = schedule.VariableSlots[variableIndex];
        if (bindings[slot] is not null)
        {
            throw new InvalidOperationException(
                "A quantified binding slot is already active.");
        }

        try
        {
            Constant[] candidates = schedule.Candidates[variableIndex];
            for (int i = 0; i < candidates.Length; i++)
            {
                bindings[slot] = candidates[i];
                bool guardsHold = EvaluateGuardGroup(
                    state,
                    schedule.GuardsByDepth[variableIndex + 1],
                    bindings,
                    evaluation,
                    workspace);
                if (guardsHold
                    && EvaluateExistentialBindings(
                        state,
                        schedule,
                        variableIndex + 1,
                        bindings,
                        evaluation,
                        workspace))
                {
                    return true;
                }
            }
            return false;
        }
        finally
        {
            bindings[slot] = null;
        }
    }

    private bool EvaluateGuardGroup(
        State? state,
        IReadOnlyList<CompiledExpression> guards,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        for (int i = 0; i < guards.Count; i++)
        {
            if (!Evaluate(
                    state,
                    guards[i],
                    bindings,
                    evaluation,
                    workspace))
                return false;
        }
        return true;
    }

    private bool EvaluateQuantifier(
        State? state,
        CompiledQuantifierExpression quantifier,
        int variableIndex,
        Constant?[] bindings,
        DerivedPredicateEvaluation? evaluation,
        DerivedEvaluationWorkspace workspace)
    {
        if (variableIndex == quantifier.VariableSlots.Length)
        {
            return Evaluate(
                state,
                quantifier.Body,
                bindings,
                evaluation,
                workspace);
        }

        int slot = quantifier.VariableSlots[variableIndex];
        if (bindings[slot] is not null)
        {
            throw new InvalidOperationException(
                "A quantified binding slot is already active.");
        }

        try
        {
            Constant[] candidates = quantifier.Candidates[variableIndex];
            bool quantifierResult = !quantifier.Any;
            for (int i = 0; i < candidates.Length; i++)
            {
                bindings[slot] = candidates[i];
                bool candidateResult = EvaluateQuantifier(
                    state,
                    quantifier,
                    variableIndex + 1,
                    bindings,
                    evaluation,
                    workspace);

                if (quantifier.Any && candidateResult)
                    return true;
                if (!quantifier.Any && !candidateResult)
                    return false;
            }
            return quantifierResult;
        }
        finally
        {
            bindings[slot] = null;
        }
    }
}
