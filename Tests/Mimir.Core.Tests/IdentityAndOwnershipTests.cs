using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using System.Runtime.CompilerServices;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class IdentityAndOwnershipTests
{
    private sealed class DelegateGrounder(Func<Problem, IEnumerable<GroundAction>> ground) : IGrounder
    {
        public IEnumerable<GroundAction> Ground(Problem problem, State startState) => ground(problem);
    }

    [Fact]
    public void SchemaEntitiesFromDifferentDomainsRemainDistinct()
    {
        Domain first = CreateDomain("first");
        Domain second = CreateDomain("second");

        Mimir.Core.Schemas.Predicate<Fluent> firstPredicate = Assert.Single(first.Fluents);
        Mimir.Core.Schemas.Predicate<Fluent> secondPredicate = Assert.Single(second.Fluents);
        NumericFunction firstFunction = Assert.Single(first.Functions);
        NumericFunction secondFunction = Assert.Single(second.Functions);
        Constant firstConstant = Assert.Single(first.Constants);
        Constant secondConstant = Assert.Single(second.Constants);
        Variable firstVariable = Assert.Single(firstPredicate.Parameters);
        Variable secondVariable = Assert.Single(secondPredicate.Parameters);
        var firstAtom = new Atom<Fluent>(firstPredicate, [firstConstant]);
        var secondAtom = new Atom<Fluent>(secondPredicate, [secondConstant]);

        Assert.NotEqual(firstPredicate, secondPredicate);
        Assert.NotEqual(firstFunction, secondFunction);
        Assert.NotEqual(firstConstant, secondConstant);
        Assert.NotEqual(firstVariable, secondVariable);
        Assert.NotEqual(firstAtom, secondAtom);
        Assert.Equal(2, new HashSet<Predicate> { firstPredicate, secondPredicate }.Count);
        Assert.Equal(2, new HashSet<NumericFunction> { firstFunction, secondFunction }.Count);
        Assert.Equal(2, new HashSet<Constant> { firstConstant, secondConstant }.Count);
        Assert.Equal(2, new HashSet<Variable> { firstVariable, secondVariable }.Count);
        Assert.Equal(2, new HashSet<Atom> { firstAtom, secondAtom }.Count);
    }

    [Fact]
    public void GroundedEntitiesAreScopedToTheirProblemContext()
    {
        Domain domain = CreateDomain("shared");
        Problem first = CreateProblem(domain, "first-problem", "a");
        Problem second = CreateProblem(domain, "second-problem", "b");
        Mimir.Core.Schemas.Predicate<Fluent> predicate = Assert.Single(domain.Fluents);
        Constant firstObject = first.ObjectLookup["a"];
        Constant secondObject = second.ObjectLookup["b"];
        Fact<Fluent> firstFact = Assert.IsType<Fact<Fluent>>(first.Context.FindFact(predicate, [firstObject]));
        Fact<Fluent> secondFact = Assert.IsType<Fact<Fluent>>(second.Context.FindFact(predicate, [secondObject]));
        var firstGenerator = new GroundedApplicableActionGenerator(first, first.InitialState, new RpgGrounder());
        var secondGenerator = new GroundedApplicableActionGenerator(second, second.InitialState, new RpgGrounder());
        Mimir.Core.Grounding.Action firstAction = Assert.Single(firstGenerator.GroundActions);
        Mimir.Core.Grounding.Action secondAction = Assert.Single(secondGenerator.GroundActions);

        Assert.NotEqual(firstObject, secondObject);
        Assert.NotEqual(firstFact, secondFact);
        Assert.NotEqual(firstAction, secondAction);
        Assert.NotEqual(first.InitialState, second.InitialState);
        Assert.Equal(2, new HashSet<Fact> { firstFact, secondFact }.Count);
        Assert.Equal(2, new HashSet<Mimir.Core.Grounding.Action> { firstAction, secondAction }.Count);
        Assert.Equal(2, new HashSet<State> { first.InitialState, second.InitialState }.Count);

        Constant sharedConstant = Assert.Single(domain.Constants);
        Assert.Same(sharedConstant, first.ObjectLookup["shared"]);
        Assert.Same(sharedConstant, second.ObjectLookup["shared"]);

        Assert.Same(firstFact, first.Context.FindFact(predicate, [firstObject]));
        Mimir.Core.Grounding.Action repeatedAction = Assert.Single(
            new GroundedApplicableActionGenerator(first, first.InitialState, new RpgGrounder()).GroundActions);
        Assert.Equal(firstAction, repeatedAction);
        Assert.Equal(firstAction.GetHashCode(), repeatedAction.GetHashCode());
    }

    [Fact]
    public void UnreferencedDynamicVariableCanBeCollectedWhileProblemRemainsAlive()
    {
        Problem problem = CreateProblem(CreateDomain("dynamic-variable"), "gc-problem", "item");
        WeakReference<Variable> variable = CreateUnreferencedDynamicVariable(problem);

        CollectGarbage();

        Assert.False(variable.TryGetTarget(out _));
        GC.KeepAlive(problem);
    }

    [Fact]
    public void ConditionKeepsDynamicVariableAliveAndOwned()
    {
        Problem problem = CreateProblem(CreateDomain("condition-variable"), "condition-problem", "item");
        (WeakReference<Variable> variableReference, ConjunctiveCondition condition) =
            CreateConditionWithDynamicVariable(problem);

        CollectGarbage();

        Assert.True(variableReference.TryGetTarget(out Variable? variable));
        Assert.Same(variable, Assert.Single(condition.Parameters));
        Assert.True(problem.ContainsVariable(variable));
        GC.KeepAlive(condition);
    }

    [Fact]
    public void GroundActionIdentityUsesContextSchemaAndOrderedArguments()
    {
        DomainBuilder domainBuilder = new DomainBuilder("action-identity")
            .Requirements().Add(":strips").Add(":typing").Close()
            .Types().Add("thing").Close()
            .Constants().Add("a", "thing").Add("b", "thing").Close()
            .Predicates().Add("marked").Close();
        ActionListBuilder actionsBuilder = domainBuilder.Actions();
        actionsBuilder.Add("first")
            .AddParameter("?x", "thing")
            .AddParameter("?y", "thing")
            .AddEffect("marked")
            .Close();
        actionsBuilder.Add("second")
            .AddParameter("?x", "thing")
            .AddParameter("?y", "thing")
            .AddEffect("marked")
            .Close();
        Domain domain = actionsBuilder.Close().Build();
        Problem firstProblem = new ProblemBuilder(domain, "first-action-identity").Build();
        Problem secondProblem = new ProblemBuilder(domain, "second-action-identity").Build();
        ActionSchema firstSchema = domain.Actions.Single(schema => schema.Name == "first");
        ActionSchema secondSchema = domain.Actions.Single(schema => schema.Name == "second");
        Constant a = domain.Constants.Single(constant => constant.Name == "a");
        Constant b = domain.Constants.Single(constant => constant.Name == "b");

        GroundAction action = CreateAction(firstProblem.Context, firstSchema, [a, b]);
        GroundAction independentlyBuilt = CreateAction(firstProblem.Context, firstSchema, [a, b]);
        GroundAction differentContext = CreateAction(secondProblem.Context, firstSchema, [a, b]);
        GroundAction differentSchema = CreateAction(firstProblem.Context, secondSchema, [a, b]);
        GroundAction differentOrder = CreateAction(firstProblem.Context, firstSchema, [b, a]);
        GroundAction differentArgument = CreateAction(firstProblem.Context, firstSchema, [a, a]);

        Assert.NotSame(action, independentlyBuilt);
        Assert.Equal(action, independentlyBuilt);
        Assert.Equal(action.GetHashCode(), independentlyBuilt.GetHashCode());
        Assert.NotEqual(action, differentContext);
        Assert.NotEqual(action, differentSchema);
        Assert.NotEqual(action, differentOrder);
        Assert.NotEqual(action, differentArgument);

        var actions = new HashSet<GroundAction>
        {
            action,
            independentlyBuilt,
            differentContext,
            differentSchema,
            differentOrder,
            differentArgument
        };

        Assert.Equal(5, actions.Count);
        Assert.Contains(CreateAction(firstProblem.Context, firstSchema, [a, b]), actions);
    }

    [Fact]
    public void ForeignGroundedInputsFailBeforeLocalIndexLookup()
    {
        Domain domain = CreateDomain("shared");
        Problem first = CreateProblem(domain, "first-problem", "a");
        Problem second = CreateProblem(domain, "second-problem", "b");
        Mimir.Core.Schemas.Predicate<Fluent> predicate = Assert.Single(domain.Fluents);
        Constant firstObject = first.ObjectLookup["a"];
        Constant secondObject = second.ObjectLookup["b"];
        Fact<Fluent> firstFact = Assert.IsType<Fact<Fluent>>(first.Context.FindFact(predicate, [firstObject]));
        Fact<Fluent> secondFact = Assert.IsType<Fact<Fluent>>(second.Context.FindFact(predicate, [secondObject]));
        var secondGenerator = new GroundedApplicableActionGenerator(second, second.InitialState, new RpgGrounder());
        Mimir.Core.Grounding.Action secondAction = Assert.Single(secondGenerator.GroundActions);

        Assert.Throws<ArgumentException>(() => first.Context.FindFact(predicate, [secondObject]));
        Assert.Throws<ArgumentException>(() => first.InitialState.IsTrue(secondFact));
        Assert.Throws<ArgumentException>(() => StateFactory.Default.Create(first.Context, [secondFact]));
        Assert.Throws<ArgumentException>(() => first.InitialState.WithAdditionalFluentFacts([secondFact]));
        ExtendedState firstExtendedState = first.InitialState.Expand();
        Assert.Throws<InvalidOperationException>(() => secondAction.IsApplicable(firstExtendedState));
        Assert.Throws<InvalidOperationException>(() => firstExtendedState.Apply(secondAction));
        Assert.Throws<InvalidOperationException>(() => secondGenerator.GetApplicableActions(firstExtendedState));

        Domain foreignDomain = CreateDomain("foreign");
        Mimir.Core.Schemas.Predicate<Fluent> foreignPredicate = Assert.Single(foreignDomain.Fluents);
        Assert.Throws<ArgumentException>(() => first.Context.FindFact(foreignPredicate, [firstObject]));
        Assert.True(first.InitialState.IsTrue(firstFact));
    }

    [Fact]
    public void GroundedGenerator_RejectsTheFirstForeignYieldBeforeAdvancing()
    {
        Domain domain = CreateDomain("shared");
        Problem first = CreateProblem(domain, "first-problem", "a");
        Problem second = CreateProblem(domain, "second-problem", "b");
        GroundAction foreignAction = Assert.Single(new RpgGrounder().Ground(second, second.InitialState));
        bool advancedPastForeignAction = false;

        IEnumerable<GroundAction> Ground(Problem _)
        {
            yield return foreignAction;
            advancedPastForeignAction = true;
            yield break;
        }

        Assert.Throws<InvalidOperationException>(
            () => new GroundedApplicableActionGenerator(
                first,
                first.InitialState,
                new DelegateGrounder(Ground)));
        Assert.False(advancedPastForeignAction);
    }

    [Fact]
    public void GroundedGenerator_RejectsNullActionsExplicitly()
    {
        Domain domain = CreateDomain("shared");
        Problem problem = CreateProblem(domain, "null-action-problem", "a");
        var grounder = new DelegateGrounder(_ => new GroundAction[] { null! });

        InvalidOperationException exception = Assert.Throws<InvalidOperationException>(
            () => new GroundedApplicableActionGenerator(problem, problem.InitialState, grounder));

        Assert.Contains("null action", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Grounding_RejectsStartStateFromAnotherProblem()
    {
        Domain domain = CreateDomain("shared");
        Problem first = CreateProblem(domain, "first-problem", "a");
        Problem second = CreateProblem(domain, "second-problem", "b");

        ArgumentException grounderException = Assert.Throws<ArgumentException>(
            () => new RpgGrounder().Ground(first, second.InitialState));
        ArgumentException generatorException = Assert.Throws<ArgumentException>(
            () => new GroundedApplicableActionGenerator(first, second.InitialState, new RpgGrounder()));

        Assert.Equal("startState", grounderException.ParamName);
        Assert.Equal("startState", generatorException.ParamName);
    }

    private static Domain CreateDomain(string name)
    {
        DomainBuilder builder = new DomainBuilder(name)
            .Requirements().Add(":strips").Add(":typing").Add(":action-costs").Close()
            .Types().Add("thing").Close()
            .Constants().Add("shared", "thing").Close()
            .Predicates().Add("present", ("?x", "thing")).Close()
            .Functions().Add("weight", ("?x", "thing")).Close();
        Domain domain = builder.Actions()
            .Add("use")
                .AddParameter("?x", "thing")
                .AddPrecondition("present", "?x")
                .AddEffect("present", "?x")
                .WithCost(Numeric.Function("weight", "?x"))
                .Close()
            .Close()
            .Build();
        return domain;
    }

    private static Problem CreateProblem(Domain domain, string name, string objectName)
        => new ProblemBuilder(domain, name)
            .Objects().Add(objectName, "thing").Close()
            .InitialState()
                .AddFact("present", objectName)
                .SetValue(Numeric.Function("weight", objectName), 1d)
                .SetValue(Numeric.Function("weight", "shared"), 1d)
                .Close()
            .Goal().Add("present", objectName).Close()
            .Build();

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static WeakReference<Variable> CreateUnreferencedDynamicVariable(Problem problem)
    {
        Variable variable = problem.NewVariable("?temporary", "thing");
        return new WeakReference<Variable>(variable);
    }

    [MethodImpl(MethodImplOptions.NoInlining)]
    private static (WeakReference<Variable>, ConjunctiveCondition)
        CreateConditionWithDynamicVariable(Problem problem)
    {
        Mimir.Core.Schemas.Predicate<Fluent> predicate = Assert.Single(problem.Domain.Fluents);
        Variable variable = problem.NewVariable("?temporary", "thing");
        Atom<Fluent> atom = problem.NewAtom(predicate, [variable]);
        Literal<Atom<Fluent>> literal = problem.NewLiteral(atom, isPositive: true);
        ConjunctiveCondition condition = problem.NewConjunctiveCondition([variable], [literal]);
        return (new WeakReference<Variable>(variable), condition);
    }

    private static void CollectGarbage()
    {
        GC.Collect();
        GC.WaitForPendingFinalizers();
        GC.Collect();
    }

    private static GroundAction CreateAction(
        InstanceContext context,
        ActionSchema schema,
        IReadOnlyList<Constant> arguments)
        => new(
            context,
            schema,
            arguments,
            default,
            default,
            default,
            default,
            default,
            default,
            Array.Empty<Literal<Fact<Derived>>>(),
            Array.Empty<GroundConditionalEffect>(),
            1d);
}
