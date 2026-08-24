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
            AddReferencedVariables(effect.FluentConditions, referencedVariables);
            AddReferencedVariables(effect.StaticConditions, referencedVariables);
            AddReferencedVariables(effect.DerivedConditions, referencedVariables);
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
                try
                {
                    cost = plan.Cost.Evaluate(problem, bindings, workspace);
                }
                catch (InvalidOperationException ex)
                {
                    string groundedSignature = string.Join(
                        ", ",
                        ownedArguments.Select(argument => argument.Name));
                    throw new InvalidOperationException(
                        $"Failed to ground action '{plan.Schema.Name}({groundedSignature})' because its cost could not be evaluated.",
                        ex);
                }

                if (!double.IsFinite(cost))
                {
                    throw new InvalidOperationException(
                        $"Grounded action '{plan.Schema.Name}' has a non-finite cost.");
                }

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
                preconditions = new ActionPreconditions(
                    problem.Context,
                    positiveFluent,
                    negativeFluent,
                    positiveStatic,
                    negativeStatic,
                    workspace.ActionDerivedConditions);
            }

            if (effects is null)
            {
                for (int i = 0; i < plan.Effects.Length; i++)
                {
                    CompiledGroundingEffect effect = plan.Effects[i];
                    if (effect.IsSuppressed)
                        continue;

                    ProcessQuantifiedEffect(
                        effect,
                        parameterIndex: 0,
                        problem,
                        bindings,
                        workspace);
                }

                effects = new ActionEffects(
                    plan.Schema,
                    OffsetBitboard.FromSetBitIndices(workspace.AddEffectIndices),
                    OffsetBitboard.FromSetBitIndices(workspace.DeleteEffectIndices),
                    workspace.ConditionalEffects,
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
        CompiledGroundingEffect effect,
        int parameterIndex,
        Problem problem,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        if (parameterIndex == effect.QuantifiedVariables.Length)
        {
            ProcessEffect(effect, problem.Context, bindings, workspace);
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
        AddReferencedVariables(effect.FluentConditions, referencedVariables);
        AddReferencedVariables(effect.StaticConditions, referencedVariables);
        AddReferencedVariables(effect.DerivedConditions, referencedVariables);

        return effect.QuantifiedVariables
            .Where(referencedVariables.Contains)
            .ToArray();
    }

    private static void ProcessEffect(
        CompiledGroundingEffect effect,
        InstanceContext context,
        Constant?[] bindings,
        ActionGroundingWorkspace workspace)
    {
        Fact<Fluent> fact = GroundFact(
            effect.Target,
            context,
            bindings,
            workspace);

        if (!effect.IsConditional)
        {
            List<int> target = effect.Polarity switch
            {
                Polarity.Positive => workspace.AddEffectIndices,
                Polarity.Negative => workspace.DeleteEffectIndices,
                _ => throw new InvalidOperationException(
                    $"Effect has invalid polarity '{effect.Polarity}'.")
            };
            target.Add(fact.LocalIndex);
            return;
        }

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

        workspace.ConditionalEffects.Add(new GroundConditionalEffect(
            context,
            positiveFluent,
            negativeFluent,
            positiveStatic,
            negativeStatic,
            workspace.EffectDerivedConditions,
            new Literal<Fact<Fluent>>(fact, effect.Polarity)));
        workspace.EffectDerivedConditions.Clear();
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
        ActionCostExpression expression,
        HashSet<Variable> referencedVariables)
    {
        switch (expression)
        {
            case ConstantActionCostExpression:
                return;
            case BinaryActionCostExpression binary:
                AddReferencedVariables(binary.Left, referencedVariables);
                AddReferencedVariables(binary.Right, referencedVariables);
                return;
            case NumericFunctionActionCostExpression function:
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
