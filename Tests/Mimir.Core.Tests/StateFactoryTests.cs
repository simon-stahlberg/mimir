using Mimir.Core.Grounding;

namespace Mimir.Core.Tests;

public class StateFactoryTests
{
    [Fact]
    public void CreateFromFluentFacts_SetsRequestedFacts()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["first", "second"]);
        Fact<Mimir.Core.Schemas.Fluent> firstFact = fixture.RegisterFluent("first");
        Fact<Mimir.Core.Schemas.Fluent> secondFact = fixture.RegisterFluent("second");

        State state = StateFactory.Default.Create(fixture.Context, [secondFact]);

        Assert.False(state.IsTrue(firstFact));
        Assert.True(state.IsTrue(secondFact));
    }

    [Fact]
    public void CreateFromFluentFacts_ThrowsForForeignFact()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["registered"]);
        TestProblemFixture foreignFixture = TestProblemFixture.Create(["registered"]);
        Fact<Mimir.Core.Schemas.Fluent> foreignFact = foreignFixture.RegisterFluent("registered");

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => StateFactory.Default.Create(fixture.Context, [foreignFact]));

        Assert.Equal("trueFacts", exception.ParamName);
    }

    [Fact]
    public void WithAdditionalFluentFacts_AddsFactsWithoutRemovingExistingOnes()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["first", "second"]);
        Fact<Mimir.Core.Schemas.Fluent> firstFact = fixture.RegisterFluent("first");
        Fact<Mimir.Core.Schemas.Fluent> secondFact = fixture.RegisterFluent("second");

        State state = StateFactory.Default.Create(fixture.Context, [firstFact]);
        State widened = state.WithAdditionalFluentFacts([secondFact]);

        Assert.True(widened.IsTrue(firstFact));
        Assert.True(widened.IsTrue(secondFact));
        Assert.False(state.IsTrue(secondFact));
    }

    [Fact]
    public void WithAdditionalFluentFacts_ThrowsForForeignFact()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["registered"]);
        TestProblemFixture foreignFixture = TestProblemFixture.Create(["registered"]);
        Fact<Mimir.Core.Schemas.Fluent> foreignFact = foreignFixture.RegisterFluent("registered");
        State state = StateFactory.Default.CreateEmpty(fixture.Context);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => state.WithAdditionalFluentFacts([foreignFact]));

        Assert.Equal("additionalFacts", exception.ParamName);
    }
}
