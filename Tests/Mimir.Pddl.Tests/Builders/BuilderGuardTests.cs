using System.Collections.Immutable;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Builders;

namespace Mimir.Pddl.Tests.Builders;

public class BuilderGuardTests
{
    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    public void ActionBuilderRequiresName(string? name)
    {
        Assert.ThrowsAny<ArgumentException>(() => new ActionBuilder(name!));
    }

    [Fact]
    public void ActionBuilderRejectsNullNodes()
    {
        var builder = new ActionBuilder("move");

        Assert.Throws<ArgumentNullException>(() => builder.WithPrecondition(null!));
        Assert.Throws<ArgumentNullException>(() => builder.WithEffect(null!));
    }

    [Theory]
    [InlineData(null, "object")]
    [InlineData("", "object")]
    [InlineData("   ", "object")]
    [InlineData("?x", null)]
    [InlineData("?x", "")]
    [InlineData("?x", "   ")]
    public void ActionBuilderRejectsInvalidParameterNames(string? name, string? typeName)
    {
        var builder = new ActionBuilder("move");

        Assert.ThrowsAny<ArgumentException>(() => builder.AddParameter(name!, typeName!));
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    public void DomainBuilderRequiresName(string? name)
    {
        Assert.ThrowsAny<ArgumentException>(() => new DomainBuilder(name!));
    }

    [Fact]
    public void AddActionCreatesNamedActionThroughCallback()
    {
        DomainDefinition domain = new DomainBuilder("transport")
            .AddAction("move", action => action
                .AddParameter("?from", "location")
                .WithPrecondition(new EmptyLogic())
                .WithEffect(new AndEffect(ImmutableArray<Mimir.Pddl.Ast.IEffect>.Empty)))
            .Build();

        ActionDefinition action = Assert.Single(domain.Actions);
        Assert.Equal("move", action.Name);
        Assert.Equal("?from", Assert.Single(action.Parameters).Name);
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    public void AddActionRequiresName(string? name)
    {
        var builder = new DomainBuilder("transport");

        Assert.ThrowsAny<ArgumentException>(() => builder.AddAction(name!, _ => { }));
    }

    [Fact]
    public void AddActionRejectsNullCallbackAndNode()
    {
        var builder = new DomainBuilder("transport");

        Assert.Throws<ArgumentNullException>(() => builder.AddAction("move", null!));
        Assert.Throws<ArgumentNullException>(() => builder.AddAction((ActionDefinition)null!));
    }

    [Fact]
    public void DomainBuilderRejectsNullDeclarationInputs()
    {
        var builder = new DomainBuilder("transport");
        Parameter[] parameters = [null!];

        Assert.Throws<ArgumentNullException>(() => builder.AddPredicate("at", null!));
        Assert.Throws<ArgumentException>(() => builder.AddPredicate("at", parameters));
        Assert.Throws<ArgumentNullException>(() => builder.AddFunction("cost", null!));
        Assert.Throws<ArgumentException>(() => builder.AddFunction("cost", parameters));
        Assert.Throws<ArgumentNullException>(() => builder.AddDerivedPredicate(null!, new EmptyLogic()));
        Assert.Throws<ArgumentNullException>(() => builder.AddDerivedPredicate(
            new PredicateDeclaration("reachable", ImmutableArray<Parameter>.Empty),
            null!));
    }

    [Fact]
    public void DomainBuilderRejectsUndefinedRequirement()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() =>
            new DomainBuilder("transport").AddRequirement((PddlRequirement)999));
    }
}
