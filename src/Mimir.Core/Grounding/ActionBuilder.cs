using Mimir.Core.Schemas;

namespace Mimir.Core.Grounding;

internal static class ActionBuilder
{
    internal static bool[] FindPreconditionRelevantParameters(
        ActionSchema schema)
    {
        ArgumentNullException.ThrowIfNull(schema);

        var referencedVariables = new HashSet<Variable>(
            ReferenceEqualityComparer.Instance);
        AddReferencedVariables(
            schema.FluentPreconditions,
            referencedVariables);
        AddReferencedVariables(
            schema.StaticPreconditions,
            referencedVariables);
        AddReferencedVariables(
            schema.DerivedPreconditions,
            referencedVariables);
        AddReferencedVariables(schema.NumericPreconditions, referencedVariables);
        return schema.Parameters
            .Select(referencedVariables.Contains)
            .ToArray();
    }

    internal static bool[] FindTransitionRelevantParameters(ActionSchema schema)
    {
        ArgumentNullException.ThrowIfNull(schema);

        var referencedVariables = new HashSet<Variable>(ReferenceEqualityComparer.Instance);
        foreach (ConditionalEffect effect in schema.Effects)
        {
            AddReferencedVariables(effect.Effect.Value.Arguments, referencedVariables);
            AddConditionVariables(effect, referencedVariables);
        }

        foreach (ConditionalNumericEffect effect in schema.NumericEffects)
        {
            AddReferencedVariables(effect.Effect.Target, referencedVariables);
            AddReferencedVariables(effect.Effect.Expression, referencedVariables);
            AddConditionVariables(effect, referencedVariables);
        }

        AddReferencedVariables(schema.CostExpression, referencedVariables);
        return schema.Parameters
            .Select(referencedVariables.Contains)
            .ToArray();
    }

    public static Action BuildAction(
        ActionSchema schema,
        IReadOnlyList<Constant> args,
        Problem problem)
    {
        problem.ValidateActionBinding(schema, args, nameof(schema), nameof(args));
        problem.Context.ScavengeActionComponentCaches();
        ActionGroundingPlan plan =
            problem.Context.GetActionGroundingPlan(schema);
        return BuildAction(plan, args, problem);
    }

    internal static Action BuildAction(
        ActionGroundingPlan plan,
        IReadOnlyList<Constant> args,
        Problem problem)
        => BuildActionCore(plan, args, problem, takeArgumentOwnership: false);

    internal static Action BuildActionWithOwnedArguments(
        ActionGroundingPlan plan,
        Constant[] args,
        Problem problem)
        => BuildActionCore(plan, args, problem, takeArgumentOwnership: true);

    private static Action BuildActionCore(
        ActionGroundingPlan plan,
        IReadOnlyList<Constant> args,
        Problem problem,
        bool takeArgumentOwnership)
    {
        if (!ReferenceEquals(plan.Context, problem.Context))
        {
            throw new InvalidOperationException(
                "Action grounding plan belongs to a different problem context.");
        }

        if (args.Count != plan.ParameterCount)
        {
            throw new InvalidOperationException(
                "Trusted action binding has an unexpected argument count.");
        }

        Constant[] ownedArguments = takeArgumentOwnership
            ? (Constant[])args
            : args.ToArray();
        var binding = new ActionBinding(
            problem.Context,
            plan.Schema,
            ownedArguments,
            takeArgumentOwnership: true);
        ActionComponentCache? cache =
            problem.Context.GetActionComponentCache(plan);
        ActionPreconditions? preconditions = null;
        ActionEffects? effects = null;
        if (cache is not null)
        {
            cache.TryGetPreconditions(ownedArguments, out preconditions);
            cache.TryGetEffects(ownedArguments, out effects);
        }
        if (preconditions is not null && effects is not null)
            return new Action(binding, preconditions, effects);

        ActionGroundingWorkspace workspace =
            problem.Context.RentActionGroundingWorkspace(plan);
        try
        {
            Constant?[] bindings = workspace.Bindings;
            for (int i = 0; i < plan.ParameterCount; i++)
                bindings[i] = ownedArguments[i];

            double cost = 0d;
            if (effects is null)
            {
                cost = plan.Cost.Evaluate(problem.Context, bindings, state: null);
                if (cost < 0d)
                {
                    throw new InvalidOperationException(
                        $"Grounded action '{plan.Schema.Name}' has a negative cost ({cost}).");
                }
            }

            if (preconditions is null)
            {
                (OffsetBitboard positiveFluent, OffsetBitboard negativeFluent) =
                    BuildBitboards(
                        plan.FluentPreconditions,
                        problem.Context,
                        bindings,
                        workspace);
                (OffsetBitboard positiveStatic, OffsetBitboard negativeStatic) =
                    BuildBitboards(
                        plan.StaticPreconditions,
                        problem.Context,
                        bindings,
                        workspace);
                GroundDerivedConditions(
                    plan.DerivedPreconditions,
                    problem.Context,
                    bindings,
                    workspace,
                    workspace.ActionDerivedConditions);
                GroundNumericComparison[] comparisons = plan.NumericPreconditions.Length == 0
                    ? Array.Empty<GroundNumericComparison>()
                    : new GroundNumericComparison[plan.NumericPreconditions.Length];
                for (int i = 0; i < comparisons.Length; i++)
                    comparisons[i] = plan.NumericPreconditions[i].Ground(problem, bindings);
                preconditions = new ActionPreconditions(
                    problem.Context,
                    positiveFluent,
                    negativeFluent,
                    positiveStatic,
                    negativeStatic,
                    workspace.ActionDerivedConditions,
                    comparisons);
            }

            if (effects is null)
            {
                foreach (CompiledGroundingEffect effect in plan.Effects)
                {
                    if (!effect.IsSuppressed)
                        ProcessQuantifiedEffect(effect, parameterIndex: 0, problem, bindings, workspace);
                }

                foreach (CompiledGroundingNumericEffect effect in plan.NumericEffects)
                {
                    if (!effect.IsSuppressed)
                        ProcessQuantifiedEffect(effect, parameterIndex: 0, problem, bindings, workspace);
                }

                effects = new ActionEffects(
                    OffsetBitboard.FromSetBitIndices(workspace.AddEffectIndices),
                    OffsetBitboard.FromSetBitIndices(workspace.DeleteEffectIndices),
                    workspace.ConditionalEffects,
                    workspace.NumericEffects,
                    workspace.ConditionalNumericEffects,
                    cost);
            }

            cache?.StorePreconditions(ownedArguments, preconditions);
            cache?.StoreEffects(ownedArguments, effects);
            return new Action(binding, preconditions, effects);
        }
        finally
        {
            problem.Context.ReturnActionGroundingWorkspace(workspace);
        }
    }

    private static void ProcessQuantifiedEffect(
        CompiledGroundingEffectBase effect,
        int parameterIndex,
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        if (parameterIndex == effect.QuantifiedVariables.Length)
        {
            ProcessEffect(effect, problem, bindings, workspace);
            return;
        }

        CompiledGroundingQuantifiedVariable variable =
            effect.QuantifiedVariables[parameterIndex];
        try
        {
            for (int i = 0; i < variable.Candidates.Length; i++)
            {
                bindings[variable.Slot] = variable.Candidates[i];
                ProcessQuantifiedEffect(
                    effect,
                    parameterIndex + 1,
                    problem,
                    bindings,
                    workspace);
            }
        }
        finally
        {
            bindings[variable.Slot] = null;
        }
    }

    internal static Variable[] GetReferencedQuantifiedVariables(
        ConditionalEffect effect)
    {
        ArgumentNullException.ThrowIfNull(effect);

        var referencedVariables = new HashSet<Variable>(
            ReferenceEqualityComparer.Instance);
        AddReferencedVariables(effect.Effect.Value.Arguments, referencedVariables);
        AddConditionVariables(effect, referencedVariables);

        return effect.QuantifiedVariables
            .Where(referencedVariables.Contains)
            .ToArray();
    }

    private static void ProcessEffect(
        CompiledGroundingEffectBase effect,
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        switch (effect)
        {
            case CompiledGroundingEffect literal:
                ProcessLiteralEffect(literal, problem.Context, bindings, workspace);
                return;
            case CompiledGroundingNumericEffect numeric:
                ProcessNumericEffect(numeric, problem, bindings, workspace);
                return;
            default:
                throw new InvalidOperationException($"Unknown compiled effect '{effect.GetType().Name}'.");
        }
    }

    private static void ProcessLiteralEffect(
        CompiledGroundingEffect effect,
        InstanceContext context,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        Fact<Fluent> fact = GroundFact(effect.Target, context, bindings, workspace);
        if (!effect.IsConditional)
        {
            (effect.Polarity == Polarity.Positive ? workspace.AddEffectIndices : workspace.DeleteEffectIndices).Add(fact.LocalIndex);
            return;
        }

        GroundedEffectCondition condition = GroundEffectCondition(effect, context.Problem, bindings, workspace);
        workspace.ConditionalEffects.Add(new GroundConditionalEffect(
            context,
            condition.PositiveFluent,
            condition.NegativeFluent,
            condition.PositiveStatic,
            condition.NegativeStatic,
            workspace.EffectDerivedConditions,
            condition.NumericConditions,
            new Literal<Fact<Fluent>>(fact, effect.Polarity)));
        workspace.EffectDerivedConditions.Clear();
    }

    private static void ProcessNumericEffect(
        CompiledGroundingNumericEffect effect,
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        GroundNumericUpdate update = effect.Update.Ground(problem, bindings);
        if (!effect.IsConditional)
        {
            workspace.NumericEffects.Add(update);
            return;
        }

        GroundedEffectCondition condition = GroundEffectCondition(effect, problem, bindings, workspace);
        workspace.ConditionalNumericEffects.Add(new GroundConditionalNumericEffect(
            problem.Context,
            condition.PositiveFluent,
            condition.NegativeFluent,
            condition.PositiveStatic,
            condition.NegativeStatic,
            workspace.EffectDerivedConditions,
            condition.NumericConditions,
            update));
        workspace.EffectDerivedConditions.Clear();
    }

    private readonly record struct GroundedEffectCondition(
        OffsetBitboard PositiveFluent,
        OffsetBitboard NegativeFluent,
        OffsetBitboard PositiveStatic,
        OffsetBitboard NegativeStatic,
        GroundNumericComparison[] NumericConditions);

    // Derived conditions are written to workspace.EffectDerivedConditions; the caller clears it after use.
    private static GroundedEffectCondition GroundEffectCondition(
        CompiledGroundingEffectBase effect,
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        InstanceContext context = problem.Context;
        (OffsetBitboard positiveFluent, OffsetBitboard negativeFluent) =
            BuildBitboards(
                effect.FluentConditions,
                context,
                bindings,
                workspace);
        (OffsetBitboard positiveStatic, OffsetBitboard negativeStatic) =
            BuildBitboards(
                effect.StaticConditions,
                context,
                bindings,
                workspace);
        GroundDerivedConditions(
            effect.DerivedConditions,
            context,
            bindings,
            workspace,
            workspace.EffectDerivedConditions);

        var comparisons = new GroundNumericComparison[effect.NumericConditions.Length];
        for (int i = 0; i < comparisons.Length; i++)
            comparisons[i] = effect.NumericConditions[i].Ground(problem, bindings);
        return new GroundedEffectCondition(positiveFluent, negativeFluent, positiveStatic, negativeStatic, comparisons);
    }

    private static (
        OffsetBitboard Positive,
        OffsetBitboard Negative) BuildBitboards<T>(
        CompiledGroundingLiteral<T>[] literals,
        InstanceContext context,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
        where T : IPredicateType
    {
        if (literals.Length == 0)
            return (default, default);

        List<int> positiveIndices = workspace.PositiveIndices;
        List<int> negativeIndices = workspace.NegativeIndices;
        for (int i = 0; i < literals.Length; i++)
        {
            CompiledGroundingLiteral<T> literal = literals[i];
            Fact<T> fact = GroundFact(
                literal.Atom,
                context,
                bindings,
                workspace);
            List<int> target = literal.Polarity switch
            {
                Polarity.Positive => positiveIndices,
                Polarity.Negative => negativeIndices,
                _ => throw new InvalidOperationException(
                    $"Condition has invalid polarity '{literal.Polarity}'.")
            };
            target.Add(fact.LocalIndex);
        }

        OffsetBitboard positive =
            OffsetBitboard.FromSetBitIndices(positiveIndices);
        OffsetBitboard negative =
            OffsetBitboard.FromSetBitIndices(negativeIndices);
        positiveIndices.Clear();
        negativeIndices.Clear();
        return (positive, negative);
    }

    private static void GroundDerivedConditions(
        CompiledGroundingLiteral<Derived>[] literals,
        InstanceContext context,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace,
        List<Literal<Fact<Derived>>> destination)
    {
        for (int i = 0; i < literals.Length; i++)
        {
            CompiledGroundingLiteral<Derived> literal = literals[i];
            Fact<Derived> fact = GroundFact(
                literal.Atom,
                context,
                bindings,
                workspace);
            destination.Add(new Literal<Fact<Derived>>(
                fact,
                literal.Polarity));
        }
    }

    private static Fact<T> GroundFact<T>(
        CompiledGroundingAtom<T> atom,
        InstanceContext context,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
        where T : IPredicateType
    {
        CompiledGroundingTerm[] terms = atom.Terms;
        return terms.Length switch
        {
            0 => context.RegisterFact(
                atom.Predicate,
                Array.Empty<Constant>()),
            1 => context.RegisterFact(
                atom.Predicate,
                terms[0].Resolve(bindings)),
            2 => context.RegisterFact(
                atom.Predicate,
                terms[0].Resolve(bindings),
                terms[1].Resolve(bindings)),
            3 => context.RegisterFact(
                atom.Predicate,
                terms[0].Resolve(bindings),
                terms[1].Resolve(bindings),
                terms[2].Resolve(bindings)),
            _ => context.RegisterFact(
                atom.Predicate,
                GroundArguments(terms, bindings, workspace))
        };
    }

    private static Constant[] GroundArguments(
        CompiledGroundingTerm[] terms,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        Constant[] arguments = workspace.GetArgumentBuffer(terms.Length);
        for (int i = 0; i < arguments.Length; i++)
            arguments[i] = terms[i].Resolve(bindings);

        return arguments;
    }

    private static void AddConditionVariables(
        ConditionalEffectBase effect,
        HashSet<Variable> referencedVariables)
    {
        AddReferencedVariables(effect.NumericConditions, referencedVariables);
        AddReferencedVariables(effect.FluentConditions, referencedVariables);
        AddReferencedVariables(effect.StaticConditions, referencedVariables);
        AddReferencedVariables(effect.DerivedConditions, referencedVariables);
    }

    private static void AddReferencedVariables(
        IReadOnlyList<ITerm> terms,
        HashSet<Variable> referencedVariables)
    {
        foreach (ITerm term in terms)
        {
            if (term is Variable variable)
                referencedVariables.Add(variable);
        }
    }

    private static void AddReferencedVariables<T>(
        IReadOnlyList<Literal<Atom<T>>> literals,
        HashSet<Variable> referencedVariables)
        where T : IPredicateType
    {
        foreach (Literal<Atom<T>> literal in literals)
            AddReferencedVariables(literal.Value.Arguments, referencedVariables);
    }

    private static void AddReferencedVariables(
        IReadOnlyList<NumericComparison> comparisons,
        HashSet<Variable> referencedVariables)
    {
        foreach (NumericComparison comparison in comparisons)
        {
            AddReferencedVariables(comparison.Left, referencedVariables);
            AddReferencedVariables(comparison.Right, referencedVariables);
        }
    }

    private static void AddReferencedVariables(
        NumericExpression expression,
        HashSet<Variable> referencedVariables)
    {
        switch (expression)
        {
            case NumericConstant:
                return;
            case NumericBinaryExpression binary:
                AddReferencedVariables(binary.Left, referencedVariables);
                AddReferencedVariables(binary.Right, referencedVariables);
                return;
            case FunctionCall function:
                AddReferencedVariables(
                    function.Arguments,
                    referencedVariables);
                return;
            default:
                throw new InvalidOperationException(
                    $"Unsupported action-cost expression '{expression.GetType().Name}'.");
        }
    }
}
