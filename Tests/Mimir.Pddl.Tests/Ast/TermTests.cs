using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Tests.Ast;

public class TermTests
{
    [Fact]
    public void VariableCreatesValidatedVariable()
    {
        Term term = Term.Variable("?item_1-a");

        Assert.Equal("?item_1-a", term.Name);
        Assert.True(term.IsVariable);
        Assert.Equal("?item_1-a", term.ToPddlString());
    }

    [Fact]
    public void ConstantCreatesValidatedConstant()
    {
        Term term = Term.Constant("Item_1-a");

        Assert.Equal("Item_1-a", term.Name);
        Assert.False(term.IsVariable);
        Assert.Equal("Item_1-a", term.ToPddlString());
    }

    [Theory]
    [InlineData("")]
    [InlineData("?")]
    [InlineData("item")]
    [InlineData("??item")]
    [InlineData("?1item")]
    [InlineData("?item!")]
    [InlineData("?item name")]
    public void VariableRejectsInvalidNames(string name)
    {
        Assert.Throws<ArgumentException>(() => Term.Variable(name));
    }

    [Fact]
    public void VariableRejectsNull()
    {
        Assert.Throws<ArgumentNullException>(() => Term.Variable(null!));
    }

    [Theory]
    [InlineData("")]
    [InlineData("?item")]
    [InlineData("1item")]
    [InlineData("-item")]
    [InlineData("item!")]
    [InlineData("item name")]
    public void ConstantRejectsInvalidNames(string name)
    {
        Assert.Throws<ArgumentException>(() => Term.Constant(name));
    }

    [Fact]
    public void ConstantRejectsNull()
    {
        Assert.Throws<ArgumentNullException>(() => Term.Constant(null!));
    }
}
