using System.Collections.Immutable;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Evaluation;
using Xunit;

namespace Mimir.Search.Tests;

public class GoalConditionTests
{
    [Fact]
    public void FromExpression_RejectsPredicateFromAnotherDomain()
    {
        var first = SearchTestHelpers.LoadProblem("ferry");
        var second = SearchTestHelpers.LoadProblem("ferry");
        var expression = new GroundedAtom(
            second.AllPredicates["empty-ferry"],
            Array.Empty<ITerm>());

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(first, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void FromExpression_RejectsObjectFromAnotherProblemSharingTheDomain()
    {
        string domainPath = Path.Combine(SearchTestHelpers.BasePath, "ferry", "domain.pddl");
        Domain domain = Domain.FromFile(domainPath);
        Problem first = CreateFerryProblem(domain, "first-ferry");
        Problem second = CreateFerryProblem(domain, "second-ferry");
        var expression = new GroundedAtom(
            first.AllPredicates["at-ferry"],
            new ITerm[] { second.ObjectLookup["loc1"] });

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(first, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void FromExpression_RejectsFreeVariable()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var expression = new GroundedAtom(
            problem.AllPredicates["at-ferry"],
            new ITerm[] { new Variable("?location", "location") });

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(problem, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void FromExpression_RejectsUnusedQuantifierVariableFromAnotherProblem()
    {
        string domainPath = Path.Combine(SearchTestHelpers.BasePath, "ferry", "domain.pddl");
        Domain domain = Domain.FromFile(domainPath);
        Problem first = CreateFerryProblem(domain, "first-ferry");
        Problem second = CreateFerryProblem(domain, "second-ferry");
        Variable foreignVariable = second.NewVariable("?location", "location");
        var expression = new GroundedForall(
            new[] { foreignVariable },
            new GroundedTrue());

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(first, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void FromExpression_RejectsUnusedQuantifierVariableWithUndeclaredType()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        var expression = new GroundedExists(
            new[] { new Variable("?location", "undeclared") },
            new GroundedTrue());

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(problem, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void FromExpression_RejectsQuantifier()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        Variable variable = problem.NewVariable("?location", "location");
        var expression = new GroundedForall(
            new[] { variable },
            new GroundedTrue());

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(problem, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    [Fact]
    public void Always_IsSatisfiedInAnyState()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var condition = GoalCondition.Always(problem);

        Assert.True(condition.IsSatisfied(problem.InitialState.Expand()));
    }

    [Fact]
    public void ContradictoryLiterals_AreNotSatisfied()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom atom = Atom(problem, "empty-ferry");
        GoalCondition condition = GoalCondition.FromExpression(
            problem,
            And(atom, new GroundedNot(atom)));

        Assert.False(condition.IsSatisfied(problem.InitialState.Expand()));
    }

    [Fact]
    public void IsSatisfied_UsesExpandedDerivedClosureWithoutMutatingContext()
    {
        Domain domain = new DomainBuilder("short-circuit-goal")
            .Requirements().Add(":strips").Add(":adl").Close()
            .Constants().Add("item").Close()
            .Predicates()
                .Add("gate")
                .Add("inner", ("?x", "object"))
                .Add("outer")
                .Close()
            .Actions()
                .Add("clear-gate")
                    .AddPrecondition("gate")
                    .AddEffect("gate", Polarity.Negative)
                    .Close()
                .Close()
            .DerivedPredicates()
                .Define("inner", Logic.Atom("gate"))
                .Define("outer", Logic.Atom("inner", "item"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "short-circuit-goal-problem")
            .InitialState().AddFact("gate").Close()
            .Goal().Add("gate").Add("outer").Close()
            .Build();
        GroundedApplicableActionGenerator generator =
            SearchTestHelpers.CreateGroundedGenerator(problem);
        Mimir.Core.Grounding.Action clearGate =
            SearchTestHelpers.GetApplicableAction(generator, problem.InitialState, "clear-gate");
        State withoutGate = problem.InitialState.Expand().Apply(clearGate);
        GoalCondition condition = GoalCondition.FromProblem(problem);
        Fact<Derived> outer = Assert.Single(
            problem.Context.Derived,
            fact => fact.Predicate.Name == "outer");
        int derivedFactCount = problem.Context.Derived.Count;
        ExtendedState extendedWithoutGate = withoutGate.Expand();

        Assert.False(condition.IsSatisfied(extendedWithoutGate));
        Assert.Equal(derivedFactCount, problem.Context.Derived.Count);

        Assert.False(extendedWithoutGate.IsTrue(outer));
        Assert.Equal(derivedFactCount, problem.Context.Derived.Count);
    }

    [Fact]
    public void Equals_TwoFromProblem_SameProblem_ReturnsTrue()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var a = GoalCondition.FromProblem(problem);
        var b = GoalCondition.FromProblem(problem);

        Assert.True(a.Equals(b));
        Assert.Equal(a.GetHashCode(), b.GetHashCode());
    }

    [Fact]
    public void Equals_DifferentProblems_ReturnsFalse()
    {
        var p1 = SearchTestHelpers.LoadProblem("ferry");
        var p2 = SearchTestHelpers.LoadProblem("gripper");

        Assert.False(GoalCondition.FromProblem(p1).Equals(GoalCondition.FromProblem(p2)));
    }

    [Fact]
    public void Always_CountUnsatisfiedGoals_ReturnsZero()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        var condition = GoalCondition.Always(problem);

        Assert.Equal(0, condition.CountUnsatisfiedGoals(problem.InitialState.Expand()));
    }

    [Fact]
    public void ContradictoryLiterals_CountOneUnsatisfiedGoal()
    {
        var problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom atom = Atom(problem, "empty-ferry");
        GoalCondition condition = GoalCondition.FromExpression(
            problem,
            And(atom, new GroundedNot(atom)));

        Assert.Equal(1, condition.CountUnsatisfiedGoals(problem.InitialState.Expand()));
    }

    [Fact]
    public void ContradictoryDerivedLiterals_CountOneUnsatisfiedGoal()
    {
        Problem problem = CreateContradictoryDerivedProblem("contradictory-derived-goal");
        GoalCondition condition = GoalCondition.FromProblem(problem);

        Assert.Equal(1, condition.CountUnsatisfiedGoals(problem.InitialState.Expand()));
        Assert.False(condition.IsSatisfied(problem.InitialState.Expand()));
    }

    [Fact]
    public void DerivedGoalChecks_AreIndependentAcrossExtendedStates()
    {
        Problem problem = CreateContradictoryDerivedProblem("derived-goal-session");
        GoalCondition condition = GoalCondition.FromProblem(problem);
        ExtendedState first = problem.InitialState.Expand();
        ExtendedState second = problem.InitialState.Expand();

        Assert.NotSame(first, second);
        Assert.Equal(1, condition.CountUnsatisfiedGoals(first));
        Assert.Equal(1, condition.CountUnsatisfiedGoals(second));
    }

    [Fact]
    public void FromExpression_AcceptsNestedConjunctionOfGroundLiterals()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom empty = Atom(problem, "empty-ferry");
        GroundedAtom atFerry = Atom(problem, "at-ferry", "loc3");
        GoalCondition condition = GoalCondition.FromExpression(
            problem,
            And(new GroundedTrue(), And(empty, new GroundedNot(atFerry))));

        Assert.Equal(1, condition.CountUnsatisfiedGoals(problem.InitialState.Expand()));
    }

    [Fact]
    public void FromExpression_DeduplicatesIdenticalLiterals()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom goal = Atom(problem, "at-ferry", "loc1");
        GoalCondition duplicate = GoalCondition.FromExpression(problem, And(goal, goal));
        GoalCondition single = GoalCondition.FromExpression(problem, goal);

        Assert.Equal(single, duplicate);
        Assert.Equal(single.GetHashCode(), duplicate.GetHashCode());
        Assert.Equal(1, duplicate.CountUnsatisfiedGoals(problem.InitialState.Expand()));
    }

    [Fact]
    public void Equals_IgnoresLiteralOrderAndConjunctionNesting()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom first = Atom(problem, "at", "car1", "loc1");
        GroundedAtom second = Atom(problem, "at", "car2", "loc1");
        GoalCondition ordered = GoalCondition.FromExpression(problem, And(first, second));
        GoalCondition reversed = GoalCondition.FromExpression(
            problem,
            And(And(second), first));

        Assert.Equal(ordered, reversed);
        Assert.Equal(ordered.GetHashCode(), reversed.GetHashCode());
    }

    [Fact]
    public void Always_EqualsOtherEmptyConjunctions()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GoalCondition emptyAnd = GoalCondition.FromExpression(problem, And());
        GoalCondition groundedTrue = GoalCondition.FromExpression(problem, new GroundedTrue());

        Assert.Equal(GoalCondition.Always(problem), emptyAnd);
        Assert.Equal(emptyAnd, groundedTrue);
    }

    [Fact]
    public void FromExpression_RejectsNonConjunctiveExpressions()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        GroundedAtom atom = Atom(problem, "empty-ferry");
        IGroundedExpression[] expressions =
        [
            new GroundedOr(ImmutableArray.Create<IGroundedExpression>(atom)),
            new GroundedImply(atom, atom),
            new GroundedNot(new GroundedNot(atom))
        ];

        foreach (IGroundedExpression expression in expressions)
        {
            ArgumentException exception = Assert.Throws<ArgumentException>(
                () => GoalCondition.FromExpression(problem, expression));
            Assert.Equal("expression", exception.ParamName);
        }
    }

    [Fact]
    public void FromExpression_RejectsConstantWithIncompatibleType()
    {
        Problem problem = SearchTestHelpers.LoadProblem("ferry");
        var expression = new GroundedAtom(
            problem.AllPredicates["at-ferry"],
            new ITerm[] { problem.ObjectLookup["car1"] });

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => GoalCondition.FromExpression(problem, expression));

        Assert.Equal("expression", exception.ParamName);
    }

    private static GroundedAtom Atom(
        Problem problem,
        string predicate,
        params string[] arguments)
    {
        ITerm[] terms = arguments
            .Select(argument => (ITerm)problem.ObjectLookup[argument])
            .ToArray();
        return new GroundedAtom(problem.AllPredicates[predicate], terms);
    }

    private static GroundedAnd And(params IGroundedExpression[] expressions)
        => new(ImmutableArray.CreateRange(expressions));

    private static Problem CreateFerryProblem(Domain domain, string name)
        => new ProblemBuilder(domain, name)
            .Objects()
                .Add("car1", "car")
                .Add("car2", "car")
                .Add("loc1", "location")
                .Add("loc3", "location")
                .Close()
            .InitialState()
                .AddFact("empty-ferry")
                .AddFact("at-ferry", "loc3")
                .AddFact("at", "car1", "loc3")
                .AddFact("at", "car2", "loc3")
                .Close()
            .Goal().Add("at", "car1", "loc1").Add("at", "car2", "loc1").Close()
            .Build();

    private static Problem CreateContradictoryDerivedProblem(string name)
    {
        Domain domain = new DomainBuilder(name)
            .Requirements().Add(":strips").Add(":adl").Close()
            .Predicates().Add("base").Add("gate").Close()
            .DerivedPredicates().Define("gate", Logic.Atom("base")).Close()
            .Build();
        return new ProblemBuilder(domain, $"{name}-problem")
            .InitialState().AddFact("base").Close()
            .Goal().Add("gate").Add("gate", Polarity.Negative).Close()
            .Build();
    }

}
