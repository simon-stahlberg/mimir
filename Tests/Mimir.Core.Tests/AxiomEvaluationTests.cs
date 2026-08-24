using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Mimir.Pddl;
using System.Collections.Immutable;
using Xunit;

namespace Mimir.Core.Tests;

public class AxiomEvaluationTests
{
    [Fact]
    public void TestEmptyTypeQuantifiers()
    {
        Domain domain = new DomainBuilder("test")
            .Requirements().Add(":typing").Add(":adl").Close()
            .Types().Add("t1").Close()
            .Predicates()
                .Add("p", ("?x", "t1"))
                .Add("axiom-forall")
                .Add("axiom-exists")
                .Close()
            .DerivedPredicates()
                .Define("axiom-forall", Logic.Forall([("?x", "t1")], Logic.Atom("p", "?x")))
                .Define("axiom-exists", Logic.Exists([("?x", "t1")], Logic.Atom("p", "?x")))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Goal().Add("axiom-forall").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        
        var forallPred = domain.Derived.First(d => d.Name.Equals("axiom-forall", StringComparison.OrdinalIgnoreCase));
        var existsPred = domain.Derived.First(d => d.Name.Equals("axiom-exists", StringComparison.OrdinalIgnoreCase));
        
        var forallFact = (Fact<Derived>)problem.Context.RegisterFact(forallPred, Array.Empty<Constant>());
        var existsFact = (Fact<Derived>)problem.Context.RegisterFact(existsPred, Array.Empty<Constant>());
        
        Assert.True(state.IsTrue(forallFact), "Forall over empty type should be true");
        Assert.False(state.IsTrue(existsFact), "Exists over empty type should be false");
    }

    [Fact]
    public void TestAxiomMerging()
    {
        Domain domain = new DomainBuilder("test")
            .Predicates()
                .Add("p", ("?x", "object"))
                .Add("q", ("?x", "object"))
                .Add("r", ("?x", "object"))
                .Add("axiom", ("?x", "object"))
                .Close()
            .DerivedPredicates()
                .Define("axiom", Logic.Or(Logic.Atom("p", "?x"), Logic.Atom("q", "?x")))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Objects().Add("o1").Add("o2").Close()
            .InitialState().AddFact("p", "o1").AddFact("q", "o2").Close()
            .Goal().Add("axiom", "o1").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        
        var axiomPred = domain.Derived.First(d => d.Name.Equals("axiom", StringComparison.OrdinalIgnoreCase));
        
        var o1 = problem.AllObjects.First(o => o.Name == "o1");
        var o2 = problem.AllObjects.First(o => o.Name == "o2");
        
        var factO1 = (Fact<Derived>)problem.Context.RegisterFact(axiomPred, new[] { o1 });
        var factO2 = (Fact<Derived>)problem.Context.RegisterFact(axiomPred, new[] { o2 });
        
        Assert.True(state.IsTrue(factO1), "Axiom should be true via first rule (p o1)");
        Assert.True(state.IsTrue(factO2), "Axiom should be true via second rule (q o2)");
    }

    [Fact]
    public void AcyclicDerivedPredicateChainEvaluates()
    {
        Domain domain = new DomainBuilder("acyclic-derived")
            .Predicates().Add("base").Add("middle").Add("top").Close()
            .DerivedPredicates()
                .Define("middle", Logic.Atom("base"))
                .Define("top", Logic.Atom("middle"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "acyclic-derived-problem")
            .InitialState().AddFact("base").Close()
            .Goal().Add("top").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        var top = domain.Derived.Single(predicate => predicate.Name == "top");
        Fact<Derived> topFact = problem.Context.RegisterFact(top, Array.Empty<Constant>());

        Assert.True(state.IsTrue(topFact));
    }

    [Fact]
    public void PositiveRecursionReachesLeastFixedPoint()
    {
        Domain domain = new DomainBuilder("recursive-derived")
            .Requirements().Add(":adl").Close()
            .Predicates()
                .Add("edge", ("?from", "object"), ("?to", "object"))
                .Add("reachable", ("?from", "object"), ("?to", "object"))
                .Close()
            .DerivedPredicates()
                .Define(
                    "reachable",
                    Logic.Or(
                        Logic.Atom("edge", "?from", "?to"),
                        Logic.Exists(
                            [("?middle", "object")],
                            Logic.And(
                                Logic.Atom("edge", "?from", "?middle"),
                                Logic.Atom("reachable", "?middle", "?to")))))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "recursive-problem")
            .Objects().Add("a").Add("b").Add("c").Add("d").Close()
            .InitialState()
                .AddFact("edge", "a", "b")
                .AddFact("edge", "b", "c")
                .AddFact("edge", "c", "d")
                .Close()
            .Goal().Add("reachable", "a", "d").Close()
            .Build();
        Mimir.Core.Schemas.Predicate<Derived> reachable = domain.Derived.Single(
            predicate => predicate.Name == "reachable");
        Constant a = problem.ObjectLookup["a"];
        Constant d = problem.ObjectLookup["d"];
        Fact<Derived> forward = problem.Context.RegisterFact(reachable, [a, d]);
        Fact<Derived> backward = problem.Context.RegisterFact(reachable, [d, a]);
        int registeredDerivedCount = problem.Context.Derived.Count;
        ExtendedState state = problem.InitialState.Expand();

        Assert.True(state.IsTrue(forward));
        Assert.False(state.IsTrue(backward));
        Assert.Equal(registeredDerivedCount, problem.Context.Derived.Count);
    }

    [Fact]
    public void SeedlessPositiveCycleRemainsFalse()
    {
        Domain domain = new DomainBuilder("recursive-derived")
            .Requirements().Add(":derived-predicates").Close()
            .Predicates().Add("loop").Close()
            .DerivedPredicates().Define("loop", Logic.Atom("loop")).Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "recursive-problem")
            .Goal().Add("loop", Polarity.Negative).Close()
            .Build();
        Fact<Derived> loop = problem.Context.RegisterFact(
            domain.Derived.Single(),
            Array.Empty<Constant>());

        Assert.False(problem.InitialState.Expand().IsTrue(loop));
    }

    [Fact]
    public void PositiveMutualRecursionPropagatesASeed()
    {
        Domain domain = new DomainBuilder("mutual-recursion")
            .Requirements().Add(":adl").Close()
            .Predicates().Add("seed").Add("first").Add("second").Close()
            .DerivedPredicates()
                .Define("first", Logic.Or(Logic.Atom("seed"), Logic.Atom("second")))
                .Define("second", Logic.Atom("first"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "mutual-problem")
            .InitialState().AddFact("seed").Close()
            .Goal().Add("second").Close()
            .Build();
        Mimir.Core.Schemas.Predicate<Derived> first = domain.Derived
            .Single(predicate => predicate.Name == "first");
        Mimir.Core.Schemas.Predicate<Derived> second = domain.Derived
            .Single(predicate => predicate.Name == "second");
        Fact<Derived> firstFact = problem.Context.RegisterFact(
            first,
            Array.Empty<Constant>());
        Fact<Derived> secondFact = problem.Context.RegisterFact(
            second,
            Array.Empty<Constant>());
        ExtendedState state = problem.InitialState.Expand();

        Assert.True(state.IsTrue(firstFact));
        Assert.True(state.IsTrue(secondFact));
    }

    [Fact]
    public void ThreePredicatePositiveSccPropagatesAgainstComponentOrder()
    {
        Domain domain = new DomainBuilder("three-way-recursion")
            .Requirements().Add(":adl").Close()
            .Predicates().Add("seed").Add("first").Add("second").Add("third").Close()
            .DerivedPredicates()
                .Define("first", Logic.Or(Logic.Atom("seed"), Logic.Atom("second")))
                .Define("second", Logic.Or(Logic.Atom("first"), Logic.Atom("third")))
                .Define("third", Logic.Atom("second"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "three-way-problem")
            .InitialState().AddFact("seed").Close()
            .Goal().Add("third").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();

        foreach (string predicateName in new[] { "first", "second", "third" })
        {
            Mimir.Core.Schemas.Predicate<Derived> predicate = domain.Derived.Single(
                candidate => candidate.Name == predicateName);
            Fact<Derived> fact = problem.Context.RegisterFact(
                predicate,
                Array.Empty<Constant>());

            Assert.True(state.IsTrue(fact));
        }
    }

    [Fact]
    public void NegativeRecursiveCycleIsRejected()
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() =>
            Domain.FromText(@"
(define (domain recursive-derived)
  (:requirements :adl)
  (:derived (loop) (not (loop))))"));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
    }

    [Fact]
    public void TestEquality()
    {
        Domain domain = new DomainBuilder("test")
            .Requirements().Add(":equality").Add(":derived-predicates").Close()
            .Constants().Add("o1").Close()
            .Predicates()
                .Add("p", ("?x", "object"))
                .Add("is-o1", ("?x", "object"))
                .Add("both-same", ("?x", "object"), ("?y", "object"))
                .Close()
            .DerivedPredicates()
                .Define("is-o1", Logic.Equal("?x", "o1"))
                .Define("both-same", Logic.Equal("?x", "?y"))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Objects().Add("o2").Close()
            .Goal().Add("is-o1", "o1").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        
        var isO1Pred = domain.Derived.First(d => d.Name == "is-o1");
        var bothSamePred = domain.Derived.First(d => d.Name == "both-same");
        
        var o1 = problem.AllObjects.First(o => o.Name == "o1");
        var o2 = problem.AllObjects.First(o => o.Name == "o2");
        
        var factIsO1_o1 = (Fact<Derived>)problem.Context.RegisterFact(isO1Pred, new[] { o1 });
        var factIsO1_o2 = (Fact<Derived>)problem.Context.RegisterFact(isO1Pred, new[] { o2 });
        var factBothSame_same = (Fact<Derived>)problem.Context.RegisterFact(bothSamePred, new[] { o1, o1 });
        var factBothSame_diff = (Fact<Derived>)problem.Context.RegisterFact(bothSamePred, new[] { o1, o2 });
        
        Assert.True(state.IsTrue(factIsO1_o1), "o1 should equal o1");
        Assert.False(state.IsTrue(factIsO1_o2), "o2 should not equal o1");
        Assert.True(state.IsTrue(factBothSame_same), "o1 should equal o1 (var-var)");
        Assert.False(state.IsTrue(factBothSame_diff), "o1 should not equal o2 (var-var)");
    }


    [Fact]
    public void Evaluate_GroundedNot_TrueWhenPredicateFalse()
    {
        Domain domain = new DomainBuilder("test")
            .Predicates()
                .Add("p", ("?x", "object"))
                .Add("not-p", ("?x", "object"))
                .Close()
            .DerivedPredicates().Define("not-p", Logic.Not(Logic.Atom("p", "?x"))).Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Objects().Add("o1").Add("o2").Close()
            .InitialState().AddFact("p", "o1").Close()
            .Goal().Add("not-p", "o2").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();

        var notPPred = domain.Derived.First(d => d.Name.Equals("not-p", StringComparison.OrdinalIgnoreCase));
        var o1 = problem.AllObjects.First(o => o.Name == "o1");
        var o2 = problem.AllObjects.First(o => o.Name == "o2");

        var factO1 = (Fact<Derived>)problem.Context.RegisterFact(notPPred, new[] { o1 });
        var factO2 = (Fact<Derived>)problem.Context.RegisterFact(notPPred, new[] { o2 });

        Assert.False(state.IsTrue(factO1), "not-p(o1) should be false since p(o1) is true");
        Assert.True(state.IsTrue(factO2), "not-p(o2) should be true since p(o2) is absent");
    }

    [Fact]
    public void Evaluate_GroundedImply_VacuouslyTrueWhenAntecedentFalse()
    {
        Domain domain = new DomainBuilder("test")
            .Requirements().Add(":adl").Close()
            .Predicates()
                .Add("risky", ("?x", "object"))
                .Add("guarded", ("?x", "object"))
                .Add("passes-test", ("?x", "object"))
                .Close()
            .DerivedPredicates()
                .Define(
                    "passes-test",
                    Logic.Imply(Logic.Atom("risky", "?x"), Logic.Atom("guarded", "?x")))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Objects().Add("a").Add("b").Add("c").Close()
            .InitialState()
                .AddFact("risky", "a")
                .AddFact("guarded", "a")
                .AddFact("risky", "b")
                .Close()
            .Goal().Add("passes-test", "a").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();

        var pred = domain.Derived.First(d => d.Name.Equals("passes-test", StringComparison.OrdinalIgnoreCase));
        var a = problem.AllObjects.First(o => o.Name == "a");
        var b = problem.AllObjects.First(o => o.Name == "b");
        var c = problem.AllObjects.First(o => o.Name == "c");

        var factA = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { a });
        var factB = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { b });
        var factC = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { c });

        Assert.True(state.IsTrue(factA), "imply(risky,guarded) with both true should be true");
        Assert.False(state.IsTrue(factB), "imply(risky,not-guarded) should be false");
        Assert.True(state.IsTrue(factC), "imply(not-risky,_) is vacuously true");
    }

    [Fact]
    public void Evaluate_GroundedOr_TrueWhenEitherDisjunctTrue()
    {
        Domain domain = new DomainBuilder("test")
            .Predicates()
                .Add("p", ("?x", "object"))
                .Add("q", ("?x", "object"))
                .Add("any-of", ("?x", "object"))
                .Close()
            .DerivedPredicates()
                .Define("any-of", Logic.Or(Logic.Atom("p", "?x"), Logic.Atom("q", "?x")))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "p1")
            .Objects().Add("o1").Add("o2").Add("o3").Close()
            .InitialState().AddFact("p", "o1").AddFact("q", "o2").Close()
            .Goal().Add("any-of", "o1").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();

        var pred = domain.Derived.First(d => d.Name.Equals("any-of", StringComparison.OrdinalIgnoreCase));
        var o1 = problem.AllObjects.First(o => o.Name == "o1");
        var o2 = problem.AllObjects.First(o => o.Name == "o2");
        var o3 = problem.AllObjects.First(o => o.Name == "o3");

        var factO1 = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { o1 });
        var factO2 = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { o2 });
        var factO3 = (Fact<Derived>)problem.Context.RegisterFact(pred, new[] { o3 });

        Assert.True(state.IsTrue(factO1), "any-of(o1) via p(o1)");
        Assert.True(state.IsTrue(factO2), "any-of(o2) via q(o2)");
        Assert.False(state.IsTrue(factO3), "any-of(o3) with neither p nor q");
    }

    [Fact]
    public void TestCaseInsensitivity()
    {
        Domain domain = new DomainBuilder("TEST-DOMAIN")
            .Types().Add("TYPE-A").Close()
            .Predicates()
                .Add("PRED-P", ("?x", "TYPE-A"))
                .Add("AXIOM-G", ("?V", "TYPE-A"))
                .Close()
            .DerivedPredicates().Define("AXIOM-G", Logic.Atom("PRED-P", "?V")).Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "P1")
            .Objects().Add("OBJ-1", "type-a").Close()
            .InitialState().AddFact("pred-p", "obj-1").Close()
            .Goal().Add("axiom-g", "obj-1").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        
        var axiomPred = domain.Derived.First(d => d.Name.Equals("AXIOM-G", StringComparison.OrdinalIgnoreCase));
        var obj1 = problem.AllObjects.First(o => o.Name.Equals("OBJ-1", StringComparison.OrdinalIgnoreCase));
        
        var fact = (Fact<Derived>)problem.Context.RegisterFact(axiomPred, new[] { obj1 });
        
        Assert.True(state.IsTrue(fact), "Axiom evaluation should be case-insensitive");
    }

    [Fact]
    public void EvaluatesOrdinaryAndDerivedAtomsAcrossAritiesZeroThroughFive()
    {
        DomainBuilder domainBuilder = new DomainBuilder("wide-derived");
        domainBuilder.Requirements().Add(":typing").Add(":derived-predicates").Close();
        domainBuilder.Types()
            .Add("first").Add("second").Add("third").Add("fourth").Add("fifth")
            .Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("base-zero-true").Add("base-zero-false");
        predicates.Add("base-one", ("?a", "first"));
        predicates.Add("base-two", ("?a", "first"), ("?b", "second"));
        predicates.Add("base-three", ("?a", "first"), ("?b", "second"), ("?c", "third"));
        predicates.Add("base-four", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"));
        predicates.Add("base-five", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"), ("?e", "fifth"));
        predicates.Add("from-base-zero-true").Add("from-base-zero-false");
        predicates.Add("from-derived-zero-true").Add("from-derived-zero-false");
        predicates.Add("from-base-one", ("?a", "first"));
        predicates.Add("from-derived-one", ("?a", "first"));
        predicates.Add("from-base-two", ("?a", "first"), ("?b", "second"));
        predicates.Add("from-derived-two", ("?a", "first"), ("?b", "second"));
        predicates.Add("from-base-three", ("?a", "first"), ("?b", "second"), ("?c", "third"));
        predicates.Add("from-derived-three", ("?a", "first"), ("?b", "second"), ("?c", "third"));
        predicates.Add("from-base-four", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"));
        predicates.Add("from-derived-four", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"));
        predicates.Add("from-base-five", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"), ("?e", "fifth"));
        predicates.Add("from-derived-five", ("?a", "first"), ("?b", "second"), ("?c", "third"), ("?d", "fourth"), ("?e", "fifth"));
        predicates.Close();
        DerivedPredicateListBuilder derived = domainBuilder.DerivedPredicates();
        derived.Define("from-base-zero-true", Logic.Atom("base-zero-true"));
        derived.Define("from-base-zero-false", Logic.Atom("base-zero-false"));
        derived.Define("from-derived-zero-true", Logic.Atom("from-base-zero-true"));
        derived.Define("from-derived-zero-false", Logic.Atom("from-base-zero-false"));
        derived.Define("from-base-one", Logic.Atom("base-one", "?a"));
        derived.Define("from-derived-one", Logic.Atom("from-base-one", "?a"));
        derived.Define("from-base-two", Logic.Atom("base-two", "?a", "?b"));
        derived.Define("from-derived-two", Logic.Atom("from-base-two", "?a", "?b"));
        derived.Define("from-base-three", Logic.Atom("base-three", "?a", "?b", "?c"));
        derived.Define("from-derived-three", Logic.Atom("from-base-three", "?a", "?b", "?c"));
        derived.Define("from-base-four", Logic.Atom("base-four", "?a", "?b", "?c", "?d"));
        derived.Define("from-derived-four", Logic.Atom("from-base-four", "?a", "?b", "?c", "?d"));
        derived.Define("from-base-five", Logic.Atom("base-five", "?a", "?b", "?c", "?d", "?e"));
        derived.Define("from-derived-five", Logic.Atom("from-base-five", "?a", "?b", "?c", "?d", "?e"));
        derived.Close();
        Domain domain = domainBuilder.Build();

        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "wide-derived-problem");
        problemBuilder.Objects()
            .Add("one", "first").Add("other-one", "first")
            .Add("two", "second").Add("other-two", "second")
            .Add("three", "third").Add("other-three", "third")
            .Add("four", "fourth").Add("other-four", "fourth")
            .Add("five", "fifth").Add("other-five", "fifth")
            .Close();
        problemBuilder.InitialState()
            .AddFact("base-zero-true")
            .AddFact("base-one", "one")
            .AddFact("base-two", "one", "two")
            .AddFact("base-three", "one", "two", "three")
            .AddFact("base-four", "one", "two", "three", "four")
            .AddFact("base-five", "one", "two", "three", "four", "five")
            .Close();
        Problem problem = problemBuilder.Goal().Add("from-derived-zero-true").Close().Build();
        ExtendedState state = problem.InitialState.Expand();

        void AssertDerived(
            bool expected,
            string predicateName,
            params string[] argumentNames)
        {
            Mimir.Core.Schemas.Predicate<Derived> predicate = domain.Derived.Single(
                candidate => candidate.Name == predicateName);
            Constant[] arguments = argumentNames
                .Select(name => problem.ObjectLookup[name])
                .ToArray();
            Fact<Derived> fact = problem.Context.RegisterFact(predicate, arguments);

            Assert.Equal(expected, state.IsTrue(fact));
        }

        AssertDerived(true, "from-base-zero-true");
        AssertDerived(false, "from-base-zero-false");
        AssertDerived(true, "from-derived-zero-true");
        AssertDerived(false, "from-derived-zero-false");

        AssertDerived(true, "from-base-one", "one");
        AssertDerived(false, "from-base-one", "other-one");
        AssertDerived(true, "from-derived-one", "one");
        AssertDerived(false, "from-derived-one", "other-one");

        AssertDerived(true, "from-base-two", "one", "two");
        AssertDerived(false, "from-base-two", "one", "other-two");
        AssertDerived(true, "from-derived-two", "one", "two");
        AssertDerived(false, "from-derived-two", "one", "other-two");

        AssertDerived(true, "from-base-three", "one", "two", "three");
        AssertDerived(false, "from-base-three", "one", "two", "other-three");
        AssertDerived(true, "from-derived-three", "one", "two", "three");
        AssertDerived(false, "from-derived-three", "one", "two", "other-three");

        AssertDerived(true, "from-base-four", "one", "two", "three", "four");
        AssertDerived(false, "from-base-four", "one", "two", "three", "other-four");
        AssertDerived(true, "from-derived-four", "one", "two", "three", "four");
        AssertDerived(false, "from-derived-four", "one", "two", "three", "other-four");

        AssertDerived(true, "from-base-five", "one", "two", "three", "four", "five");
        AssertDerived(false, "from-base-five", "one", "two", "three", "four", "other-five");
        AssertDerived(true, "from-derived-five", "one", "two", "three", "four", "five");
        AssertDerived(false, "from-derived-five", "one", "two", "three", "four", "other-five");
    }

    [Fact]
    public void TypedCandidateCacheIncludesSubtypesAndExcludesUnrelatedObjects()
    {
        Domain domain = new DomainBuilder("typed-derived")
            .Requirements().Add(":typing").Add(":derived-predicates").Close()
            .Types().Add("parent").Add("unrelated").Add("child", "parent").Close()
            .Predicates()
                .Add("seed", ("?x", "parent"))
                .Add("inherited", ("?x", "parent"))
                .Add("some-seed")
                .Add("all-seeded")
                .Close()
            .DerivedPredicates()
                .Define("inherited", Logic.Atom("seed", "?x"))
                .Define("some-seed", Logic.Exists([("?x", "parent")], Logic.Atom("seed", "?x")))
                .Define("all-seeded", Logic.Forall([("?x", "parent")], Logic.Atom("seed", "?x")))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "typed-derived-problem")
            .Objects().Add("child-object", "child").Add("unrelated-object", "unrelated").Close()
            .InitialState().AddFact("seed", "child-object").Close()
            .Goal().Add("some-seed").Add("all-seeded").Close()
            .Build();
        Constant child = problem.ObjectLookup["child-object"];
        Mimir.Core.Schemas.Predicate<Derived> inherited = domain.Derived.Single(
            predicate => predicate.Name == "inherited");
        Mimir.Core.Schemas.Predicate<Derived> someSeed = domain.Derived.Single(
            predicate => predicate.Name == "some-seed");
        Mimir.Core.Schemas.Predicate<Derived> allSeeded = domain.Derived.Single(
            predicate => predicate.Name == "all-seeded");
        Fact<Derived> inheritedChild = problem.Context.RegisterFact(inherited, [child]);
        Fact<Derived> someSeedFact = problem.Context.RegisterFact(
            someSeed,
            Array.Empty<Constant>());
        Fact<Derived> allSeededFact = problem.Context.RegisterFact(
            allSeeded,
            Array.Empty<Constant>());

        ExtendedState state = problem.InitialState.Expand();

        Assert.True(state.IsTrue(inheritedChild));
        Assert.True(state.IsTrue(someSeedFact));
        Assert.True(state.IsTrue(allSeededFact));
    }

    [Fact]
    public void NestedExistsAndEvaluatesGuardsAtTheirDependencyDepths()
    {
        Domain domain = new DomainBuilder("guard-depths")
            .Requirements().Add(":typing").Add(":adl").Close()
            .Types().Add("head").Add("outer").Add("middle").Add("inner").Close()
            .Predicates()
                .Add("head-guard", ("?h", "head"))
                .Add("outer-guard", ("?h", "head"), ("?x", "outer"))
                .Add("middle-guard", ("?h", "head"), ("?x", "outer"), ("?y", "middle"))
                .Add("inner-guard", ("?h", "head"), ("?x", "outer"), ("?y", "middle"), ("?z", "inner"))
                .Add("passes", ("?h", "head"))
                .Close()
            .DerivedPredicates()
                .Define(
                    "passes",
                    Logic.Exists(
                        [("?x", "outer")],
                        Logic.And(
                            Logic.Atom("head-guard", "?h"),
                            Logic.Atom("outer-guard", "?h", "?x"),
                            Logic.Exists(
                                [("?y", "middle")],
                                Logic.And(
                                    Logic.Atom("middle-guard", "?h", "?x", "?y"),
                                    Logic.Exists(
                                        [("?z", "inner")],
                                        Logic.Atom("inner-guard", "?h", "?x", "?y", "?z")))))))
                .Close()
            .Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "guard-depths-problem");
        problemBuilder.Objects()
            .Add("complete", "head")
            .Add("missing-head", "head")
            .Add("missing-outer", "head")
            .Add("missing-middle", "head")
            .Add("missing-inner", "head")
            .Add("outer-object", "outer")
            .Add("middle-object", "middle")
            .Add("inner-object", "inner")
            .Close();
        problemBuilder.InitialState()
            .AddFact("head-guard", "complete")
            .AddFact("head-guard", "missing-outer")
            .AddFact("head-guard", "missing-middle")
            .AddFact("head-guard", "missing-inner")
            .AddFact("outer-guard", "complete", "outer-object")
            .AddFact("outer-guard", "missing-middle", "outer-object")
            .AddFact("outer-guard", "missing-inner", "outer-object")
            .AddFact("middle-guard", "complete", "outer-object", "middle-object")
            .AddFact("middle-guard", "missing-inner", "outer-object", "middle-object")
            .AddFact("inner-guard", "complete", "outer-object", "middle-object", "inner-object")
            .Close();
        Problem problem = problemBuilder.Goal().Add("passes", "complete").Close().Build();
        Mimir.Core.Schemas.Predicate<Derived> passes = domain.Derived.Single(
            predicate => predicate.Name == "passes");
        ExtendedState state = problem.InitialState.Expand();
        (string ObjectName, bool Expected)[] expectations =
        [
            ("complete", true),
            ("missing-head", false),
            ("missing-outer", false),
            ("missing-middle", false),
            ("missing-inner", false),
        ];

        foreach ((string objectName, bool expected) in expectations)
        {
            Fact<Derived> fact = problem.Context.RegisterFact(
                passes,
                [problem.ObjectLookup[objectName]]);

            Assert.Equal(expected, state.IsTrue(fact));
        }
    }

    [Fact]
    public void ExistsWithUnusedVariableStillObservesCandidateCardinality()
    {
        Domain domain = new DomainBuilder("unused-exists")
            .Requirements().Add(":typing").Add(":adl").Close()
            .Types().Add("populated").Add("vacant").Close()
            .Predicates()
                .Add("present").Add("absent")
                .Add("populated-true").Add("populated-false").Add("vacant-false")
                .Close()
            .DerivedPredicates()
                .Define(
                    "populated-true",
                    Logic.Exists(
                        [("?ignored", "populated")],
                        Logic.And(Logic.Atom("present"), Logic.Not(Logic.Atom("absent")))))
                .Define(
                    "populated-false",
                    Logic.Exists(
                        [("?ignored", "populated")],
                        Logic.And(Logic.Atom("present"), Logic.Atom("absent"))))
                .Define(
                    "vacant-false",
                    Logic.Exists(
                        [("?ignored", "vacant")],
                        Logic.And(Logic.Atom("present"), Logic.Not(Logic.Atom("absent")))))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "unused-exists-problem")
            .Objects().Add("member", "populated").Close()
            .InitialState().AddFact("present").Close()
            .Goal().Add("populated-true").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        (string PredicateName, bool Expected)[] expectations =
        [
            ("populated-true", true),
            ("populated-false", false),
            ("vacant-false", false),
        ];

        foreach ((string predicateName, bool expected) in expectations)
        {
            Mimir.Core.Schemas.Predicate<Derived> predicate = domain.Derived.Single(
                candidate => candidate.Name == predicateName);
            Fact<Derived> fact = problem.Context.RegisterFact(
                predicate,
                Array.Empty<Constant>());

            Assert.Equal(expected, state.IsTrue(fact));
        }
    }

    [Fact]
    public void ExistsAndGuardOverParentTypeIncludesSubtypeObjects()
    {
        Domain domain = new DomainBuilder("subtype-exists")
            .Requirements().Add(":typing").Add(":adl").Close()
            .Types()
                .Add("parent").Add("marker").Add("unrelated").Add("child", "parent")
                .Close()
            .Predicates()
                .Add("enabled", ("?m", "marker"))
                .Add("seed", ("?x", "parent"))
                .Add("has-seed", ("?m", "marker"))
                .Close()
            .DerivedPredicates()
                .Define(
                    "has-seed",
                    Logic.Exists(
                        [("?x", "parent")],
                        Logic.And(Logic.Atom("enabled", "?m"), Logic.Atom("seed", "?x"))))
                .Close()
            .Build();
        Problem problem = new ProblemBuilder(domain, "subtype-exists-problem")
            .Objects()
                .Add("child-object", "child")
                .Add("unrelated-object", "unrelated")
                .Add("enabled-marker", "marker")
                .Add("disabled-marker", "marker")
                .Close()
            .InitialState()
                .AddFact("enabled", "enabled-marker")
                .AddFact("seed", "child-object")
                .Close()
            .Goal().Add("has-seed", "enabled-marker").Close()
            .Build();
        Mimir.Core.Schemas.Predicate<Derived> hasSeed = domain.Derived.Single();
        Fact<Derived> enabled = problem.Context.RegisterFact(
            hasSeed,
            [problem.ObjectLookup["enabled-marker"]]);
        Fact<Derived> disabled = problem.Context.RegisterFact(
            hasSeed,
            [problem.ObjectLookup["disabled-marker"]]);
        ExtendedState state = problem.InitialState.Expand();

        Assert.True(state.IsTrue(enabled));
        Assert.False(state.IsTrue(disabled));
    }

    [Fact]
    public void NestedExistsAndEvaluatesForallWithOuterBinding()
    {
        Domain domain = new DomainBuilder("nested-forall")
            .Requirements().Add(":typing").Add(":adl").Close()
            .Types().Add("group").Add("witness").Add("item").Close()
            .Predicates()
                .Add("selected", ("?g", "group"), ("?w", "witness"))
                .Add("belongs", ("?w", "witness"), ("?i", "item"))
                .Add("ready", ("?i", "item"))
                .Add("accepted", ("?g", "group"))
                .Close()
            .DerivedPredicates()
                .Define(
                    "accepted",
                    Logic.Exists(
                        [("?w", "witness")],
                        Logic.And(
                            Logic.Atom("selected", "?g", "?w"),
                            Logic.Forall(
                                [("?i", "item")],
                                Logic.Imply(
                                    Logic.Atom("belongs", "?w", "?i"),
                                    Logic.Atom("ready", "?i"))))))
                .Close()
            .Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "nested-forall-problem");
        problemBuilder.Objects()
            .Add("accepted-group", "group")
            .Add("rejected-group", "group")
            .Add("unselected-group", "group")
            .Add("accepted-witness", "witness")
            .Add("rejected-witness", "witness")
            .Add("ready-item", "item")
            .Add("blocked-item", "item")
            .Close();
        problemBuilder.InitialState()
            .AddFact("selected", "accepted-group", "accepted-witness")
            .AddFact("selected", "rejected-group", "rejected-witness")
            .AddFact("belongs", "accepted-witness", "ready-item")
            .AddFact("belongs", "rejected-witness", "blocked-item")
            .AddFact("ready", "ready-item")
            .Close();
        Problem problem = problemBuilder.Goal().Add("accepted", "accepted-group").Close().Build();
        Mimir.Core.Schemas.Predicate<Derived> accepted = domain.Derived.Single(
            predicate => predicate.Name == "accepted");
        ExtendedState state = problem.InitialState.Expand();
        (string GroupName, bool Expected)[] expectations =
        [
            ("accepted-group", true),
            ("rejected-group", false),
            ("unselected-group", false),
        ];

        foreach ((string groupName, bool expected) in expectations)
        {
            Fact<Derived> fact = problem.Context.RegisterFact(
                accepted,
                [problem.ObjectLookup[groupName]]);

            Assert.Equal(expected, state.IsTrue(fact));
        }
    }

    [Fact]
    public void ProgrammaticRulesReuseVariableIdentityWithIndependentNestedFrames()
    {
        var shared = new Variable("?value");
        var present = new Constant("present", "object");
        var seed = new Mimir.Core.Schemas.Predicate<Fluent>(
            "seed",
            [new Variable("?value")]);
        var lower = new Mimir.Core.Schemas.Predicate<Derived>(
            "lower",
            [shared]);
        var upper = new Mimir.Core.Schemas.Predicate<Derived>(
            "upper",
            [shared]);
        Domain domain = new(
            "shared-rule-variable",
            [seed],
            Array.Empty<Mimir.Core.Schemas.Predicate<Static>>(),
            [
                new DerivedPredicateDefinition(
                    lower,
                    new GroundedAtom(seed, [shared])),
                new DerivedPredicateDefinition(
                    upper,
                    new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
                        new GroundedAtom(lower, [present]),
                        new GroundedNot(new GroundedAtom(seed, [shared]))))),
            ],
            Array.Empty<NumericFunction>(),
            [present],
            Array.Empty<ActionSchema>(),
            new Dictionary<string, string>(),
            [":derived-predicates"]);
        Problem problem = new ProblemBuilder(domain, "shared-rule-variable-problem")
            .Objects().Add("absent").Close()
            .InitialState().AddFact("seed", "present").Close()
            .Goal().Add("upper", "absent").Close()
            .Build();
        ExtendedState state = problem.InitialState.Expand();
        Fact<Derived> upperPresent = problem.Context.RegisterFact(
            upper,
            [problem.ObjectLookup["present"]]);
        Fact<Derived> lowerAbsent = problem.Context.RegisterFact(
            lower,
            [problem.ObjectLookup["absent"]]);
        Fact<Derived> lowerPresent = problem.Context.RegisterFact(
            lower,
            [problem.ObjectLookup["present"]]);
        Fact<Derived> upperAbsent = problem.Context.RegisterFact(
            upper,
            [problem.ObjectLookup["absent"]]);

        for (int iteration = 0; iteration < 4; iteration++)
        {
            Assert.False(state.IsTrue(upperPresent));
            Assert.False(state.IsTrue(lowerAbsent));
            Assert.True(state.IsTrue(lowerPresent));
            Assert.True(state.IsTrue(upperAbsent));
        }
    }

    [Fact]
    public void SiblingQuantifiersReuseVariableSlotAfterEarlyExit()
    {
        var item = new Variable("?item");
        var selected = new Mimir.Core.Schemas.Predicate<Fluent>(
            "selected",
            [new Variable("?item")]);
        var mixed = new Mimir.Core.Schemas.Predicate<Derived>(
            "mixed",
            Array.Empty<Variable>());
        var selectedItem = new GroundedAtom(selected, [item]);
        var body = new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
            new GroundedExists([item], selectedItem),
            new GroundedOr(ImmutableArray.Create<IGroundedExpression>(
                new GroundedForall([item], selectedItem),
                new GroundedExists(
                    [item],
                    new GroundedNot(selectedItem))))));
        Domain domain = new(
            "sibling-quantifier-slots",
            [selected],
            Array.Empty<Mimir.Core.Schemas.Predicate<Static>>(),
            [new DerivedPredicateDefinition(mixed, body)],
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            new Dictionary<string, string>(),
            [":adl"]);
        Problem problem = new ProblemBuilder(domain, "sibling-quantifier-slots-problem")
            .Objects().Add("selected-item").Add("unselected-item").Close()
            .InitialState().AddFact("selected", "selected-item").Close()
            .Goal().Add("mixed").Close()
            .Build();
        Fact<Derived> mixedFact = problem.Context.RegisterFact(
            mixed,
            Array.Empty<Constant>());
        ExtendedState state = problem.InitialState.Expand();

        for (int iteration = 0; iteration < 4; iteration++)
            Assert.True(state.IsTrue(mixedFact));
    }

    [Fact]
    public void TopLevelConjunctionCombinesGuardsAcrossHeadDependencies()
    {
        var first = new Variable("?first");
        var second = new Variable("?second");
        var globalReady = new Mimir.Core.Schemas.Predicate<Static>(
            "global-ready",
            Array.Empty<Variable>());
        var prefixReady = new Mimir.Core.Schemas.Predicate<Static>(
            "prefix-ready",
            [new Variable("?value")]);
        var fullReady = new Mimir.Core.Schemas.Predicate<Static>(
            "full-ready",
            [new Variable("?first"), new Variable("?second")]);
        var accepted = new Mimir.Core.Schemas.Predicate<Derived>(
            "accepted",
            [first, second]);
        var body = new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
            new GroundedAtom(globalReady, Array.Empty<ITerm>()),
            new GroundedAtom(prefixReady, [first]),
            new GroundedAtom(fullReady, [first, second])));
        Domain domain = new(
            "head-guard-depths",
            Array.Empty<Mimir.Core.Schemas.Predicate<Fluent>>(),
            [globalReady, prefixReady, fullReady],
            [new DerivedPredicateDefinition(accepted, body)],
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            new Dictionary<string, string>(),
            [":adl"]);
        Problem enabledProblem = new ProblemBuilder(domain, "enabled-head-guards")
            .Objects().Add("allowed").Add("blocked").Add("target").Add("other").Close()
            .InitialState()
                .AddFact("global-ready")
                .AddFact("prefix-ready", "allowed")
                .AddFact("full-ready", "allowed", "target")
                .AddFact("full-ready", "blocked", "target")
                .Close()
            .Goal().Add("accepted", "allowed", "target").Close()
            .Build();
        ExtendedState enabledState = enabledProblem.InitialState.Expand();
        Fact<Derived> acceptedFact = enabledProblem.Context.RegisterFact(
            accepted,
            [enabledProblem.ObjectLookup["allowed"], enabledProblem.ObjectLookup["target"]]);
        Fact<Derived> missingFullGuard = enabledProblem.Context.RegisterFact(
            accepted,
            [enabledProblem.ObjectLookup["allowed"], enabledProblem.ObjectLookup["other"]]);
        Fact<Derived> missingPrefixGuard = enabledProblem.Context.RegisterFact(
            accepted,
            [enabledProblem.ObjectLookup["blocked"], enabledProblem.ObjectLookup["target"]]);

        Assert.True(enabledState.IsTrue(acceptedFact));
        Assert.False(enabledState.IsTrue(missingFullGuard));
        Assert.False(enabledState.IsTrue(missingPrefixGuard));

        Problem globallyBlockedProblem = new ProblemBuilder(domain, "blocked-head-guards")
            .Objects().Add("allowed").Add("target").Close()
            .InitialState()
                .AddFact("prefix-ready", "allowed")
                .AddFact("full-ready", "allowed", "target")
                .Close()
            .Goal().Add("accepted", Polarity.Negative, "allowed", "target").Close()
            .Build();
        Fact<Derived> globallyBlocked = globallyBlockedProblem.Context.RegisterFact(
            accepted,
            [
                globallyBlockedProblem.ObjectLookup["allowed"],
                globallyBlockedProblem.ObjectLookup["target"],
            ]);

        Assert.False(globallyBlockedProblem.InitialState.Expand().IsTrue(globallyBlocked));
        Assert.Same(body, domain.DerivedDefinitions["accepted"]);
    }

    [Fact]
    public void ProgrammaticSingleRuleRecursionPropagatesAndRejectsDisabledPrefixes()
    {
        var side = new Variable("?side");
        var node = new Variable("?node");
        var previous = new Variable("?previous");
        var enabled = new Mimir.Core.Schemas.Predicate<Fluent>(
            "enabled",
            [new Variable("?side")]);
        var seed = new Mimir.Core.Schemas.Predicate<Fluent>(
            "seed",
            [new Variable("?side"), new Variable("?node")]);
        var edge = new Mimir.Core.Schemas.Predicate<Static>(
            "edge",
            [new Variable("?from"), new Variable("?to")]);
        var marked = new Mimir.Core.Schemas.Predicate<Derived>(
            "marked",
            [side, node]);
        var recursiveStep = new GroundedExists(
            [previous],
            new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
                new GroundedAtom(edge, [previous, node]),
                new GroundedAtom(marked, [side, previous]))));
        var seedOrRecursiveStep = new GroundedOr(
            ImmutableArray.Create<IGroundedExpression>(
                new GroundedAtom(seed, [side, node]),
                recursiveStep));
        var body = new GroundedAnd(ImmutableArray.Create<IGroundedExpression>(
            new GroundedAtom(enabled, [side]),
            seedOrRecursiveStep));
        Domain domain = new(
            "programmatic-recursion",
            [enabled, seed],
            [edge],
            [new DerivedPredicateDefinition(marked, body)],
            Array.Empty<NumericFunction>(),
            Array.Empty<Constant>(),
            Array.Empty<ActionSchema>(),
            new Dictionary<string, string>(),
            [":adl"]);
        Problem problem = new ProblemBuilder(domain, "programmatic-recursion-problem")
            .Objects().Add("active").Add("disabled").Add("a").Add("b").Add("c").Close()
            .InitialState()
                .AddFact("enabled", "active")
                .AddFact("seed", "active", "a")
                .AddFact("seed", "disabled", "a")
                .AddFact("edge", "a", "b")
                .AddFact("edge", "b", "c")
                .Close()
            .Goal().Add("marked", "active", "c").Close()
            .Build();
        Constant active = problem.ObjectLookup["active"];
        Constant disabled = problem.ObjectLookup["disabled"];
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Constant c = problem.ObjectLookup["c"];
        Fact<Derived> activeA = problem.Context.RegisterFact(marked, [active, a]);
        Fact<Derived> activeB = problem.Context.RegisterFact(marked, [active, b]);
        Fact<Derived> activeC = problem.Context.RegisterFact(marked, [active, c]);
        Fact<Derived> disabledA = problem.Context.RegisterFact(marked, [disabled, a]);
        Fact<Derived> disabledB = problem.Context.RegisterFact(marked, [disabled, b]);
        Fact<Derived> disabledC = problem.Context.RegisterFact(marked, [disabled, c]);
        ExtendedState state = problem.InitialState.Expand();

        Assert.True(state.IsTrue(activeA));
        Assert.True(state.IsTrue(activeB));
        Assert.True(state.IsTrue(activeC));
        Assert.False(state.IsTrue(disabledA));
        Assert.False(state.IsTrue(disabledB));
        Assert.False(state.IsTrue(disabledC));
        Assert.Same(body, domain.DerivedDefinitions["marked"]);
        GroundedAnd storedBody = Assert.IsType<GroundedAnd>(
            domain.DerivedDefinitions["marked"]);
        Assert.IsType<GroundedOr>(storedBody.Expressions[1]);
    }
}
