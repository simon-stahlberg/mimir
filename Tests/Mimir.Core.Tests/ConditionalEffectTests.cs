using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;

namespace Mimir.Core.Tests;

public class ConditionalEffectTests
{
    [Fact]
    public void IsSatisfied_PositiveFluentConditionMet_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p", "q"]);
        InstanceContext context = fixture.Context;
        Fact<Fluent> condFact = fixture.RegisterFluent("p");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        State state = StateFactory.Default.Create(context, [condFact]);

        var effect = new GroundConditionalEffect(
            context,
            CreateBitboardWords(condFact),
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(state.Expand()));
    }

    [Fact]
    public void IsSatisfied_PositiveFluentConditionNotMet_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p", "q"]);
        InstanceContext context = fixture.Context;
        Fact<Fluent> condFact = fixture.RegisterFluent("p");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        State emptyState = StateFactory.Default.CreateEmpty(context);

        var effect = new GroundConditionalEffect(
            context,
            CreateBitboardWords(condFact),
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.False(effect.IsSatisfied(emptyState.Expand()));
    }

    [Fact]
    public void IsSatisfied_NegativeFluentConditionViolated_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p", "q"]);
        InstanceContext context = fixture.Context;
        Fact<Fluent> condFact = fixture.RegisterFluent("p");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        State state = StateFactory.Default.Create(context, [condFact]);

        var effect = new GroundConditionalEffect(
            context,
            default,
            CreateBitboardWords(condFact),
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.False(effect.IsSatisfied(state.Expand()));
    }

    [Fact]
    public void IsSatisfied_NegativeFluentConditionSatisfied_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p", "q"]);
        InstanceContext context = fixture.Context;
        Fact<Fluent> condFact = fixture.RegisterFluent("p");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        State emptyState = StateFactory.Default.CreateEmpty(context);

        var effect = new GroundConditionalEffect(
            context,
            default,
            CreateBitboardWords(condFact),
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(emptyState.Expand()));
    }

    [Fact]
    public void IsSatisfied_EmptyConditions_AlwaysReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"]);
        InstanceContext context = fixture.Context;
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        State emptyState = StateFactory.Default.CreateEmpty(context);

        var effect = new GroundConditionalEffect(
            context,
            default,
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(emptyState.Expand()));
    }

    [Fact]
    public void IsSatisfied_PositiveStaticConditionMet_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"], ["s"]);
        InstanceContext context = fixture.Context;
        Fact<Static> cond = fixture.RegisterStatic("s");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        context.SetStaticBitboardWords(CreateStaticBitboardWords(cond));

        var effect = new GroundConditionalEffect(
            context,
            default,
            default,
            CreateStaticMask(cond),
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(StateFactory.Default.CreateEmpty(context).Expand()));
    }

    [Fact]
    public void IsSatisfied_PositiveStaticConditionNotMet_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"], ["s"]);
        InstanceContext context = fixture.Context;
        Fact<Static> cond = fixture.RegisterStatic("s");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");

        var effect = new GroundConditionalEffect(
            context,
            default,
            default,
            CreateStaticMask(cond),
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.False(effect.IsSatisfied(StateFactory.Default.CreateEmpty(context).Expand()));
    }

    [Fact]
    public void IsSatisfied_NegativeStaticConditionViolated_ReturnsFalse()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"], ["s"]);
        InstanceContext context = fixture.Context;
        Fact<Static> cond = fixture.RegisterStatic("s");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        context.SetStaticBitboardWords(CreateStaticBitboardWords(cond));

        var effect = new GroundConditionalEffect(
            context,
            default,
            default,
            default,
            CreateStaticMask(cond),
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.False(effect.IsSatisfied(StateFactory.Default.CreateEmpty(context).Expand()));
    }

    [Fact]
    public void IsSatisfied_NegativeStaticConditionSatisfied_ReturnsTrue()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["q"], ["s"]);
        InstanceContext context = fixture.Context;
        Fact<Static> cond = fixture.RegisterStatic("s");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");

        var effect = new GroundConditionalEffect(
            context,
            default,
            default,
            default,
            CreateStaticMask(cond),
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(StateFactory.Default.CreateEmpty(context).Expand()));
    }

    [Fact]
    public void IsSatisfied_StateFromDifferentContext_ThrowsInvalidOperationException()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(["p", "q"]);
        Fact<Fluent> condFact = fixture.RegisterFluent("p");
        Fact<Fluent> effectFact = fixture.RegisterFluent("q");
        var effect = new GroundConditionalEffect(
            fixture.Context,
            CreateBitboardWords(condFact),
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));
        TestProblemFixture foreignFixture = TestProblemFixture.Create(["p", "q"]);
        State foreignState = StateFactory.Default.CreateEmpty(foreignFixture.Context);

        Assert.Throws<InvalidOperationException>(() => effect.IsSatisfied(foreignState.Expand()));
    }

    [Fact]
    public void IsSatisfied_HighOffsetConditionUsesGlobalFactIndex()
    {
        string[] names = Enumerable.Range(0, 66).Select(index => $"f-{index}").ToArray();
        TestProblemFixture fixture = TestProblemFixture.Create(names);
        Fact<Fluent>[] facts = names.Select(fixture.RegisterFluent).ToArray();
        Fact<Fluent> effectFact = facts[0];
        Fact<Fluent> conditionFact = facts[65];
        State state = StateFactory.Default.Create(fixture.Context, [conditionFact]);
        var effect = new GroundConditionalEffect(
            fixture.Context,
            CreateBitboardWords(conditionFact),
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            new Literal<Fact<Fluent>>(effectFact, Polarity.Positive));

        Assert.True(effect.IsSatisfied(state.Expand()));
    }

    private static ulong[] CreateStaticBitboardWords(Fact<Static> fact)
    {
        int wordCount = fact.LocalIndex / 64 + 1;
        var words = new ulong[wordCount];
        words[fact.LocalIndex / 64] |= 1UL << (fact.LocalIndex % 64);
        return words;
    }

    private static OffsetBitboard CreateStaticMask(Fact<Static> fact)
        => OffsetBitboard.FromSetBitIndices([fact.LocalIndex]);

    private static OffsetBitboard CreateBitboardWords(params Fact<Fluent>[] facts)
    {
        return OffsetBitboard.FromSetBitIndices(facts.Select(fact => fact.LocalIndex).ToArray());
    }
}
