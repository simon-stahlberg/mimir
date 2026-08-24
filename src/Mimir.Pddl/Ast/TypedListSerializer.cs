using System.Collections.Immutable;
using System.Text;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Ast;

internal static class TypedListSerializer
{
    public static string Serialize(ImmutableArray<Parameter> parameters) =>
        Serialize(parameters, parameter => parameter.Name, parameter => parameter.TypeName);

    public static string Serialize(ImmutableArray<TypeDeclaration> declarations) =>
        Serialize(declarations, declaration => declaration.Name, declaration => declaration.ParentType);

    private static string Serialize<T>(
        ImmutableArray<T> entries,
        Func<T, string> nameSelector,
        Func<T, string> typeSelector)
    {
        var builder = new StringBuilder();
        int groupStart = 0;

        while (groupStart < entries.Length)
        {
            string typeName = typeSelector(entries[groupStart]);
            int groupEnd = groupStart + 1;
            while (groupEnd < entries.Length
                && string.Equals(typeName, typeSelector(entries[groupEnd]), StringComparison.Ordinal))
            {
                groupEnd++;
            }

            if (builder.Length > 0)
                builder.Append(' ');

            for (int index = groupStart; index < groupEnd; index++)
            {
                if (index > groupStart)
                    builder.Append(' ');

                builder.Append(nameSelector(entries[index]));
            }

            bool isFinalRootObjectGroup = groupEnd == entries.Length && typeName == "object";
            if (!isFinalRootObjectGroup)
                builder.Append(" - ").Append(typeName);

            groupStart = groupEnd;
        }

        return builder.ToString();
    }
}
