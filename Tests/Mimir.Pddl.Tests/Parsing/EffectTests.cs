using System.Linq;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Parsing;
using Xunit;

namespace Mimir.Pddl.Tests.Parsing;

public class EffectTests
{
    [Fact]
    public void ParsesComplexEffects()
    {
        var input = "(and (not (at ?truck ?from)) (at ?truck ?to) (decrease (fuel ?truck) 10) (when (heavy ?truck) (increase (total-cost) 5)))";
        var result = PddlParser.ParseEffect(input);

        var andEffect = Assert.IsType<AndEffect>(result);
        Assert.Equal(4, andEffect.Effects.Length);

        var delEffect = Assert.Single(andEffect.Effects.OfType<DeleteEffect>());
        Assert.Equal("at", delEffect.Predicate.Name);

        var addEffect = Assert.Single(andEffect.Effects.OfType<AddEffect>());
        Assert.Equal("at", addEffect.Predicate.Name);

        var decEffect = Assert.Single(andEffect.Effects.OfType<Decrease>());
        Assert.Equal("fuel", decEffect.Fluent.Name);
        var decVal = Assert.IsType<NumberLiteral>(decEffect.Value);
        Assert.Equal(10m, decVal.Value);

        var whenEffect = Assert.Single(andEffect.Effects.OfType<ConditionalEffect>());
        var cond = Assert.IsType<PredicateCall>(whenEffect.Condition);
        Assert.Equal("heavy", cond.Name);

        var incEffect = Assert.IsType<Increase>(whenEffect.Effect);
        Assert.Equal("total-cost", incEffect.Fluent.Name);
        var incVal = Assert.IsType<NumberLiteral>(incEffect.Value);
        Assert.Equal(5m, incVal.Value);
    }
}
