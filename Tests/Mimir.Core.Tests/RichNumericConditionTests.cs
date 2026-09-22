using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class RichNumericConditionTests
{
    public static IEnumerable<object[]> Conditions()
    {
        (string Condition, string Initial, string Next)[] cases =
        [
            ("(or (> (fuel ?t) 2) (marked ?t))", "a", ""),
            ("(not (= (fuel ?t) 4))", "b", "a,b"),
            ("(imply (> (fuel ?t) 2) (< (fuel ?t) 4))", "b", "a,b"),
            ("(exists (?u - truck) (and (> (fuel ?t) (fuel ?u)) (< (fuel ?u) 2)))", "a", "a"),
            ("(forall (?u - truck) (>= (fuel ?t) (fuel ?u)))", "a", "a"),
            ("(exists (?u ?v - truck) (and (< (fuel ?u) (fuel ?v)) (> (+ (fuel ?t) (fuel ?u)) (fuel ?v))))", "a", "a"),
            ("(forall (?u - truck) (exists (?v - truck) (>= (fuel ?t) (+ (fuel ?u) (- (fuel ?v) 2)))))", "a", "a,b"),
            ("(not (or (= (fuel ?t) 4) (marked ?t)))", "b", "a,b"),
            ("(or (and) (> (fuel ?t) 0))", "a,b", "a,b")
        ];
        foreach (ApplicableActionGeneratorType type in new[] { ApplicableActionGeneratorType.Grounded, ApplicableActionGeneratorType.Lifted })
            foreach ((string condition, string initial, string next) in cases)
                yield return [type, condition, initial, next];
    }

    [Theory]
    [MemberData(nameof(Conditions))]
    public void RichPreconditionsAgreeWithDirectApplicabilityAcrossNumericStates(
        ApplicableActionGeneratorType type, string condition, string initialBindings, string nextBindings)
    {
        Domain domain = Domain.FromText($$"""
(define (domain rich) (:requirements :adl :numeric-fluents)
 (:types truck) (:predicates (marked ?t - truck) (done ?t - truck)) (:functions (fuel ?t - truck))
 (:action choose :parameters (?t - truck) :precondition {{condition}} :effect (done ?t))
 (:action drain :parameters (?t - truck) :effect (decrease (fuel ?t) 2)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain rich) (:objects a b - truck)
 (:init (= (fuel a) 4) (= (fuel b) 1)) (:goal (and)))
""", type);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        ActionSchema choose = domain.Actions.Single(action => action.Name == "choose");
        Action drain = problem.GroundAction(domain.Actions.Single(action => action.Name == "drain"), problem.ObjectLookup["a"]);
        State next = problem.InitialState.Expand().Apply(drain);
        Check(problem.InitialState, initialBindings);
        Check(next, nextBindings);
        Check(problem.InitialState, initialBindings);

        void Check(State state, string expected)
        {
            string[] actual = generator.GetApplicableActions(state.Expand()).Where(action => action.Schema == choose)
                .Select(action => action.Arguments[0].Name).Order().ToArray();
            Assert.Equal(expected.Split(',', StringSplitOptions.RemoveEmptyEntries), actual);
            foreach (string name in new[] { "a", "b" })
                Assert.Equal(actual.Contains(name), problem.GroundAction(choose, problem.ObjectLookup[name]).IsApplicable(state.Expand()));
        }
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void DisjunctiveGuardsFireOnceAndReadTheSourceState(ApplicableActionGeneratorType type)
    {
        Domain domain = Domain.FromText("""
(define (domain guarded) (:requirements :adl :numeric-fluents :action-costs)
 (:types item) (:predicates (done)) (:functions (fuel) (saved ?x - item) (total-cost))
 (:action act :parameters () :precondition (and)
  :effect (and (assign (fuel) 0) (increase (total-cost) 3)
   (when (or (> (fuel) 1) (< (fuel) 5)) (done))
   (forall (?x - item) (when (or (> (fuel) 1) (< (fuel) 5)) (increase (saved ?x) (fuel)))))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain guarded) (:objects a b - item)
 (:init (= (fuel) 4) (= (saved a) 0) (= (saved b) 1) (= (total-cost) 0))
 (:goal (done)) (:metric minimize (total-cost)))
""", type);
        Action action = Assert.Single(problem.GetApplicableActionGenerator(problem.InitialState).GetApplicableActions(problem.InitialState.Expand()));
        State next = problem.InitialState.Expand().Apply(action);
        Assert.Equal(0, next.Value(problem.FunctionCall("fuel")));
        Assert.Equal(4, next.Value(problem.FunctionCall("saved", "a")));
        Assert.Equal(5, next.Value(problem.FunctionCall("saved", "b")));
        Assert.True(next.Value(problem.Atom("done")));
        Assert.Equal(3, action.Cost);
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void NumericDerivedDependenciesAndPositiveRecursionReevaluatePerState(ApplicableActionGeneratorType type)
    {
        Domain domain = Domain.FromText("""
(define (domain derived) (:requirements :adl :derived-predicates :numeric-fluents)
 (:types item) (:predicates (link ?x ?y - item) (active ?x - item) (reachable ?x - item) (inactive ?x - item) (done ?x - item))
 (:functions (fuel ?x - item))
 (:derived (active ?x - item) (> (fuel ?x) 0))
 (:derived (reachable ?x - item) (or (active ?x) (exists (?y - item) (and (link ?x ?y) (reachable ?y)))))
 (:derived (inactive ?x - item) (not (reachable ?x)))
 (:action choose :parameters (?x - item) :precondition (inactive ?x) :effect (done ?x))
 (:action drain :parameters (?x - item) :effect (assign (fuel ?x) 0)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain derived) (:objects a b c - item)
 (:init (link a b) (link b a) (= (fuel a) 0) (= (fuel b) 1) (= (fuel c) 0)) (:goal (and)))
""", type);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        State initial = problem.InitialState;
        Action drain = problem.GroundAction(domain.Actions.Single(action => action.Name == "drain"), problem.ObjectLookup["b"]);
        Assert.True(initial.Value(problem.Atom("reachable", "a")));
        Assert.False(initial.Value(problem.Atom("inactive", "a")));
        State next = initial.Expand().Apply(drain);
        Assert.False(next.Value(problem.Atom("reachable", "a")));
        Assert.True(next.Value(problem.Atom("inactive", "a")));
        Assert.True(initial.Value(problem.Atom("reachable", "a")));
        Assert.Equal(1, generator.GetApplicableActions(initial.Expand()).Count(action => action.Schema.Name == "choose"));
        Assert.Equal(3, generator.GetApplicableActions(next.Expand()).Count(action => action.Schema.Name == "choose"));
    }

    [Theory]
    [InlineData("(forall (?x - item) (> (fuel ?x) 0))", true)]
    [InlineData("(exists (?x - item) (> (fuel ?x) 0))", false)]
    [InlineData("(not (exists (?x - item) (> (fuel ?x) 0)))", true)]
    public void EmptyQuantifierDomainsHaveBooleanSemantics(string condition, bool expected)
    {
        Domain domain = Domain.FromText($$"""
(define (domain empty) (:requirements :adl :numeric-fluents)
 (:types item) (:predicates (done)) (:functions (fuel ?x - item))
 (:action act :parameters () :precondition {{condition}} :effect (done)))
""");
        foreach (ApplicableActionGeneratorType type in new[] { ApplicableActionGeneratorType.Grounded, ApplicableActionGeneratorType.Lifted })
        {
            Problem problem = Problem.FromText(domain, "(define (problem p) (:domain empty) (:init) (:goal (done)))", type);
            Assert.Equal(expected, problem.GroundAction(domain.Actions[0]).IsApplicable(problem.InitialState.Expand()));
            Assert.Equal(expected ? 1 : 0, problem.GetApplicableActionGenerator(problem.InitialState).GetApplicableActions(problem.InitialState.Expand()).Count());
        }
    }

    [Theory]
    [InlineData("(or (> (fuel) 0) (< (fuel) 4))")]
    [InlineData("(not (= (fuel) 0))")]
    [InlineData("(exists (?x - item) (> (fuel) 0))")]
    [InlineData("(forall (?x - item) (> (fuel) 0))")]
    public void NumericGoalsRemainConjunctive(string goal)
    {
        Domain domain = Domain.FromText("(define (domain goals) (:requirements :adl :numeric-fluents) (:types item) (:functions (fuel)))");
        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, Assert.Throws<PddlLoadException>(() => Problem.FromText(domain,
            $"(define (problem p) (:domain goals) (:objects a - item) (:init (= (fuel) 1)) (:goal {goal}))")).ErrorCode);
    }
    [Fact]
    public void StaticNumericDerivedTruthIsScopedToEachProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain static-derived) (:requirements :adl :derived-predicates :numeric-fluents)
 (:predicates (available) (done)) (:functions (capacity))
 (:derived (available) (not (<= (capacity) 0)))
 (:action act :parameters () :precondition (available) :effect (done)))
""");
        foreach (int capacity in new[] { 0, 2, 0 })
        {
            Problem problem = Problem.FromText(domain,
                $"(define (problem p) (:domain static-derived) (:init (= (capacity) {capacity})) (:goal (done)))");
            Assert.True(problem.HasNumericPlanning);
            Assert.Empty(problem.InitialState.NumericValues.ToArray());
            Assert.Equal(capacity > 0, problem.InitialState.Value(problem.Atom("available")));
            Assert.Equal(capacity > 0, problem.GroundAction(domain.Actions[0]).IsApplicable(problem.InitialState.Expand()));
        }
    }

    [Theory]
    [InlineData(false, false)]
    [InlineData(true, false)]
    [InlineData(false, true)]
    public void PublicDerivedExpressionsValidateNumericOwnershipAndScope(bool foreignFunction, bool unboundVariable)
    {
        var parameter = new Variable("?x", "object");
        var predicate = new Mimir.Core.Schemas.Predicate<Derived>("available", [parameter]);
        var function = new NumericFunction("fuel", [new Variable("?x", "object")]);
        NumericFunction usedFunction = foreignFunction ? new NumericFunction("fuel", function.Parameters) : function;
        Variable usedVariable = unboundVariable ? new Variable("?x", "object") : parameter;
        var comparison = new FunctionCall(usedFunction, [usedVariable]).GreaterThan(new NumericConstant(0));
        Domain Build() => new("public-derived", [], [], [new DerivedPredicateDefinition(predicate, new GroundedNot(comparison))],
            [function], [], [], new Dictionary<string, string>(), [":adl", ":derived-predicates", ":numeric-fluents"]);
        if (foreignFunction || unboundVariable)
        {
            Assert.Throws<ArgumentException>(Build);
            return;
        }
        Domain domain = Build();
        Problem problem = Problem.FromText(domain,
            "(define (problem p) (:domain public-derived) (:objects a) (:init (= (fuel a) 0)) (:goal (and)))");
        Assert.True(problem.InitialState.Value(problem.Atom("available", "a")));
    }

    [Theory]
    [InlineData("(when (> (fuel) 0) (increase (total-cost) 1))", "Conditional")]
    [InlineData("(when (or (and) (> (fuel) 0)) (increase (total-cost) 1))", "Conditional")]
    [InlineData("(when (or) (increase (total-cost) 1))", "Conditional")]
    [InlineData("(forall (?x - item) (increase (total-cost) 1))", "Quantified")]
    public void GuardedAndQuantifiedCostsAreRejectedBeforeNormalization(string effect, string reason)
    {
        PddlLoadException error = Assert.Throws<PddlLoadException>(() => Domain.FromText($$"""
(define (domain costs) (:requirements :adl :numeric-fluents :action-costs)
 (:types item) (:functions (fuel) (total-cost))
 (:action act :parameters () :effect {{effect}}))
"""));
        Assert.Contains(reason + " action costs", error.Message);
    }

}
