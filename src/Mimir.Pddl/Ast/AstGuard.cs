using System.Collections.Immutable;

namespace Mimir.Pddl.Ast;

internal static class AstGuard
{
    public static T RequireNotNull<T>(T? value, string parameterName)
        where T : class => value ?? throw new ArgumentNullException(parameterName);

    public static ImmutableArray<T> RequireArray<T>(ImmutableArray<T> values, string parameterName)
    {
        if (values.IsDefault)
            throw new ArgumentException("The collection must be initialized.", parameterName);

        foreach (T value in values)
        {
            if (value is null)
                throw new ArgumentException("The collection cannot contain null elements.", parameterName);
        }

        return values;
    }

    public static TEnum RequireDefined<TEnum>(TEnum value, string parameterName)
        where TEnum : struct, Enum
    {
        if (!Enum.IsDefined(typeof(TEnum), value))
            throw new ArgumentOutOfRangeException(parameterName, value, null);

        return value;
    }

    public static ImmutableArray<TEnum> RequireDefinedArray<TEnum>(
        ImmutableArray<TEnum> values,
        string parameterName)
        where TEnum : struct, Enum
    {
        RequireArray(values, parameterName);

        foreach (TEnum value in values)
        {
            if (!Enum.IsDefined(typeof(TEnum), value))
                throw new ArgumentOutOfRangeException(parameterName, value, null);
        }

        return values;
    }
}
