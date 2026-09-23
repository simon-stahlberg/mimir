using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class BuilderParityTests
{
    [Fact]
    public void TypedStripsAndEqualityBuilderMatchesText()
    {
        Domain textDomain = Domain.FromText(
            """
            (define (domain typed-parity)
              (:requirements :strips :typing :equality :negative-preconditions)
              (:types item)
              (:constants home - item)
              (:predicates (ready ?item - item) (marked ?item - item))
              (:action mark
                :parameters (?item - item)
                :precondition (and (ready ?item) (not (marked ?item)) (not (= ?item home)))
                :effect (marked ?item)))
            """);
        Problem textProblem = Problem.FromText(
            textDomain,
            """
            (define (problem typed-parity-problem)
              (:domain typed-parity)
              (:objects a - item)
              (:init (ready a))
              (:goal (and (marked a) (not (marked home)) (= a a))))
            """);

        Domain builtDomain = new DomainBuilder("typed-parity")
            .Requirements()
                .Add(":strips")
                .Add(":typing")
                .Add(":equality")
                .Add(":negative-preconditions")
                .Close()
            .Types()
                .Add("item")
                .Close()
            .Constants()
                .Add("home", "item")
                .Close()
            .Predicates()
                .Add("ready", ("?item", "item"))
                .Add("marked", ("?item", "item"))
                .Close()
            .Actions()
                .Add("mark")
                    .AddParameter("?item", "item")
                    .AddPrecondition("ready", "?item")
                    .AddPrecondition("marked", Polarity.Negative, "?item")
                    .AddPrecondition("=", Polarity.Negative, "?item", "home")
                    .AddEffect("marked", "?item")
                    .Close()
                .Close()
            .Build();
        Problem builtProblem = new ProblemBuilder(builtDomain, "typed-parity-problem")
            .Objects()
                .Add("a", "item")
                .Close()
            .InitialState()
                .AddFact("ready", "a")
                .Close()
            .Goal()
                .Add("marked", "a")
                .Add("marked", Polarity.Negative, "home")
                .Add("=", "a", "a")
                .Close()
            .Build();

        AssertEquivalent(
            textProblem,
            builtProblem,
            [
                new FactProbe("ready", ["a"]),
                new FactProbe("marked", ["a"]),
                new FactProbe("marked", ["home"]),
                new FactProbe("=", ["a", "a"]),
                new FactProbe("=", ["a", "home"]),
            ],
            "mark(a)");
    }

    [Fact]
    public void ConditionalAndQuantifiedEffectBuilderMatchesText()
    {
        Domain textDomain = Domain.FromText(
            """
            (define (domain effect-parity)
              (:requirements :adl :typing)
              (:types item)
              (:predicates (enabled) (selected ?item - item) (done ?item - item))
              (:action finish
                :parameters ()
                :precondition (enabled)
                :effect (forall (?item - item)
                  (when (selected ?item) (done ?item)))))
            """);
        Problem textProblem = Problem.FromText(
            textDomain,
            """
            (define (problem effect-parity-problem)
              (:domain effect-parity)
              (:objects a b - item)
              (:init (enabled) (selected a))
              (:goal (done a)))
            """);

        DomainBuilder domainBuilder = new DomainBuilder("effect-parity")
            .Requirements()
                .Add(":adl")
                .Add(":typing")
                .Close()
            .Types()
                .Add("item")
                .Close()
            .Predicates()
                .Add("enabled")
                .Add("selected", ("?item", "item"))
                .Add("done", ("?item", "item"))
                .Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish")
            .AddPrecondition("enabled");
        action.AddConditionalEffect()
            .AddParameter("?item", "item")
            .AddCondition("selected", "?item")
            .AddEffect("done", "?item")
            .Close();
        action.Close();
        actions.Close();
        Domain builtDomain = domainBuilder.Build();
        Problem builtProblem = new ProblemBuilder(builtDomain, "effect-parity-problem")
            .Objects()
                .Add("a", "item")
                .Add("b", "item")
                .Close()
            .InitialState()
                .AddFact("enabled")
                .AddFact("selected", "a")
                .Close()
            .Goal()
                .Add("done", "a")
                .Close()
            .Build();

        AssertEquivalent(
            textProblem,
            builtProblem,
            [
                new FactProbe("enabled", []),
                new FactProbe("selected", ["a"]),
                new FactProbe("selected", ["b"]),
                new FactProbe("done", ["a"]),
                new FactProbe("done", ["b"]),
            ],
            "finish()");
    }

    [Fact]
    public void RecursiveDerivedPredicateBuilderMatchesText()
    {
        Domain textDomain = Domain.FromText(
            """
            (define (domain derived-parity)
              (:requirements :adl :typing :derived-predicates)
              (:types item)
              (:predicates
                (seed ?item - item)
                (link ?from - item ?to - item)
                (reachable ?item - item)
                (done ?item - item))
              (:derived (reachable ?item - item)
                (or
                  (seed ?item)
                  (exists (?from - item)
                    (and (reachable ?from) (link ?from ?item)))))
              (:action finish
                :parameters (?item - item)
                :precondition (reachable ?item)
                :effect (done ?item)))
            """);
        Problem textProblem = Problem.FromText(
            textDomain,
            """
            (define (problem derived-parity-problem)
              (:domain derived-parity)
              (:objects a b - item)
              (:init (seed a) (link a b))
              (:goal (done b)))
            """);

        Domain builtDomain = new DomainBuilder("derived-parity")
            .Requirements()
                .Add(":adl")
                .Add(":typing")
                .Add(":derived-predicates")
                .Close()
            .Types()
                .Add("item")
                .Close()
            .Predicates()
                .Add("seed", ("?item", "item"))
                .Add("link", ("?from", "item"), ("?to", "item"))
                .Add("reachable", ("?item", "item"))
                .Add("done", ("?item", "item"))
                .Close()
            .Actions()
                .Add("finish")
                    .AddParameter("?item", "item")
                    .AddPrecondition("reachable", "?item")
                    .AddEffect("done", "?item")
                    .Close()
                .Close()
            .DerivedPredicates()
                .Define(
                    "reachable",
                    Logic.Or(
                        Logic.Atom("seed", "?item"),
                        Logic.Exists(
                            [("?from", "item")],
                            Logic.And(
                                Logic.Atom("reachable", "?from"),
                                Logic.Atom("link", "?from", "?item")))))
                .Close()
            .Build();
        Problem builtProblem = new ProblemBuilder(builtDomain, "derived-parity-problem")
            .Objects()
                .Add("a", "item")
                .Add("b", "item")
                .Close()
            .InitialState()
                .AddFact("seed", "a")
                .AddFact("link", "a", "b")
                .Close()
            .Goal()
                .Add("done", "b")
                .Close()
            .Build();

        AssertEquivalent(
            textProblem,
            builtProblem,
            [
                new FactProbe("seed", ["a"]),
                new FactProbe("link", ["a", "b"]),
                new FactProbe("reachable", ["a"]),
                new FactProbe("reachable", ["b"]),
                new FactProbe("done", ["b"]),
            ],
            "finish(b)");
    }

    [Fact]
    public void ActionCostAndNumericInitializationBuilderMatchesText()
    {
        Domain textDomain = Domain.FromText(
            """
            (define (domain cost-parity)
              (:requirements :strips :typing :action-costs)
              (:types item)
              (:predicates (ready ?item - item) (done ?item - item))
              (:functions (price ?item - item))
              (:action buy
                :parameters (?item - item)
                :precondition (ready ?item)
                :effect (and (done ?item) (increase (total-cost) (+ (price ?item) 0.5))))
              (:action wait
                :parameters ()
                :effect (increase (total-cost) 3)))
            """);
        Problem textProblem = Problem.FromText(
            textDomain,
            """
            (define (problem cost-parity-problem)
              (:domain cost-parity)
              (:objects a - item)
              (:init (ready a) (= (price a) 2.5) (= (total-cost) 0))
              (:goal (done a))
              (:metric minimize (total-cost)))
            """);

        Domain builtDomain = new DomainBuilder("cost-parity")
            .Requirements()
                .Add(":strips")
                .Add(":typing")
                .Add(":action-costs")
                .Close()
            .Types()
                .Add("item")
                .Close()
            .Predicates()
                .Add("ready", ("?item", "item"))
                .Add("done", ("?item", "item"))
                .Close()
            .Functions()
                .Add("price", ("?item", "item"))
                .Close()
            .Actions()
                .Add("buy")
                    .AddParameter("?item", "item")
                    .AddPrecondition("ready", "?item")
                    .AddEffect("done", "?item")
                    .WithCost(Numeric.Add(
                        Numeric.Function("price", "?item"),
                        Numeric.Constant(0.5d)))
                    .Close()
                .Add("wait")
                    .WithCost(3d)
                    .Close()
                .Close()
            .Build();
        Problem builtProblem = new ProblemBuilder(builtDomain, "cost-parity-problem")
            .Objects()
                .Add("a", "item")
                .Close()
            .InitialState()
                .AddFact("ready", "a")
                .SetValue(Numeric.Function("price", "a"), 2.5d)
                .Close()
            .Goal()
                .Add("done", "a")
                .Close()
            .Build();

        Assert.Equal(
            textProblem.GetNumericFunctionValue(textDomain.Functions.Single(), [textProblem.ObjectLookup["a"]]),
            builtProblem.GetNumericFunctionValue(
                builtDomain.Functions.Single(),
                [builtProblem.ObjectLookup["a"]]));
        AssertEquivalent(
            textProblem,
            builtProblem,
            [new FactProbe("ready", ["a"]), new FactProbe("done", ["a"])],
            "buy(a)");
    }

    private static void AssertEquivalent(
        Problem textProblem,
        Problem builtProblem,
        IReadOnlyList<FactProbe> probes,
        string actionToApply)
    {
        Assert.Equal(
            string.Join(Environment.NewLine, DomainSnapshot(textProblem.Domain)),
            string.Join(Environment.NewLine, DomainSnapshot(builtProblem.Domain)));
        Assert.Equal(ProblemSnapshot(textProblem), ProblemSnapshot(builtProblem));
        Assert.Equal(
            TruthSnapshot(textProblem, textProblem.InitialState, probes),
            TruthSnapshot(builtProblem, builtProblem.InitialState, probes));

        GroundAction[] textActions = textProblem
            .GetApplicableActionGenerator(textProblem.InitialState)
            .GetApplicableActions(textProblem.InitialState.Expand())
            .ToArray();
        GroundAction[] builtActions = builtProblem
            .GetApplicableActionGenerator(builtProblem.InitialState)
            .GetApplicableActions(builtProblem.InitialState.Expand())
            .ToArray();
        Assert.Equal(textActions.Select(ActionSignature), builtActions.Select(ActionSignature));

        GroundAction textAction = Assert.Single(
            textActions,
            action => BoundActionName(action) == actionToApply);
        GroundAction builtAction = Assert.Single(
            builtActions,
            action => BoundActionName(action) == actionToApply);
        State textSuccessor = textProblem.InitialState.Expand().Apply(textAction);
        State builtSuccessor = builtProblem.InitialState.Expand().Apply(builtAction);
        Assert.Equal(
            TruthSnapshot(textProblem, textSuccessor, probes),
            TruthSnapshot(builtProblem, builtSuccessor, probes));
    }

    private static IReadOnlyList<string> DomainSnapshot(Domain domain)
    {
        var snapshot = new List<string>
        {
            domain.Name,
            $"requirements:{string.Join(',', domain.Requirements)}",
            $"types:{string.Join(',', domain.TypeHierarchy.Select(pair => $"{pair.Key}>{pair.Value}"))}",
            $"constants:{string.Join(',', domain.Constants.Select(FormatConstant))}",
            $"fluents:{string.Join(',', domain.Fluents.Select(FormatPredicate))}",
            $"statics:{string.Join(',', domain.Statics.Select(FormatPredicate))}",
            $"derived:{string.Join(',', domain.Derived.Select(FormatPredicate))}",
            $"functions:{string.Join(',', domain.Functions.Select(FormatFunction))}",
        };
        foreach (DerivedPredicate predicate in domain.Derived)
            snapshot.Add($"definition:{predicate.Name}={FormatExpression(domain.DerivedDefinitions[predicate.Name])}");
        snapshot.AddRange(domain.Actions.Select(FormatAction));
        return snapshot;
    }

    private static IReadOnlyList<string> ProblemSnapshot(Problem problem)
        =>
        [
            problem.Name,
            $"generator:{problem.GeneratorType}",
            $"objects:{string.Join(',', problem.AllObjects.Select(FormatConstant))}",
            $"goal:{string.Join(',', problem.Goal.Select(FormatGoalLiteral))}",
        ];

    private static IReadOnlyList<bool> TruthSnapshot(
        Problem problem,
        State state,
        IReadOnlyList<FactProbe> probes)
    {
        ExtendedState expanded = state.Expand();
        return probes.Select(probe => expanded.IsTrue(RegisterFact(problem, probe))).ToArray();
    }

    private static Fact RegisterFact(Problem problem, FactProbe probe)
    {
        Constant[] arguments = probe.Arguments
            .Select(name => problem.ObjectLookup[name])
            .ToArray();
        return problem.AllPredicates[probe.Predicate] switch
        {
            FluentPredicate fluent => problem.Context.RegisterFact(fluent, arguments),
            StaticPredicate stat => problem.Context.RegisterFact(stat, arguments),
            DerivedPredicate derived => problem.Context.RegisterFact(derived, arguments),
            Predicate predicate => throw new InvalidOperationException(
                $"Unsupported predicate type '{predicate.GetType().Name}'."),
        };
    }

    private static string FormatAction(ActionSchema action)
    {
        IEnumerable<string> preconditions = action.FluentPreconditions.Select(FormatLiteral)
            .Concat(action.StaticPreconditions.Select(FormatLiteral))
            .Concat(action.DerivedPreconditions.Select(FormatLiteral));
        return $"action:{action.Name}({FormatVariables(action.Parameters)})" +
               $" pre[{string.Join(',', preconditions)}]" +
               $" effects[{string.Join(',', action.Effects.Select(FormatEffect))}]" +
               $" cost[{FormatCost(action.CostExpression)}]";
    }

    private static string FormatEffect(ConditionalEffect effect)
    {
        IEnumerable<string> conditions = effect.FluentConditions.Select(FormatLiteral)
            .Concat(effect.StaticConditions.Select(FormatLiteral))
            .Concat(effect.DerivedConditions.Select(FormatLiteral));
        return $"forall({FormatVariables(effect.QuantifiedVariables)})" +
               $" when[{string.Join(',', conditions)}] {FormatLiteral(effect.Effect)}";
    }

    private static string FormatExpression(IGroundedExpression expression)
        => expression switch
        {
            GroundedTrue => "true",
            GroundedAtom atom => $"{atom.Predicate.Name}({FormatTerms(atom.Arguments)})",
            GroundedNot not => $"not({FormatExpression(not.Expression)})",
            GroundedAnd and => $"and({string.Join(',', and.Expressions.Select(FormatExpression))})",
            GroundedOr or => $"or({string.Join(',', or.Expressions.Select(FormatExpression))})",
            GroundedImply imply =>
                $"imply({FormatExpression(imply.Antecedent)},{FormatExpression(imply.Consequent)})",
            GroundedForall forall =>
                $"forall({FormatVariables(forall.Variables)}:{FormatExpression(forall.Body)})",
            GroundedExists exists =>
                $"exists({FormatVariables(exists.Variables)}:{FormatExpression(exists.Body)})",
            _ => throw new InvalidOperationException($"Unsupported expression '{expression.GetType().Name}'."),
        };

    private static string FormatCost(NumericExpression expression)
        => expression switch
        {
            NumericConstant constant => constant.Value.ToString("R"),
            FunctionCall function =>
                $"{function.Function.Name}({FormatTerms(function.Arguments)})",
            NumericBinaryExpression binary =>
                $"{binary.Operator}({FormatCost(binary.Left)},{FormatCost(binary.Right)})",
            _ => throw new InvalidOperationException($"Unsupported cost '{expression.GetType().Name}'."),
        };

    private static string FormatGoalLiteral(Literal<Fact> literal)
        => $"{literal.Polarity}:{literal.Value.Predicate.Name}({string.Join(',', literal.Value.Arguments.Select(a => a.Name))})";

    private static string FormatLiteral<T>(Literal<Atom<T>> literal)
        where T : notnull, IPredicateType
        => $"{literal.Polarity}:{literal.Value.Predicate.Name}({FormatTerms(literal.Value.Arguments)})";

    private static string FormatPredicate(Predicate predicate)
        => $"{predicate.Name}({FormatVariables(predicate.Parameters)})";

    private static string FormatFunction(NumericFunction function)
        => $"{function.Name}({FormatVariables(function.Parameters)})";

    private static string FormatConstant(Constant constant) => $"{constant.Name}:{constant.Type}";

    private static string FormatVariables(IEnumerable<Variable> variables)
        => string.Join(',', variables.Select(variable => $"{variable.Name}:{variable.Type}"));

    private static string FormatTerms(IEnumerable<ITerm> terms)
        => string.Join(',', terms.Select(term => term.Name));

    private static string ActionSignature(GroundAction action)
        => $"{BoundActionName(action)}:{action.Cost:R}";

    private static string BoundActionName(GroundAction action)
        => $"{action.Schema.Name}({string.Join(',', action.Arguments.Select(argument => argument.Name))})";

    private sealed record FactProbe(string Predicate, IReadOnlyList<string> Arguments);
}
