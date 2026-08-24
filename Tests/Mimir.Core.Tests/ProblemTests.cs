using System;
using System.IO;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class ProblemTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Fact]
    public void RequirementsAndObjectViewsPreserveSourceOrder()
    {
        Domain domain = Domain.FromText("""
(define (domain problem-metadata)
  (:requirements :strips :typing)
  (:types item)
  (:constants home - item)
  (:predicates (ready ?item - item)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem metadata-problem)
  (:domain problem-metadata)
  (:requirements :typing)
  (:objects first second - item)
  (:init (ready first))
  (:goal (ready second)))
""");

        Assert.Equal([":typing"], problem.Requirements);
        IList<string> requirements = Assert.IsAssignableFrom<IList<string>>(problem.Requirements);
        Assert.Throws<NotSupportedException>(() => requirements[0] = ":strips");
        Assert.Equal(["first", "second"], problem.DeclaredObjects.Select(value => value.Name));
        Assert.Equal(["home", "first", "second"], problem.AllObjects.Select(value => value.Name));
        Assert.Same(problem.DeclaredObjects[0], problem.AllObjects[1]);
        Assert.Same(problem.DeclaredObjects[1], problem.AllObjects[2]);
    }

    [Fact]
    public void ExplicitInitialStaticsExcludeSemanticEqualityFacts()
    {
        Domain domain = Domain.FromText("""
(define (domain static-initial-facts)
  (:requirements :strips :equality)
  (:predicates (linked ?left ?right)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem static-initial-problem)
  (:domain static-initial-facts)
  (:objects a b)
  (:init (linked a b))
  (:goal (linked a b)))
""");

        Fact<Static> explicitFact = Assert.Single(problem._initialStaticFacts);
        Assert.Equal("linked", explicitFact.Predicate.Name);

        Mimir.Core.Schemas.Predicate<Static> equality =
            Assert.IsType<Mimir.Core.Schemas.Predicate<Static>>(problem.AllPredicates["="]);
        Constant a = problem.ObjectLookup["a"];
        Fact<Static> reflexiveEquality = problem.Context.RegisterFact(equality, [a, a]);

        Assert.True(problem.InitialState.Expand().IsTrue(reflexiveEquality));
        Assert.Single(problem._initialStaticFacts);
        Assert.DoesNotContain(problem._initialStaticFacts, fact => ReferenceEquals(fact, reflexiveEquality));
    }

    [Fact]
    public void BlocksWorldProblemGrounding()
    {
        var domainPath = Path.Combine(BasePath, "blocks_4", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "blocks_4", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        Assert.Equal("blocksworld-300", problem.Name);
        Assert.Equal(3, problem.AllObjects.Count); // b1, b2, b3
        
        // Initial State
        // (arm-empty)
        // (clear b2)
        // (on-table b2)
        // (clear b1)
        // (on b1 b3)
        // (on-table b3)
        // All are fluents in blocks_4.
        
        var trueFacts = problem.InitialState.GetTrueFacts().ToList();
        Assert.Equal(6, trueFacts.Count);
        
        AssertContainsFact(trueFacts, "arm-empty");
        AssertContainsFact(trueFacts, "clear", "b2");
        AssertContainsFact(trueFacts, "on-table", "b2");
        AssertContainsFact(trueFacts, "clear", "b1");
        AssertContainsFact(trueFacts, "on", "b1", "b3");
        AssertContainsFact(trueFacts, "on-table", "b3");

        // Static Bitboard should be empty for blocks_4
        Assert.All(problem.Context.StaticBitboardWords.ToArray(), word => Assert.Equal(0UL, word));

        // Goal
        // (clear b2), (on b2 b3), (on-table b3), (clear b1), (on-table b1)
        Assert.Equal(5, problem.Goal.Count);
        AssertContainsGoal(problem.Goal, "clear", Polarity.Positive, "b2");
        AssertContainsGoal(problem.Goal, "on", Polarity.Positive, "b2", "b3");
        AssertContainsGoal(problem.Goal, "on-table", Polarity.Positive, "b3");
        AssertContainsGoal(problem.Goal, "clear", Polarity.Positive, "b1");
        AssertContainsGoal(problem.Goal, "on-table", Polarity.Positive, "b1");
    }

    [Fact]
    public void MiconicProblemGrounding()
    {
        var domainPath = Path.Combine(BasePath, "miconic", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "miconic", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        // lift-at is fluent. origin is fluent (modified in board action). 
        // served is fluent (but false in init).
        // above, destin are static.
        
        var trueFluents = problem.InitialState.GetTrueFacts().ToList();
        Assert.Equal(3, trueFluents.Count);
        AssertContainsFact(trueFluents, "lift-at", "f0");
        AssertContainsFact(trueFluents, "origin", "p0", "f0");
        AssertContainsFact(trueFluents, "origin", "p1", "f0");

        // Static Facts: 1x above, 2x destin = 3 total
        ExtendedState initialState = problem.InitialState.Expand();
        var trueStatics = problem.Context.Statics
            .Where(initialState.IsTrue)
            .ToList();
        
        Assert.Equal(3, trueStatics.Count);
        AssertContainsStaticFact(trueStatics, "above", "f0", "f1");
        AssertContainsStaticFact(trueStatics, "destin", "p0", "f1");
        AssertContainsStaticFact(trueStatics, "destin", "p1", "f1");

        // Goal: (served p0), (served p1)
        Assert.Equal(2, problem.Goal.Count);
        AssertContainsGoal(problem.Goal, "served", Polarity.Positive, "p0");
        AssertContainsGoal(problem.Goal, "served", Polarity.Positive, "p1");
    }

    [Fact]
    public void FromTextParsesProblemPddl()
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

        var problem = Problem.FromText(domain, """
(define (problem test-problem)
  (:domain test-domain)
  (:objects a)
  (:init)
  (:goal (p a)))
""");

        Assert.Equal("test-problem", problem.Name);
        Assert.Equal(["a"], problem.AllObjects.Select(obj => obj.Name).ToArray());
        Assert.Single(problem.Goal);
    }

    [Theory]
    [InlineData("blocks_4", "p01.pddl", 2)]
    [InlineData("miconic", "p01.pddl", 3)]
    [InlineData("gripper", "p01.pddl", 6)]
    [InlineData("ferry", "p01.pddl", 3)]
    [InlineData("logistics", "p01.pddl", 6)]
    [InlineData("visitall", "p01.pddl", 2)]
    [InlineData("satellite", "p01.pddl", 4)]
    [InlineData("rovers", "p01.pddl", 2)]
    [InlineData("airport", "p01.pddl", 2)]
    [InlineData("assembly", "p01.pddl", 3)]
    [InlineData("barman", "p01.pddl", 4)]
    [InlineData("childsnack", "p01.pddl", 3)]
    [InlineData("delivery", "p01.pddl", 2)]
    [InlineData("driverlog", "p01.pddl", 2)]
    [InlineData("grid", "p01.pddl", 1)]
    [InlineData("hiking", "p01.pddl", 17)]
    [InlineData("miconic-simpleadl", "p01.pddl", 2)]
    [InlineData("schedule", "p01.pddl", 44)]
    public void InitialStateApplicableActionCount(string domainDir, string problemFile, int expectedCount)
    {
        var domainPath = Path.Combine(BasePath, domainDir, "domain.pddl");
        var problemPath = Path.Combine(BasePath, domainDir, problemFile);

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        
        var grounder = new RpgGrounder();
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, grounder);

        var applicable = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();

        Assert.Equal(expectedCount, applicable.Count);
    }

    [Theory]
    [InlineData("refuel")]
    [InlineData("refuel-adl")]
    [InlineData("zenotravel")]
    public void RejectsBenchmarksWithUnsupportedNumericFragments(string domainDir)
    {
        string domainPath = Path.Combine(BasePath, domainDir, "domain.pddl");
        string problemPath = Path.Combine(BasePath, domainDir, "p01.pddl");

        PddlLoadException exception = Assert.Throws<PddlLoadException>(() =>
        {
            var domain = Domain.FromFile(domainPath);
            _ = Problem.FromFile(domain, problemPath);
        });

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Fact]
    public void RejectsMiconicFullAdlQuantifiedGoal()
    {
        string domainPath = Path.Combine(BasePath, "miconic-fulladl", "domain.pddl");
        string problemPath = Path.Combine(BasePath, "miconic-fulladl", "p01.pddl");
        Domain domain = Domain.FromFile(domainPath);

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Problem.FromFile(domain, problemPath));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Equal(problemPath, exception.SourcePath);
    }

    private void AssertContainsFact(IEnumerable<Fact<Fluent>> facts, string predicateName, params string[] args)
    {
        Assert.Contains(facts, f => 
            f.Predicate.Name == predicateName && 
            f.Arguments.Select(a => a.Name).SequenceEqual(args));
    }

    private void AssertContainsStaticFact(IEnumerable<Fact<Static>> facts, string predicateName, params string[] args)
    {
        Assert.Contains(facts, f => 
            f.Predicate.Name == predicateName && 
            f.Arguments.Select(a => a.Name).SequenceEqual(args));
    }

    private void AssertContainsGoal(IEnumerable<Literal<Fact>> goal, string predicateName, Polarity polarity, params string[] args)
    {
        Assert.Contains(goal, l => 
            l.Polarity == polarity &&
            l.Value.Predicate.Name == predicateName &&
            l.Value.Arguments.Select(a => a.Name).SequenceEqual(args));
    }
}
