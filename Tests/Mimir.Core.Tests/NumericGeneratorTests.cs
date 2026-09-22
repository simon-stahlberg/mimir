using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class NumericGeneratorTests
{
    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void NumericChangesEnableActionsAndConditionalBooleanReachability(ApplicableActionGeneratorType type)
    {
        Domain domain = Domain.FromText("""
(define (domain refuel) (:requirements :strips :numeric-fluents :conditional-effects)
 (:predicates (ready) (done)) (:functions (fuel))
 (:action refuel :parameters () :precondition (< (fuel) 2) :effect (increase (fuel) 1))
 (:action trigger :parameters () :precondition (and) :effect (when (>= (fuel) 2) (ready)))
 (:action finish :parameters () :precondition (ready) :effect (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain refuel) (:init (= (fuel) 0)) (:goal (done)))
""", type);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        State initial = problem.InitialState;
        State full = initial;
        Action refuel = problem.GroundAction(domain.Actions.Single(action => action.Name == "refuel"));
        full = full.Expand().Apply(refuel).Expand().Apply(refuel);
        Assert.NotEqual(initial, full);
        Assert.Contains(generator.GetApplicableActions(initial.Expand()), action => action.Schema.Name == "refuel");
        Assert.DoesNotContain(generator.GetApplicableActions(full.Expand()), action => action.Schema.Name == "refuel");
        Action trigger = generator.GetApplicableActions(full.Expand()).Single(action => action.Schema.Name == "trigger");
        State ready = full.Expand().Apply(trigger);
        Assert.Contains(generator.GetApplicableActions(ready.Expand()), action => action.Schema.Name == "finish");
        Assert.DoesNotContain(generator.GetApplicableActions(initial.Expand()), action => action.Schema.Name == "finish");
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded, false)]
    [InlineData(ApplicableActionGeneratorType.Lifted, false)]
    [InlineData(ApplicableActionGeneratorType.Grounded, true)]
    [InlineData(ApplicableActionGeneratorType.Lifted, true)]
    public void NumericWitnessesAreCheckedBeforeRepresentativeSelectionAndLimits(
        ApplicableActionGeneratorType type, bool higherArity)
    {
        string parameters = higherArity ? "?x ?w ?z - item" : "?x ?w - item";
        string right = higherArity ? "(+ (threshold ?w) (threshold ?z))" : "(threshold ?w)";
        Domain domain = Domain.FromText($$"""
(define (domain witness) (:requirements :strips :typing :numeric-fluents)
 (:types item) (:predicates (done ?x - item))
 (:functions (value ?x - item) (threshold ?x - item))
 (:action choose :parameters ({{parameters}})
  :precondition (>= (value ?x) {{right}})
  :effect (and (done ?x) (decrease (value ?x) 1))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain witness) (:objects a b c - item)
 (:init (= (value a) 0) (= (value b) 2) (= (value c) 3)
        (= (threshold a) 10) (= (threshold b) 2) (= (threshold c) 1)) (:goal (and)))
""", type);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        State state = problem.InitialState;
        for (int depth = 0; depth < 4; depth++)
        {
            Action[] actual = generator.GetApplicableActions(state.Expand()).ToArray();
            HashSet<State> expected = EnumerateGroundActions(problem).Where(action => action.IsApplicable(state.Expand()))
                .Select(action => state.Expand().Apply(action)).ToHashSet();
            Assert.Equal(expected.Count, actual.Length);
            Assert.True(expected.SetEquals(actual.Select(action => state.Expand().Apply(action))));
            for (int limit = 0; limit <= actual.Length + 1; limit++)
                Assert.Equal(actual.Take(limit), generator.GetApplicableActions(state.Expand(), limit));
            if (actual.Length == 0) break;
            state = state.Expand().Apply(actual[0]);
        }
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void StaticNumericConditionsAndFalseConditionalGuardsPruneReachability(ApplicableActionGeneratorType type)
    {
        Domain domain = Domain.FromText("""
(define (domain static-numeric) (:requirements :strips :typing :numeric-fluents :conditional-effects)
 (:types item) (:predicates (ready) (done)) (:functions (rank ?x - item))
 (:action select :parameters (?x ?y - item)
  :precondition (and (> (rank ?x) 0) (< (rank ?x) (rank ?y))) :effect (done))
 (:action trigger :parameters () :precondition (and) :effect (when (> 0 1) (ready)))
 (:action finish :parameters () :precondition (ready) :effect (done)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain static-numeric) (:objects a b - item)
 (:init (= (rank a) 1) (= (rank b) 2)) (:goal (done)))
""", type);
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(problem.InitialState);
        Action selected = generator.GetApplicableActions(problem.InitialState.Expand()).Single(action => action.Schema.Name == "select");
        Assert.Equal(new[] { "a", "b" }, selected.Arguments.Select(argument => argument.Name));
        if (generator is GroundedApplicableActionGenerator grounded)
            Assert.DoesNotContain(grounded.GroundActions, action => action.Schema.Name == "finish");
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void GeneratedQuantifiedUpdatesPreserveMultiplicityAndSourceValues(ApplicableActionGeneratorType type)
    {
        Problem source = NumericPlanningTests.Load("""
(assign (saved) (fuel ?t))
(forall (?u - truck) (when (>= (fuel ?t) 4) (increase (fuel ?t) 1)))
""");
        IApplicableActionGenerator generator = type == ApplicableActionGeneratorType.Grounded
            ? new GroundedApplicableActionGenerator(source, source.InitialState, new RpgGrounder())
            : new CliqueApplicableActionGenerator(source);
        Action action = generator.GetApplicableActions(source.InitialState.Expand()).Single(action => action.Arguments[0].Name == "a");
        State next = source.InitialState.Expand().Apply(action);
        Assert.Equal(6, next.Value(source.FunctionCall("fuel", "a")));
        Assert.Equal(4, next.Value(source.FunctionCall("saved")));
        Assert.Equal(4, action.Cost);
    }

    [Theory]
    [InlineData(ApplicableActionGeneratorType.Grounded)]
    [InlineData(ApplicableActionGeneratorType.Lifted)]
    public void UndefinedNumericReferencesMakeBindingsInapplicable(ApplicableActionGeneratorType type)
    {
        Domain domain = Domain.FromText("""
(define (domain missing) (:requirements :strips :typing :numeric-fluents)
 (:types item) (:functions (value ?x - item))
 (:action use :parameters (?x - item) :precondition (> (value ?x) 0) :effect (decrease (value ?x) 1)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem p) (:domain missing) (:objects a - item) (:init) (:goal (and)))
""", type);
        Assert.Empty(problem.GetApplicableActionGenerator(problem.InitialState).GetApplicableActions(problem.InitialState.Expand()));
    }

    [Fact]
    public void PublicNumericBindingsReevaluateTheCurrentStateAndAllowNestedEnumeration()
    {
        Problem problem = NumericPlanningTests.Load("(decrease (fuel ?t) 2)");
        Variable variable = problem.NewVariable("?t", "truck");
        NumericFunction function = problem.Domain.Functions.Single(function => function.Name == "fuel");
        FunctionCall call = problem.NewFunctionCall(function, [variable]);
        ConjunctiveCondition condition = problem.NewConjunctiveCondition([variable], [],
            [new NumericComparison(call, ComparisonOperator.GreaterThanOrEqual, new NumericConstant(4))]);
        var generator = new ConjunctiveConditionBindingGenerator();
        CompiledConjunctiveCondition compiled = generator.Compile(condition, problem);
        Assert.Equal(2, generator.CountBindings(compiled, problem.InitialState.Expand()));
        Action action = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"]);
        State next = problem.InitialState.Expand().Apply(action);
        Assert.Equal(1, generator.CountBindings(compiled, next.Expand()));
        Assert.Equal(1, generator.EnumerateBindings(compiled, next.Expand(), values =>
        {
            Assert.Equal("b", values[0].Name);
            Assert.Equal(2, generator.CountBindings(compiled, problem.InitialState.Expand()));
            return true;
        }));
    }

    private static IEnumerable<Action> EnumerateGroundActions(Problem problem)
    {
        foreach (ActionSchema schema in problem.Domain.Actions)
        {
            var binding = new Constant[schema.Parameters.Count];
            foreach (Action action in Bind(0)) yield return action;
            IEnumerable<Action> Bind(int index)
            {
                if (index == binding.Length)
                {
                    yield return problem.GroundAction(schema, binding);
                    yield break;
                }
                foreach (Constant value in problem.Context.GetCompatibleObjects(schema.Parameters[index].Type))
                {
                    binding[index] = value;
                    foreach (Action action in Bind(index + 1)) yield return action;
                }
            }
        }
    }
}
