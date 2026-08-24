using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Tests.Ast;

public class PddlRequirementTests
{
    [Theory]
    [InlineData(PddlRequirement.Strips, ":strips")]
    [InlineData(PddlRequirement.Typing, ":typing")]
    [InlineData(PddlRequirement.Equality, ":equality")]
    [InlineData(PddlRequirement.NegativePreconditions, ":negative-preconditions")]
    [InlineData(PddlRequirement.DisjunctivePreconditions, ":disjunctive-preconditions")]
    [InlineData(PddlRequirement.NumericFluents, ":numeric-fluents")]
    [InlineData(PddlRequirement.ConditionalEffects, ":conditional-effects")]
    [InlineData(PddlRequirement.ExistentialPreconditions, ":existential-preconditions")]
    [InlineData(PddlRequirement.UniversalPreconditions, ":universal-preconditions")]
    [InlineData(PddlRequirement.Adl, ":adl")]
    [InlineData(PddlRequirement.DerivedPredicates, ":derived-predicates")]
    [InlineData(PddlRequirement.ActionCosts, ":action-costs")]
    public void RequirementUsesCanonicalSpelling(PddlRequirement requirement, string spelling)
    {
        Assert.Equal(spelling, requirement.ToPddlString());
    }

    [Theory]
    [InlineData(PddlRequirement.Strips)]
    [InlineData(PddlRequirement.Typing)]
    [InlineData(PddlRequirement.Equality)]
    [InlineData(PddlRequirement.NegativePreconditions)]
    [InlineData(PddlRequirement.DisjunctivePreconditions)]
    [InlineData(PddlRequirement.ConditionalEffects)]
    [InlineData(PddlRequirement.ExistentialPreconditions)]
    [InlineData(PddlRequirement.UniversalPreconditions)]
    public void AdlImpliesItsComponentRequirements(PddlRequirement impliedRequirement)
    {
        PddlRequirement[] requirements = [PddlRequirement.Adl];

        Assert.True(requirements.HasRequirement(impliedRequirement));
    }

    [Theory]
    [InlineData(PddlRequirement.NumericFluents)]
    [InlineData(PddlRequirement.DerivedPredicates)]
    [InlineData(PddlRequirement.ActionCosts)]
    public void AdlDoesNotImplyIndependentRequirements(PddlRequirement independentRequirement)
    {
        PddlRequirement[] requirements = [PddlRequirement.Adl];

        Assert.False(requirements.HasRequirement(independentRequirement));
    }

    [Fact]
    public void EmptyRequirementsImplicitlyProvideStripsOnly()
    {
        PddlRequirement[] requirements = [];

        Assert.True(requirements.HasRequirement(PddlRequirement.Strips));
        Assert.False(requirements.HasRequirement(PddlRequirement.Typing));
    }

    [Fact]
    public void ExplicitRequirementIsRecognized()
    {
        PddlRequirement[] requirements = [PddlRequirement.NumericFluents];

        Assert.True(requirements.HasRequirement(PddlRequirement.NumericFluents));
        Assert.False(requirements.HasRequirement(PddlRequirement.Strips));
    }

    [Fact]
    public void UndefinedRequirementCannotBeSerialized()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() => ((PddlRequirement)999).ToPddlString());
    }
}
