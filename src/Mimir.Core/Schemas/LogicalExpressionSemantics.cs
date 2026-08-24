using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;

namespace Mimir.Core.Schemas;

internal static class LogicalExpressionSemantics
{
    public static bool IsAlwaysTrue(ILogicalExpression expression)
    {
        return expression switch
        {
            EmptyLogic => true,
            And and => and.Expressions.All(IsAlwaysTrue),
            _ => false
        };
    }
}
