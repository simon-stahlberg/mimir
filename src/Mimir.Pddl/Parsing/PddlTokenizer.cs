using Superpower;
using Superpower.Model;
using Superpower.Parsers;
using Superpower.Tokenizers;

namespace Mimir.Pddl.Parsing;

internal static class PddlTokenizer
{
    private static readonly TextParser<char> AsciiLetter =
        Character.Matching(PddlName.IsAsciiLetter, "ASCII letter");

    private static readonly TextParser<char> AsciiDigit =
        Character.Matching(PddlName.IsAsciiDigit, "ASCII digit");

    private static readonly TextParser<char> NameCharacter =
        Character.Matching(PddlName.IsNameCharacter, "ASCII letter, digit, '-' or '_'");

    private static readonly TextParser<TextSpan> CommentParser =
        Span.MatchedBy(
            Character.EqualTo(';')
            .IgnoreThen(Character.ExceptIn('\r', '\n').Many())
        );

    private static readonly TextParser<TextSpan> IdentifierParser =
        Span.MatchedBy(
            Character.EqualTo(':').OptionalOrDefault().IgnoreThen(
                AsciiLetter.IgnoreThen(NameCharacter.Many())
            )
        );

    private static readonly TextParser<TextSpan> VariableParser =
        Span.MatchedBy(
            Character.EqualTo('?').IgnoreThen(
                AsciiLetter.IgnoreThen(NameCharacter.Many()))
        );

    private static readonly TextParser<TextSpan> NumberParser =
        Span.MatchedBy(
            Character.In('+', '-').OptionalOrDefault().IgnoreThen(
                AsciiDigit.AtLeastOnce().IgnoreThen(
                    Character.EqualTo('.')
                        .IgnoreThen(AsciiDigit.AtLeastOnce())
                        .Value(Unit.Value)
                        .OptionalOrDefault())
                .Value(Unit.Value)
                .Or(Character.EqualTo('.')
                    .IgnoreThen(AsciiDigit.AtLeastOnce())
                    .Value(Unit.Value)))
        );

    public static TokenList<PddlToken> Tokenize(string text)
    {
        ArgumentNullException.ThrowIfNull(text);

        string normalizedText = NormalizeLineBreaks(text);
        return Create().Tokenize(normalizedText);
    }

    private static string NormalizeLineBreaks(string text)
    {
        char[]? normalizedText = null;

        for (int index = 0; index < text.Length; index++)
        {
            char character = text[index];
            bool isCrLf = character == '\r'
                && index + 1 < text.Length
                && text[index + 1] == '\n';

            if (isCrLf
                || character is not ('\r' or '\v' or '\f' or '\u0085' or '\u2028' or '\u2029'))
            {
                continue;
            }

            // Keeping every replacement one UTF-16 code unit preserves source offsets.
            normalizedText ??= text.ToCharArray();
            normalizedText[index] = '\n';
        }

        return normalizedText == null ? text : new string(normalizedText);
    }

    private static Tokenizer<PddlToken> Create()
    {
        return new TokenizerBuilder<PddlToken>()
            .Ignore(Span.WhiteSpace)
            .Ignore(CommentParser)
            .Match(Character.EqualTo('('), PddlToken.OpenParen)
            .Match(Character.EqualTo(')'), PddlToken.CloseParen)
            .Match(NumberParser, PddlToken.Number)
            .Match(Character.EqualTo('-'), PddlToken.Dash)
            .Match(Character.EqualTo('='), PddlToken.Equals)
            .Match(Character.EqualTo('+'), PddlToken.Plus)
            .Match(Character.EqualTo('*'), PddlToken.Asterisk)
            .Match(Character.EqualTo('/'), PddlToken.Slash)
            .Match(Span.EqualTo("<="), PddlToken.LessThanOrEqual)
            .Match(Span.EqualTo(">="), PddlToken.GreaterThanOrEqual)
            .Match(Character.EqualTo('<'), PddlToken.LessThan)
            .Match(Character.EqualTo('>'), PddlToken.GreaterThan)
            .Match(VariableParser, PddlToken.Variable)
            .Match(IdentifierParser, PddlToken.Identifier)
            .Build();
    }
}
