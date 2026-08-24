using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class StrictParserTests
{
    [Theory]
    [InlineData("domain", "(define (domain d))")]
    [InlineData("problem", "(define (problem p) (:domain d) (:init) (:goal ()))")]
    [InlineData("predicate", "(p)")]
    [InlineData("logical", "()")]
    [InlineData("numeric", "1")]
    [InlineData("effect", "(p)")]
    public void PublicParsers_RequireEndOfInput(string construct, string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => Parse(construct, $"{input} (trailing)"));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(input.Length + 1, exception.Span.Offset);
        Assert.Equal(1, exception.Span.Length);
    }

    [Theory]
    [InlineData("domain", "(define (domain d))")]
    [InlineData("problem", "(define (problem p) (:domain d) (:init) (:goal ()))")]
    [InlineData("predicate", "(p)")]
    [InlineData("logical", "()")]
    [InlineData("numeric", "1")]
    [InlineData("effect", "(p)")]
    public void PublicParsers_AllowTrailingWhitespaceAndComments(string construct, string input)
    {
        Parse(construct, $"{input}  ; trailing comment\n\t");
    }

    [Theory]
    [InlineData(":requirements :strips")]
    [InlineData(":types item")]
    [InlineData(":constants item")]
    [InlineData(":predicates (p)")]
    [InlineData(":functions (f)")]
    public void Domain_RejectsDuplicateSingletonSections(string section)
    {
        string input = $"(define (domain d) ({section}) ({section}))";

        PddlParseException exception = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain(input));

        Assert.Equal(PddlParseErrorCode.DuplicateSection, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Domain, exception.ParseTarget);
    }

    [Theory]
    [InlineData("(:domain d)")]
    [InlineData("(:requirements :strips)")]
    [InlineData("(:objects item)")]
    [InlineData("(:init)")]
    [InlineData("(:goal ())")]
    [InlineData("(:metric minimize 0)")]
    public void Problem_RejectsDuplicateSingletonSections(string section)
    {
        string input = $"(define (problem p) (:domain d) (:init) (:goal ()) {section} {section})";

        PddlParseException exception = Assert.Throws<PddlParseException>(() => PddlParser.ParseProblem(input));

        Assert.Equal(PddlParseErrorCode.DuplicateSection, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.Problem, exception.ParseTarget);
    }

    [Theory]
    [InlineData("(:domain d) (:goal ())", ":init")]
    [InlineData("(:domain d) (:init)", ":goal")]
    [InlineData("(:init) (:goal ())", ":domain")]
    public void Problem_RequiresMandatorySections(string sections, string missingSection)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseProblem($"(define (problem p) {sections})"));

        Assert.Equal(PddlParseErrorCode.MissingSection, exception.ErrorCode);
        Assert.Contains(missingSection, exception.Message);
    }

    [Fact]
    public void Action_RequiresExactlyOneParametersSection_AndAllowsEmptyParameters()
    {
        var domain = PddlParser.ParseDomain("(define (domain d) (:action a :parameters ()))");
        ActionDefinition action = Assert.Single(domain.Actions);
        Assert.Empty(action.Parameters);
        Assert.Null(action.Precondition);
        Assert.Null(action.Effect);

        PddlParseException missing = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain("(define (domain d) (:action a))"));
        Assert.Equal(PddlParseErrorCode.MissingSection, missing.ErrorCode);

        PddlParseException duplicate = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain(
                "(define (domain d) (:action a :parameters () :parameters ()))"));
        Assert.Equal(PddlParseErrorCode.DuplicateSection, duplicate.ErrorCode);
    }

    [Theory]
    [InlineData("(define (domain d) (:types - object))")]
    [InlineData("(define (domain d) (:action a :parameters (- object)))")]
    public void TypedLists_RequireAtLeastOneNameBeforeDash(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain(input));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
    }

    [Theory]
    [InlineData("(p ?)", 3, 1)]
    [InlineData("(p ?1)", 3, 2)]
    public void Variables_MustContainAValidName(string input, int offset, int length)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParsePredicateCall(input));

        Assert.Equal(PddlParseErrorCode.Lexical, exception.ErrorCode);
        Assert.Equal(new PddlSourceSpan(offset, length, 1, offset + 1), exception.Span);
    }

    [Fact]
    public void MalformedReservedForms_DoNotFallBackToPredicateCalls()
    {
        PddlParseException predicate = Assert.Throws<PddlParseException>(
            () => PddlParser.ParsePredicateCall("(and)"));
        Assert.Equal(PddlParseErrorCode.Syntax, predicate.ErrorCode);

        PddlParseException logical = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseLogicalExpression("(not)"));
        Assert.Equal(PddlParseErrorCode.Syntax, logical.ErrorCode);

        PddlParseException effect = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseEffect("(increase)"));
        Assert.Equal(PddlParseErrorCode.Syntax, effect.ErrorCode);
    }

    [Fact]
    public void Requirements_AreCaseInsensitiveAndNormalizeFluentsAlias()
    {
        var domain = PddlParser.ParseDomain(
            "(define (domain d) (:requirements :StRiPs :FLUENTS :AdL))");

        HashSet<PddlRequirement> expectedRequirements =
        [
            PddlRequirement.Strips,
            PddlRequirement.NumericFluents,
            PddlRequirement.Adl
        ];

        Assert.True(expectedRequirements.SetEquals(domain.Requirements));
    }

    [Fact]
    public void UnsupportedRequirement_HasStructuredDiagnostic()
    {
        const string input = "(define (domain d) (:requirements :durative-actions))";

        PddlParseException exception = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain(input));

        Assert.Equal(PddlParseErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Equal(input.IndexOf(":durative-actions", StringComparison.Ordinal), exception.Span.Offset);
        Assert.Equal(":durative-actions".Length, exception.Span.Length);
    }

    [Fact]
    public void FunctionResultTypes_AreOptionalOrNumberOnly()
    {
        var domain = PddlParser.ParseDomain(
            "(define (domain d) (:functions (f) (g) - NUMBER))");
        Assert.Equal(2, domain.Functions.Length);

        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain("(define (domain d) (:functions (f) - object))"));
        Assert.Equal(PddlParseErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Fact]
    public void UnionTypes_AreRecognizedAsUnsupported()
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain(
                "(define (domain d) (:types item - (either physical abstract)))"));

        Assert.Equal(PddlParseErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("either", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Theory]
    [InlineData("(always (ready))")]
    [InlineData("(preference p (ready))")]
    [InlineData("(sometime (ready))")]
    public void RecognizedUnsupportedExpressions_HaveStructuredDiagnostics(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseLogicalExpression(input));

        Assert.Equal(PddlParseErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Theory]
    [InlineData("9007199254740993")]
    [InlineData("79228162514264337593543950335")]
    [InlineData("-79228162514264337593543950335")]
    [InlineData("+1.25")]
    [InlineData("-1.25")]
    [InlineData(".5")]
    [InlineData("-.5")]
    public void DecimalLiterals_PreserveExactValuesThroughSerialization(string input)
    {
        var literal = Assert.IsType<NumberLiteral>(PddlParser.ParseNumericExpression(input));

        NumberLiteral reparsed = Assert.IsType<NumberLiteral>(
            PddlParser.ParseNumericExpression(literal.ToPddlString()));
        Assert.Equal(literal.Value, reparsed.Value);
    }

    [Theory]
    [InlineData("79228162514264337593543950336")]
    [InlineData("0.00000000000000000000000000001")]
    public void DecimalLiterals_RejectValuesOutsideExactDecimalRange(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseNumericExpression(input));

        Assert.Equal(PddlParseErrorCode.InvalidNumber, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.NumericExpression, exception.ParseTarget);
        Assert.Equal(new PddlSourceSpan(0, input.Length, 1, 1), exception.Span);
    }

    [Fact]
    public void ExponentNotation_IsRejected()
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseNumericExpression("1e3"));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(1, exception.Span.Offset);
    }

    [Fact]
    public void Minus_ParsesUnaryAndBinaryForms()
    {
        var negate = Assert.IsType<Negate>(PddlParser.ParseNumericExpression("(- (fuel))"));
        Assert.IsType<FluentCall>(negate.Operand);

        var subtract = Assert.IsType<Subtract>(PddlParser.ParseNumericExpression("(- 4 1)"));
        Assert.Equal(4m, Assert.IsType<NumberLiteral>(subtract.Left).Value);
        Assert.Equal(1m, Assert.IsType<NumberLiteral>(subtract.Right).Value);
    }

    [Theory]
    [InlineData("(-)")]
    [InlineData("(- 1 2 3)")]
    [InlineData("(+)")]
    [InlineData("(+ 1)")]
    [InlineData("(*)")]
    [InlineData("(* 1)")]
    public void NumericOperators_RejectUnsupportedArities(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseNumericExpression(input));

        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.NumericExpression, exception.ParseTarget);
    }

    [Fact]
    public void BareZeroArityFluents_RoundTripAsFluentCalls()
    {
        var fluent = Assert.IsType<FluentCall>(PddlParser.ParseNumericExpression("fuel"));

        Assert.Empty(fluent.Arguments);
        Assert.Equal(fluent, Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(fluent.ToPddlString())));
    }

    [Fact]
    public void Equality_PreservesBareNameAmbiguityAndClassifiesExplicitOperands()
    {
        Assert.IsType<AmbiguousEquality>(PddlParser.ParseLogicalExpression("(= left right)"));

        var numeric = Assert.IsType<Comparison>(
            PddlParser.ParseLogicalExpression("(= (left) right)"));
        Assert.Equal(ComparisonOperator.Equal, numeric.Operator);
        Assert.IsType<FluentCall>(numeric.Right);

        Assert.IsType<Equality>(PddlParser.ParseLogicalExpression("(= ?left right)"));
    }

    [Fact]
    public void ProblemInit_AllowsNegativeFactsAndLiteralNumericValuesOnly()
    {
        var problem = PddlParser.ParseProblem(
            "(define (problem p) (:domain d) (:init (not (ready)) (= fuel -1.5)) (:goal ()))");

        Assert.Equal(2, problem.Init.Length);
        Assert.Single(problem.Init.OfType<NegativePredicateInitialization>());
        NumericInitialization numeric = Assert.Single(problem.Init.OfType<NumericInitialization>());
        Assert.Equal(-1.5m, numeric.Value.Value);
        Assert.Equal(numeric.Fluent, Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(numeric.Fluent.ToPddlString())));

        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseProblem(
                "(define (problem p) (:domain d) (:init (= (fuel) (+ 1 2))) (:goal ()))"));
        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
    }

    [Fact]
    public void StructuredDiagnostics_ReportExactSpans()
    {
        const string duplicateInput = "(define (domain d)\n  (:types a)\n  (:types b))";
        int duplicateOffset = duplicateInput.LastIndexOf(":types", StringComparison.Ordinal);
        PddlParseException duplicate = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain(duplicateInput));
        Assert.Equal(PddlParseErrorCode.DuplicateSection, duplicate.ErrorCode);
        Assert.Equal(new PddlSourceSpan(duplicateOffset, 6, 3, 4), duplicate.Span);

        const string missingInput = "(define (problem p) (:domain d) (:init))";
        PddlParseException missing = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseProblem(missingInput));
        Assert.Equal(PddlParseErrorCode.MissingSection, missing.ErrorCode);
        Assert.Equal(new PddlSourceSpan(missingInput.Length - 1, 1, 1, missingInput.Length), missing.Span);

        const string unknownInput = "(define (domain d) (:mystery value))";
        int unknownOffset = unknownInput.IndexOf(":mystery", StringComparison.Ordinal);
        PddlParseException unknown = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain(unknownInput));
        Assert.Equal(PddlParseErrorCode.UnknownConstruct, unknown.ErrorCode);
        Assert.Equal(new PddlSourceSpan(unknownOffset, 8, 1, unknownOffset + 1), unknown.Span);

        PddlParseException syntax = Assert.Throws<PddlParseException>(
            () => PddlParser.ParsePredicateCall("(p"));
        Assert.Equal(PddlParseErrorCode.Syntax, syntax.ErrorCode);
        Assert.Equal(new PddlSourceSpan(2, 0, 1, 3), syntax.Span);
    }

    private static object Parse(string construct, string input) => construct switch
    {
        "domain" => PddlParser.ParseDomain(input),
        "problem" => PddlParser.ParseProblem(input),
        "predicate" => PddlParser.ParsePredicateCall(input),
        "logical" => PddlParser.ParseLogicalExpression(input),
        "numeric" => PddlParser.ParseNumericExpression(input),
        "effect" => PddlParser.ParseEffect(input),
        _ => throw new ArgumentOutOfRangeException(nameof(construct))
    };
}
