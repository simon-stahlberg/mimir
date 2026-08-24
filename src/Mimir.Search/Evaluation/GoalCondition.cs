using System.Runtime.CompilerServices;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;

namespace Mimir.Search.Evaluation;

public sealed class GoalCondition : IEquatable<GoalCondition>
{
    public Problem Problem { get; }
    public IGroundedExpression? Expression { get; }
    internal IReadOnlyList<Literal<Fact>> GoalLiterals { get; }

    private GoalCondition(
        Problem problem,
        IGroundedExpression? expression,
        IReadOnlyList<Literal<Fact>> goalLiterals)
    {
        Problem = problem;
        Expression = expression;
        GoalLiterals = Normalize(goalLiterals);
    }

    public static GoalCondition FromProblem(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);

        return new GoalCondition(problem, expression: null, problem.Goal);
    }

    public static GoalCondition FromExpression(Problem problem, IGroundedExpression expression)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(expression);

        var atoms = new List<(GroundedAtom Atom, Polarity Polarity)>();
        CollectAtoms(problem, expression, atoms, nameof(expression));
        Literal<Fact>[] literals = atoms
            .Select(atom => new Literal<Fact>(
                RegisterFact(problem, atom.Atom),
                atom.Polarity))
            .ToArray();
        return new GoalCondition(problem, expression, literals);
    }

    public static GoalCondition FromLiterals(
        Problem problem,
        IReadOnlyList<Literal<Fact>> literals)
    {
        ArgumentNullException.ThrowIfNull(problem);
        ArgumentNullException.ThrowIfNull(literals);

        foreach (Literal<Fact> literal in literals)
        {
            if (!ReferenceEquals(literal.Value.Context.Problem, problem))
            {
                throw new ArgumentException(
                    "A goal literal belongs to a different problem instance.",
                    nameof(literals));
            }
        }

        return new GoalCondition(problem, expression: null, literals);
    }

    public static GoalCondition Always(Problem problem)
    {
        ArgumentNullException.ThrowIfNull(problem);
        return new GoalCondition(problem, new GroundedTrue(), Array.Empty<Literal<Fact>>());
    }

    public bool IsSatisfied(ExtendedState state)
    {
        EnsureCompatible(state);

        for (int i = 0; i < GoalLiterals.Count; i++)
        {
            Literal<Fact> goalLiteral = GoalLiterals[i];
            bool holds = state.IsTrue(goalLiteral.Value);
            if (goalLiteral.IsPositive != holds)
                return false;
        }

        return true;
    }

    public int CountUnsatisfiedGoals(ExtendedState state)
    {
        EnsureCompatible(state);

        int unsatisfied = 0;
        for (int i = 0; i < GoalLiterals.Count; i++)
        {
            Literal<Fact> goalLiteral = GoalLiterals[i];
            bool holds = state.IsTrue(goalLiteral.Value);
            if (goalLiteral.IsPositive != holds)
                unsatisfied++;
        }

        return unsatisfied;
    }

    internal void EnsureCompatible(ExtendedState state)
    {
        ArgumentNullException.ThrowIfNull(state);
        if (!ReferenceEquals(state.State.Context.Problem, Problem))
            throw new ArgumentException("State belongs to a different problem instance than this goal condition.", nameof(state));
    }

    public bool Equals(GoalCondition? other)
    {
        if (other is null || !ReferenceEquals(Problem, other.Problem))
            return false;

        return GoalLiterals.SequenceEqual(other.GoalLiterals);
    }

    public override bool Equals(object? obj) => Equals(obj as GoalCondition);

    public override int GetHashCode()
    {
        var hash = new HashCode();
        hash.Add(RuntimeHelpers.GetHashCode(Problem));
        foreach (Literal<Fact> literal in GoalLiterals)
            hash.Add(literal);

        return hash.ToHashCode();
    }

    private static IReadOnlyList<Literal<Fact>> Normalize(
        IReadOnlyList<Literal<Fact>> literals)
    {
        Literal<Fact>[] normalized = literals
            .Distinct()
            .OrderBy(literal => literal.Value.Index.Value)
            .ThenBy(literal => literal.Polarity)
            .ToArray();
        return Array.AsReadOnly(normalized);
    }

    private static void CollectAtoms(
        Problem problem,
        IGroundedExpression expression,
        List<(GroundedAtom Atom, Polarity Polarity)> atoms,
        string parameterName)
    {
        switch (expression)
        {
            case GroundedTrue:
                return;

            case GroundedAnd and:
                foreach (IGroundedExpression child in and.Expressions)
                    CollectAtoms(problem, child, atoms, parameterName);
                return;

            case GroundedAtom atom:
                ValidateAtom(problem, atom, parameterName);
                atoms.Add((atom, Polarity.Positive));
                return;

            case GroundedNot { Expression: GroundedAtom atom }:
                ValidateAtom(problem, atom, parameterName);
                atoms.Add((atom, Polarity.Negative));
                return;

            default:
                throw new ArgumentException(
                    "Goal conditions must be conjunctions of ground literals.",
                    parameterName);
        }
    }

    private static void ValidateAtom(
        Problem problem,
        GroundedAtom atom,
        string parameterName)
    {
        if (!problem.Domain.Contains(atom.Predicate))
        {
            throw new ArgumentException(
                $"Predicate '{atom.Predicate.Name}' does not belong to the goal problem's domain.",
                parameterName);
        }

        if (atom.Arguments.Count != atom.Predicate.Parameters.Count)
        {
            throw new ArgumentException(
                $"Predicate '{atom.Predicate.Name}' expects {atom.Predicate.Parameters.Count} arguments, but received {atom.Arguments.Count}.",
                parameterName);
        }

        for (int i = 0; i < atom.Arguments.Count; i++)
        {
            if (atom.Arguments[i] is not Constant constant)
            {
                throw new ArgumentException(
                    "Goal atoms may contain only constants.",
                    parameterName);
            }

            if (!problem.AllObjects.Any(candidate => ReferenceEquals(candidate, constant)))
            {
                throw new ArgumentException(
                    $"Constant '{constant.Name}' does not belong to the goal problem.",
                    parameterName);
            }

            string expectedType = atom.Predicate.Parameters[i].Type;
            if (!problem.Domain.IsCompatible(constant.Type, expectedType))
            {
                throw new ArgumentException(
                    $"Argument {i} of predicate '{atom.Predicate.Name}' has type '{constant.Type}', which is incompatible with '{expectedType}'.",
                    parameterName);
            }
        }

        if (atom.Predicate is not Core.Schemas.Predicate<Fluent>
            and not Core.Schemas.Predicate<Static>
            and not Core.Schemas.Predicate<Derived>)
        {
            throw new ArgumentException(
                $"Unsupported goal predicate type '{atom.Predicate.GetType().Name}'.",
                parameterName);
        }
    }

    private static Fact RegisterFact(Problem problem, GroundedAtom atom)
    {
        Constant[] arguments = atom.Arguments.Cast<Constant>().ToArray();

        return atom.Predicate switch
        {
            Core.Schemas.Predicate<Fluent> fluent => problem.Context.RegisterFact(fluent, arguments),
            Core.Schemas.Predicate<Static> stat => problem.Context.RegisterFact(stat, arguments),
            Core.Schemas.Predicate<Derived> derived => problem.Context.RegisterFact(derived, arguments),
            _ => throw new InvalidOperationException(
                $"Validated goal predicate type '{atom.Predicate.GetType().Name}' is unsupported.")
        };
    }
}
