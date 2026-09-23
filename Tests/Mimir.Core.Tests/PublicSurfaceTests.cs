using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using System.Reflection;
using Xunit;
using Action = Mimir.Core.Grounding.Action;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public class PublicSurfaceTests
{
    [Fact]
    public void ExtendedState_IsThePublicDerivedClosureStateType()
    {
        Assembly assembly = typeof(State).Assembly;

        Assert.Equal(typeof(ExtendedState), typeof(State).GetMethod(nameof(State.Expand))!.ReturnType);
        Assert.Null(assembly.GetType("Mimir.Core.Grounding.ExpandedState"));
    }

    [Fact]
    public void SchemaConstructors_CopyMutableInputs()
    {
        var parameters = new List<Variable> { new("?x") };
        var predicate = new FluentPredicate("p", parameters);
        var function = new NumericFunction("cost", parameters);
        var atomArguments = new List<ITerm> { parameters[0] };
        var atom = new Atom<Fluent>(predicate, atomArguments);
        var literal = new Literal<Atom<Fluent>>(atom, Polarity.Positive);
        var fluentPreconditions = new List<Literal<Atom<Fluent>>> { literal };
        var effects = new List<ConditionalEffect>
        {
            new(
                Array.Empty<Variable>(),
                Array.Empty<Literal<Atom<Fluent>>>(),
                Array.Empty<Literal<Atom<Static>>>(),
                Array.Empty<Literal<Atom<Derived>>>(),
                [],
                literal)
        };
        var actionSchema = new ActionSchema(
            "move",
            parameters,
            fluentPreconditions,
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            effects,
            [],
            new NumericConstant(1d));
        TestProblemFixture fixture = TestProblemFixture.Create(["fixture"]);
        var condition = new ConjunctiveCondition(
            fixture.Problem,
            parameters,
            Array.Empty<Literal<Atom<Static>>>(),
            fluentPreconditions,
            Array.Empty<Literal<Atom<Derived>>>(),
            []);
        var costExpression = new FunctionCall(function, atomArguments);

        parameters.Add(new Variable("?y"));
        atomArguments.Clear();
        fluentPreconditions.Clear();
        effects.Clear();

        Assert.Single(predicate.Parameters);
        Assert.Single(function.Parameters);
        Assert.Single(atom.Arguments);
        Assert.Single(actionSchema.Parameters);
        Assert.Single(actionSchema.FluentPreconditions);
        Assert.Single(actionSchema.Effects);
        Assert.Single(condition.Parameters);
        Assert.Single(condition.FluentLiterals);
        Assert.Single(costExpression.Arguments);
    }

    [Fact]
    public void GroundingConstructors_CopyMutableInputs()
    {
        Domain domain = new DomainBuilder("copy-test")
            .Requirements().Add(":strips").Add(":derived-predicates").Close()
            .Predicates().Add("p", ("?x", "object")).Add("d", ("?x", "object")).Close()
            .Actions()
                .Add("noop").AddParameter("?x").AddEffect("p", "?x").Close()
                .Close()
            .DerivedPredicates().Define("d", Logic.Atom("p", "?x")).Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "copy-problem")
            .Objects().Add("a").Close()
            .Build();
        InstanceContext context = problem.Context;
        FluentPredicate predicate = Assert.Single(domain.Fluents);
        DerivedPredicate derivedPredicate = Assert.Single(domain.Derived);
        Constant constant = Assert.Single(problem.AllObjects);
        var arguments = new List<Constant> { constant };
        Fact<Fluent> fact = context.RegisterFact(predicate, arguments);
        Fact<Derived> derivedFact = context.RegisterFact(derivedPredicate, arguments);
        var derivedPreconditions = new List<Literal<Fact<Derived>>>();
        var conditionalEffects = new List<GroundConditionalEffect>();
        var bitboard = new[] { 1UL << fact.LocalIndex };
        ActionSchema schema = Assert.Single(domain.Actions);
        var action = new Action(
            context,
            schema,
            arguments,
            OffsetBitboard.FromDense(bitboard),
            default,
            default,
            default,
            OffsetBitboard.FromDense(bitboard),
            default,
            derivedPreconditions,
            conditionalEffects,
            1d);

        arguments.Clear();
        bitboard[0] = 0UL;
        derivedPreconditions.Add(new Literal<Fact<Derived>>(derivedFact, Polarity.Positive));

        Assert.Single(fact.Arguments);
        Assert.Single(action.Arguments);
        Assert.True(action.IsApplicable(StateFactory.Default.Create(context, [fact]).Expand()));
        Assert.Empty(action.DerivedPreconditions);
    }

    [Fact]
    public void GroundedExpressionRecords_CopyMutableInputs()
    {
        var variable = new Variable("?x");
        var predicate = new FluentPredicate("p", new[] { variable });
        var arguments = new List<ITerm> { variable };
        var variables = new List<Variable> { variable };
        var body = new GroundedTrue();
        var atom = new GroundedAtom(predicate, arguments);
        var forall = new GroundedForall(variables, body);
        var exists = new GroundedExists(variables, body);

        var replacementArguments = new List<ITerm> { variable };
        var replacementVariables = new List<Variable> { variable };
        var replacedAtom = atom with { Arguments = replacementArguments };
        var replacedForall = forall with { Variables = replacementVariables };
        var replacedExists = exists with { Variables = replacementVariables };

        arguments.Clear();
        variables.Clear();
        replacementArguments.Clear();
        replacementVariables.Clear();

        Assert.Single(atom.Arguments);
        Assert.Single(forall.Variables);
        Assert.Single(exists.Variables);
        Assert.Single(replacedAtom.Arguments);
        Assert.Single(replacedForall.Variables);
        Assert.Single(replacedExists.Variables);

        var (deconstructedPredicate, deconstructedArguments) = atom;
        var (deconstructedVariables, deconstructedBody) = forall;
        var (deconstructedExistsVariables, deconstructedExistsBody) = exists;
        Assert.Same(predicate, deconstructedPredicate);
        Assert.Single(deconstructedArguments);
        Assert.Single(deconstructedVariables);
        Assert.Same(body, deconstructedBody);
        Assert.Single(deconstructedExistsVariables);
        Assert.Same(body, deconstructedExistsBody);
    }

    [Fact]
    public void State_IsTrue_ReturnsStaticFactState()
    {
        TestProblemFixture fixture = TestProblemFixture.Create(staticNames: ["s"]);
        Fact<Static> fact = fixture.RegisterStatic("s");
        fixture.Context.SetStaticBitboardWords([1UL << fact.LocalIndex]);
        State state = StateFactory.Default.CreateEmpty(fixture.Context);

        Assert.True(state.Expand().IsTrue(fact));
    }
}
