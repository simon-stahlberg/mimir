using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public sealed class ActionCostTests : IDisposable
{
  private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    private readonly List<string> _tempFiles = new();

  public static TheoryData<string, string[]> SupportedBenchmarkInitialApplicableActionCosts => new()
  {
    {
      "barman",
      new[]
      {
        "grasp(left, shaker1)=1",
        "grasp(left, shot1)=1",
        "grasp(right, shaker1)=1",
        "grasp(right, shot1)=1",
      }
    },
    {
      "transport",
      new[]
      {
        "drive(truck-1, city-loc-3, city-loc-1)=22",
        "drive(truck-1, city-loc-3, city-loc-2)=50",
        "drive(truck-2, city-loc-1, city-loc-3)=22",
        "pick-up(truck-1, city-loc-3, package-1, capacity-3, capacity-4)=1",
        "pick-up(truck-1, city-loc-3, package-2, capacity-3, capacity-4)=1",
      }
    },
    {
      "woodworking",
      new[]
      {
        "do-saw-small(b0, p0, mahogany, smooth, s1, s0)=30",
        "load-highspeed-saw(b0, highspeed-saw0)=30",
      }
    }
  };

    private string WriteTempFile(string content)
    {
        var path = Path.GetTempFileName();
        File.WriteAllText(path, content);
        _tempFiles.Add(path);
        return path;
    }

    private static Problem CreateProblemForStaticFunctionValue_UsesCurrentArgumentValues()
    {
        DomainBuilder domainBuilder = new DomainBuilder("numeric-lookup");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("price", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddEffect("done");
        action.WithCost(Numeric.Function("price", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.SetValue(Numeric.Function("price", "a"), 2d);
        initialState.SetValue(Numeric.Function("price", "b"), 5d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticFunctionValue_HigherArityKeyUsesArgumentIdentityAndOrder()
    {
        DomainBuilder domainBuilder = new DomainBuilder("higher-arity-lookup");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("route-cost", ("?a", "item"), ("?b", "item"), ("?c", "item"), ("?d", "item"));
        functions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Add("c", "item");
        objects.Add("d", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.SetValue(Numeric.Function("route-cost", "a", "b", "c", "d"), 9d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_DefaultToUnitCost_WhenNoExplicitActionCostsExist()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unit-cost");
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
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_EvaluateStaticProblemDependentCostExpressions_WhenGrounded()
    {
        DomainBuilder domainBuilder = new DomainBuilder("problem-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("at", ("?x", "location"));
        predicates.Add("visited", ("?x", "location"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("distance", ("?from", "location"), ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?from", "location");
        action.AddParameter("?to", "location");
        action.AddPrecondition("at", "?from");
        action.AddEffect("at", Polarity.Negative, "?from");
        action.AddEffect("at", "?to");
        action.AddEffect("visited", "?to");
        action.WithCost(Numeric.Constant(1d) + Numeric.Function("distance", "?from", "?to"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "location");
        objects.Add("b", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("at", "a");
        initialState.SetValue(Numeric.Function("distance", "a", "a"), 0d);
        initialState.SetValue(Numeric.Function("distance", "a", "b"), 2.5d);
        initialState.SetValue(Numeric.Function("distance", "b", "a"), 4d);
        initialState.SetValue(Numeric.Function("distance", "b", "b"), 0d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_SumMultipleTotalCostIncreases_WhenGrounded()
    {
        DomainBuilder domainBuilder = new DomainBuilder("split-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("depot", "location");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("at", ("?x", "location"));
        predicates.Add("visited", ("?x", "location"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("entry-fee", ("?x", "location"));
        functions.Add("distance", ("?from", "location"), ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?to", "location");
        action.AddPrecondition("at", "depot");
        action.AddEffect("at", Polarity.Negative, "depot");
        action.AddEffect("at", "?to");
        action.AddEffect("visited", "?to");
        action.WithCost((Numeric.Function("entry-fee", "depot") + (Numeric.Constant(2d) * Numeric.Constant(1.25d))) + (Numeric.Function("distance", "depot", "?to") / Numeric.Constant(2d)));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("site", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("at", "depot");
        initialState.SetValue(Numeric.Function("entry-fee", "depot"), 4d);
        initialState.SetValue(Numeric.Function("entry-fee", "site"), 0d);
        initialState.SetValue(Numeric.Function("distance", "depot", "depot"), 0d);
        initialState.SetValue(Numeric.Function("distance", "depot", "site"), 7d);
        initialState.SetValue(Numeric.Function("distance", "site", "depot"), 7d);
        initialState.SetValue(Numeric.Function("distance", "site", "site"), 0d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "site");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_EvaluateCostExpressionsWithDomainConstantsAndCaseInsensitiveNames()
    {
        DomainBuilder domainBuilder = new DomainBuilder("constant-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("vehicle", "object");
        types.Add("location", "object");
        types.Add("package", "object");
        types.Close();
        ConstantListBuilder constants = domainBuilder.Constants();
        constants.Add("HOME", "location");
        constants.Add("PKG1", "package");
        constants.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready", ("?truck", "vehicle"));
        predicates.Add("done", ("?truck", "vehicle"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("rate", ("?truck", "vehicle"), ("?where", "location"));
        functions.Add("fee", ("?pkg", "package"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("work");
        action.AddParameter("?truck", "vehicle");
        action.AddPrecondition("ready", "?truck");
        action.AddEffect("done", "?truck");
        action.WithCost((Numeric.Constant(2d) * Numeric.Function("RaTe", "?truck", "home")) + (Numeric.Function("FEE", "pkg1") / Numeric.Constant(4d)));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("van1", "vehicle");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready", "van1");
        initialState.SetValue(Numeric.Function("rate", "van1", "home"), 3.25d);
        initialState.SetValue(Numeric.Function("fee", "pkg1"), 9d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "van1");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_EvaluateStaticProblemDependentCostExpressions_ForAllApplicableBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("binding-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("visited", ("?to", "location"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("distance", ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("visit");
        action.AddParameter("?to", "location");
        action.AddPrecondition("ready");
        action.AddEffect("visited", "?to");
        action.WithCost(Numeric.Function("distance", "?to"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "location");
        objects.Add("b", "location");
        objects.Add("c", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.SetValue(Numeric.Function("distance", "a"), 2d);
        initialState.SetValue(Numeric.Function("distance", "b"), 5.5d);
        initialState.SetValue(Numeric.Function("distance", "c"), 8d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "a");
        goals.Add("visited", "b");
        goals.Add("visited", "c");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_WithUndefinedCostFunctionValueAreInapplicable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("incomplete-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("at", ("?x", "location"));
        predicates.Add("visited", ("?x", "location"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("distance", ("?from", "location"), ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?from", "location");
        action.AddParameter("?to", "location");
        action.AddPrecondition("at", "?from");
        action.AddEffect("at", Polarity.Negative, "?from");
        action.AddEffect("at", "?to");
        action.AddEffect("visited", "?to");
        action.WithCost(Numeric.Function("distance", "?from", "?to"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "location");
        objects.Add("b", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("at", "a");
        initialState.SetValue(Numeric.Function("distance", "a", "b"), 2d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForStaticallyInfeasibleBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("static-pruned-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("location", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("allowed", ("?to", "location"));
        predicates.Add("visited", ("?to", "location"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("distance", ("?to", "location"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("visit");
        action.AddParameter("?to", "location");
        action.AddPrecondition("ready");
        action.AddPrecondition("allowed", "?to");
        action.AddEffect("visited", "?to");
        action.WithCost(Numeric.Function("distance", "?to"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "location");
        objects.Add("b", "location");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.AddFact("allowed", "b");
        initialState.SetValue(Numeric.Function("distance", "b"), 7d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("visited", "b");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForRelaxedUnreachableBindings()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unreachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled", "good");
        initialState.SetValue(Numeric.Function("item-cost", "good"), 4d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "good");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_DoNotThrowWhenMissingCostBindingIsBlockedByConditionalEffectStaticGuard()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-unreachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("source", ("?x", "item"));
        predicates.Add("allowed", ("?x", "item"));
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("enable");
        action.AddParameter("?x", "item");
        action.AddPrecondition("source", "?x");
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("allowed", "?x");
        conditional0.AddEffect("enabled", "?x");
        conditional0.Close();
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("source", "good");
        initialState.AddFact("source", "bad");
        initialState.AddFact("allowed", "good");
        initialState.SetValue(Numeric.Function("item-cost", "good"), 4d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "good");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_WithUndefinedCostAreDroppedWhenReachedByConditionalEffect()
    {
        DomainBuilder domainBuilder = new DomainBuilder("conditional-reachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":conditional-effects");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("source", ("?x", "item"));
        predicates.Add("allowed", ("?x", "item"));
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("enable");
        action.AddParameter("?x", "item");
        action.AddPrecondition("source", "?x");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("allowed", "?x");
        conditional1.AddEffect("enabled", "?x");
        conditional1.Close();
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("source", "good");
        initialState.AddFact("source", "bad");
        initialState.AddFact("allowed", "good");
        initialState.AddFact("allowed", "bad");
        initialState.SetValue(Numeric.Function("item-cost", "good"), 4d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "bad");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_WithUndefinedCostAreDroppedWhenRelaxedReachable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("later-reachable-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("enabled", ("?x", "item"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("item-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("enable");
        action.AddParameter("?x", "item");
        action.AddPrecondition("start");
        action.AddEffect("enabled", "?x");
        action.Close();
        action = actions.Add("finish");
        action.AddParameter("?x", "item");
        action.AddPrecondition("enabled", "?x");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("item-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("good", "item");
        objects.Add("bad", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("start");
        initialState.AddFact("enabled", "good");
        initialState.SetValue(Numeric.Function("item-cost", "good"), 4d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done", "bad");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_ThrowWhenGroundedCostIsNegative()
    {
        DomainBuilder domainBuilder = new DomainBuilder("negative-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(0d) - Numeric.Constant(1d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_EvaluateUnaryNegationInCostExpressions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("unary-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(0d) - Numeric.Constant(-5d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_WithOverflowingCostAreInapplicable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("non-finite-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * (Numeric.Constant(1e+28d) * Numeric.Constant(1e+28d))))))))))));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActions_WithCostDivisionByZeroAreInapplicable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("divide-by-zero-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("ready");
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(1d) / Numeric.Constant(0d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStateApply_DoesNotEncodeActionCostInSuccessorState()
    {
        DomainBuilder domainBuilder = new DomainBuilder("apply-cost");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("cheap");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(2d));
        action.Close();
        action = actions.Add("expensive");
        action.AddPrecondition("start");
        action.AddEffect("start", Polarity.Negative);
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(9d));
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

    private Problem CreateProblem(string domainPddl, string problemPddl)
    {
        var domainPath = WriteTempFile(domainPddl);
        var problemPath = WriteTempFile(problemPddl);
        var domain = Domain.FromFile(domainPath);
        return Problem.FromFile(domain, problemPath);
    }

    public void Dispose()
    {
        foreach (var file in _tempFiles)
        {
            if (File.Exists(file))
                File.Delete(file);
        }
    }

    [Fact]
    public void StaticFunctionValue_UsesCurrentArgumentValues()
    {
        Problem problem = CreateProblemForStaticFunctionValue_UsesCurrentArgumentValues();
        NumericFunction price = Assert.Single(problem.Domain.Functions);
        var arguments = new List<Constant> { problem.ObjectLookup["a"] };

        Assert.Equal(2d, problem.InitialState.Value(problem.FunctionCall(price, arguments)));
        arguments[0] = problem.ObjectLookup["b"];
        Assert.Equal(5d, problem.InitialState.Value(problem.FunctionCall(price, arguments)));
    }

    [Fact]
    public void StaticFunctionValue_HigherArityKeyUsesArgumentIdentityAndOrder()
    {
        Problem problem = CreateProblemForStaticFunctionValue_HigherArityKeyUsesArgumentIdentityAndOrder();
        NumericFunction routeCost = Assert.Single(problem.Domain.Functions);
        Constant[] ordered = [
            problem.ObjectLookup["a"],
            problem.ObjectLookup["b"],
            problem.ObjectLookup["c"],
            problem.ObjectLookup["d"]
        ];

        Assert.Equal(9d, problem.InitialState.Value(problem.FunctionCall(routeCost, ordered)));

        Constant[] reordered = [ordered[0], ordered[1], ordered[3], ordered[2]];
        Assert.True(double.IsNaN(problem.InitialState.Value(problem.FunctionCall(routeCost, reordered))));
    }

      [Theory]
      [MemberData(nameof(SupportedBenchmarkInitialApplicableActionCosts))]
      public void SupportedBenchmarks_HaveExpectedCostsForAllInitiallyApplicableActions(string domainDir, string[] expectedActions)
      {
        var domainPath = Path.Combine(BasePath, domainDir, "domain.pddl");
        var problemPath = Path.Combine(BasePath, domainDir, "p01.pddl");

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);
        var generator = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());

        var actualActions = generator.GetApplicableActions(problem.InitialState.Expand())
          .Select(action => $"{action.Schema.Name}({string.Join(", ", action.Arguments.Select(argument => argument.Name))})={action.Cost:G17}")
          .OrderBy(text => text)
          .ToArray();

        Assert.Equal(expectedActions.OrderBy(text => text), actualActions);
      }

    [Fact]
    public void Actions_DefaultToUnitCost_WhenNoExplicitActionCostsExist()
    {
        var problem = CreateProblemForActions_DefaultToUnitCost_WhenNoExplicitActionCostsExist();

        var action = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .Single();

        Assert.Equal(1d, action.Cost);
    }

    [Fact]
    public void Actions_WithoutExplicitCost_DefaultToZero_WhenAnyActionCostExists()
    {
        var problem = CreateProblem(
            """
(define (domain mixed-costs)
  (:requirements :strips :action-costs)
  (:predicates (ready-a) (ready-b) (done-a) (done-b))

  (:action paid
    :parameters ()
    :precondition (ready-a)
    :effect (and (done-a) (increase (total-cost) 5)))

  (:action free
    :parameters ()
    :precondition (ready-b)
    :effect (done-b)))
""",
            """
(define (problem p)
  (:domain mixed-costs)
  (:init (ready-a) (ready-b) (= (total-cost) 0))
  (:goal (and (done-a) (done-b)))
  (:metric minimize (total-cost)))
""");

        var actions = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .ToList();

        Assert.Equal(2, actions.Count);
        Assert.Equal(0d, actions.Single(a => a.Schema.Name == "free").Cost);
        Assert.Equal(5d, actions.Single(a => a.Schema.Name == "paid").Cost);
    }

    [Fact]
    public void Actions_DefaultToZero_WhenActionCostsAreDeclaredWithoutAnyIncrease()
    {
        var problem = CreateProblem(
            """
(define (domain declared-costs)
  (:requirements :strips :action-costs)
  (:predicates (ready) (done))

  (:action finish
    :parameters ()
    :precondition (ready)
    :effect (done)))
""",
            """
(define (problem p)
  (:domain declared-costs)
  (:init (ready) (= (total-cost) 0))
  (:goal (done))
  (:metric minimize (total-cost)))
""");

        GroundAction action = Assert.Single(
            new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
                .GetApplicableActions(problem.InitialState.Expand()));

        Assert.Equal(0d, action.Cost);
    }

    [Fact]
    public void BuilderActionsWithoutExplicitCostDefaultToZeroUnderActionCostsLikePddl()
    {
        var builder = new DomainBuilder("declared-costs");
        builder.Requirements().Add(":strips").Add(":action-costs").Close();
        builder.Predicates().Add("ready").Add("done").Close();
        ActionListBuilder actions = builder.Actions();
        ActionSchemaBuilder finish = actions.Add("finish");
        finish.AddPrecondition("ready");
        finish.AddEffect("done");
        finish.Close();
        actions.Close();
        Domain domain = builder.Build();
        var problemBuilder = new ProblemBuilder(domain, "p");
        problemBuilder.InitialState().AddFact("ready").Close();
        Problem problem = problemBuilder.Build();

        Assert.Equal(0d, problem.GroundAction(domain.Actions[0]).Cost);
    }

    [Fact]
    public void Problem_RejectsUnsupportedMetric()
    {
        string domainPath = WriteTempFile("""
(define (domain unsupported-metric)
  (:requirements :strips :action-costs)
  (:predicates (done))
  (:functions (fuel)))
""");
        string problemPath = WriteTempFile("""
(define (problem p)
  (:domain unsupported-metric)
  (:init (= (fuel) 0) (= (total-cost) 0))
  (:goal (done))
  (:metric minimize (fuel)))
""");
        Domain domain = Domain.FromFile(domainPath);

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Problem.FromFile(domain, problemPath));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("minimize (total-cost)", exception.Message);
    }

    [Fact]
    public void Problem_RejectsDuplicateTotalCostInitialization()
    {
        string domainPath = WriteTempFile("""
(define (domain duplicate-cost)
  (:requirements :strips :action-costs)
  (:predicates (done)))
""");
        string problemPath = WriteTempFile("""
(define (problem p)
  (:domain duplicate-cost)
  (:init (= (total-cost) 0) (= (total-cost) 0))
  (:goal (done)))
""");
        Domain domain = Domain.FromFile(domainPath);

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Problem.FromFile(domain, problemPath));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("only be initialized once", exception.Message);
    }

    [Fact]
    public void Actions_EvaluateStaticProblemDependentCostExpressions_WhenGrounded()
    {
        var problem = CreateProblemForActions_EvaluateStaticProblemDependentCostExpressions_WhenGrounded();

        var action = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .Single(candidate => candidate.Arguments.Select(argument => argument.Name).SequenceEqual(new[] { "a", "b" }));

        Assert.Equal(3.5, action.Cost);
    }

    [Fact]
    public void Actions_SumMultipleTotalCostIncreases_WhenGrounded()
    {
        var problem = CreateProblemForActions_SumMultipleTotalCostIncreases_WhenGrounded();

        var action = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .Single(candidate => candidate.Schema.Name == "move" && candidate.Arguments[0].Name == "site");

        Assert.Equal(10d, action.Cost);
    }

    [Fact]
    public void Actions_EvaluateCostExpressionsWithDomainConstantsAndCaseInsensitiveNames()
    {
        var problem = CreateProblemForActions_EvaluateCostExpressionsWithDomainConstantsAndCaseInsensitiveNames();

        var action = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .Single();

        Assert.Equal(8.75, action.Cost);
    }

    [Fact]
    public void Actions_EvaluateStaticProblemDependentCostExpressions_ForAllApplicableBindings()
    {
        var problem = CreateProblemForActions_EvaluateStaticProblemDependentCostExpressions_ForAllApplicableBindings();

        var actions = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .OrderBy(action => action.Arguments[0].Name)
            .ToList();

        Assert.Equal(3, actions.Count);
        Assert.Equal(new[] { "a", "b", "c" }, actions.Select(action => action.Arguments[0].Name));
        Assert.Equal(new[] { 2d, 5.5d, 8d }, actions.Select(action => action.Cost));
    }

    [Fact]
    public void Actions_WithUndefinedCostFunctionValueAreInapplicable()
    {
        var problem = CreateProblemForActions_WithUndefinedCostFunctionValueAreInapplicable();

        foreach (IApplicableActionGenerator generator in Generators(problem))
        {
            GroundAction action = Assert.Single(generator.GetApplicableActions(problem.InitialState.Expand()));
            Assert.Equal("b", action.Arguments[1].Name);
            Assert.Equal(2d, action.Cost);
        }
        GroundAction undefined = problem.GroundAction(problem.Domain.Actions[0], problem.ObjectLookup["a"], problem.ObjectLookup["a"]);
        Assert.True(double.IsNaN(undefined.Cost));
        Assert.False(undefined.IsApplicable(problem.InitialState.Expand()));
    }

    [Fact]
    public void Actions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForStaticallyInfeasibleBindings()
    {
        var problem = CreateProblemForActions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForStaticallyInfeasibleBindings();

        var actions = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .ToList();

        var action = Assert.Single(actions);
        Assert.Equal("b", action.Arguments[0].Name);
        Assert.Equal(7d, action.Cost);
    }

    [Fact]
    public void Actions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForRelaxedUnreachableBindings()
    {
        var problem = CreateProblemForActions_DoNotThrowWhenMissingCostFunctionValueOnlyOccursForRelaxedUnreachableBindings();

        var groundedActions = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .ToList();
        var cliqueActions = new CliqueApplicableActionGenerator(problem)
            .GetApplicableActions(problem.InitialState.Expand())
            .ToList();

        GroundAction groundedAction = Assert.Single(groundedActions);
        GroundAction cliqueAction = Assert.Single(cliqueActions);
        Assert.Equal("good", groundedAction.Arguments[0].Name);
        Assert.Equal(4d, groundedAction.Cost);
        Assert.Equal(groundedAction, cliqueAction);
    }

    [Fact]
    public void Actions_DoNotThrowWhenMissingCostBindingIsBlockedByConditionalEffectStaticGuard()
    {
        var problem = CreateProblemForActions_DoNotThrowWhenMissingCostBindingIsBlockedByConditionalEffectStaticGuard();

        List<GroundAction> finishActions = new RpgGrounder().Ground(problem, problem.InitialState)
            .Where(action => action.Schema.Name == "finish")
            .ToList();

        GroundAction finish = Assert.Single(finishActions);
        Assert.Equal("good", finish.Arguments[0].Name);
        Assert.Equal(4d, finish.Cost);
    }

    [Fact]
    public void Actions_WithUndefinedCostAreDroppedWhenReachedByConditionalEffect()
    {
        var problem = CreateProblemForActions_WithUndefinedCostAreDroppedWhenReachedByConditionalEffect();

        GroundAction finish = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState),
            action => action.Schema.Name == "finish");
        Assert.Equal("good", finish.Arguments[0].Name);
    }

    [Fact]
    public void Actions_WithUndefinedCostAreDroppedWhenRelaxedReachable()
    {
        var problem = CreateProblemForActions_WithUndefinedCostAreDroppedWhenRelaxedReachable();

        GroundAction finish = Assert.Single(new RpgGrounder().Ground(problem, problem.InitialState),
            action => action.Schema.Name == "finish");
        Assert.Equal("good", finish.Arguments[0].Name);
    }

    [Fact]
    public void Actions_ThrowWhenGroundedCostIsNegative()
    {
        var problem = CreateProblemForActions_ThrowWhenGroundedCostIsNegative();

        var exception = Assert.Throws<InvalidOperationException>(() =>
            new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder()));

        Assert.Contains("negative cost", exception.Message, StringComparison.OrdinalIgnoreCase);
    }

    [Fact]
    public void Actions_EvaluateUnaryNegationInCostExpressions()
    {
        var problem = CreateProblemForActions_EvaluateUnaryNegationInCostExpressions();

        var action = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .Single();

        Assert.Equal(5d, action.Cost);
    }

    [Fact]
    public void Actions_WithOverflowingCostAreInapplicable()
    {
        var problem = CreateProblemForActions_WithOverflowingCostAreInapplicable();

        foreach (IApplicableActionGenerator generator in Generators(problem))
            Assert.Empty(generator.GetApplicableActions(problem.InitialState.Expand()));
    }

    [Fact]
    public void Actions_WithCostDivisionByZeroAreInapplicable()
    {
        var problem = CreateProblemForActions_WithCostDivisionByZeroAreInapplicable();

        foreach (IApplicableActionGenerator generator in Generators(problem))
            Assert.Empty(generator.GetApplicableActions(problem.InitialState.Expand()));
    }

    private static IApplicableActionGenerator[] Generators(Problem problem)
        => [new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder()), new CliqueApplicableActionGenerator(problem)];

    [Fact]
    public void Domain_AllowsOrdinaryNumericIncrease()
    {
        var domainPddl = """
(define (domain mutable-cost)
  (:requirements :strips :action-costs)
  (:predicates (done))
  (:functions (total-cost) (energy) (wage))

  (:action refuel
    :parameters ()
    :precondition ()
    :effect (increase (energy) 1))

  (:action work
    :parameters ()
    :precondition ()
    :effect (and
      (done)
      (increase (total-cost) (wage)))))
""";
        Domain domain = Domain.FromFile(WriteTempFile(domainPddl));
        Assert.NotEmpty(domain.Actions.SelectMany(action => action.NumericEffects));
    }

    [Fact]
    public void Domain_AllowsOrdinaryNumericDecreaseBeforeTranslatingActionCost()
    {
        var domainPddl = """
(define (domain same-action-mutable-cost)
  (:requirements :strips :action-costs)
  (:predicates (done))
  (:functions (total-cost) (fuel) (toll))

  (:action drive
    :parameters ()
    :precondition ()
    :effect (and
      (done)
      (decrease (fuel) 1)
      (increase (total-cost) (toll)))))
""";

        Domain domain = Domain.FromFile(WriteTempFile(domainPddl));
        Assert.NotEmpty(domain.Actions.SelectMany(action => action.NumericEffects));
    }

    [Fact]
    public void Domain_RejectsQuantifiedActionCosts()
    {
        var domainPddl = """
(define (domain quantified-cost)
  (:requirements :strips :typing :action-costs)
  (:types location)
  (:predicates (done))
  (:functions (total-cost) (distance ?where - location))

  (:action scan
    :parameters ()
    :precondition ()
    :effect (and
      (done)
      (forall (?where - location)
        (increase (total-cost) (distance ?where))))))
""";

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Domain.FromFile(WriteTempFile(domainPddl)));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("Quantified action costs", exception.Message);
    }

    [Fact]
    public void StateApply_DoesNotEncodeActionCostInSuccessorState()
    {
        var problem = CreateProblemForStateApply_DoesNotEncodeActionCostInSuccessorState();

        var actions = new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder())
            .GetApplicableActions(problem.InitialState.Expand())
            .ToDictionary(action => action.Schema.Name, StringComparer.OrdinalIgnoreCase);

        var cheap = actions["cheap"];
        var expensive = actions["expensive"];
        var initialState = problem.InitialState.Expand();
        var cheapSuccessor = initialState.Apply(cheap);
        var expensiveSuccessor = initialState.Apply(expensive);

        Assert.Equal(2d, cheap.Cost);
        Assert.Equal(9d, expensive.Cost);
        Assert.Equal(cheapSuccessor, expensiveSuccessor);
    }

    [Fact]
    public void Domain_AllowsOrdinaryNumericMutationInConditionalEffect()
    {
        var domainPddl = """
(define (domain cond-mutable)
  (:requirements :strips :action-costs :conditional-effects)
  (:predicates (flag) (done))
  (:functions (total-cost) (battery))

  (:action act
    :parameters ()
    :precondition ()
    :effect (and
      (done)
      (when (flag) (increase (battery) 1))
      (increase (total-cost) 1))))
""";
        Domain domain = Domain.FromFile(WriteTempFile(domainPddl));
        Assert.NotEmpty(domain.Actions.SelectMany(action => action.NumericEffects));
    }

    [Fact]
    public void Domain_WithDomainLevelConstants_InActionPrecondition()
    {
        var domainPddl = """
(define (domain d)
  (:requirements :strips :typing)
  (:types loc)
  (:constants home - loc)
  (:predicates (at ?x - loc) (done))
  (:action go-home :parameters ()
    :precondition (at home)
    :effect (done)))
""";
        var domain = Domain.FromFile(WriteTempFile(domainPddl));
        Assert.Single(domain.Constants);
        Assert.Equal("home", domain.Constants[0].Name, ignoreCase: true);
    }

    [Fact]
    public void Domain_RejectsConditionalActionCosts()
    {
        var domainPddl = """
(define (domain conditional-cost)
  (:requirements :strips :action-costs)
  (:predicates (ready) (flag) (done))

  (:action act
    :parameters ()
    :precondition (ready)
    :effect (and
      (done)
      (when (flag) (increase (total-cost) 2)))))
""";

        var domainPath = WriteTempFile(domainPddl);

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Domain.FromFile(domainPath));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("Conditional action costs", exception.Message);
    }

    [Theory]
    [InlineData("(and (and))")]
    [InlineData("(and (and) ())")]
    [InlineData("(not (or))")]
    [InlineData("(imply (or) (ready ?x))")]
    [InlineData("(forall (?other - item) (and))")]
    [InlineData("(= ?x ?X)")]
    public void StructurallyTrueActionCostConditions_AreRejectedBeforeNormalization(string condition)
    {
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() => CreateProblem(
            $$"""
(define (domain structurally-unconditional-cost)
  (:requirements :adl :action-costs :numeric-fluents)
  (:types item)
  (:predicates (ready ?x - item) (done ?x - item))
  (:functions (score))

  (:action act
    :parameters (?x - item)
    :precondition (ready ?x)
    :effect (and
      (done ?x)
      (when {{condition}} (increase (total-cost) 2)))))
""",
            """
(define (problem p)
  (:domain structurally-unconditional-cost)
  (:objects value - item)
  (:init (ready value) (= (score) 0) (= (total-cost) 0))
  (:goal (done value))
  (:metric minimize (total-cost)))
"""));

        Assert.Contains("Conditional action costs", exception.Message);
    }

    [Theory]
    [InlineData("(or)")]
    [InlineData("(not (and))")]
    [InlineData("(exists (?other - item) (and))")]
    [InlineData("(forall (?other - item) (or))")]
    public void NonTrueActionCostConditions_RemainUnsupported(string condition)
    {
        string domainPddl = $$"""
(define (domain non-unconditional-cost)
  (:requirements :adl :action-costs)
  (:types item)
  (:predicates (done))

  (:action act
    :parameters ()
    :precondition ()
    :effect (and
      (done)
      (when {{condition}} (increase (total-cost) 2)))))
""";

        PddlLoadException exception = Assert.Throws<PddlLoadException>(
            () => Domain.FromFile(WriteTempFile(domainPddl)));

        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature, exception.ErrorCode);
        Assert.Contains("Conditional action costs", exception.Message);
    }
}
