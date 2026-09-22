using System;
using System.IO;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class GrounderTests
{
    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private static Problem CreateGeneratedProblem2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.AddEffect("ready", Polarity.Negative);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("ready");
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("blocked", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("work");
        action.AddParameter("?x", "object");
        action.AddPrecondition("blocked", Polarity.Negative, "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("blocked", "a");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("link", ("?from", "object"), ("?to", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("work");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        action = actions.Add("spread");
        action.AddParameter("?from", "object");
        action.AddParameter("?to", "object");
        action.AddPrecondition("ready", "?from");
        action.AddPrecondition("link", "?from", "?to");
        action.AddEffect("ready", "?to");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "a");
        initialState.AddFact("link", "a", "b");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "object");
        constants.Add("b", "object");
        constants.Add("c", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("unary", ("?x", "object"));
        predicates.Add("binary", ("?x", "object"), ("?y", "object"));
        predicates.Add("ternary", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done-unary", ("?x", "object"));
        predicates.Add("done-binary", ("?x", "object"), ("?y", "object"));
        predicates.Add("done-ternary", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("seed-ternary");
        action.AddEffect("ternary", "a", "b", "c");
        action.Close();
        action = actions.Add("use-unary");
        action.AddParameter("?x", "object");
        action.AddPrecondition("unary", "?x");
        action.AddEffect("done-unary", "?x");
        action.Close();
        action = actions.Add("use-binary");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("binary", "?x", "?y");
        action.AddEffect("done-binary", "?x", "?y");
        action.Close();
        action = actions.Add("use-ternary");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("ternary", "?x", "?y", "?z");
        action.AddEffect("done-ternary", "?x", "?y", "?z");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("unary", "a");
        initialState.AddFact("binary", "a", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("a", "object");
        constants.Add("b", "object");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("seed");
        action.AddEffect("ready", "b");
        action.Close();
        action = actions.Add("work");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("ready", "?x");
        action.AddPrecondition("ready", "?y");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("end");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("step1");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("mid");
        conditional0.AddEffect("end");
        conditional0.Close();
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("end");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("fixed");
        predicates.Add("phase1");
        predicates.Add("phase2");
        predicates.Add("gate");
        predicates.Add("result");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("seed");
        action.AddPrecondition("start");
        action.AddEffect("phase1");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("gate");
        conditional1.AddEffect("result");
        conditional1.Close();
        action.Close();
        action = actions.Add("advance1");
        action.AddPrecondition("phase1");
        action.AddEffect("phase2");
        action.Close();
        action = actions.Add("advance2");
        action.AddPrecondition("phase2");
        action.AddEffect("gate");
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("fixed");
        action.AddPrecondition("result");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("fixed");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("allowed");
        predicates.Add("missing");
        predicates.Add("blocked");
        predicates.Add("good");
        predicates.Add("bad-positive");
        predicates.Add("bad-negative");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("seed");
        action.AddPrecondition("start");
        ConditionalEffectBuilder conditional2 = action.AddConditionalEffect();
        conditional2.AddCondition("allowed");
        conditional2.AddEffect("good");
        conditional2.Close();
        ConditionalEffectBuilder conditional3 = action.AddConditionalEffect();
        conditional3.AddCondition("missing");
        conditional3.AddEffect("bad-positive");
        conditional3.Close();
        ConditionalEffectBuilder conditional4 = action.AddConditionalEffect();
        conditional4.AddCondition("blocked", Polarity.Negative);
        conditional4.AddEffect("bad-negative");
        conditional4.Close();
        action.Close();
        action = actions.Add("finish-good");
        action.AddPrecondition("good");
        action.AddEffect("done");
        action.Close();
        action = actions.Add("finish-positive");
        action.AddPrecondition("bad-positive");
        action.AddEffect("done");
        action.Close();
        action = actions.Add("finish-negative");
        action.AddPrecondition("bad-negative");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("allowed");
        initialState.AddFact("blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem10()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("never");
        predicates.Add("blocked");
        predicates.Add("base");
        predicates.Add("produced");
        predicates.Add("done");
        predicates.Add("enabled");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("make-blocked");
        action.AddPrecondition("never");
        action.AddEffect("blocked");
        action.Close();
        action = actions.Add("seed");
        action.AddPrecondition("start");
        ConditionalEffectBuilder conditional5 = action.AddConditionalEffect();
        conditional5.AddCondition("blocked", Polarity.Negative);
        conditional5.AddCondition("enabled");
        conditional5.AddEffect("produced");
        conditional5.Close();
        action.Close();
        action = actions.Add("finish");
        action.AddPrecondition("produced");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("enabled", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("blocked");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem11()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("end");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("step1");
        action.AddPrecondition("start");
        action.AddEffect("mid");
        ConditionalEffectBuilder conditional6 = action.AddConditionalEffect();
        conditional6.AddCondition("mid");
        conditional6.AddEffect("end");
        conditional6.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("end");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem12()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        ConditionalEffectBuilder conditional7 = action.AddConditionalEffect();
        conditional7.AddParameter("?unused", "object");
        conditional7.AddCondition("ready");
        conditional7.AddEffect("done");
        conditional7.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem13()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":adl");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("unused", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?x", "item"));
        predicates.Add("blocked", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        ConditionalEffectBuilder conditional8 = action.AddConditionalEffect();
        conditional8.AddParameter("?guarded", "item");
        conditional8.AddParameter("?ignored", "unused");
        conditional8.AddParameter("?effect", "item");
        conditional8.AddCondition("ready", "?guarded");
        conditional8.AddEffect("done", "?effect");
        conditional8.Close();
        action.Close();
        action = actions.Add("make-ready");
        action.AddParameter("?x", "item");
        action.AddPrecondition("blocked", "?x");
        action.AddEffect("ready", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Add("x", "unused");
        objects.Add("y", "unused");
        objects.Add("z", "unused");
        objects.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "a");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateGeneratedProblem14()
    {
        DomainBuilder domainBuilder = new DomainBuilder("high-arity");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("p1", ("?x", "object"));
        predicates.Add("p2", ("?x", "object"));
        predicates.Add("p3", ("?x", "object"));
        predicates.Add("p4", ("?x", "object"));
        predicates.Add("p5", ("?x", "object"));
        predicates.Add("p6", ("?x", "object"));
        predicates.Add("p7", ("?x", "object"));
        predicates.Add("p8", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("combine");
        action.AddParameter("?x1", "object");
        action.AddParameter("?x2", "object");
        action.AddParameter("?x3", "object");
        action.AddParameter("?x4", "object");
        action.AddParameter("?x5", "object");
        action.AddParameter("?x6", "object");
        action.AddParameter("?x7", "object");
        action.AddParameter("?x8", "object");
        action.AddPrecondition("p1", "?x1");
        action.AddPrecondition("p2", "?x2");
        action.AddPrecondition("p3", "?x3");
        action.AddPrecondition("p4", "?x4");
        action.AddPrecondition("p5", "?x5");
        action.AddPrecondition("p6", "?x6");
        action.AddPrecondition("p7", "?x7");
        action.AddPrecondition("p8", "?x8");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "high-arity-problem");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Add("o4", "object");
        objects.Add("o5", "object");
        objects.Add("o6", "object");
        objects.Add("o7", "object");
        objects.Add("o8", "object");
        objects.Add("o9", "object");
        objects.Add("o10", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("p1", "o1");
        initialState.AddFact("p2", "o2");
        initialState.AddFact("p3", "o3");
        initialState.AddFact("p4", "o4");
        initialState.AddFact("p5", "o5");
        initialState.AddFact("p6", "o6");
        initialState.AddFact("p7", "o7");
        initialState.AddFact("p8", "o8");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForRpgGrounder_ThrowsForNullStartState()
    {
        Domain domain = new DomainBuilder("d")
            .Requirements().Add(":strips").Close()
            .Predicates().Add("ready").Add("done").Close()
            .Actions().Add("finish").AddPrecondition("ready")
                .AddEffect("done").AddEffect("ready", Polarity.Negative).Close().Close().Build();
        return new ProblemBuilder(domain, "p")
            .Goal().Add("ready").Add("done").Close().Build();
    }

    [Fact]
    public void RpgGrounder_ThrowsForNullProblem()
    {
        ArgumentNullException exception = Assert.Throws<ArgumentNullException>(
            () => new RpgGrounder().Ground(null!, null!));

        Assert.Equal("problem", exception.ParamName);
    }

    [Fact]
    public void RpgGrounder_ThrowsForNullStartState()
    {
        Problem problem = CreateProblemForRpgGrounder_ThrowsForNullStartState();

        ArgumentNullException exception = Assert.Throws<ArgumentNullException>(
            () => new RpgGrounder().Ground(problem, null!));

        Assert.Equal("startState", exception.ParamName);
    }

    [Fact]
    public void RpgGrounder_UsesProvidedStartState()
    {
        Problem problem = CreateGeneratedProblem2();
        Mimir.Core.Schemas.Predicate<Fluent> readyPredicate = problem.Domain.Fluents
            .Single(predicate => predicate.Name == "ready");
        Fact<Fluent> readyFact = Assert.IsType<Fact<Fluent>>(
            problem.Context.FindFact(readyPredicate, Array.Empty<Constant>()));
        State customStart = StateFactory.Default.Create(problem.Context, [readyFact]);
        var grounder = new RpgGrounder();

        Assert.Empty(grounder.Ground(problem, problem.InitialState));
        Mimir.Core.Grounding.Action groundedAction = Assert.Single(
            grounder.Ground(problem, customStart));
        Mimir.Core.Grounding.Action repeatedAction = Assert.Single(
            grounder.Ground(problem, customStart));
        var generator = new GroundedApplicableActionGenerator(
            problem,
            customStart,
            grounder);

        Assert.Equal("finish", groundedAction.Schema.Name);
        Assert.Equal(groundedAction, repeatedAction);
        Assert.Same(customStart, generator.GroundingStartState);
        Assert.Single(generator.GetApplicableActions(customStart.Expand()));
    }

    [Fact]
    public void BlocksWorldRpgGrounding()
    {
        var domainPath = Path.Combine(BasePath, "blocks_4", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "blocks_4", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var grounder = new RpgGrounder();

        var actions = grounder.Ground(problem, problem.InitialState).ToList();

        // 3 blocks: b1, b2, b3
        // pickup: 3 (b1, b2, b3)
        // putdown: 3
        // stack: 3*2 = 6 (cannot stack block on itself, wait, does Pddl allow stacking on itself?
        // usually the precondition (clear ?y) and (holding ?x) ensures ?x != ?y if ?x is not clear.
        // Actually, without an explicit (?x != ?y) check, we might generate 3*3 = 9.
        // However, in RPG, if (on b1 b1) is never achievable, it won't be reachable.

        // 3 blocks: pickup=3, putdown=3, stack=9 (incl. self-stack, never achievable but reachable in RPG), unstack=9
        Assert.Equal(24, actions.Count);
        Assert.Equal(3, actions.Count(a => a.Schema.Name == "pickup"));
        Assert.Equal(3, actions.Count(a => a.Schema.Name == "putdown"));
        Assert.Equal(9, actions.Count(a => a.Schema.Name == "stack"));
        Assert.Equal(9, actions.Count(a => a.Schema.Name == "unstack"));
    }

    [Fact]
    public void MiconicRpgGrounding()
    {
        var domainPath = Path.Combine(BasePath, "miconic", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "miconic", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var grounder = new RpgGrounder();

        var actions = grounder.Ground(problem, problem.InitialState).ToList();

        // Miconic p01: 2 passengers (p0, p1), 2 floors (f0, f1)
        // board: (?f - floor, ?p - passenger) = 2*2 = 4 candidates
        // depart: (?f - floor, ?p - passenger) = 2*2 = 4 candidates
        // up: (?f1, ?f2 - floor) = 2*2 = 4 candidates
        // down: (?f1, ?f2 - floor) = 2*2 = 4 candidates

        // Static pruning: (above f0 f1) only, so up=1 and down=1. board/depart=2 each (origin/destin both on f0â†’f1).
        Assert.Equal(6, actions.Count);
        Assert.Equal(2, actions.Count(a => a.Schema.Name == "board"));
        Assert.Equal(2, actions.Count(a => a.Schema.Name == "depart"));
        Assert.Equal(1, actions.Count(a => a.Schema.Name == "up"));
        Assert.Equal(1, actions.Count(a => a.Schema.Name == "down"));
        // (above f0 f1) is true; (above f1 f0) is false.
        Assert.Contains(actions, a => a.Schema.Name == "up" && a.Arguments[0].Name == "f0" && a.Arguments[1].Name == "f1");
        Assert.DoesNotContain(actions, a => a.Schema.Name == "up" && a.Arguments[0].Name == "f1" && a.Arguments[1].Name == "f0");
    }

    [Fact]
    public void RpgGrounder_FiltersActionsWithNegativeStaticPrecondition()
    {
        var problem = CreateGeneratedProblem3();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        Assert.Single(actions);
        Assert.Equal("b", actions[0].Arguments[0].Name);
        Assert.DoesNotContain(actions, a => a.Arguments[0].Name == "a");
    }

    [Fact]
    public void RpgGrounder_DoesNotDuplicateBindingsWhenAnActionIsReevaluated()
    {
        Problem problem = CreateGeneratedProblem4();

        Mimir.Core.Grounding.Action[] workActions = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Where(action => action.Schema.Name == "work")
            .ToArray();

        string[] bindings = workActions.Select(action => action.Arguments[0].Name).ToArray();

        Assert.Equal(new[] { "a", "b" }.Order(), bindings.Order());
        Assert.Equal(bindings.Length, bindings.Distinct().Count());
    }

    [Fact]
    public void RpgGrounder_SemiNaiveDriversCoverUnaryBinaryAndHigherArity()
    {
        Problem problem = CreateGeneratedProblem5();

        string[] actions = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Select(ActionSignature)
            .ToArray();

        Assert.Equal(
            new[]
            {
                "seed-ternary()",
                "use-unary(a)",
                "use-binary(a,b)",
                "use-ternary(a,b,c)",
            }.Order(),
            actions.Order());
        Assert.Equal(actions.Length, actions.Distinct().Count());
    }

    [Fact]
    public void RpgGrounder_RepeatedDeltaPredicateEmitsEachBindingOnce()
    {
        Problem problem = CreateGeneratedProblem6();

        string[] actions = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Select(ActionSignature)
            .ToArray();

        Assert.Equal(
            new[]
            {
                "seed()",
                "work(a,a)",
                "work(b,a)",
                "work(b,b)",
                "work(a,b)",
            }.Order(),
            actions.Order());
        Assert.Equal(actions.Length, actions.Distinct().Count());
    }

    [Fact]
    public void RpgGrounder_DiscoversFacts_ViaConditionalEffectFixpoint()
    {
        var problem = CreateGeneratedProblem7();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        var action = actions.Single(candidate => candidate.Schema.Name == "step1");
        var ce = Assert.Single(action.ConditionalEffects);
        Assert.Equal("end", ce.LiteralEffect!.Value.Predicate.Name, ignoreCase: true);
        Assert.True(ce.LiteralEffect!.IsPositive);
        Assert.Contains(actions, candidate => candidate.Schema.Name == "finish");
    }

    [Fact]
    public void RpgGrounder_RetainsConditionalEffectUntilGuardBecomesReachable()
    {
        Problem problem = CreateGeneratedProblem8();

        string[] actions = new RpgGrounder()
            .Ground(problem, problem.InitialState)
            .Select(ActionSignature)
            .ToArray();

        Assert.Equal(
            new[] { "seed()", "advance1()", "advance2()", "finish()" }.Order(),
            actions.Order());
        Assert.Equal(actions.Length, actions.Distinct().Count());
    }

    [Fact]
    public void RpgGrounder_RequiresStaticConditionalEffectGuards()
    {
        var problem = CreateGeneratedProblem9();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        Assert.Equal(2, actions.Count);
        Assert.Contains(actions, action => action.Schema.Name == "seed");
        Assert.Contains(actions, action => action.Schema.Name == "finish-good");
        Assert.DoesNotContain(actions, action => action.Schema.Name == "finish-positive");
        Assert.DoesNotContain(actions, action => action.Schema.Name == "finish-negative");
    }

    [Fact]
    public void RpgGrounder_OverapproximatesNegativeFluentAndDerivedConditionalEffectGuards()
    {
        var problem = CreateGeneratedProblem10();

        var actions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();

        Assert.Contains(actions, action => action.Schema.Name == "seed");
        Assert.Contains(actions, action => action.Schema.Name == "finish");
        Assert.DoesNotContain(actions, action => action.Schema.Name == "make-blocked");
    }

    [Fact]
    public void RpgGrounder_ConditionalEffectsReachGoal_WhenFixpointConverges()
    {
        var problem = CreateGeneratedProblem11();

        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());
        ExtendedState initialState = problem.InitialState.Expand();
        var action = generator.GetApplicableActions(initialState).Single();

        // Conditional effects use pre-state semantics: (when mid â†’ end) requires mid in pre-state.
        // First application: start â†’ {start, mid}
        // Second application (start still holds): {start, mid} â†’ {start, mid, end}
        var stateAfterFirst = initialState.Apply(action);
        var stateAfterSecond = stateAfterFirst.Expand().Apply(action);

        Assert.Contains(stateAfterSecond.GetTrueFacts(), f => f.Predicate.Name.Equals("end", StringComparison.OrdinalIgnoreCase));
    }

    [Fact]
    public void RpgGrounder_QuantifiedEffect_GroundsOnceWhenVariableIsUnused()
    {
        Problem problem = CreateGeneratedProblem12();

        Mimir.Core.Grounding.Action action = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState));

        Assert.Single(action.ConditionalEffects);
    }

    [Fact]
    public void RpgGrounder_QuantifiedEffect_EnumeratesOnlyReferencedVariables()
    {
        Problem problem = CreateGeneratedProblem13();

        Mimir.Core.Grounding.Action action = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState));
        Mimir.Core.Schemas.Predicate<Fluent> ready = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "ready");
        Mimir.Core.Schemas.Predicate<Fluent> done = problem.Domain.Fluents.Single(
            predicate => predicate.Name == "done");
        Constant a = problem.ObjectLookup["a"];
        Constant b = problem.ObjectLookup["b"];
        Fact<Fluent> readyA = problem.Context.RegisterFact(ready, [a]);
        Fact<Fluent> readyB = problem.Context.RegisterFact(ready, [b]);
        Fact<Fluent> doneA = problem.Context.RegisterFact(done, [a]);
        Fact<Fluent> doneB = problem.Context.RegisterFact(done, [b]);

        Assert.Equal(4, action.ConditionalEffects.Count);

        State withoutReady = StateFactory.Default.CreateEmpty(problem.Context);
        State withReadyA = StateFactory.Default.Create(problem.Context, [readyA]);
        State withReadyB = StateFactory.Default.Create(problem.Context, [readyB]);

        Assert.False(withoutReady.Expand().Apply(action).IsTrue(doneA));
        Assert.False(withoutReady.Expand().Apply(action).IsTrue(doneB));
        Assert.True(withReadyA.Expand().Apply(action).IsTrue(doneA));
        Assert.True(withReadyA.Expand().Apply(action).IsTrue(doneB));
        Assert.True(withReadyB.Expand().Apply(action).IsTrue(doneA));
        Assert.True(withReadyB.Expand().Apply(action).IsTrue(doneB));
    }

    [Theory]
    [InlineData(true)]
    [InlineData(false)]
    public void RpgGrounder_QuantifiedEffect_ProducesNoEffectWhenAnyUnusedTypeIsEmpty(
        bool nonemptyFirst)
    {
        DomainBuilder builder = new DomainBuilder("d")
            .Requirements().Add(":adl").Add(":typing").Close()
            .Types().Add("item").Add("empty").Close()
            .Predicates().Add("ready", ("?x", "item")).Add("done", ("?x", "item")).Close();
        ActionSchemaBuilder actionBuilder = builder.Actions().Add("act");
        ConditionalEffectBuilder effect = actionBuilder.AddConditionalEffect()
            .AddParameter("?used", "item");
        if (nonemptyFirst)
            effect.AddParameter("?nonempty", "item").AddParameter("?missing", "empty");
        else
            effect.AddParameter("?missing", "empty").AddParameter("?nonempty", "item");
        effect.AddCondition("ready", "?used").AddEffect("done", "?used").Close();
        Domain domain = actionBuilder.Close().Close().Build();
        Problem problem = new ProblemBuilder(domain, "p")
            .Objects().Add("a", "item").Add("b", "item").Close()
            .Goal().Add("done", "a").Close().Build();

        Mimir.Core.Grounding.Action action = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState));

        Assert.Empty(action.ConditionalEffects);
    }

    [Fact]
    public void TransportRpgGrounding_PrunesBindingsUsingStaticRoadPreconditions()
    {
        var domainPath = Path.Combine(BasePath, "transport", "domain.pddl");
        var problemPath = Path.Combine(BasePath, "transport", "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var grounder = new RpgGrounder();

        var actions = grounder.Ground(problem, problem.InitialState).Where(action => action.Schema.Name == "drive").ToList();

        Assert.Equal(8, actions.Count);
        Assert.Contains(actions, action => action.Arguments[1].Name == "city-loc-3" && action.Arguments[2].Name == "city-loc-1");
        Assert.DoesNotContain(actions, action => action.Arguments[1].Name == "city-loc-1" && action.Arguments[2].Name == "city-loc-2");
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void ConstrainedHighArityAction_AvoidsEnumeratingTheCompleteObjectProduct(bool useClique)
    {
        var problem = CreateGeneratedProblem14();

        IApplicableActionGenerator generator = useClique
            ? new CliqueApplicableActionGenerator(problem)
            : new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());

        Mimir.Core.Grounding.Action action = Assert.Single(
            generator.GetApplicableActions(problem.InitialState.Expand()));

        Assert.Equal(
            new[] { "o1", "o2", "o3", "o4", "o5", "o6", "o7", "o8" },
            action.Arguments.Select(argument => argument.Name));
    }

    private static string ActionSignature(Mimir.Core.Grounding.Action action)
        => $"{action.Schema.Name}({string.Join(",", action.Arguments.Select(argument => argument.Name))})";
}
