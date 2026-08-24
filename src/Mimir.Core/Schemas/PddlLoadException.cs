using Mimir.Pddl.Analysis;
using Mimir.Pddl.Parsing;

namespace Mimir.Core.Schemas;

public enum PddlDocumentType
{
    Domain,
    Problem
}

public enum PddlLoadErrorCode
{
    Lexical,
    Syntax,
    UnknownConstruct,
    UnsupportedFeature,
    DuplicateSection,
    MissingSection,
    InvalidNumber,
    Validation
}

public readonly record struct PddlTextSpan(int Offset, int Length, int Line, int Column);

public sealed class PddlLoadException : Exception
{
    public PddlDocumentType DocumentType { get; }
    public PddlLoadErrorCode ErrorCode { get; }
    public PddlTextSpan? Span { get; }
    public string? SourcePath { get; }

    public PddlLoadException(
        PddlDocumentType documentType,
        PddlLoadErrorCode errorCode,
        string message,
        PddlTextSpan? span = null,
        string? sourcePath = null,
        Exception? innerException = null)
        : base(message, innerException)
    {
        DocumentType = documentType;
        ErrorCode = errorCode;
        Span = span;
        SourcePath = sourcePath;
    }
}

internal static class PddlLoading
{
    internal static T Execute<T>(
        PddlDocumentType documentType,
        string? sourcePath,
        Func<T> load)
    {
        ArgumentNullException.ThrowIfNull(load);

        try
        {
            return load();
        }
        catch (PddlLoadException)
        {
            throw;
        }
        catch (PddlParseException exception)
        {
            var span = new PddlTextSpan(
                exception.Span.Offset,
                exception.Span.Length,
                exception.Span.Line,
                exception.Span.Column);

            throw new PddlLoadException(
                documentType,
                MapErrorCode(exception.ErrorCode),
                exception.Message,
                span,
                sourcePath,
                exception);
        }
        catch (PddlValidationException exception)
        {
            throw new PddlLoadException(
                documentType,
                PddlLoadErrorCode.Validation,
                exception.Message,
                sourcePath: sourcePath,
                innerException: exception);
        }
        catch (NotSupportedException exception)
        {
            throw new PddlLoadException(
                documentType,
                PddlLoadErrorCode.UnsupportedFeature,
                exception.Message,
                sourcePath: sourcePath,
                innerException: exception);
        }
    }

    private static PddlLoadErrorCode MapErrorCode(PddlParseErrorCode errorCode)
    {
        return errorCode switch
        {
            PddlParseErrorCode.Lexical => PddlLoadErrorCode.Lexical,
            PddlParseErrorCode.Syntax => PddlLoadErrorCode.Syntax,
            PddlParseErrorCode.UnknownConstruct => PddlLoadErrorCode.UnknownConstruct,
            PddlParseErrorCode.UnsupportedFeature => PddlLoadErrorCode.UnsupportedFeature,
            PddlParseErrorCode.DuplicateSection => PddlLoadErrorCode.DuplicateSection,
            PddlParseErrorCode.MissingSection => PddlLoadErrorCode.MissingSection,
            PddlParseErrorCode.InvalidNumber => PddlLoadErrorCode.InvalidNumber,
            _ => throw new ArgumentOutOfRangeException(nameof(errorCode), errorCode, null)
        };
    }
}
