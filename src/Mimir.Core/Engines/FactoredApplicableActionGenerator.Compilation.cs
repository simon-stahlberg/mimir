using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Core.Engines;

public sealed partial class FactoredApplicableActionGenerator
{
    internal enum FactoredAtomKind
    {
        Static,
        Fluent,
        Derived,
        Equality,
        FunctionDefined,
    }

    /// <summary>
    /// One precondition literal, projected onto the schema parameters it mentions. A literal whose arguments repeat
    /// a parameter or name a constant is still described here; the repetition is resolved when facts are projected.
    /// </summary>
    internal sealed class FactoredAtom
    {
        public required FactoredAtomKind Kind { get; init; }
        public required Polarity Polarity { get; init; }
        public Predicate? Predicate { get; init; }
        public NumericFunction? Function { get; init; }

        /// <summary>Parameter index per argument position, or -1 when the position holds a constant.</summary>
        public required int[] ArgumentSlots { get; init; }
        public required Constant?[] ArgumentConstants { get; init; }

        /// <summary>The distinct parameters this atom mentions, ascending. Projected tuples follow this order.</summary>
        public required int[] Parameters { get; init; }

        public bool IsPositive => Polarity == Polarity.Positive;

        /// <summary>Derived extensions can be far larger than the bindings that reach them, so they are probed
        /// one candidate at a time instead of being materialized.</summary>
        public bool IsProbed => Kind == FactoredAtomKind.Derived;

        /// <summary>
        /// Projects a ground argument list onto <see cref="Parameters"/>, or returns false when the arguments
        /// contradict a constant position or a repeated parameter.
        /// </summary>
        public bool TryProject(IReadOnlyList<Constant> arguments, Constant[] destination)
        {
            Array.Clear(destination);
            for (int position = 0; position < ArgumentSlots.Length; position++)
            {
                Constant argument = arguments[position];
                int parameter = ArgumentSlots[position];
                if (parameter < 0)
                {
                    if (!ReferenceEquals(argument, ArgumentConstants[position])) return false;
                    continue;
                }

                int column = Array.IndexOf(Parameters, parameter);
                if (destination[column] is { } bound)
                {
                    if (!ReferenceEquals(bound, argument)) return false;
                    continue;
                }

                destination[column] = argument;
            }

            return true;
        }
    }

    internal sealed class SchemaPlan
    {
        public required ActionGroundingPlan GroundingPlan { get; init; }
        public required ActionSchema Schema { get; init; }
        public required int ParameterCount { get; init; }
        public required Constant[][] Domains { get; init; }
        public required FactoredAtom[] Atoms { get; init; }

        /// <summary>Parameters absent from every effect, effect condition and the cost; one representative suffices.</summary>
        public required bool[] WitnessParameters { get; init; }

        /// <summary>Set when a nullary precondition or an undefined ground cost makes the schema permanently inapplicable.</summary>
        public required bool IsPermanentlyInapplicable { get; init; }

        /// <summary>Atom indices per parameter, so propagation revisits only the atoms a change can affect.</summary>
        public required int[][] AtomsByParameter { get; init; }

        /// <summary>Atoms over constants alone, which no parameter change can reach.</summary>
        public required int[] ParameterlessAtoms { get; init; }
    }

    private static SchemaPlan CompileSchema(Problem problem, ActionSchema schema)
    {
        if (schema.NumericPreconditions.Count > 0)
        {
            throw new NotSupportedException(
                $"Action '{schema.Name}' has numeric preconditions; the factored generator supports "
                + "no numeric conditions. Use ApplicableActionGeneratorType.Lifted instead.");
        }

        if (schema.NumericEffects.Count > 0)
        {
            throw new NotSupportedException(
                $"Action '{schema.Name}' has numeric effects; the factored generator supports "
                + "no numeric effects. Use ApplicableActionGeneratorType.Lifted instead.");
        }

        var parameterSlots = new Dictionary<Variable, int>(
            schema.Parameters.Count,
            ReferenceEqualityComparer.Instance);
        for (int i = 0; i < schema.Parameters.Count; i++)
            parameterSlots[schema.Parameters[i]] = i;

        var atoms = new List<FactoredAtom>();
        foreach (Literal<Atom<Static>> literal in schema.StaticPreconditions)
        {
            FactoredAtomKind kind = problem.Domain.IsEqualityPredicate(literal.Value.Predicate)
                ? FactoredAtomKind.Equality
                : FactoredAtomKind.Static;
            atoms.Add(CompileAtom(kind, literal.Value.Predicate, literal.Value.Arguments, literal.Polarity, parameterSlots));
        }

        foreach (Literal<Atom<Fluent>> literal in schema.FluentPreconditions)
            atoms.Add(CompileAtom(FactoredAtomKind.Fluent, literal.Value.Predicate, literal.Value.Arguments, literal.Polarity, parameterSlots));

        foreach (Literal<Atom<Derived>> literal in schema.DerivedPreconditions)
            atoms.Add(CompileAtom(FactoredAtomKind.Derived, literal.Value.Predicate, literal.Value.Arguments, literal.Polarity, parameterSlots));

        bool permanentlyInapplicable = AddCostDefinednessAtoms(schema, parameterSlots, atoms);

        var domains = new Constant[schema.Parameters.Count][];
        for (int i = 0; i < domains.Length; i++)
        {
            domains[i] = problem.Context.GetCompatibleObjects(schema.Parameters[i].Type);
            if (domains[i].Length == 0) permanentlyInapplicable = true;
        }

        var atomsByParameter = new List<int>[schema.Parameters.Count];
        for (int parameter = 0; parameter < atomsByParameter.Length; parameter++)
            atomsByParameter[parameter] = new List<int>();

        var parameterlessAtoms = new List<int>();
        for (int index = 0; index < atoms.Count; index++)
        {
            if (atoms[index].Parameters.Length == 0) parameterlessAtoms.Add(index);
            foreach (int parameter in atoms[index].Parameters)
                atomsByParameter[parameter].Add(index);
        }

        ActionGroundingPlan groundingPlan = problem.Context.GetActionGroundingPlan(schema);
        return new SchemaPlan
        {
            GroundingPlan = groundingPlan,
            Schema = schema,
            ParameterCount = schema.Parameters.Count,
            Domains = domains,
            Atoms = atoms.ToArray(),
            WitnessParameters = groundingPlan.TransitionRelevantParameters
                .Select(isRelevant => !isRelevant)
                .ToArray(),
            IsPermanentlyInapplicable = permanentlyInapplicable,
            AtomsByParameter = atomsByParameter.Select(indices => indices.ToArray()).ToArray(),
            ParameterlessAtoms = parameterlessAtoms.ToArray(),
        };
    }

    private static FactoredAtom CompileAtom(
        FactoredAtomKind kind,
        Predicate predicate,
        IReadOnlyList<ITerm> arguments,
        Polarity polarity,
        IReadOnlyDictionary<Variable, int> parameterSlots)
    {
        var slots = new int[arguments.Count];
        var constants = new Constant?[arguments.Count];
        var parameters = new SortedSet<int>();
        for (int position = 0; position < arguments.Count; position++)
        {
            slots[position] = -1;
            if (arguments[position] is Variable variable)
            {
                slots[position] = parameterSlots[variable];
                parameters.Add(slots[position]);
                continue;
            }

            constants[position] = (Constant)arguments[position];
        }

        return new FactoredAtom
        {
            Kind = kind,
            Polarity = polarity,
            Predicate = predicate,
            ArgumentSlots = slots,
            ArgumentConstants = constants,
            Parameters = parameters.ToArray(),
        };
    }

    /// <summary>
    /// PDDL 2.1 makes an action inapplicable when its cost is undefined. Because Mimir rejects state-dependent
    /// costs, "the cost is defined" is a static relation over the parameters, so it is compiled into ordinary
    /// positive atoms instead of forcing a per-member probe after grounding.
    /// </summary>
    private static bool AddCostDefinednessAtoms(
        ActionSchema schema,
        IReadOnlyDictionary<Variable, int> parameterSlots,
        List<FactoredAtom> atoms)
    {
        bool permanentlyInapplicable = false;
        foreach (NumericExpression node in Flatten(schema.CostExpression))
        {
            switch (node)
            {
                case NumericConstant constant when double.IsNaN(constant.Value):
                    permanentlyInapplicable = true;
                    break;
                case NumericConstant:
                    break;
                case NumericBinaryExpression binary when binary.Operator == NumericOperator.Divide:
                    throw new NotSupportedException(
                        $"Action '{schema.Name}' divides inside its cost expression, so the factored generator "
                        + "cannot prove the cost is defined for a group of bindings. "
                        + "Use ApplicableActionGeneratorType.Lifted instead.");
                case NumericBinaryExpression:
                    break;
                case GroundFunctionCall call:
                    if (double.IsNaN(call.InitialValue)) permanentlyInapplicable = true;
                    break;
                case FunctionCall call:
                    atoms.Add(CompileFunctionDefinedAtom(call, parameterSlots));
                    break;
                default:
                    throw new NotSupportedException(
                        $"Action '{schema.Name}' has an unsupported cost expression '{node.GetType().Name}'.");
            }
        }

        return permanentlyInapplicable;
    }

    private static FactoredAtom CompileFunctionDefinedAtom(
        FunctionCall call,
        IReadOnlyDictionary<Variable, int> parameterSlots)
    {
        var slots = new int[call.Arguments.Count];
        var constants = new Constant?[call.Arguments.Count];
        var parameters = new SortedSet<int>();
        for (int position = 0; position < call.Arguments.Count; position++)
        {
            slots[position] = -1;
            if (call.Arguments[position] is Variable variable)
            {
                slots[position] = parameterSlots[variable];
                parameters.Add(slots[position]);
                continue;
            }

            constants[position] = (Constant)call.Arguments[position];
        }

        return new FactoredAtom
        {
            Kind = FactoredAtomKind.FunctionDefined,
            Polarity = Polarity.Positive,
            Function = call.Function,
            ArgumentSlots = slots,
            ArgumentConstants = constants,
            Parameters = parameters.ToArray(),
        };
    }

    private static IEnumerable<NumericExpression> Flatten(NumericExpression expression)
    {
        yield return expression;
        if (expression is not NumericBinaryExpression binary) yield break;
        foreach (NumericExpression node in Flatten(binary.Left)) yield return node;
        foreach (NumericExpression node in Flatten(binary.Right)) yield return node;
    }
}
