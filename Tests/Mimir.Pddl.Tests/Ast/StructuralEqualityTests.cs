using System.Collections.Immutable;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Tests.Ast;

public class StructuralEqualityTests
{
    [Fact]
    public void CollectionBearingRecordsCompareIndependentArraysByValue()
    {
        foreach ((object left, object right) in EquivalentPairs())
        {
            Assert.NotSame(left, right);
            Assert.Equal(left, right);
            Assert.Equal(left.GetHashCode(), right.GetHashCode());
            Assert.Single(new HashSet<object> { left, right });
        }
    }

    [Fact]
    public void CollectionOrderLengthAndElementsAffectEquality()
    {
        foreach ((object original, object reordered, object changed, object shorter) in SequenceVariants())
        {
            Assert.NotEqual(original, reordered);
            Assert.NotEqual(original, changed);
            Assert.NotEqual(original, shorter);
        }
    }

    [Fact]
    public void NamesRemainCaseSensitive()
    {
        var upper = new PredicateCall("Ready", ImmutableArray<Term>.Empty);
        var lower = new PredicateCall("ready", ImmutableArray<Term>.Empty);

        Assert.NotEqual(upper, lower);
    }

    [Fact]
    public void WithExpressionPreservesStructuralValueSemantics()
    {
        PredicateCall original = CreatePredicateCall();
        PredicateCall copy = original with { Arguments = CreateTerms() };

        Assert.Equal(original, copy);
        Assert.Equal(original.GetHashCode(), copy.GetHashCode());
    }

    private static IEnumerable<(object Left, object Right)> EquivalentPairs()
    {
        yield return (CreatePredicateDeclaration(), CreatePredicateDeclaration());
        yield return (CreateFunctionDeclaration(), CreateFunctionDeclaration());
        yield return (CreateActionDefinition(), CreateActionDefinition());
        yield return (CreateDomainDefinition(), CreateDomainDefinition());
        yield return (CreateProblemDefinition(), CreateProblemDefinition());
        yield return (CreatePredicateCall(), CreatePredicateCall());
        yield return (CreateFluentCall(), CreateFluentCall());
        yield return (CreateAnd(), CreateAnd());
        yield return (CreateOr(), CreateOr());
        yield return (CreateForall(), CreateForall());
        yield return (CreateExists(), CreateExists());
        yield return (CreateAndEffect(), CreateAndEffect());
        yield return (CreateForallEffect(), CreateForallEffect());
    }

    private static IEnumerable<(object Original, object Reordered, object Changed, object Shorter)>
        SequenceVariants()
    {
        PredicateDeclaration predicate = CreatePredicateDeclaration();
        yield return (
            predicate,
            predicate with { Parameters = Reverse(predicate.Parameters) },
            predicate with { Parameters = ReplaceSecond(predicate.Parameters, new Parameter("?elsewhere", "place")) },
            predicate with { Parameters = RemoveSecond(predicate.Parameters) });

        FunctionDeclaration function = CreateFunctionDeclaration();
        yield return (
            function,
            function with { Parameters = Reverse(function.Parameters) },
            function with { Parameters = ReplaceSecond(function.Parameters, new Parameter("?elsewhere", "place")) },
            function with { Parameters = RemoveSecond(function.Parameters) });

        ActionDefinition action = CreateActionDefinition();
        yield return (
            action,
            action with { Parameters = Reverse(action.Parameters) },
            action with { Parameters = ReplaceSecond(action.Parameters, new Parameter("?elsewhere", "place")) },
            action with { Parameters = RemoveSecond(action.Parameters) });

        DomainDefinition domain = CreateDomainDefinition();
        yield return (
            domain,
            domain with { Predicates = Reverse(domain.Predicates) },
            domain with
            {
                Predicates = ReplaceSecond(
                    domain.Predicates,
                    new PredicateDeclaration("blocked", CreateParameters()))
            },
            domain with { Predicates = RemoveSecond(domain.Predicates) });

        ProblemDefinition problem = CreateProblemDefinition();
        yield return (
            problem,
            problem with { Objects = Reverse(problem.Objects) },
            problem with { Objects = ReplaceSecond(problem.Objects, new TypeDeclaration("van", "vehicle")) },
            problem with { Objects = RemoveSecond(problem.Objects) });

        PredicateCall predicateCall = CreatePredicateCall();
        yield return (
            predicateCall,
            predicateCall with { Arguments = Reverse(predicateCall.Arguments) },
            predicateCall with { Arguments = ReplaceSecond(predicateCall.Arguments, Term.Constant("other")) },
            predicateCall with { Arguments = RemoveSecond(predicateCall.Arguments) });

        FluentCall fluentCall = CreateFluentCall();
        yield return (
            fluentCall,
            fluentCall with { Arguments = Reverse(fluentCall.Arguments) },
            fluentCall with { Arguments = ReplaceSecond(fluentCall.Arguments, Term.Constant("other")) },
            fluentCall with { Arguments = RemoveSecond(fluentCall.Arguments) });

        And and = CreateAnd();
        yield return (
            and,
            and with { Expressions = Reverse(and.Expressions) },
            and with { Expressions = ReplaceSecond(and.Expressions, new EmptyLogic()) },
            and with { Expressions = RemoveSecond(and.Expressions) });

        Or or = CreateOr();
        yield return (
            or,
            or with { Expressions = Reverse(or.Expressions) },
            or with { Expressions = ReplaceSecond(or.Expressions, new EmptyLogic()) },
            or with { Expressions = RemoveSecond(or.Expressions) });

        Forall forall = CreateForall();
        yield return (
            forall,
            forall with { Variables = Reverse(forall.Variables) },
            forall with { Variables = ReplaceSecond(forall.Variables, new Parameter("?elsewhere", "place")) },
            forall with { Variables = RemoveSecond(forall.Variables) });

        Exists exists = CreateExists();
        yield return (
            exists,
            exists with { Variables = Reverse(exists.Variables) },
            exists with { Variables = ReplaceSecond(exists.Variables, new Parameter("?elsewhere", "place")) },
            exists with { Variables = RemoveSecond(exists.Variables) });

        AndEffect andEffect = CreateAndEffect();
        yield return (
            andEffect,
            andEffect with { Effects = Reverse(andEffect.Effects) },
            andEffect with
            {
                Effects = ReplaceSecond(
                    andEffect.Effects,
                    new AddEffect(new PredicateCall("loaded", CreateTerms())))
            },
            andEffect with { Effects = RemoveSecond(andEffect.Effects) });

        ForallEffect forallEffect = CreateForallEffect();
        yield return (
            forallEffect,
            forallEffect with { Variables = Reverse(forallEffect.Variables) },
            forallEffect with
            {
                Variables = ReplaceSecond(forallEffect.Variables, new Parameter("?elsewhere", "place"))
            },
            forallEffect with { Variables = RemoveSecond(forallEffect.Variables) });
    }

    private static PredicateDeclaration CreatePredicateDeclaration() =>
        new("connected", CreateParameters());

    private static FunctionDeclaration CreateFunctionDeclaration() =>
        new("distance", CreateParameters());

    private static ActionDefinition CreateActionDefinition() =>
        new("move", CreateParameters(), CreateAnd(), CreateAndEffect());

    private static DomainDefinition CreateDomainDefinition() =>
        new(
            "transport",
            ImmutableArray.Create(PddlRequirement.Strips, PddlRequirement.Typing),
            ImmutableArray.Create(new TypeDeclaration("place"), new TypeDeclaration("vehicle")),
            ImmutableArray.Create(new TypeDeclaration("depot", "place"), new TypeDeclaration("truck", "vehicle")),
            ImmutableArray.Create(
                CreatePredicateDeclaration(),
                new PredicateDeclaration("at", CreateParameters())),
            ImmutableArray.Create(
                CreateFunctionDeclaration(),
                new FunctionDeclaration("fuel", CreateParameters())),
            ImmutableArray.Create(CreateActionDefinition()),
            ImmutableArray.Create(
                new DerivedPredicate(
                    new PredicateDeclaration("reachable", CreateParameters()),
                    CreateAnd())));

    private static ProblemDefinition CreateProblemDefinition() =>
        new(
            "delivery",
            "transport",
            ImmutableArray.Create(PddlRequirement.Strips, PddlRequirement.Typing),
            ImmutableArray.Create(new TypeDeclaration("depot", "place"), new TypeDeclaration("lorry", "vehicle")),
            ImmutableArray.Create<IProblemInitElement>(
                CreatePredicateCall(),
                new NumericInitialization(CreateFluentCall(), new NumberLiteral(4))),
            CreateAnd(),
            new Metric(MetricDirection.Minimize, CreateFluentCall()));

    private static PredicateCall CreatePredicateCall() =>
        new("connected", CreateTerms());

    private static FluentCall CreateFluentCall() =>
        new("distance", CreateTerms());

    private static And CreateAnd() =>
        new(ImmutableArray.Create<ILogicalExpression>(
            CreatePredicateCall(),
            new Not(new PredicateCall("blocked", CreateTerms()))));

    private static Or CreateOr() =>
        new(ImmutableArray.Create<ILogicalExpression>(
            CreatePredicateCall(),
            new Equality(Term.Variable("?from"), Term.Constant("depot"))));

    private static Forall CreateForall() =>
        new(CreateParameters(), CreateAnd());

    private static Exists CreateExists() =>
        new(CreateParameters(), CreateOr());

    private static AndEffect CreateAndEffect() =>
        new(ImmutableArray.Create<IEffect>(
            new AddEffect(CreatePredicateCall()),
            new DeleteEffect(new PredicateCall("blocked", CreateTerms()))));

    private static ForallEffect CreateForallEffect() =>
        new(CreateParameters(), CreateAndEffect());

    private static ImmutableArray<Parameter> CreateParameters() =>
        ImmutableArray.Create(new Parameter("?from", "place"), new Parameter("?to", "place"));

    private static ImmutableArray<Term> CreateTerms() =>
        ImmutableArray.Create(Term.Variable("?from"), Term.Constant("depot"));

    private static ImmutableArray<T> Reverse<T>(ImmutableArray<T> values) =>
        ImmutableArray.Create(values[1], values[0]);

    private static ImmutableArray<T> ReplaceSecond<T>(ImmutableArray<T> values, T replacement) =>
        ImmutableArray.Create(values[0], replacement);

    private static ImmutableArray<T> RemoveSecond<T>(ImmutableArray<T> values) =>
        ImmutableArray.Create(values[0]);
}
