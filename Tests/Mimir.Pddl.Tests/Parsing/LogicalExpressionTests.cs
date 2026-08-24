using System.Linq;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class LogicalExpressionTests
{
    [Fact]
    public void ParsesNestedAndOrNot()
    {
        var input = "(and (at ?truck loc) (or (not (empty ?truck)) (= ?truck t1)))";
        var result = PddlParser.ParseLogicalExpression(input);

        Assert.IsType<And>(result);
        var andExpr = (And)result;
        
        Assert.Equal(2, andExpr.Expressions.Length);
        
        var atExpr = Assert.Single(andExpr.Expressions.OfType<PredicateCall>());
        Assert.Equal("at", atExpr.Name);
        Assert.Equal("?truck", atExpr.Arguments[0].Name);
        Assert.Equal("loc", atExpr.Arguments[1].Name);

        var orExpr = Assert.Single(andExpr.Expressions.OfType<Or>());
        Assert.Equal(2, orExpr.Expressions.Length);

        var notExpr = Assert.Single(orExpr.Expressions.OfType<Not>());
        var emptyExpr = Assert.IsType<PredicateCall>(notExpr.Expression);
        Assert.Equal("empty", emptyExpr.Name);

        var eqExpr = Assert.Single(orExpr.Expressions.OfType<Equality>());
        Assert.Equal("?truck", eqExpr.Left.Name);
        Assert.Equal("t1", eqExpr.Right.Name);
    }

    [Fact]
    public void ParsesQuantifiersAndParameters()
    {
        var input = "(forall (?t - truck ?l - location) (imply (at ?t ?l) (visited ?l)))";
        var result = PddlParser.ParseLogicalExpression(input);

        var forall = Assert.IsType<Forall>(result);
        Assert.Equal(2, forall.Variables.Length);
        var truck = Assert.Single(forall.Variables, variable => variable.Name == "?t");
        Assert.Equal("truck", truck.TypeName);
        var location = Assert.Single(forall.Variables, variable => variable.Name == "?l");
        Assert.Equal("location", location.TypeName);

        var imply = Assert.IsType<Imply>(forall.Body);
        var ant = Assert.IsType<PredicateCall>(imply.Antecedent);
        Assert.Equal("at", ant.Name);

        var cons = Assert.IsType<PredicateCall>(imply.Consequent);
        Assert.Equal("visited", cons.Name);
    }
}
