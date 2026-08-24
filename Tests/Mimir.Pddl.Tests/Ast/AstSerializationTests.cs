using System.Collections.Immutable;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Parsing;

namespace Mimir.Pddl.Tests.Ast;

public class AstSerializationTests
{
    [Fact]
    public void NumberLiteralPreservesIntegerBeyondDoublePrecision()
    {
        var literal = new NumberLiteral(9007199254740993m);

        Assert.Equal(9007199254740993m, literal.Value);
        NumberLiteral reparsed = Assert.IsType<NumberLiteral>(
            PddlParser.ParseNumericExpression(literal.ToPddlString()));
        Assert.Equal(literal.Value, reparsed.Value);
    }

    [Theory]
    [InlineData(ComparisonOperator.Equal)]
    [InlineData(ComparisonOperator.LessThan)]
    [InlineData(ComparisonOperator.LessThanOrEqual)]
    [InlineData(ComparisonOperator.GreaterThan)]
    [InlineData(ComparisonOperator.GreaterThanOrEqual)]
    public void ComparisonRoundTripsWithTypedOperatorAndOperands(ComparisonOperator comparisonOperator)
    {
        var comparison = new Comparison(comparisonOperator, new NumberLiteral(1m), new NumberLiteral(2m));

        Comparison reparsed = Assert.IsType<Comparison>(
            PddlParser.ParseLogicalExpression(comparison.ToPddlString()));
        Assert.Equal(comparison, reparsed);
    }

    [Fact]
    public void ComparisonRejectsUndefinedOperator()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            new Comparison((ComparisonOperator)999, new NumberLiteral(1m), new NumberLiteral(2m)));
    }

    [Fact]
    public void UnaryAndBinaryMinusRemainDistinct()
    {
        var operand = new FluentCall("fuel", ImmutableArray<Term>.Empty);
        var negate = new Negate(operand);
        var subtract = new Subtract(operand, new NumberLiteral(1m));

        Assert.Equal(negate, Assert.IsType<Negate>(PddlParser.ParseNumericExpression(negate.ToPddlString())));
        Assert.Equal(subtract, Assert.IsType<Subtract>(PddlParser.ParseNumericExpression(subtract.ToPddlString())));
    }

    [Fact]
    public void ZeroArityFluentRoundTrips()
    {
        var fluent = new FluentCall("total-cost", ImmutableArray<Term>.Empty);

        FluentCall reparsed = Assert.IsType<FluentCall>(
            PddlParser.ParseNumericExpression(fluent.ToPddlString()));
        Assert.Equal("total-cost", reparsed.Name);
        Assert.Empty(reparsed.Arguments);
    }

    [Fact]
    public void NegativeInitialFactSerializesAsLogicalNegation()
    {
        var predicate = new PredicateCall("clear", ImmutableArray.Create(Term.Constant("a")));
        var initialization = new NegativePredicateInitialization(predicate);

        Not reparsed = Assert.IsType<Not>(PddlParser.ParseLogicalExpression(initialization.ToPddlString()));
        PredicateCall reparsedPredicate = Assert.IsType<PredicateCall>(reparsed.Expression);
        Assert.Equal(predicate, reparsedPredicate);
    }

    [Fact]
    public void EmptyProblemAlwaysSerializesInitAndGoalSections()
    {
        var problem = CreateProblem();

        ProblemDefinition reparsed = PddlParser.ParseProblem(problem.ToPddlString());

        Assert.Empty(reparsed.Init);
        Assert.IsType<EmptyLogic>(reparsed.Goal);
    }

    private static ProblemDefinition CreateProblem() => new(
        "sample-problem",
        "sample",
        ImmutableArray<PddlRequirement>.Empty,
        ImmutableArray<TypeDeclaration>.Empty,
        ImmutableArray<IProblemInitElement>.Empty,
        new EmptyLogic(),
        null);
}
