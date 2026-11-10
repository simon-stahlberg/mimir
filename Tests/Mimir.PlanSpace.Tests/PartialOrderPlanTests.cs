using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search.Planning;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace.Tests;

public class PartialOrderPlanTests
{
    private static readonly PlanSpaceOptions NoTombstones = new();
    private static string ExamplesPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private static Problem LoadBlocks()
    {
        Domain domain = Domain.FromFile(Path.Combine(ExamplesPath, "blocks_4", "domain.pddl"));
        return Problem.FromFile(domain, Path.Combine(ExamplesPath, "blocks_4", "p01.pddl"));
    }

    private static IReadOnlyList<Action> SolveWithBfs(Problem problem)
    {
        var result = new SearchBuilder().WithInitialState(problem.InitialState).WithGoal(problem).BuildBfs().Search();
        Assert.True(result.IsSuccess);
        return result.Plan;
    }

    private static Problem LoadThreatProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain threat)
  (:requirements :strips)
  (:predicates (p) (q) (r))
  (:action a :parameters () :precondition (p) :effect (q))
  (:action b :parameters () :precondition (p) :effect (and (r) (not (p)))))
""");
        return Problem.FromText(domain, """
(define (problem threat-1)
  (:domain threat)
  (:init (p))
  (:goal (and (q) (r))))
""");
    }

    private static Problem LoadDriveProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain drive)
  (:requirements :strips :typing :equality)
  (:types truck location - object)
  (:predicates (at ?t - truck ?l - location) (road ?a ?b - location))
  (:action drive
    :parameters (?t - truck ?from ?to - location)
    :precondition (and (at ?t ?from) (road ?from ?to) (not (= ?from ?to)))
    :effect (and (at ?t ?to) (not (at ?t ?from)))))
""");
        return Problem.FromText(domain, """
(define (problem drive-1)
  (:domain drive)
  (:objects t1 - truck l1 l2 l3 - location)
  (:init (at t1 l1) (road l1 l2) (road l2 l3))
  (:goal (at t1 l3)))
""");
    }

    // a produces p for b; c deletes p, so c must be demoted before a or b promoted before c.
    private static Problem LoadDemoteProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain demote)
  (:requirements :strips)
  (:predicates (s) (p) (g1) (g2))
  (:action a :parameters () :precondition (s) :effect (p))
  (:action b :parameters () :precondition (p) :effect (g1))
  (:action c :parameters () :precondition (s) :effect (and (g2) (not (p)))))
""");
        return Problem.FromText(domain, """
(define (problem demote-1)
  (:domain demote)
  (:init (s))
  (:goal (and (g1) (g2))))
""");
    }

    // Reaches the round in which c threatens the link a -> b on p; (s) is static, so the threat is the only flaw.
    private static PartialOrderPlan BuildDemoteThreat(Problem problem)
    {
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        plan.Apply(plan.ComputeFlaws(), [1], []);
        Flaw threat = Assert.Single(plan.ComputeFlaws().Flaws);
        Assert.Equal(FlawKind.Threat, threat.Kind);
        Assert.Equal([ResolverKind.NoOp, ResolverKind.Promote, ResolverKind.Demote, ResolverKind.DeleteStep], threat.Resolvers.Select(resolver => resolver.Kind));
        return plan;
    }

    private static Action Ground(Problem problem, string actionName, params string[] objectNames)
    {
        ActionSchema schema = problem.Domain.Actions.Single(candidate => candidate.Name == actionName);
        Constant[] objects = objectNames.Select(name => problem.AllObjects.Single(constant => constant.Name == name)).ToArray();
        return problem.GroundAction(schema, objects);
    }

    private static int IndexOf(Flaw flaw, Func<Resolver, bool> predicate)
    {
        for (int i = 0; i < flaw.Resolvers.Count; i++)
        {
            if (predicate(flaw.Resolvers[i])) return i;
        }

        throw new InvalidOperationException($"No matching resolver in {flaw}: {string.Join(", ", flaw.Resolvers)}");
    }

    private static int[] AllNoOp(FlawSet flaws) => new int[flaws.Flaws.Count];

    private static void AssertValidPlan(Problem problem, IReadOnlyList<Action> actions) => AssertValidPlan(problem, actions, problem.Goal);

    private static void AssertValidPlan(Problem problem, IReadOnlyList<Action> actions, IReadOnlyList<Literal<Fact>> goal)
    {
        ExtendedState state = problem.InitialState.Expand();
        foreach (Action action in actions)
        {
            Assert.True(action.IsApplicable(state), $"{action} is not applicable.");
            state = state.Apply(action).Expand();
        }

        foreach (Literal<Fact> literal in goal)
            Assert.True(state.IsTrue(literal.Value), $"Goal {literal} does not hold.");
    }

    // a and c both achieve a goal atom, but c deletes the one a produces, so c must be demoted before a.
    private static Problem LoadRelabelProblem()
    {
        Domain domain = Domain.FromText("""
(define (domain relabel)
  (:requirements :strips)
  (:predicates (s) (g1) (g2))
  (:action a :parameters () :precondition (s) :effect (g1))
  (:action c :parameters () :precondition (s) :effect (and (g2) (not (g1)))))
""");
        return Problem.FromText(domain, """
(define (problem relabel-1)
  (:domain relabel)
  (:init (s))
  (:goal (and (g1) (g2))))
""");
    }

    private static Literal<Fact> GoalLiteral(Problem problem, string text) => problem.Goal.Single(literal => literal.Value.ToString() == text);

    [Fact]
    public void EmptyPlanHasOneOpenConditionPerGoalFactWithNoOpFirst()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);

        FlawSet flaws = plan.ComputeFlaws();

        Assert.Equal(2, plan.Steps.Count);
        Assert.Equal(StepKind.Initial, plan.GetStep(0).Kind);
        Assert.Equal(StepKind.Goal, plan.GetStep(1).Kind);
        Assert.True(plan.IsBefore(0, 1));
        Assert.Equal(problem.Goal.Count, flaws.Flaws.Count);
        Assert.All(flaws.Flaws, flaw =>
        {
            Assert.Equal(FlawKind.OpenCondition, flaw.Kind);
            Assert.Equal(1, flaw.StepId);
            Assert.Equal(ResolverKind.NoOp, flaw.Resolvers[0].Kind);
        });
        Assert.False(plan.IsSolution());
    }

    [Fact]
    public void AchieverIndexUnifiesRepeatedVariablesAndConstants()
    {
        Domain domain = Domain.FromText("""
(define (domain repeat)
  (:requirements :strips :equality)
  (:constants c0)
  (:predicates (p ?x ?y) (q ?x))
  (:action dup :parameters (?x) :precondition (q ?x) :effect (p ?x ?x))
  (:action pair :parameters (?x ?y) :precondition (and (q ?x) (not (= ?x ?y))) :effect (p ?x ?y))
  (:action fixed :parameters (?x) :precondition (q ?x) :effect (p c0 ?x)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem repeat-1)
  (:domain repeat)
  (:objects a b)
  (:init (q a) (q b) (q c0))
  (:goal (and (p a a) (p c0 b))))
""");
        var index = new AchieverIndex(domain);
        Fact<Fluent> paa = (Fact<Fluent>)problem.Goal.Single(literal => literal.Value.ToString() == "(p a a)").Value;
        Fact<Fluent> pc0b = (Fact<Fluent>)problem.Goal.Single(literal => literal.Value.ToString() == "(p c0 b)").Value;

        Assert.Equal(["(dup a)", "(pair a a)"], index.Achievers(paa).Select(ghost => ghost.ToString()).Order());
        Assert.Equal(["(fixed b)", "(pair c0 b)"], index.Achievers(pc0b).Select(ghost => ghost.ToString()).Order());

        FlawSet flaws = PartialOrderPlan.Empty(problem, NoTombstones).ComputeFlaws();
        Flaw flawPaa = flaws.Flaws.Single(flaw => flaw.Fact!.Equals(paa));
        Flaw flawPc0b = flaws.Flaws.Single(flaw => flaw.Fact!.Equals(pc0b));
        // The equality precondition of pair rules out (pair a a); the static (q ?x) holds for every ghost.
        Assert.Equal(["(dup a)"], flawPaa.Resolvers.Skip(1).Select(resolver => flaws.Ghosts[resolver.GhostIndex].ToString()));
        Assert.Equal(["(pair c0 b)", "(fixed b)"], flawPc0b.Resolvers.Skip(1).Select(resolver => flaws.Ghosts[resolver.GhostIndex].ToString()));
    }

    [Fact]
    public void SameGhostAdoptedFromTwoFlawsCreatesOneStepWithTwoLinks()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        FlawSet flaws = plan.ComputeFlaws();

        Flaw clearB1 = flaws.Flaws.Single(flaw => flaw.Fact!.ToString() == "(clear b1)");
        Flaw onTableB1 = flaws.Flaws.Single(flaw => flaw.Fact!.ToString() == "(on-table b1)");
        int ghostFromClear = clearB1.Resolvers[IndexOf(clearB1, resolver => resolver.Kind == ResolverKind.AdoptGhost && flaws.Ghosts[resolver.GhostIndex].Schema.Name == "putdown")].GhostIndex;
        int ghostFromOnTable = onTableB1.Resolvers[IndexOf(onTableB1, resolver => resolver.Kind == ResolverKind.AdoptGhost && flaws.Ghosts[resolver.GhostIndex].Schema.Name == "putdown")].GhostIndex;
        Assert.Equal(ghostFromClear, ghostFromOnTable);
        Assert.Equal("(putdown b1)", flaws.Ghosts[ghostFromClear].ToString());

        int[] chosen = AllNoOp(flaws);
        chosen[flaws.Flaws.ToList().IndexOf(clearB1)] = IndexOf(clearB1, resolver => resolver.Kind == ResolverKind.AdoptGhost && resolver.GhostIndex == ghostFromClear);
        chosen[flaws.Flaws.ToList().IndexOf(onTableB1)] = IndexOf(onTableB1, resolver => resolver.Kind == ResolverKind.AdoptGhost && resolver.GhostIndex == ghostFromClear);
        EditResult result = plan.Apply(flaws, chosen, []);

        Assert.Equal(2, result.AppliedCount);
        Assert.Empty(result.DroppedFlawIndices);
        int created = Assert.Single(result.CreatedStepIds);
        Assert.Equal(3, plan.Steps.Count);
        PlanStep step = plan.GetStep(created);
        Assert.True(step.IsFullyBound);
        Assert.Equal("putdown", step.Schema!.Name);
        Assert.Equal(2, plan.Links.Count);
        Assert.All(plan.Links, link => Assert.Equal(created, link.ProducerId));
        Assert.True(plan.IsBefore(created, 1));
        Assert.True(plan.IsBefore(0, created));
        Assert.Equal(1, plan.Round);

        Assert.Throws<InvalidOperationException>(() => plan.Apply(flaws, chosen, []));
    }

    [Fact]
    public void BindCandidatesRespectTypesAndStaticPreconditions()
    {
        Problem problem = LoadDriveProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        FlawSet flaws = plan.ComputeFlaws();

        Flaw goalFlaw = Assert.Single(flaws.Flaws);
        Assert.Equal(2, goalFlaw.Resolvers.Count);
        Assert.Equal("(drive t1 ? l3)", flaws.Ghosts[goalFlaw.Resolvers[1].GhostIndex].ToString());
        plan.Apply(flaws, [1], []);

        flaws = plan.ComputeFlaws();
        Flaw unbound = Assert.Single(flaws.Flaws);
        Assert.Equal(FlawKind.UnboundParameter, unbound.Kind);
        Assert.Equal(1, unbound.ParameterIndex);
        Assert.False(plan.GetStep(unbound.StepId).IsFullyBound);
        // Trucks are excluded by type, l3 by equality and l1 because (road l1 l3) does not hold.
        Assert.Equal(["l2"], unbound.Resolvers.Skip(1).Select(resolver => resolver.Object!.Name));

        plan.Apply(flaws, [1], []);
        PlanStep step = plan.GetStep(unbound.StepId);
        Assert.True(step.IsFullyBound);
        Assert.Equal("(drive t1 l2 l3)", step.ToString().Substring(step.ToString().IndexOf('(')));
        flaws = plan.ComputeFlaws();
        Flaw open = Assert.Single(flaws.Flaws);
        Assert.Equal("(at t1 l2)", open.Fact!.ToString());
    }

    [Fact]
    public void ThreatIsDetectedAndPromoteOrdersConsumerBeforeThreatener()
    {
        Problem problem = LoadThreatProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        FlawSet flaws = plan.ComputeFlaws();
        plan.Apply(flaws, [1, 1], []);
        flaws = plan.ComputeFlaws();
        Assert.All(flaws.Flaws, flaw => Assert.Equal(FlawKind.OpenCondition, flaw.Kind));
        Assert.All(flaws.Flaws, flaw => Assert.Equal(ResolverKind.LinkExisting, flaw.Resolvers[1].Kind));
        plan.Apply(flaws, [1, 1], []);

        flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);
        Assert.Equal(FlawKind.Threat, threat.Kind);
        PlanStep threatener = plan.GetStep(threat.StepId);
        Assert.Equal("b", threatener.Schema!.Name);
        CausalLink link = plan.GetLink(threat.LinkId);
        Assert.Equal(0, link.ProducerId);
        Assert.Equal("a", plan.GetStep(link.ConsumerId).Schema!.Name);
        // Demote would order b before the initial step, so only Promote and DeleteStep are offered.
        Assert.Equal([ResolverKind.NoOp, ResolverKind.Promote, ResolverKind.DeleteStep], threat.Resolvers.Select(resolver => resolver.Kind));

        PartialOrderPlan promoted = plan.Clone();
        FlawSet promotedFlaws = promoted.ComputeFlaws();
        EditResult result = promoted.Apply(promotedFlaws, [1], []);
        Assert.Equal(1, result.AppliedCount);
        Assert.True(promoted.IsBefore(link.ConsumerId, threatener.Id));
        Assert.True(promoted.IsSolution());
        Assert.Equal(["(a)", "(b)"], promoted.Linearize().Select(action => action.ToString()));
        AssertValidPlan(problem, promoted.Linearize());
        Assert.False(plan.IsBefore(link.ConsumerId, threatener.Id));
    }

    [Fact]
    public void DemoteOrdersThreatenerBeforeProducer()
    {
        Problem problem = LoadDemoteProblem();
        PartialOrderPlan plan = BuildDemoteThreat(problem);
        FlawSet flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);
        CausalLink link = plan.GetLink(threat.LinkId);
        Assert.Equal("a", plan.GetStep(link.ProducerId).Schema!.Name);
        Assert.Equal("b", plan.GetStep(link.ConsumerId).Schema!.Name);
        Assert.Equal("c", plan.GetStep(threat.StepId).Schema!.Name);

        EditResult result = plan.Apply(flaws, [IndexOf(threat, resolver => resolver.Kind == ResolverKind.Demote)], []);

        Assert.Equal(1, result.AppliedCount);
        Assert.True(plan.IsBefore(threat.StepId, link.ProducerId));
        Assert.True(plan.IsBefore(threat.StepId, link.ConsumerId));
        Assert.True(plan.IsSolution());
        Assert.Equal(["(c)", "(a)", "(b)"], plan.Linearize().Select(action => action.ToString()));
        AssertValidPlan(problem, plan.Linearize());
    }

    [Fact]
    public void DeletingALinkedStepRemovesTheOrderingsThatResolvedThreatsOnItsLinks()
    {
        Problem problem = LoadDemoteProblem();
        PartialOrderPlan plan = BuildDemoteThreat(problem);
        FlawSet flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);
        CausalLink link = plan.GetLink(threat.LinkId);
        plan.Apply(flaws, [IndexOf(threat, resolver => resolver.Kind == ResolverKind.Promote)], []);
        Assert.True(plan.IsBefore(link.ConsumerId, threat.StepId));
        Assert.True(plan.IsSolution());

        flaws = plan.ComputeFlaws();
        EditResult result = plan.Apply(flaws, AllNoOp(flaws), [link.ProducerId]);

        Assert.Equal(1, result.AppliedCount);
        Assert.Throws<ArgumentException>(() => plan.GetLink(link.Id));
        // b before c was only needed while a produced p for b.
        Assert.False(plan.IsBefore(link.ConsumerId, threat.StepId));
        Assert.False(plan.IsBefore(threat.StepId, link.ConsumerId));
        Flaw reopened = Assert.Single(plan.ComputeFlaws().Flaws);
        Assert.Equal(FlawKind.OpenCondition, reopened.Kind);
        Assert.Equal(link.ConsumerId, reopened.StepId);
    }

    [Fact]
    public void CloneAfterDeletionPreservesClosureAndEvolvesIndependently()
    {
        Problem problem = LoadDemoteProblem();
        PartialOrderPlan plan = BuildDemoteThreat(problem);
        FlawSet flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);
        CausalLink link = plan.GetLink(threat.LinkId);
        plan.Apply(flaws, [IndexOf(threat, resolver => resolver.Kind == ResolverKind.Promote)], []);
        flaws = plan.ComputeFlaws();
        plan.Apply(flaws, AllNoOp(flaws), [link.ProducerId]);

        PartialOrderPlan clone = plan.Clone();

        Assert.Equal(plan.OrderingClosure, clone.OrderingClosure);
        Assert.DoesNotContain(clone.OrderingClosure, pair => pair.Before == link.ProducerId || pair.After == link.ProducerId);
        foreach (PlanStep first in plan.Steps)
            foreach (PlanStep second in plan.Steps)
                Assert.Equal(plan.IsBefore(first.Id, second.Id), clone.IsBefore(first.Id, second.Id));
        Assert.Equal(plan.ComputeFlaws().Flaws.Select(flaw => flaw.ToString()), clone.ComputeFlaws().Flaws.Select(flaw => flaw.ToString()));

        FlawSet cloneFlaws = clone.ComputeFlaws();
        EditResult result = clone.Apply(cloneFlaws, [1], []);
        int created = Assert.Single(result.CreatedStepIds);
        Assert.True(clone.IsBefore(0, created));
        Assert.True(clone.IsBefore(created, link.ConsumerId));
        Assert.Throws<ArgumentException>(() => plan.GetStep(created));
        Assert.Equal(plan.Steps.Count + 1, clone.Steps.Count);
    }

    [Fact]
    public void DeleteStepChosenAndListedExplicitlyCountsOnce()
    {
        Problem problem = LoadThreatProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        FlawSet flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);

        EditResult result = plan.Apply(flaws, [IndexOf(threat, resolver => resolver.Kind == ResolverKind.DeleteStep)], [threat.StepId]);

        Assert.Equal(1, result.AppliedCount);
        Assert.Equal([threat.StepId], result.DeletedStepIds);
    }

    [Fact]
    public void BindingTwoParametersToConflictingObjectsInOneRoundDropsTheSecond()
    {
        Domain domain = Domain.FromText("""
(define (domain roads)
  (:requirements :strips :typing)
  (:types truck location - object)
  (:predicates (at ?t - truck ?l - location) (road ?a ?b - location) (moved ?t - truck))
  (:action drive
    :parameters (?t - truck ?from ?to - location)
    :precondition (and (at ?t ?from) (road ?from ?to))
    :effect (and (at ?t ?to) (not (at ?t ?from)) (moved ?t))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem roads-1)
  (:domain roads)
  (:objects t1 - truck l1 l2 - location)
  (:init (at t1 l1) (road l1 l2) (road l2 l1))
  (:goal (moved t1)))
""");
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        EditResult adopted = plan.Apply(plan.ComputeFlaws(), [1], []);
        int stepId = Assert.Single(adopted.CreatedStepIds);
        Assert.Equal("(drive t1 ? ?)", plan.GetStep(stepId).ToString().Substring(2));

        FlawSet flaws = plan.ComputeFlaws();
        Assert.Equal(2, flaws.Flaws.Count);
        Assert.All(flaws.Flaws, flaw => Assert.Equal(FlawKind.UnboundParameter, flaw.Kind));
        Assert.Equal([1, 2], flaws.Flaws.Select(flaw => flaw.ParameterIndex));
        // Each parameter is filtered with the other unbound, so l1 is offered for both.
        Assert.All(flaws.Flaws, flaw => Assert.Equal(["l1", "l2"], flaw.Resolvers.Skip(1).Select(resolver => resolver.Object!.Name)));

        EditResult result = plan.Apply(flaws, [1, 1], []);

        Assert.Equal(1, result.AppliedCount);
        Assert.Equal([1], result.DroppedFlawIndices);
        PlanStep step = plan.GetStep(stepId);
        Assert.False(step.IsFullyBound);
        Assert.Equal("(drive t1 l1 ?)", step.ToString().Substring(2));

        flaws = plan.ComputeFlaws();
        Flaw remaining = Assert.Single(flaws.Flaws);
        Assert.Equal(2, remaining.ParameterIndex);
        Assert.Equal(["l2"], remaining.Resolvers.Skip(1).Select(resolver => resolver.Object!.Name));
        plan.Apply(flaws, [1], []);
        Assert.True(step.IsFullyBound);
        Assert.Equal("(drive t1 l1 l2)", step.ToString().Substring(2));
    }

    [Fact]
    public void ConflictingOrderingsInOneRoundDropTheCyclicOne()
    {
        Domain domain = Domain.FromText("""
(define (domain mutex)
  (:requirements :strips)
  (:predicates (p) (q) (gp) (gq))
  (:action a :parameters () :precondition (p) :effect (and (gp) (not (q))))
  (:action b :parameters () :precondition (q) :effect (and (gq) (not (p)))))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem mutex-1)
  (:domain mutex)
  (:init (p) (q))
  (:goal (and (gp) (gq))))
""");
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);

        FlawSet flaws = plan.ComputeFlaws();
        Assert.Equal(2, flaws.Flaws.Count);
        Assert.All(flaws.Flaws, flaw => Assert.Equal(FlawKind.Threat, flaw.Kind));
        Assert.All(flaws.Flaws, flaw => Assert.Equal(ResolverKind.Promote, flaw.Resolvers[1].Kind));

        EditResult result = plan.Apply(flaws, [1, 1], []);

        Assert.Equal(1, result.AppliedCount);
        Assert.Equal([1], result.DroppedFlawIndices);
        Assert.True(plan.IsBefore(plan.GetLink(flaws.Flaws[0].LinkId).ConsumerId, flaws.Flaws[0].StepId));
        FlawSet remaining = plan.ComputeFlaws();
        Flaw left = Assert.Single(remaining.Flaws);
        Assert.Equal(flaws.Flaws[1].StepId, left.StepId);
        Assert.Equal([ResolverKind.NoOp, ResolverKind.DeleteStep], left.Resolvers.Select(resolver => resolver.Kind));
    }

    [Fact]
    public void DeletingAStepRemovesItsLinksAndReopensConditions()
    {
        Problem problem = LoadThreatProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        Assert.Equal(4, plan.Links.Count);

        FlawSet flaws = plan.ComputeFlaws();
        Flaw threat = Assert.Single(flaws.Flaws);
        EditResult result = plan.Apply(flaws, [IndexOf(threat, resolver => resolver.Kind == ResolverKind.DeleteStep)], []);

        Assert.Equal([threat.StepId], result.DeletedStepIds);
        Assert.Equal(3, plan.Steps.Count);
        Assert.Equal(2, plan.Links.Count);
        Assert.DoesNotContain(plan.Links, link => link.ProducerId == threat.StepId || link.ConsumerId == threat.StepId);
        Assert.Throws<ArgumentException>(() => plan.GetStep(threat.StepId));
        Assert.DoesNotContain(plan.OrderingClosure, pair => pair.Before == threat.StepId || pair.After == threat.StepId);

        flaws = plan.ComputeFlaws();
        Flaw reopened = Assert.Single(flaws.Flaws);
        Assert.Equal(FlawKind.OpenCondition, reopened.Kind);
        Assert.Equal("(r)", reopened.Fact!.ToString());
        Assert.Equal(1, reopened.StepId);

        Assert.Throws<ArgumentException>(() => plan.Apply(flaws, AllNoOp(flaws), [0]));
        Assert.Throws<ArgumentException>(() => plan.Apply(flaws, AllNoOp(flaws), [threat.StepId]));
    }

    [Fact]
    public void TombstonesHonourCapacityAndLifespan()
    {
        Problem problem = LoadThreatProblem();

        PartialOrderPlan disabled = PartialOrderPlan.Empty(problem, NoTombstones);
        EditResult created = disabled.Apply(disabled.ComputeFlaws(), [1, 1], []);
        disabled.Apply(disabled.ComputeFlaws(), AllNoOp(disabled.ComputeFlaws()), created.CreatedStepIds);
        Assert.Empty(disabled.Tombstones);
        Assert.Equal(2, disabled.Steps.Count);

        PartialOrderPlan capped = PartialOrderPlan.Empty(problem, new PlanSpaceOptions(TombstoneCapacity: 1, TombstoneLifespan: 5));
        created = capped.Apply(capped.ComputeFlaws(), [1, 1], []);
        capped.Apply(capped.ComputeFlaws(), AllNoOp(capped.ComputeFlaws()), created.CreatedStepIds);
        Tombstone survivor = Assert.Single(capped.Tombstones);
        Assert.Equal("b", survivor.Schema.Name);
        Assert.Equal(1, survivor.Age);

        PartialOrderPlan shortLived = PartialOrderPlan.Empty(problem, new PlanSpaceOptions(TombstoneCapacity: 4, TombstoneLifespan: 1));
        created = shortLived.Apply(shortLived.ComputeFlaws(), [1, 1], []);
        shortLived.Apply(shortLived.ComputeFlaws(), AllNoOp(shortLived.ComputeFlaws()), created.CreatedStepIds);
        Assert.Equal(2, shortLived.Tombstones.Count);
        Assert.All(shortLived.Tombstones, tombstone => Assert.Equal(1, tombstone.Age));
        shortLived.Apply(shortLived.ComputeFlaws(), AllNoOp(shortLived.ComputeFlaws()), []);
        Assert.Empty(shortLived.Tombstones);

        // Partially bound steps leave no tombstone.
        PartialOrderPlan drive = PartialOrderPlan.Empty(LoadDriveProblem(), new PlanSpaceOptions(TombstoneCapacity: 4, TombstoneLifespan: 4));
        created = drive.Apply(drive.ComputeFlaws(), [1], []);
        drive.Apply(drive.ComputeFlaws(), [0], created.CreatedStepIds);
        Assert.Empty(drive.Tombstones);
    }

    [Fact]
    public void SequentialPlanDeordersWithoutFlawsAndLinearizesToAValidPlan()
    {
        Problem problem = LoadBlocks();
        IReadOnlyList<Action> sequential = SolveWithBfs(problem);

        PartialOrderPlan plan = PartialOrderPlan.FromSequentialPlan(problem, sequential, NoTombstones);

        Assert.Empty(plan.ComputeFlaws().Flaws);
        Assert.True(plan.IsSolution());
        Assert.Equal(sequential.Count + 2, plan.Steps.Count);
        Assert.All(plan.Steps, step => Assert.True(step.IsFullyBound));
        IReadOnlyList<Action> linearized = plan.Linearize();
        Assert.Equal(sequential.Count, linearized.Count);
        AssertValidPlan(problem, linearized);

        PartialOrderPlan clone = plan.Clone();
        Assert.Equal(plan.OrderingClosure, clone.OrderingClosure);
        Assert.Equal(plan.Links.Select(link => link.ToString()), clone.Links.Select(link => link.ToString()));
        Assert.True(clone.IsSolution());
    }

    [Fact]
    public void SequentialPlanNeedingADemotionIsDeorderedWithoutFlaws()
    {
        Problem problem = LoadDemoteProblem();

        PartialOrderPlan plan = PartialOrderPlan.FromSequentialPlan(problem, [Ground(problem, "c"), Ground(problem, "a"), Ground(problem, "b")], NoTombstones);

        Assert.True(plan.IsSolution());
        PlanStep c = plan.Steps.Single(step => step.Schema?.Name == "c");
        PlanStep a = plan.Steps.Single(step => step.Schema?.Name == "a");
        PlanStep b = plan.Steps.Single(step => step.Schema?.Name == "b");
        Assert.True(plan.IsBefore(c.Id, a.Id));
        Assert.True(plan.IsBefore(a.Id, b.Id));
        Assert.Equal(["(c)", "(a)", "(b)"], plan.Linearize().Select(action => action.ToString()));

        PartialOrderPlan promoted = PartialOrderPlan.FromSequentialPlan(problem, [Ground(problem, "a"), Ground(problem, "b"), Ground(problem, "c")], NoTombstones);
        Assert.True(promoted.IsSolution());
        Assert.True(promoted.IsBefore(promoted.Steps.Single(step => step.Schema?.Name == "b").Id, promoted.Steps.Single(step => step.Schema?.Name == "c").Id));
        Assert.Throws<InvalidOperationException>(() => PartialOrderPlan.FromSequentialPlan(problem, [Ground(problem, "a"), Ground(problem, "c"), Ground(problem, "b")], NoTombstones));
    }

    [Fact]
    public void SequentialPlanWithUnsatisfiedPreconditionThrows()
    {
        Problem problem = LoadBlocks();
        ActionSchema pickup = problem.Domain.Actions.Single(schema => schema.Name == "pickup");
        Constant b1 = problem.AllObjects.Single(constant => constant.Name == "b1");
        Action invalid = problem.GroundAction(pickup, b1);

        Assert.Throws<InvalidOperationException>(() => PartialOrderPlan.FromSequentialPlan(problem, [invalid], NoTombstones));
    }

    [Fact]
    public void ReferenceOracleGuidesEmptyPlanToASolution()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan reference = PartialOrderPlan.FromSequentialPlan(problem, SolveWithBfs(problem), NoTombstones);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);

        int rounds = RollOut(current, reference, 40);

        Assert.True(rounds > 0);
        Assert.True(current.IsSolution());
        AssertValidPlan(problem, current.Linearize());
        Assert.Equal(reference.Steps.Count, current.Steps.Count);
    }

    [Fact]
    public void ReferenceOracleRequestsDeletionOfSpuriousStep()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan reference = PartialOrderPlan.FromSequentialPlan(problem, SolveWithBfs(problem), NoTombstones);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);

        FlawSet flaws = current.ComputeFlaws();
        Flaw clearB1 = flaws.Flaws.Single(flaw => flaw.Fact!.ToString() == "(clear b1)");
        int[] chosen = AllNoOp(flaws);
        chosen[flaws.Flaws.ToList().IndexOf(clearB1)] = IndexOf(clearB1, resolver => resolver.Kind == ResolverKind.AdoptGhost
            && flaws.Ghosts[resolver.GhostIndex].ToString() == "(unstack ? b1)");
        EditResult result = current.Apply(flaws, chosen, []);
        int spurious = Assert.Single(result.CreatedStepIds);

        flaws = current.ComputeFlaws();
        OracleTargets targets = ReferenceOracle.Targets(current, flaws, reference);

        Assert.Equal([spurious], targets.StepIdsToDelete);
        Assert.Equal(0, targets.MatchedStepCount);
        Flaw unbound = Assert.Single(flaws.Flaws, flaw => flaw.Kind == FlawKind.UnboundParameter);
        Assert.Equal(0, targets.ResolverIndexPerFlaw[flaws.Flaws.ToList().IndexOf(unbound)]);
        Flaw onB2B3 = flaws.Flaws.Single(flaw => flaw.Fact?.ToString() == "(on b2 b3)");
        Resolver target = onB2B3.Resolvers[targets.ResolverIndexPerFlaw[flaws.Flaws.ToList().IndexOf(onB2B3)]];
        Assert.Equal(ResolverKind.AdoptGhost, target.Kind);
        Assert.Equal("(stack b2 b3)", flaws.Ghosts[target.GhostIndex].ToString());

        current.Apply(flaws, targets.ResolverIndexPerFlaw, targets.StepIdsToDelete);
        Assert.Throws<ArgumentException>(() => current.GetStep(spurious));
        RollOut(current, reference, 40);
        Assert.True(current.IsSolution());
        AssertValidPlan(problem, current.Linearize());
    }

    [Fact]
    public void ReferenceOracleRejectsStaleFlawSet()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan reference = PartialOrderPlan.FromSequentialPlan(problem, SolveWithBfs(problem), NoTombstones);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);
        FlawSet flaws = current.ComputeFlaws();
        current.Apply(flaws, AllNoOp(flaws), []);

        Assert.Throws<ArgumentException>(() => ReferenceOracle.Targets(current, flaws, reference));
        Assert.Throws<ArgumentException>(() => ReferenceOracle.Targets(current.Clone(), current.ComputeFlaws(), reference));
    }

    [Fact]
    public void ReferenceOracleMatchesRepeatedStepsAlongCausalLinks()
    {
        Domain domain = Domain.FromText("""
(define (domain repeated)
  (:requirements :strips)
  (:predicates (p) (q) (g1) (g2))
  (:action a :parameters () :precondition (p) :effect (and (q) (not (p))))
  (:action b :parameters () :precondition (q) :effect (and (p) (g1) (not (q))))
  (:action c :parameters () :precondition (and (q) (g1)) :effect (g2)))
""");
        Problem problem = Problem.FromText(domain, """
(define (problem repeated-1)
  (:domain repeated)
  (:init (p))
  (:goal (g2)))
""");
        IReadOnlyList<Action> sequential = [Ground(problem, "a"), Ground(problem, "b"), Ground(problem, "a"), Ground(problem, "c")];
        AssertValidPlan(problem, sequential);
        PartialOrderPlan reference = PartialOrderPlan.FromSequentialPlan(problem, sequential, NoTombstones);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);

        // Adopting c, then a and b for its open conditions, puts a's link to c in place before the
        // b -> a link, so a must be matched to the second reference a or b's threat on it is unresolvable.
        RollOut(current, reference, 50);

        Assert.True(current.IsSolution());
        Assert.Equal(reference.Steps.Count, current.Steps.Count);
        Assert.Equal(sequential.Select(action => action.ToString()), current.Linearize().Select(action => action.ToString()));
    }

    [Fact]
    public void ReferenceOracleGuidesGripperWithTwoIdenticalMoves()
    {
        Domain domain = Domain.FromFile(Path.Combine(ExamplesPath, "gripper", "domain.pddl"));
        Problem problem = Problem.FromText(domain, """
(define (problem gripper-one-gripper)
  (:domain gripper-strips)
  (:objects left ball1 ball2)
  (:init (room rooma) (room roomb) (gripper left) (ball ball1) (ball ball2)
         (free left) (at ball1 rooma) (at ball2 rooma) (at-robby rooma))
  (:goal (and (at ball1 roomb) (at ball2 roomb))))
""");
        IReadOnlyList<Action> sequential = SolveWithBfs(problem);
        Assert.Equal(7, sequential.Count);
        Assert.Equal(2, sequential.Count(action => action.ToString() == "(move rooma roomb)"));
        PartialOrderPlan reference = PartialOrderPlan.FromSequentialPlan(problem, sequential, NoTombstones);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);

        RollOut(current, reference, 50);

        Assert.True(current.IsSolution());
        Assert.Equal(reference.Steps.Count, current.Steps.Count);
        AssertValidPlan(problem, current.Linearize());
    }

    private static int RollOut(PartialOrderPlan current, PartialOrderPlan reference, int maxRounds)
    {
        int rounds = 0;
        while (!current.IsSolution())
        {
            Assert.True(rounds < maxRounds, $"No solution after {maxRounds} rounds; flaws: {string.Join(", ", current.ComputeFlaws().Flaws)}");
            FlawSet flaws = current.ComputeFlaws();
            OracleTargets targets = ReferenceOracle.Targets(current, flaws, reference);
            current.Apply(flaws, targets.ResolverIndexPerFlaw, targets.StepIdsToDelete);
            rounds++;
        }

        return rounds;
    }

    [Fact]
    public void UnsupportedFeaturesAreRejectedAtConstruction()
    {
        Domain negative = Domain.FromText("""
(define (domain negative)
  (:requirements :strips :negative-preconditions)
  (:predicates (p) (q))
  (:action a :parameters () :precondition (not (p)) :effect (q))
  (:action b :parameters () :precondition (q) :effect (p)))
""");
        Problem negativeProblem = Problem.FromText(negative, """
(define (problem negative-1)
  (:domain negative)
  (:init)
  (:goal (q)))
""");
        Assert.Throws<NotSupportedException>(() => PartialOrderPlan.Empty(negativeProblem, NoTombstones));

        Domain conditional = Domain.FromText("""
(define (domain conditional)
  (:requirements :strips :conditional-effects)
  (:predicates (p) (q) (r))
  (:action a :parameters () :precondition (p) :effect (and (q) (when (r) (not (p))))))
""");
        Problem conditionalProblem = Problem.FromText(conditional, """
(define (problem conditional-1)
  (:domain conditional)
  (:init (p))
  (:goal (q)))
""");
        Assert.Throws<NotSupportedException>(() => PartialOrderPlan.Empty(conditionalProblem, NoTombstones));

        Problem negativeGoal = Problem.FromText(LoadThreatProblem().Domain, """
(define (problem negative-goal)
  (:domain threat)
  (:init (p))
  (:goal (and (q) (not (p)))))
""");
        Assert.Throws<NotSupportedException>(() => PartialOrderPlan.Empty(negativeGoal, NoTombstones));
    }

    [Fact]
    public void WithGoalDropsLinksAndOrderingsOfRemovedAtomsAndKeepsTheRest()
    {
        Problem problem = LoadRelabelProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        Flaw threat = Assert.Single(plan.ComputeFlaws().Flaws);
        Assert.Equal(FlawKind.Threat, threat.Kind);
        plan.Apply(plan.ComputeFlaws(), [IndexOf(threat, resolver => resolver.Kind == ResolverKind.Demote)], []);
        Assert.True(plan.IsSolution());
        PlanStep a = plan.Steps.Single(step => step.Schema?.Name == "a");
        PlanStep c = plan.Steps.Single(step => step.Schema?.Name == "c");
        Assert.True(plan.IsBefore(c.Id, a.Id));
        Assert.Equal(problem.Goal, plan.Goal);

        PartialOrderPlan relabeled = plan.WithGoal([GoalLiteral(problem, "(g2)")]);

        Assert.Equal([GoalLiteral(problem, "(g2)")], relabeled.Goal);
        Assert.Equal(plan.Steps.Select(step => step.ToString()), relabeled.Steps.Select(step => step.ToString()));
        CausalLink kept = Assert.Single(relabeled.Links);
        Assert.Equal((c.Id, 1, "(g2)"), (kept.ProducerId, kept.ConsumerId, kept.Fact.ToString()));
        Assert.False(relabeled.IsBefore(c.Id, a.Id));
        Assert.True(relabeled.IsSolution());
        Assert.Equal(["(c)"], relabeled.Linearize().Select(action => action.ToString()).Where(action => action == "(c)"));
        Assert.Equal(2, plan.Links.Count);
        Assert.True(plan.IsBefore(c.Id, a.Id));

        PartialOrderPlan restored = relabeled.WithGoal(problem.Goal);
        Flaw reopened = Assert.Single(restored.ComputeFlaws().Flaws);
        Assert.Equal((FlawKind.OpenCondition, 1, "(g1)"), (reopened.Kind, reopened.StepId, reopened.Fact!.ToString()));
        Assert.Contains(reopened.Resolvers, resolver => resolver.Kind == ResolverKind.LinkExisting && resolver.StepId == a.Id);
        Assert.Single(restored.Links);

        Problem other = LoadRelabelProblem();
        Assert.Throws<ArgumentException>(() => plan.WithGoal(other.Goal));
    }

    [Fact]
    public void EmptyPlanWithASubsetGoalHasOneOpenConditionPerSubsetAtom()
    {
        Problem problem = LoadBlocks();
        Literal<Fact>[] subset = [GoalLiteral(problem, "(on b2 b3)"), GoalLiteral(problem, "(on-table b1)")];

        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, subset, NoTombstones);

        Assert.Equal(subset, plan.Goal);
        FlawSet flaws = plan.ComputeFlaws();
        Assert.Equal(["(on b2 b3)", "(on-table b1)"], flaws.Flaws.Select(flaw => flaw.Fact!.ToString()));
        Assert.All(flaws.Flaws, flaw => Assert.Equal(FlawKind.OpenCondition, flaw.Kind));
        Assert.True(PartialOrderPlan.Empty(problem, [], NoTombstones).IsSolution());
        Assert.Throws<ArgumentException>(() => PartialOrderPlan.Empty(LoadBlocks(), subset, NoTombstones));
    }

    [Fact]
    public void PruneToAchievedGoalOnASolvedPlanKeepsTheFullGoal()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = PartialOrderPlan.FromSequentialPlan(problem, SolveWithBfs(problem), NoTombstones);

        PartialOrderPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal(problem.Goal, pruned.Goal);
        Assert.True(pruned.IsSolution());
        Assert.Equal(plan.Steps.Count, pruned.Steps.Count);
        Assert.Equal(plan.Links.Count, pruned.Links.Count);
        Assert.Equal(plan.OrderingClosure.Count, pruned.OrderingClosure.Count);
        Assert.Equal(plan.Linearize().Select(action => action.ToString()), pruned.Linearize().Select(action => action.ToString()));
        AssertValidPlan(problem, pruned.Linearize());
    }

    [Fact]
    public void PruneToAchievedGoalDropsAtomsWhoseSubPlanHasAnOpenCondition()
    {
        Problem problem = LoadDemoteProblem();
        PartialOrderPlan plan = PartialOrderPlan.Empty(problem, NoTombstones);
        plan.Apply(plan.ComputeFlaws(), [1, 1], []);
        Flaw open = Assert.Single(plan.ComputeFlaws().Flaws);
        Assert.Equal("(p)", open.Fact!.ToString());

        PartialOrderPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal([GoalLiteral(problem, "(g2)")], pruned.Goal);
        Assert.True(pruned.IsSolution());
        Assert.Equal([(0, StepKind.Initial, null), (1, StepKind.Goal, null), (2, StepKind.Step, "c")], pruned.Steps.Select(step => (step.Id, step.Kind, step.Schema?.Name)));
        CausalLink link = Assert.Single(pruned.Links);
        Assert.Equal((2, 1, "(g2)"), (link.ProducerId, link.ConsumerId, link.Fact.ToString()));
        AssertValidPlan(problem, pruned.Linearize(), pruned.Goal);
        Assert.Equal(4, plan.Steps.Count);
        Assert.False(plan.IsSolution());
    }

    [Fact]
    public void PruneToAchievedGoalOnABrokenBlocksPlanKeepsTheIntactSubPlan()
    {
        Problem problem = LoadBlocks();
        PartialOrderPlan plan = PartialOrderPlan.FromSequentialPlan(problem, SolveWithBfs(problem), NoTombstones);
        CausalLink inner = plan.Links.First(link => plan.GetStep(link.ProducerId).Kind == StepKind.Step && plan.GetStep(link.ConsumerId).Kind == StepKind.Step);
        FlawSet flaws = plan.ComputeFlaws();
        plan.Apply(flaws, AllNoOp(flaws), [inner.ProducerId]);
        Assert.False(plan.IsSolution());

        PartialOrderPlan pruned = plan.PruneToAchievedGoal();

        var expected = new List<Literal<Fact>>();
        foreach (Literal<Fact> literal in problem.Goal)
        {
            var reachable = new HashSet<int>();
            var frontier = new Stack<int>(plan.Links.Where(link => link.ConsumerId == 1 && link.Fact.Equals(literal.Value)).Select(link => link.ProducerId));
            while (frontier.Count > 0)
            {
                int id = frontier.Pop();
                if (!reachable.Add(id)) continue;
                foreach (CausalLink link in plan.Links.Where(link => link.ConsumerId == id)) frontier.Push(link.ProducerId);
            }

            if (reachable.Count > 0 && !reachable.Contains(inner.ConsumerId)) expected.Add(literal);
        }

        Assert.Equal(expected, pruned.Goal);
        Assert.True(pruned.Goal.Count < problem.Goal.Count);
        Assert.True(pruned.IsSolution());
        Assert.DoesNotContain(pruned.Steps, step => step.ToString().Substring(2) == plan.GetStep(inner.ConsumerId).ToString().Substring(2));
        AssertValidPlan(problem, pruned.Linearize(), pruned.Goal);
    }

    [Fact]
    public void PruneToAchievedGoalDropsAtomsWhoseSubPlanIsThreatened()
    {
        Problem problem = LoadDemoteProblem();
        PartialOrderPlan plan = BuildDemoteThreat(problem);

        PartialOrderPlan pruned = plan.PruneToAchievedGoal();

        Assert.Equal([GoalLiteral(problem, "(g2)")], pruned.Goal);
        Assert.True(pruned.IsSolution());
        Assert.Equal(["(c)"], pruned.Linearize().Select(action => action.ToString()));
        AssertValidPlan(problem, pruned.Linearize(), pruned.Goal);

        // Once the threat is resolved the whole plan is achieved, including the demotion ordering.
        Flaw threat = Assert.Single(plan.ComputeFlaws().Flaws);
        plan.Apply(plan.ComputeFlaws(), [IndexOf(threat, resolver => resolver.Kind == ResolverKind.Demote)], []);
        PartialOrderPlan full = plan.PruneToAchievedGoal();
        Assert.Equal(problem.Goal, full.Goal);
        Assert.Equal(["(c)", "(a)", "(b)"], full.Linearize().Select(action => action.ToString()));
        PlanStep c = full.Steps.Single(step => step.Schema?.Name == "c");
        PlanStep a = full.Steps.Single(step => step.Schema?.Name == "a");
        Assert.True(full.IsBefore(c.Id, a.Id));
    }

    [Fact]
    public void PruneToAchievedGoalOfTheEmptyPlanIsAnEmptyGoalSolution()
    {
        Problem problem = LoadBlocks();

        PartialOrderPlan pruned = PartialOrderPlan.Empty(problem, NoTombstones).PruneToAchievedGoal();

        Assert.Empty(pruned.Goal);
        Assert.Equal(2, pruned.Steps.Count);
        Assert.Empty(pruned.Links);
        Assert.True(pruned.IsSolution());
        Assert.Empty(pruned.Linearize());
        Assert.Single(pruned.WithGoal(problem.Goal).ComputeFlaws().Flaws.Select(flaw => flaw.Kind).Distinct());
        Assert.Equal(problem.Goal.Count, pruned.WithGoal(problem.Goal).ComputeFlaws().Flaws.Count);
    }
}
