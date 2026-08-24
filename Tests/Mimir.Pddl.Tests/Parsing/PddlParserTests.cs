using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class PddlParserTests
{
    [Fact]
    public void ParsesPredicateCall()
    {
        var result = PddlParser.ParsePredicateCall("(at ?truck location-1)");

        Assert.Equal("at", result.Name);
        Assert.Equal(2, result.Arguments.Length);
        
        Assert.Equal("?truck", result.Arguments[0].Name);
        Assert.True(result.Arguments[0].IsVariable);

        Assert.Equal("location-1", result.Arguments[1].Name);
        Assert.False(result.Arguments[1].IsVariable);
        
    }

    [Fact]
    public void ParseDomain_UnsupportedDurativeAction_ThrowsParseException()
    {
        var ex = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain("""
(define (domain unsupported-domain)
  (:durative-action wait)
)
"""));

        Assert.Equal(PddlParseTarget.Domain, ex.ParseTarget);
        Assert.Contains("Unsupported PDDL domain construct ':durative-action'", ex.Message);
    }

    [Fact]
    public void ParseProblem_UnsupportedConstraints_ThrowsParseException()
    {
        var ex = Assert.Throws<PddlParseException>(() => PddlParser.ParseProblem("""
(define (problem unsupported-problem)
  (:domain unsupported-domain)
  (:constraints (always (ready)))
)
"""));

        Assert.Equal(PddlParseTarget.Problem, ex.ParseTarget);
        Assert.Contains("Unsupported PDDL problem construct ':constraints'", ex.Message);
    }

    [Fact]
    public void ParseDomain_UnknownTopLevelSection_NamesUnexpectedConstruct()
    {
        var ex = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain("""
(define (domain unknown-section)
  (:mystery value)
)
"""));

        Assert.Equal(PddlParseTarget.Domain, ex.ParseTarget);
        Assert.Contains("Unknown PDDL domain construct ':mystery'", ex.Message);
    }

    [Fact]
    public void ParseDomain_MalformedActionMissingEffectBody_ThrowsParseException()
    {
        var ex = Assert.Throws<PddlParseException>(() => PddlParser.ParseDomain("""
(define (domain malformed-action)
  (:predicates (ready))
  (:action bad
    :parameters ()
    :effect)
)
"""));

        Assert.Equal(PddlParseTarget.Domain, ex.ParseTarget);
        Assert.Equal(PddlParseErrorCode.Syntax, ex.ErrorCode);
    }

    [Fact]
    public void ParseProblem_MalformedMetric_ThrowsParseException()
    {
        var ex = Assert.Throws<PddlParseException>(() => PddlParser.ParseProblem("""
(define (problem malformed-metric)
  (:domain d)
  (:metric minimize)
)
"""));

        Assert.Equal(PddlParseTarget.Problem, ex.ParseTarget);
        Assert.Equal(PddlParseErrorCode.Syntax, ex.ErrorCode);
    }

    [Theory]
    [InlineData("\n")]
    [InlineData("\r\n")]
    [InlineData("\r")]
    [InlineData("\v")]
    [InlineData("\f")]
    [InlineData("\u0085")]
    [InlineData("\u2028")]
    [InlineData("\u2029")]
    public void ParsePredicateCall_ReportsPositionAfterCommentLineBreak(string lineBreak)
    {
        string input = $";c{lineBreak}(p ?1)";

        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParsePredicateCall(input));

        int offset = input.IndexOf("?1", StringComparison.Ordinal);
        Assert.Equal(PddlParseErrorCode.Lexical, exception.ErrorCode);
        Assert.Equal(new PddlSourceSpan(offset, 2, 2, 4), exception.Span);
    }

    [Theory]
    [InlineData("+1")]
    [InlineData("-1")]
    [InlineData(".5")]
    [InlineData("+.5")]
    [InlineData("-.5")]
    [InlineData("+1.5")]
    [InlineData("-1.5")]
    public void ParsePredicateCall_ReportsFullNumericTokenSpan(string number)
    {
        string input = $";c\r\n(p {number})";

        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParsePredicateCall(input));

        int offset = input.IndexOf(number, StringComparison.Ordinal);
        Assert.Equal(PddlParseErrorCode.Syntax, exception.ErrorCode);
        Assert.Equal(new PddlSourceSpan(offset, number.Length, 2, 4), exception.Span);
    }

    [Fact]
    public void ParseDomain_ReportsGrammarPositionAfterUnicodeLineBreak()
    {
        const string input = "(define (domain d)\u2029(:mystery value))";

        PddlParseException exception = Assert.Throws<PddlParseException>(
            () => PddlParser.ParseDomain(input));

        int offset = input.IndexOf(":mystery", StringComparison.Ordinal);
        Assert.Equal(PddlParseErrorCode.UnknownConstruct, exception.ErrorCode);
        Assert.Equal(new PddlSourceSpan(offset, 8, 2, 2), exception.Span);
    }
}
