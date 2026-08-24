using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Pddl.Tests.Parsing;

public class GrammarRegressionTests
{
    [Fact]
    public void EmptyEffectParsesAsEmptyConjunction()
    {
        AndEffect effect = Assert.IsType<AndEffect>(PddlParser.ParseEffect("()"));

        Assert.Empty(effect.Effects);
        AndEffect reparsed = Assert.IsType<AndEffect>(PddlParser.ParseEffect(effect.ToPddlString()));
        Assert.Empty(reparsed.Effects);
    }

    [Fact]
    public void ActionEmptyEffectRoundTrips()
    {
        DomainDefinition domain = PddlParser.ParseDomain(
            "(define (domain no-op) (:action wait :parameters () :effect ()))");

        Assert.Empty(Assert.IsType<AndEffect>(Assert.Single(domain.Actions).Effect).Effects);

        DomainDefinition reparsed = PddlParser.ParseDomain(domain.ToPddlString());
        Assert.Empty(Assert.IsType<AndEffect>(Assert.Single(reparsed.Actions).Effect).Effects);
    }

    [Theory]
    [InlineData("(:init (p ?x))")]
    [InlineData("(:init (not (p ?x)))")]
    [InlineData("(:init (= (f ?x) 1))")]
    public void ProblemInitRejectsVariables(string section)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            PddlParser.ParseProblem(
                $"(define (problem p) (:domain d) {section} (:goal ()))"));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Problem, exception.ParseTarget);
    }

    [Theory]
    [InlineData("(f ?x)")]
    [InlineData("(+ (f ?x) 1)")]
    [InlineData("(- (f ?x) 1)")]
    [InlineData("(- (f ?x))")]
    [InlineData("(* (f ?x) 2)")]
    [InlineData("(/ (f ?x) 2)")]
    [InlineData("(+ 1 2 (f ?x))")]
    [InlineData("(* 1 (f ?x) 2)")]
    public void ProblemMetricRejectsVariablesAtEveryDepth(string expression)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            PddlParser.ParseProblem(
                $"(define (problem p) (:domain d) (:init) (:goal ()) (:metric minimize {expression}))"));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Problem, exception.ParseTarget);
    }

    [Fact]
    public void ProblemAcceptsGroundInitAndNestedGroundMetric()
    {
        ProblemDefinition problem = PddlParser.ParseProblem(
            """
            (define (problem p)
              (:domain d)
              (:init (p a) (not (q b)) (= (f a) 1))
              (:goal ())
              (:metric minimize (+ (- (* (f a) 2) 1) (/ (- (g b)) 2))))
            """);

        Assert.Equal(3, problem.Init.Length);
        Assert.IsType<Add>(problem.Metric!.Expression);
    }

    [Fact]
    public void ProblemMetricNaryAddAndMultiplyPreserveAllOperandsAfterRoundTrip()
    {
        ProblemDefinition problem = ParseProblemWithMetric("(+ 1 (* 2 3 4) 5)");
        INumericExpression expression = Assert.IsType<Add>(problem.Metric!.Expression);
        Assert.True(ContainsMultiply(expression));

        ProblemDefinition reparsed = PddlParser.ParseProblem(problem.ToPddlString());
        Assert.Equal(expression, reparsed.Metric!.Expression);
    }

    [Fact]
    public void ProblemMetricMinusParsesUnaryAndBinaryArities()
    {
        Negate negate = Assert.IsType<Negate>(ParseProblemWithMetric("(- 4)").Metric!.Expression);
        AssertNumber(4m, negate.Operand);
        Assert.Equal(negate, Assert.IsType<Negate>(PddlParser.ParseNumericExpression(negate.ToPddlString())));

        Subtract subtract = Assert.IsType<Subtract>(ParseProblemWithMetric("(- 4 1)").Metric!.Expression);
        AssertNumber(4m, subtract.Left);
        AssertNumber(1m, subtract.Right);
        Assert.Equal(subtract, Assert.IsType<Subtract>(PddlParser.ParseNumericExpression(subtract.ToPddlString())));
    }

    [Theory]
    [InlineData("(-)")]
    [InlineData("(- 1 2 3)")]
    [InlineData("(+)")]
    [InlineData("(+ 1)")]
    [InlineData("(*)")]
    [InlineData("(* 1)")]
    public void ProblemMetricNumericOperatorsRejectUnsupportedArities(string expression)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            ParseProblemWithMetric(expression));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Problem, exception.ParseTarget);
    }

    [Theory]
    [InlineData("total-time", "total-time")]
    [InlineData("(+ 1 ToTaL-TiMe 2)", "ToTaL-TiMe")]
    [InlineData("(* 1 (+ 2 total-time 3) 4)", "total-time")]
    public void ProblemMetricRejectsBareTotalTimeAtExactSpan(string expression, string token)
    {
        string input = ProblemWithMetric(expression);
        int offset = input.IndexOf(token, StringComparison.Ordinal);

        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            PddlParser.ParseProblem(input));

        Assert.Equal(PddlParseErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Problem, exception.ParseTarget);
        Assert.Equal(new PddlSourceSpan(offset, token.Length, 1, offset + 1), exception.Span);
    }

    [Fact]
    public void ParenthesizedTotalTimeRemainsAFluentCallInProblemMetrics()
    {
        FluentCall fluent = Assert.IsType<FluentCall>(
            ParseProblemWithMetric("(ToTaL-TiMe)").Metric!.Expression);

        Assert.Equal("ToTaL-TiMe", fluent.Name);
        Assert.Empty(fluent.Arguments);
        Assert.Equal(fluent, Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(fluent.ToPddlString())));
    }

    [Fact]
    public void BareNonTemporalMetricFluentRemainsAFluentCall()
    {
        FluentCall fluent = Assert.IsType<FluentCall>(
            ParseProblemWithMetric("fuel").Metric!.Expression);

        Assert.Equal("fuel", fluent.Name);
        Assert.Empty(fluent.Arguments);
        Assert.Equal(fluent, Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(fluent.ToPddlString())));
    }

    [Fact]
    public void StandaloneParsersStillAllowVariables()
    {
        Assert.True(Assert.Single(PddlParser.ParsePredicateCall("(p ?x)").Arguments).IsVariable);
        Assert.True(Assert.Single(
            Assert.IsType<FluentCall>(PddlParser.ParseNumericExpression("(f ?x)")).Arguments).IsVariable);
    }

    [Theory]
    [InlineData("(define (domain d) (:requirements))", PddlParseTarget.Domain)]
    [InlineData("(define (domain d) (:predicates))", PddlParseTarget.Domain)]
    [InlineData(
        "(define (problem p) (:domain d) (:requirements) (:init) (:goal ()))",
        PddlParseTarget.Problem)]
    public void ExplicitEmptyRequiredSectionsAreRejected(string input, PddlParseTarget parseTarget)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
        {
            if (parseTarget == PddlParseTarget.Domain)
                PddlParser.ParseDomain(input);
            else
                PddlParser.ParseProblem(input);
        });

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(parseTarget, exception.ParseTarget);
    }

    [Fact]
    public void RequirementsAndPredicatesSectionsMayBeOmitted()
    {
        Assert.Empty(PddlParser.ParseDomain("(define (domain d))").Requirements);
        Assert.Empty(PddlParser.ParseDomain("(define (domain d))").Predicates);
        Assert.Empty(PddlParser.ParseProblem(
            "(define (problem p) (:domain d) (:init) (:goal ()))").Requirements);
    }

    private static ProblemDefinition ParseProblemWithMetric(string expression)
    {
        return PddlParser.ParseProblem(ProblemWithMetric(expression));
    }

    private static string ProblemWithMetric(string expression)
    {
        return $"(define (problem p) (:domain d) (:init) (:goal ()) (:metric minimize {expression}))";
    }

    private static void AssertNumber(decimal expected, INumericExpression expression)
    {
        Assert.Equal(expected, Assert.IsType<NumberLiteral>(expression).Value);
    }

    private static bool ContainsMultiply(INumericExpression expression)
    {
        return expression switch
        {
            Multiply => true,
            Add add => ContainsMultiply(add.Left) || ContainsMultiply(add.Right),
            _ => false
        };
    }
}
