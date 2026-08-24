using System;
using System.IO;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class DomainTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Fact]
    public void AdlRequirementMetadataIncludesCanonicalImplications()
    {
        Domain domain = Domain.FromText("""
(define (domain requirement-metadata)
  (:requirements :adl)
  (:predicates (done)))
""");

        Assert.Equal([":adl"], domain.Requirements);
        Assert.Equal(
            [
                ":strips",
                ":typing",
                ":equality",
                ":negative-preconditions",
                ":disjunctive-preconditions",
                ":conditional-effects",
                ":existential-preconditions",
                ":universal-preconditions",
                ":adl",
            ],
            domain.ExpandedRequirements);
        Assert.True(domain.UsesTyping);
        Assert.True(domain.UsesEquality);
        Assert.True(domain.UsesConditionalEffects);
    }

    [Fact]
    public void EmptyRequirementsImplyOnlyStrips()
    {
        Domain domain = Domain.FromText("""
(define (domain implicit-strips)
  (:predicates (done)))
""");

        Assert.Empty(domain.Requirements);
        Assert.Equal([":strips"], domain.ExpandedRequirements);
        Assert.False(domain.UsesTyping);
        Assert.False(domain.UsesEquality);
        Assert.False(domain.UsesConditionalEffects);
    }

    [Fact]
    public void TypeHierarchyPreservesDeclaredOrderAndDirectParents()
    {
        Domain domain = Domain.FromText("""
(define (domain hierarchy)
  (:requirements :typing)
  (:types vehicle - object car - vehicle location - object)
  (:predicates (at ?vehicle - vehicle ?location - location)))
""");

        Assert.Equal(["vehicle", "car", "location"], domain.TypeHierarchy.Keys);
        Assert.Equal("object", domain.TypeHierarchy["vehicle"]);
        Assert.Equal("vehicle", domain.TypeHierarchy["car"]);
        Assert.Equal("object", domain.TypeHierarchy["location"]);
    }

    [Theory]
    [InlineData("blocks_4", "blocksworld", 5, 0, 0, 4)]
    [InlineData("rovers", "rover", 14, 11, 0, 9)]
    [InlineData("logistics", "logistics-strips", 2, 7, 0, 6)]
    [InlineData("airport", "airport", 8, 8, 10, 5)]
    [InlineData("assembly", "assembly", 4, 7, 20, 4)]
    [InlineData("barman", "barman", 10, 5, 0, 12)]
    [InlineData("childsnack", "child-snack", 8, 6, 0, 6)]
    [InlineData("delivery", "delivery", 3, 2, 0, 3)]
    [InlineData("driverlog", "driverlog", 4, 2, 0, 6)]
    [InlineData("ferry", "ferry", 4, 0, 0, 3)]
    [InlineData("grid", "grid", 6, 6, 0, 5)]
    [InlineData("gripper", "gripper-strips", 4, 3, 0, 3)]
    [InlineData("hiking", "hiking", 6, 3, 0, 7)]
    [InlineData("miconic", "miconic", 4, 2, 0, 4)]
    [InlineData("satellite", "satellite", 5, 3, 0, 5)]
    [InlineData("visitall", "grid-visit-all", 2, 1, 0, 1)]
    [InlineData("miconic-fulladl", "miconic", 3, 13, 35, 3)]
    [InlineData("miconic-simpleadl", "miconic", 3, 4, 0, 3)]
    [InlineData("schedule", "schedule", 8, 4, 0, 9)]
    public void ParsesBenchmarkDomains(
        string domainDir, string expectedName,
        int expectedFluents, int expectedStatics, int expectedDerived, int expectedActions)
    {
        var path = Path.Combine(BasePath, domainDir, "domain.pddl");
        Assert.True(File.Exists(path), $"Domain file not found: {path}");

        var domain = Domain.FromFile(path);

        Assert.Equal(expectedName, domain.Name);
        Assert.Equal(expectedFluents, domain.Fluents.Count);
        Assert.Equal(expectedStatics, domain.Statics.Count);
        Assert.Equal(expectedDerived, domain.Derived.Count);
        Assert.Equal(expectedActions, domain.Actions.Count);
    }

    [Theory]
    [InlineData("refuel")]
    [InlineData("refuel-adl")]
    [InlineData("zenotravel")]
    public void RejectsBenchmarkDomainsWithUnsupportedNumericFragments(string domainDir)
    {
        string path = Path.Combine(BasePath, domainDir, "domain.pddl");

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => Domain.FromFile(path));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Equal(path, exception.SourcePath);
    }

    [Fact]
    public void BlocksWorldSchemaInspection()
    {
        var domain = Domain.FromFile(Path.Combine(BasePath, "blocks_4", "domain.pddl"));

        // All 5 predicates are fluent in blocksworld (no statics, no types)
        Assert.Equal(5, domain.Fluents.Count);
        Assert.Empty(domain.Statics);
        Assert.Empty(domain.Derived);

        var predicateNames = domain.Fluents.Select(p => p.Name).ToHashSet();
        Assert.Contains("clear", predicateNames);
        Assert.Contains("on-table", predicateNames);
        Assert.Contains("arm-empty", predicateNames);
        Assert.Contains("holding", predicateNames);
        Assert.Contains("on", predicateNames);

        // === pickup ===
        var pickup = domain.Actions.Single(a => a.Name == "pickup");
        Assert.Single(pickup.Parameters);
        Assert.Equal("?ob", pickup.Parameters[0].Name);

        // Preconditions: (clear ?ob), (on-table ?ob), (arm-empty)
        Assert.Equal(3, pickup.FluentPreconditions.Count);
        Assert.All(pickup.FluentPreconditions, p => Assert.True(p.IsPositive));
        AssertContainsPredicate(pickup.FluentPreconditions, "clear", Polarity.Positive);
        AssertContainsPredicate(pickup.FluentPreconditions, "on-table", Polarity.Positive);
        AssertContainsPredicate(pickup.FluentPreconditions, "arm-empty", Polarity.Positive);

        // Effects: +holding(?ob), -clear(?ob), -on-table(?ob), -arm-empty()
        Assert.Equal(4, pickup.Effects.Count);
        Assert.All(pickup.Effects, e => Assert.Empty(e.QuantifiedVariables));
        Assert.All(pickup.Effects, e => Assert.Empty(e.FluentConditions));
        AssertContainsEffect(pickup.Effects, "holding", Polarity.Positive);
        AssertContainsEffect(pickup.Effects, "clear", Polarity.Negative);
        AssertContainsEffect(pickup.Effects, "on-table", Polarity.Negative);
        AssertContainsEffect(pickup.Effects, "arm-empty", Polarity.Negative);

        // === putdown ===
        var putdown = domain.Actions.Single(a => a.Name == "putdown");
        Assert.Single(putdown.Parameters);

        // Preconditions: (holding ?ob)
        Assert.Single(putdown.FluentPreconditions);
        AssertContainsPredicate(putdown.FluentPreconditions, "holding", Polarity.Positive);

        // Effects: +clear, +arm-empty, +on-table, -holding
        Assert.Equal(4, putdown.Effects.Count);
        AssertContainsEffect(putdown.Effects, "clear", Polarity.Positive);
        AssertContainsEffect(putdown.Effects, "arm-empty", Polarity.Positive);
        AssertContainsEffect(putdown.Effects, "on-table", Polarity.Positive);
        AssertContainsEffect(putdown.Effects, "holding", Polarity.Negative);

        // === stack ===
        var stack = domain.Actions.Single(a => a.Name == "stack");
        Assert.Equal(2, stack.Parameters.Count);

        // Preconditions: (clear ?underob), (holding ?ob)
        Assert.Equal(2, stack.FluentPreconditions.Count);
        AssertContainsPredicate(stack.FluentPreconditions, "clear", Polarity.Positive);
        AssertContainsPredicate(stack.FluentPreconditions, "holding", Polarity.Positive);

        // Effects: +arm-empty, +clear(?ob), +on(?ob,?underob), -clear(?underob), -holding(?ob)
        Assert.Equal(5, stack.Effects.Count);
        AssertContainsEffect(stack.Effects, "arm-empty", Polarity.Positive);
        AssertContainsEffect(stack.Effects, "clear", Polarity.Positive);
        AssertContainsEffect(stack.Effects, "on", Polarity.Positive);
        AssertContainsEffect(stack.Effects, "clear", Polarity.Negative);
        AssertContainsEffect(stack.Effects, "holding", Polarity.Negative);

        // === unstack ===
        var unstack = domain.Actions.Single(a => a.Name == "unstack");
        Assert.Equal(2, unstack.Parameters.Count);

        // Preconditions: (on ?ob ?underob), (clear ?ob), (arm-empty)
        Assert.Equal(3, unstack.FluentPreconditions.Count);
        AssertContainsPredicate(unstack.FluentPreconditions, "on", Polarity.Positive);
        AssertContainsPredicate(unstack.FluentPreconditions, "clear", Polarity.Positive);
        AssertContainsPredicate(unstack.FluentPreconditions, "arm-empty", Polarity.Positive);

        // Effects: +holding, +clear, -on, -clear, -arm-empty
        Assert.Equal(5, unstack.Effects.Count);
        AssertContainsEffect(unstack.Effects, "holding", Polarity.Positive);
        AssertContainsEffect(unstack.Effects, "clear", Polarity.Positive);
        AssertContainsEffect(unstack.Effects, "on", Polarity.Negative);
        AssertContainsEffect(unstack.Effects, "clear", Polarity.Negative);
        AssertContainsEffect(unstack.Effects, "arm-empty", Polarity.Negative);
    }

    [Fact]
    public void FromTextParsesDomainPddl()
    {
        var domain = Domain.FromText("""
(define (domain test-domain)
  (:requirements :strips :negative-preconditions)
  (:predicates (p ?x))
  (:action a
    :parameters (?x)
    :precondition (and (not (p ?x)))
    :effect (p ?x)))
""");

        Assert.Equal("test-domain", domain.Name);
        Assert.Equal("a", domain.Actions.Single().Name);
    }

    [Fact]
    public void ParsedDomainOmitsExplicitTotalCostFromNumericFunctions()
    {
        Domain domain = Domain.FromText("""
(define (domain explicit-total-cost)
  (:requirements :strips :action-costs)
  (:predicates (done))
  (:functions (total-cost) (fare))
  (:action finish
    :parameters ()
    :effect (and (done) (increase (total-cost) (fare)))))
""");

        NumericFunction fare = Assert.Single(domain.Functions);
        Assert.Equal("fare", fare.Name);
        var cost = Assert.IsType<NumericFunctionActionCostExpression>(domain.Actions.Single().CostExpression);
        Assert.Same(fare, cost.Function);
    }

    [Fact]
    public void ParsedDomainOmitsImplicitTotalCostFromNumericFunctions()
    {
        Domain domain = Domain.FromText("""
(define (domain implicit-total-cost)
  (:requirements :strips :action-costs)
  (:predicates (done))
  (:action finish
    :parameters ()
    :effect (and (done) (increase (total-cost) 3))))
""");

        Assert.Empty(domain.Functions);
        ConstantActionCostExpression cost = Assert.IsType<ConstantActionCostExpression>(
            domain.Actions.Single().CostExpression);
        Assert.Equal(3d, cost.Value);
    }

    [Fact]
    public void ProblemRejectsQueryingTotalCostAsNumericFunctionValue()
    {
        Domain domain = Domain.FromText("""
(define (domain total-cost-query)
  (:requirements :strips :action-costs)
  (:predicates (done)))
""");
        Problem problem = Problem.FromText(
            domain,
            """
(define (problem p)
  (:domain total-cost-query)
  (:init (= (total-cost) 0))
  (:goal (done))
  (:metric minimize (total-cost)))
""");
        var totalCost = new NumericFunction("total-cost", Array.Empty<Variable>());

        NotSupportedException exception = Assert.Throws<NotSupportedException>(
            () => problem.GetNumericFunctionValue(totalCost, Array.Empty<Constant>()));

        Assert.Contains("planner bookkeeping", exception.Message);
    }

    [Fact]
    public void MiconicFullAdlSchemaInspection()
    {
        var domain = Domain.FromFile(Path.Combine(BasePath, "miconic-fulladl", "domain.pddl"));

        // Fluents: boarded, served, lift-at (predicates modified by effects)
        Assert.Equal(3, domain.Fluents.Count);
        var fluentNames = domain.Fluents.Select(p => p.Name).ToHashSet(StringComparer.OrdinalIgnoreCase);
        Assert.Contains("boarded", fluentNames);
        Assert.Contains("served", fluentNames);
        Assert.Contains("lift-at", fluentNames);

        // Statics: all the passenger properties + floor topology
        Assert.Equal(13, domain.Statics.Count);
        var staticNames = domain.Statics.Select(p => p.Name).ToHashSet(StringComparer.OrdinalIgnoreCase);
        Assert.Contains("going_up", staticNames);
        Assert.Contains("going_down", staticNames);
        Assert.Contains("origin", staticNames);
        Assert.Contains("destin", staticNames);
        Assert.Contains("above", staticNames);
        Assert.Contains("vip", staticNames);
        Assert.Contains("conflict_A", staticNames);
        Assert.Contains("conflict_B", staticNames);

        // Derived predicates extracted by the canonicalizer from complex ADL preconditions
        Assert.Equal(35, domain.Derived.Count);

        // === stop ===
        var stop = domain.Actions.Single(a => a.Name == "stop");
        Assert.Single(stop.Parameters);
        Assert.Equal("?f", stop.Parameters[0].Name);
        Assert.Equal("floor", stop.Parameters[0].Type);

        // 1 fluent precondition: lift-at(?f)
        Assert.Single(stop.FluentPreconditions);
        Assert.True(stop.FluentPreconditions[0].IsPositive);
        Assert.Equal("lift-at", stop.FluentPreconditions[0].Value.Predicate.Name);

        // 0 static preconditions (all complex logic was extracted into axioms)
        Assert.Empty(stop.StaticPreconditions);

        // 6 derived preconditions (the complex ADL logic)
        Assert.Equal(6, stop.DerivedPreconditions.Count);
        Assert.All(stop.DerivedPreconditions, p => Assert.True(p.IsPositive));
        Assert.All(stop.DerivedPreconditions, p => Assert.StartsWith("axiom_", p.Value.Predicate.Name));

        // 3 conditional effects (from 2 forall blocks, the first producing 2 effects)
        Assert.Equal(3, stop.Effects.Count);

        // Effect 1: (forall (?p) (when (boarded ?p) (destin ?p ?f)) (not (boarded ?p))))
        var eff0 = stop.Effects[0];
        Assert.Single(eff0.QuantifiedVariables);
        Assert.Equal("?p", eff0.QuantifiedVariables[0].Name);
        Assert.Equal("passenger", eff0.QuantifiedVariables[0].Type);
        Assert.Equal("boarded", eff0.Effect.Value.Predicate.Name);
        Assert.True(eff0.Effect.IsNegative);
        Assert.Single(eff0.FluentConditions);  // boarded(?p)
        Assert.Single(eff0.StaticConditions);   // destin(?p, ?f)

        // Effect 2: same forall, adds served(?p)
        var eff1 = stop.Effects[1];
        Assert.Equal("served", eff1.Effect.Value.Predicate.Name);
        Assert.True(eff1.Effect.IsPositive);

        // Effect 3: (forall (?p) (when (origin ?p ?f) (not (served ?p))) (boarded ?p))
        var eff2 = stop.Effects[2];
        Assert.Single(eff2.QuantifiedVariables);
        Assert.Equal("boarded", eff2.Effect.Value.Predicate.Name);
        Assert.True(eff2.Effect.IsPositive);
        Assert.Single(eff2.FluentConditions);  // NOT served(?p)
        Assert.True(eff2.FluentConditions[0].IsNegative);
        Assert.Single(eff2.StaticConditions);   // origin(?p, ?f)

        // === up ===
        var up = domain.Actions.Single(a => a.Name == "up");
        Assert.Equal(2, up.Parameters.Count);

        // Preconditions: lift-at(?f1) [fluent], above(?f1,?f2) [static], axiom_6() [derived]
        Assert.Single(up.FluentPreconditions);
        Assert.Equal("lift-at", up.FluentPreconditions[0].Value.Predicate.Name);
        Assert.Single(up.StaticPreconditions);
        Assert.Equal("above", up.StaticPreconditions[0].Value.Predicate.Name);
        Assert.Single(up.DerivedPreconditions);

        // Effects: +lift-at(?f2), -lift-at(?f1) — simple unconditional
        Assert.Equal(2, up.Effects.Count);
        Assert.All(up.Effects, e => Assert.Empty(e.QuantifiedVariables));
        AssertContainsEffect(up.Effects, "lift-at", Polarity.Positive);
        AssertContainsEffect(up.Effects, "lift-at", Polarity.Negative);

        // === down ===
        var down = domain.Actions.Single(a => a.Name == "down");
        Assert.Equal(2, down.Parameters.Count);

        // Same structure as up, but above(?f2, ?f1) reversed
        Assert.Single(down.FluentPreconditions);
        Assert.Single(down.StaticPreconditions);
        Assert.Equal("above", down.StaticPreconditions[0].Value.Predicate.Name);
        // Verify the argument order is (?f2, ?f1), not (?f1, ?f2)
        var aboveArgs = down.StaticPreconditions[0].Value.Arguments;
        Assert.Equal("?f2", aboveArgs[0].Name);
        Assert.Equal("?f1", aboveArgs[1].Name);

        Assert.Equal(2, down.Effects.Count);
    }

    // === Helper assertion methods ===

    private static void AssertContainsPredicate<T>(
        IReadOnlyList<Literal<Atom<T>>> literals, string predicateName, Polarity polarity) where T : IPredicateType
    {
        Assert.Contains(literals, l =>
            l.Value.Predicate.Name == predicateName && l.Polarity == polarity);
    }

    private static void AssertContainsEffect(
        IReadOnlyList<ConditionalEffect> effects, string predicateName, Polarity polarity)
    {
        Assert.Contains(effects, e =>
            e.Effect.Value.Predicate.Name == predicateName && e.Effect.Polarity == polarity);
    }
}
