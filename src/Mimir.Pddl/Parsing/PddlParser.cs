using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Superpower;
using Superpower.Model;

namespace Mimir.Pddl.Parsing;

public enum PddlParseTarget
{
    Domain,
    Problem,
    PredicateCall,
    LogicalExpression,
    NumericExpression,
    Effect
}

public enum PddlParseErrorCode
{
    Lexical,
    Syntax,
    UnknownConstruct,
    UnsupportedFeature,
    DuplicateSection,
    MissingSection,
    InvalidNumber
}

public readonly record struct PddlSourceSpan(int Offset, int Length, int Line, int Column);

public sealed class PddlParseException : Exception
{
    public PddlParseTarget ParseTarget { get; }
    public PddlParseErrorCode ErrorCode { get; }
    public PddlSourceSpan Span { get; }

    public PddlParseException(
        PddlParseTarget parseTarget,
        PddlParseErrorCode errorCode,
        string message,
        PddlSourceSpan span,
        Exception? innerException = null)
        : base(FormatMessage(parseTarget, errorCode, message, span), innerException)
    {
        ParseTarget = parseTarget;
        ErrorCode = errorCode;
        Span = span;
    }

    private static string FormatMessage(
        PddlParseTarget parseTarget,
        PddlParseErrorCode errorCode,
        string message,
        PddlSourceSpan span)
    {
        return $"Failed to parse {parseTarget} ({errorCode}) at line {span.Line}, column {span.Column}: {message}";
    }
}

internal sealed class PddlGrammarException : Exception
{
    internal PddlParseErrorCode ErrorCode { get; }
    internal PddlSourceSpan Span { get; }

    internal PddlGrammarException(PddlParseErrorCode errorCode, string message, PddlSourceSpan span)
        : base(message)
    {
        ErrorCode = errorCode;
        Span = span;
    }
}

public static class PddlParser
{
    public static DomainDefinition ParseDomain(string text)
        => ParseText(text, PddlGrammar.DomainDecl, PddlParseTarget.Domain);

    public static ProblemDefinition ParseProblem(string text)
        => ParseText(text, PddlGrammar.ProblemDecl, PddlParseTarget.Problem);

    public static PredicateCall ParsePredicateCall(string text)
        => ParseText(text, PddlGrammar.PredicateCall, PddlParseTarget.PredicateCall);

    public static ILogicalExpression ParseLogicalExpression(string text)
        => ParseText(text, PddlGrammar.LogicalExpression, PddlParseTarget.LogicalExpression);

    public static INumericExpression ParseNumericExpression(string text)
        => ParseText(text, PddlGrammar.NumericExpression, PddlParseTarget.NumericExpression);

    public static IEffect ParseEffect(string text)
        => ParseText(text, PddlGrammar.Effect, PddlParseTarget.Effect);

    private static T ParseText<T>(
        string text,
        TokenListParser<PddlToken, T> parser,
        PddlParseTarget parseTarget)
    {
        ArgumentNullException.ThrowIfNull(text);

        TokenList<PddlToken> tokens;
        try
        {
            tokens = PddlTokenizer.Tokenize(text);
        }
        catch (ParseException exception)
        {
            throw CreateException(
                parseTarget,
                PddlParseErrorCode.Lexical,
                exception.Message,
                exception.ErrorPosition,
                text,
                exception);
        }

        try
        {
            return parser.AtEnd().Parse(tokens);
        }
        catch (PddlGrammarException exception)
        {
            throw new PddlParseException(
                parseTarget,
                exception.ErrorCode,
                exception.Message,
                exception.Span,
                exception);
        }
        catch (ParseException exception)
        {
            throw CreateException(
                parseTarget,
                PddlParseErrorCode.Syntax,
                exception.Message,
                exception.ErrorPosition,
                text,
                exception,
                tokens);
        }
    }

    private static PddlParseException CreateException(
        PddlParseTarget parseTarget,
        PddlParseErrorCode errorCode,
        string message,
        Position position,
        string text,
        Exception innerException,
        TokenList<PddlToken>? tokens = null)
    {
        Position errorPosition = position.HasValue ? position : Position.Zero;
        int length = MeasureTokenAt(text, errorPosition.Absolute);

        if (tokens is { } tokenList)
        {
            foreach (Token<PddlToken> token in tokenList)
            {
                if (token.Position.Absolute != errorPosition.Absolute)
                    continue;

                length = token.Span.Length;
                break;
            }
        }

        var span = new PddlSourceSpan(
            errorPosition.Absolute,
            length,
            errorPosition.Line,
            errorPosition.Column);

        return new PddlParseException(parseTarget, errorCode, message, span, innerException);
    }

    private static int MeasureTokenAt(string text, int offset)
    {
        if (offset < 0 || offset >= text.Length)
            return 0;

        char first = text[offset];
        if (first is '(' or ')' or '=' or '+' or '*' or '/')
            return 1;

        if (first is '<' or '>')
            return offset + 1 < text.Length && text[offset + 1] == '=' ? 2 : 1;

        if (first == '-' && (offset + 1 >= text.Length || !PddlName.IsAsciiDigit(text[offset + 1])))
            return 1;

        if (first == '?' || first == ':' || first == '-' || PddlName.IsNameCharacter(first))
        {
            int end = offset + 1;
            while (end < text.Length && IsTokenCharacter(text[end]))
                end++;

            return end - offset;
        }

        return 1;
    }

    private static bool IsTokenCharacter(char character)
    {
        return PddlName.IsNameCharacter(character) || character is '.' or '?';
    }
}
