using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Builders;
using Mimir.Pddl.Parsing;

namespace Mimir.Pddl.Tests.Parsing;

public class NamePolicyTests
{
    [Theory]
    [InlineData(PddlParseTarget.Domain, "(define (domain d\u00F6main))")]
    [InlineData(
        PddlParseTarget.Problem,
        "(define (problem pr\u00F6blem) (:domain d) (:init) (:goal ()))")]
    [InlineData(PddlParseTarget.PredicateCall, "(pr\u00E9)")]
    [InlineData(PddlParseTarget.LogicalExpression, "(pr\u00E9)")]
    [InlineData(PddlParseTarget.NumericExpression, "(fl\u00FCent)")]
    [InlineData(PddlParseTarget.Effect, "(pr\u00E9)")]
    public void PublicParsersReportUnicodeNamesAsLexicalErrors(
        PddlParseTarget parseTarget,
        string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() => Parse(parseTarget, input));

        Assert.Equal(PddlParseErrorCode.Lexical, exception.ErrorCode);
        Assert.Equal(parseTarget, exception.ParseTarget);
    }

    [Theory]
    [InlineData("(p ?\u00E9)")]
    [InlineData("(p move\u00E9)")]
    [InlineData("(p ?x\u0661)")]
    public void PredicateParserReportsUnicodeTermsAsLexicalErrors(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            PddlParser.ParsePredicateCall(input));

        Assert.Equal(PddlParseErrorCode.Lexical, exception.ErrorCode);
    }

    [Theory]
    [InlineData("\u0661")]
    [InlineData("1\u0661")]
    public void NumericParserReportsUnicodeDigitsAsLexicalErrors(string input)
    {
        PddlParseException exception = Assert.Throws<PddlParseException>(() =>
            PddlParser.ParseNumericExpression(input));

        Assert.Equal(PddlParseErrorCode.Lexical, exception.ErrorCode);
        Assert.Equal(PddlParseTarget.NumericExpression, exception.ParseTarget);
    }

    [Fact]
    public void ParserAcceptsAsciiNameBoundariesAndReservedVariableSuffix()
    {
        PredicateCall call = PddlParser.ParsePredicateCall("(Move_to-2 ?and Location_2)");

        Assert.Equal("Move_to-2", call.Name);
        Assert.True(call.Arguments[0].IsVariable);
        Assert.Equal("Location_2", call.Arguments[1].Name);
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("1name")]
    [InlineData("has space")]
    [InlineData("na\u00EFve")]
    [InlineData("and")]
    [InlineData("DOMAIN")]
    public void NamePolicyRejectsInvalidOrReservedNames(string? name)
    {
        Assert.ThrowsAny<ArgumentException>(() => PddlName.RequireName(name, nameof(name)));
    }

    [Theory]
    [InlineData("A")]
    [InlineData("x1")]
    [InlineData("x-y_z")]
    public void NamePolicyAcceptsValidNames(string name)
    {
        Assert.Equal(name, PddlName.RequireName(name, nameof(name)));
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("x")]
    [InlineData("?1")]
    [InlineData("?na\u00EFve")]
    public void NamePolicyRejectsInvalidVariables(string? name)
    {
        Assert.ThrowsAny<ArgumentException>(() => PddlName.RequireVariable(name, nameof(name)));
    }

    [Theory]
    [InlineData("?and")]
    [InlineData("?X_2-y")]
    public void NamePolicyAcceptsValidVariables(string name)
    {
        Assert.Equal(name, PddlName.RequireVariable(name, nameof(name)));
    }

    [Fact]
    public void BuildersUseTheSharedNamePolicy()
    {
        Assert.Throws<ArgumentException>(() => new DomainBuilder("domain"));
        Assert.Throws<ArgumentException>(() => new ActionBuilder("and"));
        Assert.Throws<ArgumentException>(() => new DomainBuilder("valid").AddType("type", "or"));
        Assert.Throws<ArgumentException>(() => new DomainBuilder("valid").AddPredicate("not"));
        Assert.Throws<ArgumentException>(() => new ActionBuilder("valid").AddParameter("x"));

        var domain = new DomainBuilder("valid-domain")
            .AddAction("move-2", action => action.AddParameter("?and", "object"))
            .Build();

        var action = Assert.Single(domain.Actions);
        Assert.Equal("?and", Assert.Single(action.Parameters).Name);
    }

    private static object Parse(PddlParseTarget parseTarget, string input) => parseTarget switch
    {
        PddlParseTarget.Domain => PddlParser.ParseDomain(input),
        PddlParseTarget.Problem => PddlParser.ParseProblem(input),
        PddlParseTarget.PredicateCall => PddlParser.ParsePredicateCall(input),
        PddlParseTarget.LogicalExpression => PddlParser.ParseLogicalExpression(input),
        PddlParseTarget.NumericExpression => PddlParser.ParseNumericExpression(input),
        PddlParseTarget.Effect => PddlParser.ParseEffect(input),
        _ => throw new ArgumentOutOfRangeException(nameof(parseTarget))
    };
}
