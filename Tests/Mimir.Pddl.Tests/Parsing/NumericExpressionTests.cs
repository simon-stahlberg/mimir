using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class NumericExpressionTests
{
    [Fact]
    public void ParsesArithmeticAndFluents()
    {
        var input = "(+ (distance ?loc1 ?loc2) (* 2.5 4))";
        var result = PddlParser.ParseNumericExpression(input);

        var add = Assert.IsType<Add>(result);

        // Left side: fluent
        var fluent = Assert.IsType<FluentCall>(add.Left);
        Assert.Equal("distance", fluent.Name);
        Assert.Equal("?loc1", fluent.Arguments[0].Name);
        Assert.Equal("?loc2", fluent.Arguments[1].Name);

        // Right side: multiply
        var mult = Assert.IsType<Multiply>(add.Right);
        var lit1 = Assert.IsType<NumberLiteral>(mult.Left);
        Assert.Equal(2.5m, lit1.Value);

        var lit2 = Assert.IsType<NumberLiteral>(mult.Right);
        Assert.Equal(4m, lit2.Value);
    }

    [Fact]
    public void MinusParsesUnaryAndBinaryArities()
    {
        Negate negate = Assert.IsType<Negate>(PddlParser.ParseNumericExpression("(- 4)"));
        AssertNumber(4m, negate.Operand);
        Assert.Equal(negate, Assert.IsType<Negate>(PddlParser.ParseNumericExpression(negate.ToPddlString())));

        Subtract subtract = Assert.IsType<Subtract>(PddlParser.ParseNumericExpression("(- 4 1)"));
        AssertNumber(4m, subtract.Left);
        AssertNumber(1m, subtract.Right);
        Assert.Equal(subtract, Assert.IsType<Subtract>(PddlParser.ParseNumericExpression(subtract.ToPddlString())));
    }

    [Fact]
    public void NaryAddAndMultiplyPreserveAllOperandsAfterRoundTrip()
    {
        Add add = Assert.IsType<Add>(PddlParser.ParseNumericExpression("(+ 1 2 3)"));
        Assert.Equal(new[] { 1m, 2m, 3m }, NumberLiterals(add).Order());
        Add reparsedAdd = Assert.IsType<Add>(PddlParser.ParseNumericExpression(add.ToPddlString()));
        Assert.Equal(NumberLiterals(add).Order(), NumberLiterals(reparsedAdd).Order());

        Multiply multiply = Assert.IsType<Multiply>(PddlParser.ParseNumericExpression("(* 2 3 4)"));
        Assert.Equal(new[] { 2m, 3m, 4m }, NumberLiterals(multiply).Order());
        Multiply reparsedMultiply = Assert.IsType<Multiply>(PddlParser.ParseNumericExpression(multiply.ToPddlString()));
        Assert.Equal(NumberLiterals(multiply).Order(), NumberLiterals(reparsedMultiply).Order());
    }

    [Fact]
    public void StandaloneTotalTimeRemainsAFluentCall()
    {
        FluentCall fluent = Assert.IsType<FluentCall>(PddlParser.ParseNumericExpression("ToTaL-TiMe"));

        Assert.Equal("ToTaL-TiMe", fluent.Name);
        Assert.Empty(fluent.Arguments);
        Assert.Equal(fluent, Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(fluent.ToPddlString())));
    }

    private static void AssertNumber(decimal expected, INumericExpression expression)
    {
        Assert.Equal(expected, Assert.IsType<NumberLiteral>(expression).Value);
    }

    private static IEnumerable<decimal> NumberLiterals(INumericExpression expression)
    {
        return expression switch
        {
            NumberLiteral literal => [literal.Value],
            Add add => NumberLiterals(add.Left).Concat(NumberLiterals(add.Right)),
            Multiply multiply => NumberLiterals(multiply.Left).Concat(NumberLiterals(multiply.Right)),
            _ => throw new InvalidOperationException($"Unexpected numeric expression '{expression.GetType().Name}'.")
        };
    }
}
