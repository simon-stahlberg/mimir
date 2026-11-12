using System.Diagnostics;
using System.Text.Json;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit.Abstractions;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.PlanSpace.Tests;

public class ReferenceOracleBlocksTests
{
    // Reference plans produced by GBFS/FF on Blocksworld; the test skips when they are absent.
    private const string ReferencesPath = "/private/tmp/claude-501/-Users-simonstahlberg-git-foundation-planning-models/ca055d87-c424-4bc2-b8a0-814476536e43/scratchpad/sizes/blocks";
    private static readonly PlanSpaceOptions NoTombstones = new();
    private readonly ITestOutputHelper _output;

    public ReferenceOracleBlocksTests(ITestOutputHelper output) => _output = output;

    private static Domain LoadBlocksworld() => Domain.FromText("""
(define (domain blocksworld)
  (:requirements :strips)
  (:predicates (clear ?x) (on-table ?x) (arm-empty) (holding ?x) (on ?x ?y))
  (:action pickup :parameters (?ob)
    :precondition (and (clear ?ob) (on-table ?ob) (arm-empty))
    :effect (and (holding ?ob) (not (clear ?ob)) (not (on-table ?ob)) (not (arm-empty))))
  (:action putdown :parameters (?ob)
    :precondition (holding ?ob)
    :effect (and (clear ?ob) (arm-empty) (on-table ?ob) (not (holding ?ob))))
  (:action stack :parameters (?ob ?underob)
    :precondition (and (clear ?underob) (holding ?ob))
    :effect (and (arm-empty) (clear ?ob) (on ?ob ?underob) (not (clear ?underob)) (not (holding ?ob))))
  (:action unstack :parameters (?ob ?underob)
    :precondition (and (on ?ob ?underob) (clear ?ob) (arm-empty))
    :effect (and (holding ?ob) (clear ?underob) (not (on ?ob ?underob)) (not (clear ?ob)) (not (arm-empty)))))
""");

    private static Action Parse(Problem problem, string text)
    {
        string[] parts = text.Split(' ', StringSplitOptions.RemoveEmptyEntries);
        ActionSchema schema = problem.Domain.Actions.Single(candidate => candidate.Name == parts[0]);
        Constant[] objects = parts.Skip(1).Select(name => problem.AllObjects.Single(constant => constant.Name == name)).ToArray();
        return problem.GroundAction(schema, objects);
    }

    private static PartialOrderPlan Reference(Problem problem, IEnumerable<string> plan)
        => PartialOrderPlan.FromSequentialPlan(problem, plan.Select(text => Parse(problem, text)).ToList(), NoTombstones);

    private static int RollOut(PartialOrderPlan current, PartialOrderPlan reference, int maxRounds)
    {
        int rounds = 0;
        while (!current.IsSolution() && rounds < maxRounds)
        {
            FlawSet flaws = current.ComputeFlaws();
            OracleTargets targets = ReferenceOracle.Targets(current, flaws, reference);
            current.Apply(flaws, targets.ResolverIndexPerFlaw, targets.StepIdsToDelete);
            rounds++;
        }

        return rounds;
    }

    private static void AssertReproduces(PartialOrderPlan reference)
    {
        PartialOrderPlan current = PartialOrderPlan.Empty(reference.Problem, NoTombstones);
        int rounds = RollOut(current, reference, 4 * reference.Steps.Count);
        Assert.True(current.IsSolution(), $"No solution after {rounds} rounds; flaws: {string.Join(", ", current.ComputeFlaws().Flaws)}");
        Assert.Equal(reference.Steps.Count, current.Steps.Count);
    }

    // The goal's (on b1 b3) and (clear b1) both come from the reference's second stack, but their ghosts
    // differ: (stack b1 b3) and (stack b1 ?). Adopting both creates two steps for one reference step; the
    // spare one is then matched to the first stack and bound to b2, which leaves a link
    // (stack b1 b2) -> Goal (clear b1) that the reference does not have. The reference's unstack b1 b2
    // threatens that link, comes after its producer and before the goal, so the oracle keeps deleting
    // and re-adopting it. The second open condition has to wait a round and link to the created step.
    [Fact]
    public void OneReferenceStepIsAdoptedByOneGhostPerRound()
    {
        Problem problem = Problem.FromText(LoadBlocksworld(), """
(define (problem stack-twice)
  (:domain blocksworld)
  (:objects b1 b2 b3)
  (:init (arm-empty) (on-table b1) (on-table b2) (on-table b3) (clear b1) (clear b2) (clear b3))
  (:goal (and (on b1 b3) (clear b1) (clear b2))))
""");
        PartialOrderPlan reference = Reference(problem, ["pickup b1", "stack b1 b2", "unstack b1 b2", "stack b1 b3"]);
        PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);

        FlawSet flaws = current.ComputeFlaws();
        OracleTargets targets = ReferenceOracle.Targets(current, flaws, reference);
        string[] adopted = flaws.Flaws
            .Select((flaw, index) => flaw.Resolvers[targets.ResolverIndexPerFlaw[index]])
            .Where(resolver => resolver.Kind == ResolverKind.AdoptGhost)
            .Select(resolver => flaws.Ghosts[resolver.GhostIndex].ToString())
            .ToArray();
        Assert.Equal(["(stack b1 b3)", "(unstack ? b2)"], adopted);

        AssertReproduces(reference);
    }

    [Fact]
    public void GreedyRolloutReproducesAReferenceWithRepeatedActions()
    {
        Problem problem = Problem.FromText(LoadBlocksworld(), """
(define (problem blocks-train-017-n8)
  (:domain blocksworld)
  (:objects b1 b2 b3 b4 b5 b6 b7 b8)
  (:init (arm-empty) (on-table b3) (on b5 b3) (clear b5) (on-table b6) (on b7 b6) (on b1 b7) (on b8 b1) (clear b8)
         (on-table b4) (on b2 b4) (clear b2))
  (:goal (and (on-table b8) (on b1 b8) (on b7 b1) (on b3 b7) (on b6 b3) (on b2 b6) (on b5 b2) (clear b5) (on-table b4) (clear b4))))
""");
        string[] plan =
        [
            "unstack b5 b3", "stack b5 b2", "unstack b8 b1", "putdown b8", "unstack b1 b7", "stack b1 b8", "unstack b7 b6", "stack b7 b1",
            "pickup b3", "stack b3 b7", "pickup b6", "stack b6 b3", "unstack b5 b2", "putdown b5", "unstack b2 b4", "stack b2 b6", "pickup b5", "stack b5 b2"
        ];
        Assert.Equal(2, plan.Count(action => action == "stack b5 b2"));

        AssertReproduces(Reference(problem, plan));
    }

    [Fact]
    public void GreedyRolloutReproducesEveryReferencePlan()
    {
        string referencesFile = Path.Combine(ReferencesPath, "references.json");
        if (!File.Exists(referencesFile)) return;

        Domain domain = Domain.FromFile(Path.Combine(ReferencesPath, "domain.pddl"));
        var references = JsonSerializer.Deserialize<Dictionary<string, List<string>>>(File.ReadAllText(referencesFile))!;
        var failures = new List<string>();
        var watch = Stopwatch.StartNew();
        foreach ((string file, List<string> plan) in references.OrderBy(pair => pair.Key))
        {
            Problem problem = Problem.FromFile(domain, Path.Combine(ReferencesPath, file));
            PartialOrderPlan reference = Reference(problem, plan);
            PartialOrderPlan current = PartialOrderPlan.Empty(problem, NoTombstones);
            int rounds = RollOut(current, reference, 4 * plan.Count);
            _output.WriteLine($"{file}: {problem.AllObjects.Count} blocks, {plan.Count} actions, {rounds} rounds, solved={current.IsSolution()}");
            if (!current.IsSolution() || current.Steps.Count != reference.Steps.Count) failures.Add(file);
        }

        _output.WriteLine($"{failures.Count}/{references.Count} failed in {watch.Elapsed.TotalSeconds:F1}s");
        Assert.Empty(failures);
    }
}
