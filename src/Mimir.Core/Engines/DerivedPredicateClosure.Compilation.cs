namespace Mimir.Core.Engines;

using Grounding;
using Schemas;

internal sealed partial class DerivedPredicateClosure
{
    private abstract class CompiledExpression;

    private sealed class CompiledTrueExpression : CompiledExpression
    {
        internal static CompiledTrueExpression Instance { get; } = new();
    }

    private readonly struct CompiledTerm
    {
        private readonly Constant? _constant;
        private readonly int _slot;

        internal CompiledTerm(Constant constant)
        {
            _constant = constant;
            _slot = -1;
        }

        internal CompiledTerm(int slot)
        {
            _constant = null;
            _slot = slot;
        }

        internal Constant Resolve(Constant?[] bindings)
        {
            if (_slot < 0)
                return _constant!;

            return bindings[_slot]
                ?? throw new InvalidOperationException(
                    "A compiled variable slot has no binding.");
        }
    }

    private sealed class CompiledAtomExpression : CompiledExpression
    {
        internal Predicate Predicate { get; }
        internal CompiledTerm[] Arguments { get; }

        internal CompiledAtomExpression(
            Predicate predicate,
            CompiledTerm[] arguments)
        {
            Predicate = predicate;
            Arguments = arguments;
        }
    }

    private sealed class CompiledNotExpression : CompiledExpression
    {
        internal CompiledExpression Expression { get; }

        internal CompiledNotExpression(CompiledExpression expression)
        {
            Expression = expression;
        }
    }

    private sealed class CompiledAndExpression : CompiledExpression
    {
        internal CompiledExpression[] Expressions { get; }

        internal CompiledAndExpression(CompiledExpression[] expressions)
        {
            Expressions = expressions;
        }
    }

    private sealed class CompiledOrExpression : CompiledExpression
    {
        internal CompiledExpression[] Expressions { get; }

        internal CompiledOrExpression(CompiledExpression[] expressions)
        {
            Expressions = expressions;
        }
    }

    private sealed class CompiledImplyExpression : CompiledExpression
    {
        internal CompiledExpression Antecedent { get; }
        internal CompiledExpression Consequent { get; }

        internal CompiledImplyExpression(
            CompiledExpression antecedent,
            CompiledExpression consequent)
        {
            Antecedent = antecedent;
            Consequent = consequent;
        }
    }

    private sealed class CompiledQuantifierExpression : CompiledExpression
    {
        internal int[] VariableSlots { get; }
        internal Constant[][] Candidates { get; }
        internal CompiledExpression Body { get; }
        internal bool Any { get; }

        internal CompiledQuantifierExpression(
            int[] variableSlots,
            Constant[][] candidates,
            CompiledExpression body,
            bool any)
        {
            VariableSlots = variableSlots;
            Candidates = candidates;
            Body = body;
            Any = any;
        }
    }

    private sealed class CompiledExistentialSchedule : CompiledExpression
    {
        internal int[] VariableSlots { get; }
        internal Constant[][] Candidates { get; }
        internal CompiledExpression[][] GuardsByDepth { get; }
        internal bool HasEmptyCandidates { get; }

        internal CompiledExistentialSchedule(
            int[] variableSlots,
            Constant[][] candidates,
            CompiledExpression[][] guardsByDepth)
        {
            VariableSlots = variableSlots;
            Candidates = candidates;
            GuardsByDepth = guardsByDepth;

            for (int i = 0; i < candidates.Length; i++)
            {
                if (candidates[i].Length == 0)
                {
                    HasEmptyCandidates = true;
                    break;
                }
            }
        }
    }

    private sealed class CompiledRule
    {
        internal Predicate<Derived> Predicate { get; }
        internal CompiledExpression Body { get; }
        internal int[] HeadSlots { get; }
        internal Constant[][] HeadCandidates { get; }
        internal int SlotCount { get; }

        internal CompiledRule(
            Predicate<Derived> predicate,
            CompiledExpression body,
            int[] headSlots,
            Constant[][] headCandidates,
            int slotCount)
        {
            Predicate = predicate;
            Body = body;
            HeadSlots = headSlots;
            HeadCandidates = headCandidates;
            SlotCount = slotCount;
        }
    }

    private void CompileRules()
    {
        foreach (DerivedPredicateComponent component in _plan.Components)
        {
            var componentRules = new CompiledRule[component.Predicates.Count];
            for (int i = 0; i < component.Predicates.Count; i++)
            {
                Predicate<Derived> predicate = component.Predicates[i];
                IGroundedExpression body =
                    _context.Problem.Domain.DerivedDefinitions[predicate.Name];
                var slotsByVariable = new Dictionary<Variable, int>(
                    ReferenceEqualityComparer.Instance);
                var activeVariables = new HashSet<Variable>(
                    ReferenceEqualityComparer.Instance);
                int slotCount = 0;
                int[] headSlots = EnterVariables(
                    predicate.Parameters,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount);
                CompiledExpression compiledBody = CompileExpression(
                    body,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount);
                var candidates = new Constant[predicate.Parameters.Count][];
                for (int parameterIndex = 0;
                    parameterIndex < predicate.Parameters.Count;
                    parameterIndex++)
                {
                    candidates[parameterIndex] =
                        _context.GetCompatibleObjects(
                            predicate.Parameters[parameterIndex].Type);
                }

                var rule = new CompiledRule(
                    predicate,
                    compiledBody,
                    headSlots,
                    candidates,
                    slotCount);
                componentRules[i] = rule;
                _rulesByPredicate.Add(predicate, rule);
            }

            _rulesByComponent.Add(component, componentRules);
        }
    }

    private CompiledExpression CompileExpression(
        IGroundedExpression expression,
        Dictionary<Variable, int> slotsByVariable,
        HashSet<Variable> activeVariables,
        ref int slotCount)
    {
        switch (expression)
        {
            case GroundedTrue:
                return CompiledTrueExpression.Instance;
            case GroundedAtom atom:
                return CompileAtom(atom, slotsByVariable);
            case GroundedNot not:
                return new CompiledNotExpression(CompileExpression(
                    not.Expression,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount));
            case GroundedAnd and:
                return new CompiledAndExpression(CompileExpressions(
                    and.Expressions,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount));
            case GroundedOr or:
                return new CompiledOrExpression(CompileExpressions(
                    or.Expressions,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount));
            case GroundedImply imply:
                return new CompiledImplyExpression(
                    CompileExpression(
                        imply.Antecedent,
                        slotsByVariable,
                        activeVariables,
                        ref slotCount),
                    CompileExpression(
                        imply.Consequent,
                        slotsByVariable,
                        activeVariables,
                        ref slotCount));
            case GroundedForall forall:
                return CompileQuantifier(
                    forall.Variables,
                    forall.Body,
                    any: false,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount);
            case GroundedExists exists:
                return TryCompileExistentialSchedule(
                        exists,
                        slotsByVariable,
                        activeVariables,
                        ref slotCount)
                    ?? CompileQuantifier(
                        exists.Variables,
                        exists.Body,
                        any: true,
                        slotsByVariable,
                        activeVariables,
                        ref slotCount);
            default:
                throw new InvalidOperationException(expression.GetType().Name);
        }
    }

    private CompiledExpression[] CompileExpressions(
        IReadOnlyList<IGroundedExpression> expressions,
        Dictionary<Variable, int> slotsByVariable,
        HashSet<Variable> activeVariables,
        ref int slotCount)
    {
        var compiled = new CompiledExpression[expressions.Count];
        for (int i = 0; i < expressions.Count; i++)
        {
            compiled[i] = CompileExpression(
                expressions[i],
                slotsByVariable,
                activeVariables,
                ref slotCount);
        }
        return compiled;
    }

    private static CompiledAtomExpression CompileAtom(
        GroundedAtom atom,
        IReadOnlyDictionary<Variable, int> slotsByVariable)
    {
        var arguments = new CompiledTerm[atom.Arguments.Count];
        for (int i = 0; i < atom.Arguments.Count; i++)
        {
            if (atom.Arguments[i] is Constant constant)
            {
                arguments[i] = new CompiledTerm(constant);
                continue;
            }

            var variable = (Variable)atom.Arguments[i];
            if (!slotsByVariable.TryGetValue(variable, out int slot))
            {
                throw new InvalidOperationException(
                    $"Variable '{variable.Name}' has no compiled binding slot.");
            }
            arguments[i] = new CompiledTerm(slot);
        }

        return new CompiledAtomExpression(atom.Predicate, arguments);
    }

    private CompiledExpression CompileQuantifier(
        IReadOnlyList<Variable> variables,
        IGroundedExpression body,
        bool any,
        Dictionary<Variable, int> slotsByVariable,
        HashSet<Variable> activeVariables,
        ref int slotCount)
    {
        int[] slots = EnterVariables(
            variables,
            slotsByVariable,
            activeVariables,
            ref slotCount);
        try
        {
            var candidates = new Constant[variables.Count][];
            for (int i = 0; i < variables.Count; i++)
                candidates[i] = _context.GetCompatibleObjects(variables[i].Type);

            return new CompiledQuantifierExpression(
                slots,
                candidates,
                CompileExpression(
                    body,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount),
                any);
        }
        finally
        {
            ExitVariables(variables, activeVariables);
        }
    }

    private CompiledExistentialSchedule? TryCompileExistentialSchedule(
        GroundedExists exists,
        Dictionary<Variable, int> slotsByVariable,
        HashSet<Variable> activeVariables,
        ref int slotCount)
    {
        var variables = new List<Variable>();
        IGroundedExpression terminal = exists;
        while (terminal is GroundedExists nested)
        {
            for (int i = 0; i < nested.Variables.Count; i++)
                variables.Add(nested.Variables[i]);
            terminal = nested.Body;
        }

        if (terminal is not GroundedAnd conjunction)
            return null;

        int[] slots = EnterVariables(
            variables,
            slotsByVariable,
            activeVariables,
            ref slotCount);
        try
        {
            var variableDepths = new Dictionary<Variable, int>(
                variables.Count,
                ReferenceEqualityComparer.Instance);
            var candidates = new Constant[variables.Count][];
            for (int i = 0; i < variables.Count; i++)
            {
                variableDepths.Add(variables[i], i);
                candidates[i] = _context.GetCompatibleObjects(variables[i].Type);
            }

            var groups = new List<CompiledExpression>[variables.Count + 1];
            for (int i = 0; i < groups.Length; i++)
                groups[i] = new List<CompiledExpression>();

            for (int i = 0; i < conjunction.Expressions.Length; i++)
            {
                IGroundedExpression conjunct = conjunction.Expressions[i];
                int deepestVariable = FindDeepestVariable(
                    conjunct,
                    variableDepths);
                groups[deepestVariable + 1].Add(CompileExpression(
                    conjunct,
                    slotsByVariable,
                    activeVariables,
                    ref slotCount));
            }

            var guardsByDepth = new CompiledExpression[groups.Length][];
            for (int i = 0; i < groups.Length; i++)
                guardsByDepth[i] = groups[i].ToArray();

            return new CompiledExistentialSchedule(
                slots,
                candidates,
                guardsByDepth);
        }
        finally
        {
            ExitVariables(variables, activeVariables);
        }
    }

    private static int[] EnterVariables(
        IReadOnlyList<Variable> variables,
        Dictionary<Variable, int> slotsByVariable,
        HashSet<Variable> activeVariables,
        ref int slotCount)
    {
        var slots = new int[variables.Count];
        for (int i = 0; i < variables.Count; i++)
        {
            Variable variable = variables[i];
            if (!activeVariables.Add(variable))
            {
                throw new InvalidOperationException(
                    $"Variable '{variable.Name}' is already active during derived compilation.");
            }

            if (!slotsByVariable.TryGetValue(variable, out int slot))
            {
                slot = slotCount++;
                slotsByVariable.Add(variable, slot);
            }
            slots[i] = slot;
        }
        return slots;
    }

    private static void ExitVariables(
        IReadOnlyList<Variable> variables,
        HashSet<Variable> activeVariables)
    {
        for (int i = variables.Count - 1; i >= 0; i--)
        {
            if (!activeVariables.Remove(variables[i]))
            {
                throw new InvalidOperationException(
                    "A derived compilation variable was not active during scope exit.");
            }
        }
    }

    private static int FindDeepestVariable(
        IGroundedExpression expression,
        IReadOnlyDictionary<Variable, int> variableDepths)
    {
        switch (expression)
        {
            case GroundedTrue:
                return -1;
            case GroundedAtom atom:
            {
                int deepest = -1;
                for (int i = 0; i < atom.Arguments.Count; i++)
                {
                    if (atom.Arguments[i] is Variable variable
                        && variableDepths.TryGetValue(variable, out int depth))
                    {
                        deepest = Math.Max(deepest, depth);
                    }
                }
                return deepest;
            }
            case GroundedNot not:
                return FindDeepestVariable(not.Expression, variableDepths);
            case GroundedAnd and:
                return FindDeepestVariable(and.Expressions, variableDepths);
            case GroundedOr or:
                return FindDeepestVariable(or.Expressions, variableDepths);
            case GroundedImply imply:
                return Math.Max(
                    FindDeepestVariable(imply.Antecedent, variableDepths),
                    FindDeepestVariable(imply.Consequent, variableDepths));
            case GroundedForall forall:
                return FindDeepestVariable(forall.Body, variableDepths);
            case GroundedExists exists:
                return FindDeepestVariable(exists.Body, variableDepths);
            default:
                throw new InvalidOperationException(expression.GetType().Name);
        }
    }

    private static int FindDeepestVariable(
        IEnumerable<IGroundedExpression> expressions,
        IReadOnlyDictionary<Variable, int> variableDepths)
    {
        int deepest = -1;
        foreach (IGroundedExpression expression in expressions)
        {
            deepest = Math.Max(
                deepest,
                FindDeepestVariable(expression, variableDepths));
        }
        return deepest;
    }
}
