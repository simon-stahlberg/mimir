using Mimir.Pddl.Parsing;
using Superpower;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class PddlTokenizerTests
{
    [Fact]
    public void Tokenizer_IgnoresWhitespaceAndComments()
    {
        var result = PddlTokenizer.Tokenize("  ; this is a comment\n  (  )").ToArray();

        Assert.Equal(2, result.Length);
        Assert.Equal(PddlToken.OpenParen, result.ElementAt(0).Kind);
        Assert.Equal(PddlToken.CloseParen, result.ElementAt(1).Kind);
    }

    [Fact]
    public void Tokenizer_ParsesIdentifiersAndVariables()
    {
        var result = PddlTokenizer.Tokenize("(move-to ?loc-1)").ToArray();

        Assert.Equal(4, result.Length); // '(', 'move-to', '?loc-1', ')'
        
        var tokens = result.ToArray();
        Assert.Equal(PddlToken.OpenParen, tokens[0].Kind);
        
        Assert.Equal(PddlToken.Identifier, tokens[1].Kind);
        Assert.Equal("move-to", tokens[1].ToStringValue());
        
        Assert.Equal(PddlToken.Variable, tokens[2].Kind);
        Assert.Equal("?loc-1", tokens[2].ToStringValue());
        
        Assert.Equal(PddlToken.CloseParen, tokens[3].Kind);
    }

    [Fact]
    public void Tokenizer_IgnoresSupportedUnicodeWhitespace()
    {
        const string whitespace =
            "\u0009\u000A\u000B\u000C\u000D\u0020\r\n\u0085\u00A0\u1680"
            + "\u2000\u2001\u2002\u2003\u2004\u2005\u2006\u2007\u2008\u2009\u200A"
            + "\u2028\u2029\u202F\u205F\u3000";

        var tokens = PddlTokenizer.Tokenize($"({whitespace})").ToArray();

        Assert.Equal(2, tokens.Length);
        Assert.Equal(PddlToken.OpenParen, tokens[0].Kind);
        Assert.Equal(PddlToken.CloseParen, tokens[1].Kind);
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
    public void Tokenizer_CommentsEndAtSupportedLineBreaks(string lineBreak)
    {
        string input = $"; comment{lineBreak}(p)";

        var tokens = PddlTokenizer.Tokenize(input).ToArray();

        Assert.Equal(3, tokens.Length);
        Assert.Equal(PddlToken.OpenParen, tokens[0].Kind);
        Assert.Equal(PddlToken.Identifier, tokens[1].Kind);
        Assert.Equal(PddlToken.CloseParen, tokens[2].Kind);
        Assert.Equal(input.IndexOf('('), tokens[0].Position.Absolute);
        Assert.Equal(2, tokens[0].Position.Line);
        Assert.Equal(1, tokens[0].Position.Column);
    }
}
