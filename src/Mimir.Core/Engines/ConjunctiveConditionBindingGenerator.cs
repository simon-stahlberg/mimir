using Mimir.Core.Algorithms.Graph;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public delegate bool BindingCallback(ReadOnlySpan<Constant> binding);
public delegate bool BindingCallback<TState>(
    ReadOnlySpan<Constant> binding,
    ref TState state);

public sealed class CompiledConjunctiveCondition
{
    public Problem Problem { get; }
    public IReadOnlyList<Variable> Variables { get; }

    internal ConjunctiveConditionBindingGenerator.CompiledConjunctiveConditionData Data { get; }

    internal CompiledConjunctiveCondition(
        Problem problem,
        IReadOnlyList<Variable> variables,
        ConjunctiveConditionBindingGenerator.CompiledConjunctiveConditionData compiledCondition)
    {
        Problem = problem;
        Variables = Array.AsReadOnly(variables.ToArray());
        Data = compiledCondition;
    }
}

public sealed partial class ConjunctiveConditionBindingGenerator
{
    private struct NoCallbackState
    {
    }

    private static bool InvokeBindingCallback(
        ReadOnlySpan<Constant> binding,
        ref BindingCallback callback)
        => callback(binding);

    public CompiledConjunctiveCondition Compile(ConjunctiveCondition condition, Problem problem)
    {
        ArgumentNullException.ThrowIfNull(condition);
        ArgumentNullException.ThrowIfNull(problem);
        if (!ReferenceEquals(condition.Problem, problem))
            throw new ArgumentException("Condition belongs to a different problem.", nameof(condition));

        return Compile(
            problem,
            condition.Parameters,
            condition.FluentLiterals,
            condition.StaticLiterals,
            condition.DerivedLiterals,
            condition.NumericConditions);
    }

    public CompiledConjunctiveCondition Compile(
        Problem problem,
        IReadOnlyList<Variable> variables,
        IReadOnlyList<Literal<Atom<Fluent>>> fluentConditions,
        IReadOnlyList<Literal<Atom<Static>>> staticConditions,
        IReadOnlyList<Literal<Atom<Derived>>> derivedConditions,
        IReadOnlyList<NumericComparison> numericConditions)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(variables);
        ArgumentNullException.ThrowIfNull(fluentConditions);
        ArgumentNullException.ThrowIfNull(staticConditions);
        ArgumentNullException.ThrowIfNull(derivedConditions);
        ArgumentNullException.ThrowIfNull(numericConditions);

        var compiledVariables = variables.ToArray();
        var variableIndices = new Dictionary<Variable, int>(compiledVariables.Length, ReferenceEqualityComparer.Instance);
        var variableSet = new HashSet<Variable>(ReferenceEqualityComparer.Instance);
        for (int i = 0; i < compiledVariables.Length; i++)
        {
            Variable variable = compiledVariables[i]
                ?? throw new ArgumentException("Compiled variable list cannot contain null values.", nameof(variables));
            if (!problem.ContainsVariable(variable))
                throw new ArgumentException(
                    $"Variable '{variable.Name}' belongs to a different domain or problem.",
                    nameof(variables));
            if (!problem.Domain.ContainsType(variable.Type))
                throw new ArgumentException(
                    $"Variable '{variable.Name}' uses undeclared type '{variable.Type}'.",
                    nameof(variables));
            if (!variableIndices.TryAdd(variable, i))
                throw new ArgumentException("Compiled variable list contains the same variable more than once.", nameof(variables));
            variableSet.Add(variable);
        }

        foreach (Literal<Atom<Fluent>> literal in fluentConditions)
            problem.ValidateConditionLiteral(literal, variableSet, nameof(fluentConditions));
        foreach (Literal<Atom<Static>> literal in staticConditions)
            problem.ValidateConditionLiteral(literal, variableSet, nameof(staticConditions));
        foreach (Literal<Atom<Derived>> literal in derivedConditions)
            problem.ValidateConditionLiteral(literal, variableSet, nameof(derivedConditions));

        foreach (NumericComparison comparison in numericConditions)
            problem.ValidateNumericComparison(comparison, variableSet, nameof(numericConditions));

        CompiledConjunctiveConditionData compiledCondition = CompileConditionData(
            problem,
            compiledVariables,
            variableIndices,
            fluentConditions,
            staticConditions,
            derivedConditions,
            numericConditions);
        return new CompiledConjunctiveCondition(
            problem,
            compiledVariables,
            compiledCondition);
    }

    public int EnumerateBindings(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        BindingCallback callback)
        => EnumerateBindings(condition, state, int.MaxValue, callback);

    public int EnumerateBindings<TState>(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        ref TState callbackState,
        BindingCallback<TState> callback)
        => EnumerateBindings(
            condition,
            state,
            int.MaxValue,
            ref callbackState,
            callback);

    public int EnumerateBindings(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        int maxBindings,
        BindingCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        return EnumerateBindings(
            condition,
            state,
            maxBindings,
            ref callback,
            InvokeBindingCallback);
    }

    public int EnumerateBindings<TState>(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        int maxBindings,
        ref TState callbackState,
        BindingCallback<TState> callback)
    {
        ArgumentNullException.ThrowIfNull(state);
        ValidateEvaluationInputs(condition, state.State, maxBindings);
        ArgumentNullException.ThrowIfNull(callback);
        return EnumerateBindingsCore(
            condition,
            new BindingEvaluationState(state),
            maxBindings,
            ref callbackState,
            callback);
    }

    internal int EnumerateRepresentativeBindings(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        bool[] retainedVariables,
        BindingCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        return EnumerateRepresentativeBindings(
            condition,
            state,
            retainedVariables,
            ref callback,
            InvokeBindingCallback);
    }

    internal int EnumerateRepresentativeBindings<TState>(
        CompiledConjunctiveCondition condition,
        ExtendedState state,
        bool[] retainedVariables,
        ref TState callbackState,
        BindingCallback<TState> callback)
    {
        ArgumentNullException.ThrowIfNull(state);
        ValidateEvaluationInputs(condition, state.State, int.MaxValue);
        ArgumentNullException.ThrowIfNull(retainedVariables);
        ArgumentNullException.ThrowIfNull(callback);
        if (retainedVariables.Length != condition.Variables.Count)
            throw new ArgumentException(
                "The retained-variable mask must have one entry per compiled variable.",
                nameof(retainedVariables));

        return EnumerateRepresentativeBindingsCore(
            condition,
            new BindingEvaluationState(state),
            retainedVariables,
            ref callbackState,
            callback);
    }

    internal int EnumerateBindingsUsingFluentState(
        CompiledConjunctiveCondition condition,
        State state,
        BindingCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        return EnumerateBindingsUsingFluentState(
            condition,
            state,
            ref callback,
            InvokeBindingCallback);
    }

    internal int EnumerateBindingsUsingFluentState<TState>(
        CompiledConjunctiveCondition condition,
        State state,
        ref TState callbackState,
        BindingCallback<TState> callback)
    {
        ValidateEvaluationInputs(condition, state, int.MaxValue);
        ArgumentNullException.ThrowIfNull(callback);
        if (condition.Data.HasDerivedConstraints)
        {
            throw new InvalidOperationException(
                "Fluent-only binding enumeration cannot evaluate derived conditions.");
        }

        return EnumerateBindingsCore(
            condition,
            new BindingEvaluationState(state),
            int.MaxValue,
            ref callbackState,
            callback);
    }

    internal int EnumerateBindingsUsingFluentDelta(
        CompiledConjunctiveCondition condition,
        State state,
        FluentDelta delta,
        FluentDeltaMode[] deltaModes,
        BindingCallback callback)
    {
        ArgumentNullException.ThrowIfNull(callback);
        return EnumerateBindingsUsingFluentDelta(
            condition,
            state,
            delta,
            deltaModes,
            ref callback,
            InvokeBindingCallback);
    }

    internal int EnumerateBindingsUsingFluentDelta<TState>(
        CompiledConjunctiveCondition condition,
        State state,
        FluentDelta delta,
        FluentDeltaMode[] deltaModes,
        ref TState callbackState,
        BindingCallback<TState> callback)
    {
        ValidateEvaluationInputs(condition, state, int.MaxValue);
        ArgumentNullException.ThrowIfNull(delta);
        ArgumentNullException.ThrowIfNull(deltaModes);
        ArgumentNullException.ThrowIfNull(callback);
        if (condition.Data.HasDerivedConstraints)
        {
            throw new InvalidOperationException(
                "Fluent-only binding enumeration cannot evaluate derived conditions.");
        }
        if (deltaModes.Length != condition.Data.PositiveFluentCount)
        {
            throw new ArgumentException(
                "The delta-mode array must have one entry per positive fluent condition.",
                nameof(deltaModes));
        }

        return EnumerateBindingsCore(
            condition,
            new BindingEvaluationState(state, delta, deltaModes),
            int.MaxValue,
            ref callbackState,
            callback);
    }

    public bool HasAnyBinding(CompiledConjunctiveCondition condition, ExtendedState state)
        => CountBindings(condition, state, 1) > 0;

    public int CountBindings(CompiledConjunctiveCondition condition, ExtendedState state)
        => CountBindings(condition, state, int.MaxValue);

    public int CountBindings(CompiledConjunctiveCondition condition, ExtendedState state, int maxBindings)
    {
        ArgumentNullException.ThrowIfNull(state);
        ValidateEvaluationInputs(condition, state.State, maxBindings);
        var callbackState = new NoCallbackState();
        return EnumerateBindingsCore(
            condition,
            new BindingEvaluationState(state),
            maxBindings,
            ref callbackState,
            callback: null);
    }

    private static void ValidateEvaluationInputs(
        CompiledConjunctiveCondition condition,
        State state,
        int maxBindings)
    {
        ArgumentNullException.ThrowIfNull(condition);
        ArgumentNullException.ThrowIfNull(state);
        if (maxBindings < 0)
            throw new ArgumentOutOfRangeException(nameof(maxBindings));
        if (!ReferenceEquals(state.Context, condition.Problem.Context))
            throw new InvalidOperationException("State and compiled condition belong to different problem contexts.");
    }
}
