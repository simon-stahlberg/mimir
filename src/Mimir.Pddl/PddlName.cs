namespace Mimir.Pddl;

internal static class PddlName
{
    private static readonly HashSet<string> ReservedNames = new(StringComparer.OrdinalIgnoreCase)
    {
        "and", "or", "not", "imply", "exists", "forall", "when",
        "assign", "increase", "decrease", "scale-up", "scale-down",
        "define", "domain", "problem", "minimize", "maximize", "either",
        "preference", "always", "sometime", "within", "at-most-once",
        "sometime-after", "sometime-before", "always-within", "hold-during",
        "hold-after", "probabilistic", "oneof", "is-violated"
    };

    internal static bool IsAsciiLetter(char character)
    {
        return character is >= 'A' and <= 'Z' or >= 'a' and <= 'z';
    }

    internal static bool IsAsciiDigit(char character)
    {
        return character is >= '0' and <= '9';
    }

    internal static bool IsNameCharacter(char character)
    {
        return IsAsciiLetter(character) || IsAsciiDigit(character) || character is '-' or '_';
    }

    internal static bool IsReserved(string name)
    {
        return ReservedNames.Contains(name);
    }

    internal static string RequireName(string? name, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(name, parameterName);

        if (!HasValidNameSyntax(name) || IsReserved(name))
        {
            throw new ArgumentException(
                $"'{name}' is not a valid PDDL name.",
                parameterName);
        }

        return name;
    }

    internal static string RequireVariable(string? variable, string parameterName)
    {
        ArgumentNullException.ThrowIfNull(variable, parameterName);

        if (variable.Length < 2 || variable[0] != '?' || !HasValidNameSyntax(variable.AsSpan(1)))
        {
            throw new ArgumentException(
                $"'{variable}' is not a valid PDDL variable.",
                parameterName);
        }

        return variable;
    }

    private static bool HasValidNameSyntax(ReadOnlySpan<char> name)
    {
        if (name.IsEmpty || !IsAsciiLetter(name[0]))
            return false;

        for (int index = 1; index < name.Length; index++)
        {
            if (!IsNameCharacter(name[index]))
                return false;
        }

        return true;
    }
}
