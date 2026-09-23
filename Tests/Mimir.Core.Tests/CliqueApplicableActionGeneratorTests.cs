using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;
using GroundAction = Mimir.Core.Grounding.Action;

namespace Mimir.Core.Tests;

public class CliqueApplicableActionGeneratorTests
{
    private static Problem CreateProblemForNullaryAction_UsesAllConditionTypesCostAndConditionalEffects()
    {
        DomainBuilder domainBuilder = new DomainBuilder("complete-nullary");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Add(":conditional-effects");
        requirements.Add(":derived-predicates");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("allowed");
        predicates.Add("ready");
        predicates.Add("bonus-enabled");
        predicates.Add("bonus-ready");
        predicates.Add("can-act");
        predicates.Add("bonus-derived");
        predicates.Add("done");
        predicates.Add("bonus");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("step-cost");
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("advance");
        action.AddPrecondition("allowed");
        action.AddPrecondition("ready");
        action.AddPrecondition("can-act");
        action.AddEffect("ready", Polarity.Negative);
        action.AddEffect("done");
        action.AddEffect("bonus-ready", Polarity.Negative);
        ConditionalEffectBuilder conditional0 = action.AddConditionalEffect();
        conditional0.AddCondition("bonus-enabled");
        conditional0.AddCondition("bonus-ready");
        conditional0.AddCondition("bonus-derived");
        conditional0.AddEffect("bonus");
        conditional0.Close();
        action.WithCost(Numeric.Function("step-cost"));
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("can-act", Logic.And(Logic.Atom("allowed"), Logic.Atom("ready")));
        derivedPredicates.Define("bonus-derived", Logic.Atom("bonus-ready"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("allowed");
        initialState.AddFact("ready");
        initialState.AddFact("bonus-enabled");
        initialState.AddFact("bonus-ready");
        initialState.SetValue(Numeric.Function("step-cost"), 7d);
        initialState.Close();
        GoalBuilder goals = problemBuilder.Goal();
        goals.Add("done");
        goals.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("witness", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?w", "witness"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "item");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("eligible", "?w");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("i2", "item");
        objects.Add("w1", "witness");
        objects.Add("w2", "witness");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("eligible", "w1");
        initialState.AddFact("eligible", "w2");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("witness", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?w", "witness"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "item");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("eligible", "?w");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("i2", "item");
        objects.Add("w1", "witness");
        objects.Add("w2", "witness");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("eligible", "w1");
        initialState.AddFact("eligible", "w2");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedCollectApplicableActions_ReusesDestinationWithoutChangingPublicSnapshots()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled");
        predicates.Add("first-done");
        predicates.Add("second-done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("first");
        action.AddPrecondition("enabled");
        action.AddEffect("enabled", Polarity.Negative);
        action.AddEffect("first-done");
        action.Close();
        action = actions.Add("second");
        action.AddPrecondition("enabled");
        action.AddEffect("second-done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedCollectApplicableActions_ThrowingTraversalLeavesScratchReusable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":derived-predicates");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("can-act");
        predicates.Add("first-done");
        predicates.Add("second-done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("first");
        action.AddPrecondition("ready");
        action.AddEffect("ready", Polarity.Negative);
        action.AddEffect("first-done");
        action.Close();
        action = actions.Add("second");
        action.AddPrecondition("ready");
        action.AddPrecondition("can-act");
        action.AddEffect("second-done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("can-act", Logic.Atom("ready"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForFluentPreconditionOnlyParameter_SelectsWitnessFromCurrentState()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("witness", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?w", "witness"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "item");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("eligible", "?w");
        action.AddEffect("done", "?x");
        action.Close();
        action = actions.Add("disable");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("eligible", "?w");
        action.AddEffect("eligible", Polarity.Negative, "?w");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("w1", "witness");
        objects.Add("w2", "witness");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("eligible", "w1");
        initialState.AddFact("eligible", "w2");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForEffectBindingWithoutWitness_IsNotEmitted()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("witness", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("supports", ("?x", "item"), ("?w", "witness"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "item");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("supports", "?x", "?w");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("i2", "item");
        objects.Add("w1", "witness");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("supports", "i1", "w1");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForConditionalEffectConditionParameter_RemainsDistinct()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":conditional-effects");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("enabled", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        ConditionalEffectBuilder conditional1 = action.AddConditionalEffect();
        conditional1.AddCondition("enabled", "?x");
        conditional1.AddEffect("done");
        conditional1.Close();
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("enabled", "a");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForActionCostParameter_RemainsDistinct()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done");
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("binding-cost", ("?x", "object"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddEffect("done");
        action.WithCost(Numeric.Function("binding-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.SetValue(Numeric.Function("binding-cost", "a"), 2d);
        initialState.SetValue(Numeric.Function("binding-cost", "b"), 3d);
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("eligible", "?x");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("eligible", "a");
        initialState.AddFact("eligible", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects1()
    {
        DomainBuilder domainBuilder = new DomainBuilder("sparse-static");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("blocked", ("?x", "object"), ("?y", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("blocked", Polarity.Negative, "?x", "?y");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects2()
    {
        DomainBuilder domainBuilder = new DomainBuilder("ordered-branches");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("f");
        predicates.Add("g");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("first");
        action.AddPrecondition("f");
        action.Close();
        action = actions.Add("second");
        action.Close();
        action = actions.Add("third");
        action.AddPrecondition("g", Polarity.Negative);
        action.Close();
        action = actions.Add("fourth");
        action.AddPrecondition("f");
        action.AddPrecondition("g", Polarity.Negative);
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "ordered-branches-problem");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("f");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":negative-preconditions");
        requirements.Add(":derived-predicates");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Add("witness", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("blocked");
        predicates.Add("base");
        predicates.Add("permitted");
        predicates.Add("eligible", ("?w", "witness"));
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("empty");
        action.Close();
        action = actions.Add("positive");
        action.AddPrecondition("ready");
        action.Close();
        action = actions.Add("negative");
        action.AddPrecondition("blocked", Polarity.Negative);
        action.Close();
        action = actions.Add("derived");
        action.AddPrecondition("permitted");
        action.Close();
        action = actions.Add("witnessed");
        action.AddParameter("?x", "item");
        action.AddParameter("?w", "witness");
        action.AddPrecondition("eligible", "?w");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("permitted", Logic.Atom("base"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("i1", "item");
        objects.Add("i2", "item");
        objects.Add("w1", "witness");
        objects.Add("w2", "witness");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.AddFact("base");
        initialState.AddFact("eligible", "w1");
        initialState.AddFact("eligible", "w2");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects4()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Add(":action-costs");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("item", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done", ("?x", "item"));
        predicates.Close();
        NumericFunctionListBuilder functions = domainBuilder.Functions();
        functions.Add("binding-cost", ("?x", "item"));
        functions.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "item");
        action.AddPrecondition("ready");
        action.AddEffect("done", "?x");
        action.WithCost(Numeric.Function("binding-cost", "?x"));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "item");
        objects.Add("b", "item");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.SetValue(Numeric.Function("binding-cost", "a"), 4d);
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("start");
        predicates.Add("mid");
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("finish");
        action.AddPrecondition("mid");
        action.AddEffect("done");
        action.Close();
        action = actions.Add("advance");
        action.AddPrecondition("start");
        action.AddEffect("mid");
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

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForZeroArity_NoPreconditions_YieldsOneAction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("a");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects8()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("f");
        predicates.Add("g");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("a_true");
        action.AddPrecondition("f");
        action.Close();
        action = actions.Add("a_false");
        action.AddPrecondition("g");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("f");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("a");
        action.AddParameter("?x", "object");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("available");
        predicates.Add("blocked");
        predicates.Add("first-done");
        predicates.Add("middle-done", ("?x", "object"));
        predicates.Add("last-done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("first");
        action.AddPrecondition("available");
        action.AddEffect("first-done");
        action.Close();
        action = actions.Add("middle");
        action.AddParameter("?x", "object");
        action.AddEffect("middle-done", "?x");
        action.Close();
        action = actions.Add("unavailable");
        action.AddPrecondition("blocked");
        action.Close();
        action = actions.Add("last");
        action.AddPrecondition("available");
        action.AddEffect("last-done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("available");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForNullaryAction_UndefinedCostIsNeverApplicable()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":action-costs");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("broken");
        action.AddEffect("done");
        action.WithCost(Numeric.Constant(1d) / Numeric.Constant(0d));
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGetApplicableActions_MaximumBuildsOnlyTheRequestedBinding()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("ready");
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("ready");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("ready");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGetApplicableActions_ReusesCachedActionInstances()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedPreSearchCompactionClaim_IsThresholdedAndOneShot()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedWitnessGrouping_IsInvocationLocal()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("eligible", "?x");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("eligible", "a");
        initialState.AddFact("eligible", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding6()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":derived-predicates");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("eligible", ("?x", "object"));
        predicates.Add("allowed", ("?x", "object"));
        predicates.Add("done");
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("allowed", "?x");
        action.AddEffect("done");
        action.Close();
        actions.Close();
        DerivedPredicateListBuilder derivedPredicates = domainBuilder.DerivedPredicates();
        derivedPredicates.Define("allowed", Logic.Atom("eligible", "?x"));
        derivedPredicates.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding7()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("noop");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForBinary_NoPreconditions_YieldsNxMActions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("t1", "object");
        types.Add("t2", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?x", "t1"), ("?y", "t2"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("a");
        action.AddParameter("?x", "t1");
        action.AddParameter("?y", "t2");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "t1");
        objects.Add("o2", "t1");
        objects.Add("o3", "t2");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding9()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("vehicle", "object");
        types.Add("loc", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?v", "vehicle"), ("?l", "loc"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("drive");
        action.AddParameter("?v", "vehicle");
        action.AddParameter("?l", "loc");
        action.AddEffect("done", "?v", "?l");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("v1", "vehicle");
        objects.Add("v2", "vehicle");
        objects.Add("l1", "loc");
        objects.Add("l2", "loc");
        objects.Add("l3", "loc");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState0()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":typing");
        requirements.Close();
        TypeListBuilder types = domainBuilder.Types();
        types.Add("car", "vehicle");
        types.Add("truck", "vehicle");
        types.Add("vehicle", "object");
        types.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("done", ("?v", "vehicle"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("drive");
        action.AddParameter("?v", "vehicle");
        action.AddEffect("done", "?v");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("c1", "car");
        objects.Add("t1", "truck");
        objects.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForUnary_FluentPreconditions_Positive()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("active", "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "o1");
        initialState.AddFact("active", "o3");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForUnary_FluentPreconditions_Negative()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("active", ("?x", "object"));
        predicates.Add("done", ("?x", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddPrecondition("active", Polarity.Negative, "?x");
        action.AddEffect("done", "?x");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("o1", "object");
        objects.Add("o2", "object");
        objects.Add("o3", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("active", "o1");
        initialState.AddFact("active", "o3");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState3()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("conn", ("?x", "object"), ("?y", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("conn", "?x", "?y");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("conn", "a", "b");
        initialState.AddFact("conn", "b", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForBinary_FluentPreconditions_Negative()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Add(":negative-preconditions");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("conn", ("?x", "object"), ("?y", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("conn", Polarity.Negative, "?x", "?y");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("conn", "a", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState5()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("conn", ("?x", "object"), ("?y", "object"));
        predicates.Add("same-color", ("?x", "object"), ("?y", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddPrecondition("conn", "?x", "?y");
        action.AddPrecondition("same-color", "?x", "?y");
        action.AddEffect("done", "?x", "?y");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("conn", "a", "b");
        initialState.AddFact("conn", "b", "c");
        initialState.AddFact("same-color", "a", "b");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForCross_Partition_Interaction()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("conn", ("?a", "object"), ("?b", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("move");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("conn", "?x", "?y");
        action.AddPrecondition("conn", "?y", "?z");
        action.AddEffect("done", "?x", "?y", "?z");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Add("d", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("conn", "a", "b");
        initialState.AddFact("conn", "b", "c");
        initialState.AddFact("conn", "c", "d");
        initialState.Close();
        return problemBuilder.Build();
    }

    private static Problem CreateProblemForHigher_Arity_Preconditions()
    {
        DomainBuilder domainBuilder = new DomainBuilder("d");
        RequirementListBuilder requirements = domainBuilder.Requirements();
        requirements.Add(":strips");
        requirements.Close();
        PredicateListBuilder predicates = domainBuilder.Predicates();
        predicates.Add("between", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Add("done", ("?x", "object"), ("?y", "object"), ("?z", "object"));
        predicates.Close();
        ActionListBuilder actions = domainBuilder.Actions();
        ActionSchemaBuilder action = actions.Add("act");
        action.AddParameter("?x", "object");
        action.AddParameter("?y", "object");
        action.AddParameter("?z", "object");
        action.AddPrecondition("between", "?x", "?y", "?z");
        action.AddEffect("done", "?x", "?y", "?z");
        action.Close();
        actions.Close();
        Domain domain = domainBuilder.Build();
        ProblemBuilder problemBuilder = new ProblemBuilder(domain, "p");
        ProblemObjectListBuilder objects = problemBuilder.Objects();
        objects.Add("a", "object");
        objects.Add("b", "object");
        objects.Add("c", "object");
        objects.Add("d", "object");
        objects.Close();
        InitialStateBuilder initialState = problemBuilder.InitialState();
        initialState.AddFact("between", "a", "b", "c");
        initialState.Close();
        return problemBuilder.Build();
    }

    [Fact]
    public void EmptyDomain_YieldsNoActions()
    {
        var problem = CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects6();
        var generator = new CliqueApplicableActionGenerator(problem);
        Assert.Empty(generator.GetApplicableActions(problem.InitialState.Expand()));
    }

    [Fact]
    public void ZeroArity_NoPreconditions_YieldsOneAction()
    {
        var problem = CreateProblemForZeroArity_NoPreconditions_YieldsOneAction();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Single(actions);
        Assert.Equal("a", actions[0].Schema.Name);
    }

    [Fact]
    public void ZeroArity_NullaryPreconditions_TrueAndFalse()
    {
        var problem = CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects8();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Single(actions);
        Assert.Equal("a_true", actions[0].Schema.Name);
    }

    [Fact]
    public void Unary_NoPreconditions_YieldsNActions()
    {
        var problem = CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects9();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(3, actions.Count);
    }

    [Fact]
    public void NullaryAction_UsesAllConditionTypesCostAndConditionalEffects()
    {
        Problem problem = CreateProblemForNullaryAction_UsesAllConditionTypesCostAndConditionalEffects();
        var generator = new CliqueApplicableActionGenerator(problem);
        ExtendedState initial = problem.InitialState.Expand();

        GroundAction action = Assert.Single(generator.GetApplicableActions(initial));

        Assert.Equal(7d, action.Cost);
        Assert.Single(action.ConditionalEffects);
        Assert.True(action.IsApplicable(initial));
        ExtendedState successor = initial.Apply(action).Expand();
        Mimir.Core.Schemas.Predicate<Fluent> donePredicate =
            problem.Domain.Fluents.Single(predicate => predicate.Name == "done");
        Mimir.Core.Schemas.Predicate<Fluent> bonusPredicate =
            problem.Domain.Fluents.Single(predicate => predicate.Name == "bonus");
        Mimir.Core.Schemas.Predicate<Fluent> readyPredicate =
            problem.Domain.Fluents.Single(predicate => predicate.Name == "ready");
        Mimir.Core.Schemas.Predicate<Fluent> bonusReadyPredicate =
            problem.Domain.Fluents.Single(predicate => predicate.Name == "bonus-ready");
        Fact<Fluent> done = problem.Context.RegisterFact(
            donePredicate,
            Array.Empty<Constant>());
        Fact<Fluent> bonus = problem.Context.RegisterFact(
            bonusPredicate,
            Array.Empty<Constant>());
        Fact<Fluent> ready = problem.Context.RegisterFact(
            readyPredicate,
            Array.Empty<Constant>());
        Fact<Fluent> bonusReady = problem.Context.RegisterFact(
            bonusReadyPredicate,
            Array.Empty<Constant>());

        Assert.True(successor.IsTrue(done));
        Assert.True(successor.IsTrue(bonus));
        Assert.False(successor.IsTrue(ready));
        Assert.False(successor.IsTrue(bonusReady));
        Assert.False(action.IsApplicable(successor));
        Assert.Empty(generator.GetApplicableActions(successor));
    }

    [Fact]
    public void NullaryAction_UndefinedCostIsNeverApplicable()
    {
        Problem problem = CreateProblemForNullaryAction_UndefinedCostIsNeverApplicable();

        Assert.Empty(new CliqueApplicableActionGenerator(problem).GetApplicableActions(problem.InitialState.Expand()));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void StaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding(bool useClique)
    {
        Problem problem = CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        ExtendedState state = problem.InitialState.Expand();
        GroundAction[] actions = generator.GetApplicableActions(state).ToArray();

        Assert.Equal(2, actions.Length);
        Assert.Equal(
            new[] { "i1", "i2" },
            actions.Select(action => action.Arguments[0].Name).OrderBy(name => name));
        Assert.All(actions, action => Assert.True(action.IsApplicable(state)));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void FluentPreconditionOnlyParameter_SelectsWitnessFromCurrentState(bool useClique)
    {
        Problem problem = CreateProblemForFluentPreconditionOnlyParameter_SelectsWitnessFromCurrentState();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);
        State initialState = problem.InitialState;
        ExtendedState extendedInitialState = initialState.Expand();

        GroundAction initialAct = generator.GetApplicableActions(extendedInitialState)
            .Single(action => action.Schema.Name == "act");
        GroundAction disableW1 = generator.GetApplicableActions(extendedInitialState)
            .Single(action => action.Schema.Name == "disable" && action.Arguments[0].Name == "w1");
        State nextState = extendedInitialState.Apply(disableW1);
        ExtendedState extendedNextState = nextState.Expand();
        GroundAction nextAct = generator.GetApplicableActions(extendedNextState)
            .Single(action => action.Schema.Name == "act");

        Assert.Contains(initialAct.Arguments[1].Name, new[] { "w1", "w2" });
        Assert.True(initialAct.IsApplicable(extendedInitialState));
        Assert.Equal("w2", nextAct.Arguments[1].Name);
        Assert.True(nextAct.IsApplicable(extendedNextState));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void EffectBindingWithoutWitness_IsNotEmitted(bool useClique)
    {
        Problem problem = CreateProblemForEffectBindingWithoutWitness_IsNotEmitted();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        GroundAction action = Assert.Single(generator.GetApplicableActions(problem.InitialState.Expand()));

        Assert.Equal("i1", action.Arguments[0].Name);
        Assert.Equal("w1", action.Arguments[1].Name);
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void ConditionalEffectConditionParameter_RemainsDistinct(bool useClique)
    {
        Problem problem = CreateProblemForConditionalEffectConditionParameter_RemainsDistinct();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        GroundAction[] actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToArray();

        Assert.Equal(
            new[] { "a", "b" },
            actions.Select(action => action.Arguments[0].Name).OrderBy(name => name));
        Assert.All(actions, action => Assert.Single(action.ConditionalEffects));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void ActionCostParameter_RemainsDistinct(bool useClique)
    {
        Problem problem = CreateProblemForActionCostParameter_RemainsDistinct();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        GroundAction[] actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToArray();

        Assert.Equal(
            new[] { ("a", 2d), ("b", 3d) },
            actions.Select(action => (action.Arguments[0].Name, action.Cost))
                .OrderBy(action => action.Name));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void GetApplicableActions_ZeroMaximumReturnsNoActions(bool useClique)
    {
        Problem problem = CreateOrderedApplicableActionsProblem();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        Assert.Empty(generator.GetApplicableActions(problem.InitialState.Expand(), 0));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void GetApplicableActions_NegativeMaximumIsOutOfRange(bool useClique)
    {
        Problem problem = CreateOrderedApplicableActionsProblem();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        ArgumentOutOfRangeException exception = Assert.Throws<ArgumentOutOfRangeException>(
            () => generator.GetApplicableActions(problem.InitialState.Expand(), -1));

        Assert.Equal("maxActions", exception.ParamName);
    }

    [Theory]
    [InlineData(false, 1)]
    [InlineData(false, 3)]
    [InlineData(true, 1)]
    [InlineData(true, 3)]
    public void GetApplicableActions_MaximumBoundsReturnedActionCount(bool useClique, int maximum)
    {
        Problem problem = CreateOrderedApplicableActionsProblem();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);
        ExtendedState state = problem.InitialState.Expand();

        GroundAction[] allActions = generator.GetApplicableActions(state).ToArray();
        GroundAction[] boundedActions = generator.GetApplicableActions(state, maximum).ToArray();

        Assert.True(allActions.Length > maximum);
        Assert.Equal(maximum, boundedActions.Length);
        Assert.All(boundedActions, action => Assert.True(action.IsApplicable(state)));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void GetApplicableActions_ValidatesInputs(bool useClique)
    {
        Problem problem = CreateOrderedApplicableActionsProblem();
        Problem otherProblem = CreateOrderedApplicableActionsProblem();
        IApplicableActionGenerator generator = CreateGenerator(problem, useClique);

        ArgumentNullException stateException = Assert.Throws<ArgumentNullException>(
            () => generator.GetApplicableActions(null!));

        Assert.Equal("state", stateException.ParamName);
        Assert.Throws<InvalidOperationException>(
            () => generator.GetApplicableActions(otherProblem.InitialState.Expand()));
        Assert.Throws<InvalidOperationException>(
            () => generator.GetApplicableActions(otherProblem.InitialState.Expand(), 0));
    }

    [Fact]
    public void Binary_NoPreconditions_YieldsNxMActions()
    {
        var problem = CreateProblemForBinary_NoPreconditions_YieldsNxMActions();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(2, actions.Count); // 2 of t1 * 1 of t2 = 2
    }

    [Fact]
    public void TypeEnforcement_OnlyYieldsMatchingTypes()
    {
        var problem = CreateProblemForStaticPreconditionOnlyParameter_UsesOneWitnessPerEffectBinding9();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(6, actions.Count);
    }

    [Fact]
    public void TypeHierarchy_SubtypesAreValid()
    {
        var problem = CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState0();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(2, actions.Count);
    }

    [Fact]
    public void Unary_FluentPreconditions_Positive()
    {
        var problem = CreateProblemForUnary_FluentPreconditions_Positive();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(2, actions.Count);
        Assert.Contains(actions, a => a.Arguments[0].Name == "o1");
        Assert.Contains(actions, a => a.Arguments[0].Name == "o3");
    }

    [Fact]
    public void Unary_FluentPreconditions_Negative()
    {
        var problem = CreateProblemForUnary_FluentPreconditions_Negative();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Single(actions);
        Assert.Equal("o2", actions[0].Arguments[0].Name);
    }

    [Fact]
    public void Binary_FluentPreconditions_Positive()
    {
        var problem = CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState3();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Equal(2, actions.Count);
    }

    [Fact]
    public void Binary_FluentPreconditions_Negative()
    {
        var problem = CreateProblemForBinary_FluentPreconditions_Negative();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        // total pairs = 4 (a a, a b, b a, b b). conn a b is true, so 3 actions.
        Assert.Equal(3, actions.Count);
        Assert.DoesNotContain(actions, a => a.Arguments[0].Name == "a" && a.Arguments[1].Name == "b");
    }

    [Fact]
    public void Multiple_Binary_Preconditions()
    {
        var problem = CreateProblemForGroundedGetApplicableActions_BoundedCallDoesNotLeaveTraversalOrWitnessState5();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Single(actions);
        Assert.Equal("a", actions[0].Arguments[0].Name);
        Assert.Equal("b", actions[0].Arguments[1].Name);
    }

    [Fact]
    public void Cross_Partition_Interaction()
    {
        // 3 parameters requiring conn(x,y) and conn(y,z)
        var problem = CreateProblemForCross_Partition_Interaction();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        // valid triplets: (a,b,c), (b,c,d)
        Assert.Equal(2, actions.Count);
    }

    [Fact]
    public void Higher_Arity_Preconditions()
    {
        var problem = CreateProblemForHigher_Arity_Preconditions();
        var generator = new CliqueApplicableActionGenerator(problem);
        var actions = generator.GetApplicableActions(problem.InitialState.Expand()).ToList();
        Assert.Single(actions);
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void ActionIdentity_IsStableAcrossGeneratorsRegardlessOfRegistrationOrder(bool cliqueFirst)
    {
        Problem problem = CreateProblemForNullaryAction_UsesAllConditionKindsCostAndConditionalEffects5();

        List<GroundAction> rpgActions;
        GroundAction cliqueAction;
        if (cliqueFirst)
        {
            var cliqueGenerator = new CliqueApplicableActionGenerator(problem);
            cliqueAction = Assert.Single(
                cliqueGenerator.GetApplicableActions(problem.InitialState.Expand()));
            rpgActions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();
        }
        else
        {
            rpgActions = new RpgGrounder().Ground(problem, problem.InitialState).ToList();
            var cliqueGenerator = new CliqueApplicableActionGenerator(problem);
            cliqueAction = Assert.Single(
                cliqueGenerator.GetApplicableActions(problem.InitialState.Expand()));
        }

        GroundAction matchingAction = rpgActions.Single(action => action.Schema.Name == "advance");
        GroundAction differentAction = rpgActions.Single(action => action.Schema.Name == "finish");

        Assert.Equal(matchingAction, cliqueAction);
        Assert.Equal(matchingAction.GetHashCode(), cliqueAction.GetHashCode());
        Assert.NotEqual(differentAction, cliqueAction);
        Assert.Single(new HashSet<GroundAction> { matchingAction, cliqueAction });
        Assert.Equal(2, new HashSet<GroundAction> { differentAction, cliqueAction }.Count);
    }

    private Problem CreateOrderedApplicableActionsProblem()
    {
        DomainBuilder builder = new DomainBuilder("d").Requirements().Add(":strips").Close()
            .Predicates().Add("first-done", ("?x", "object"))
                .Add("second-done", ("?x", "object")).Close();
        ActionListBuilder actions = builder.Actions();
        actions.Add("first").AddParameter("?x").AddEffect("first-done", "?x").Close();
        actions.Add("second").AddParameter("?x").AddEffect("second-done", "?x").Close();
        Domain domain = actions.Close().Build();
        return new ProblemBuilder(domain, "p").Objects().Add("a").Add("b").Close().Build();
    }

    private static IApplicableActionGenerator CreateGenerator(Problem problem, bool useClique)
        => useClique
            ? new CliqueApplicableActionGenerator(problem)
            : new GroundedApplicableActionGenerator(problem, problem.InitialState, new RpgGrounder());

    private static string BasePath => Path.Combine(AppContext.BaseDirectory, "../../../../../Tests/Examples");

    [Theory]
    [InlineData("assembly", "p01.pddl")]
    [InlineData("barman", "p01.pddl")]
    [InlineData("blocks_4", "p01.pddl")]
    [InlineData("childsnack", "p01.pddl")]
    [InlineData("delivery", "p01.pddl")]
    [InlineData("driverlog", "p01.pddl")]
    [InlineData("ferry", "p01.pddl")]
    [InlineData("grid", "p01.pddl")]
    [InlineData("gripper", "p01.pddl")]
    [InlineData("logistics", "p01.pddl")]
    [InlineData("miconic-simpleadl", "p01.pddl")]
    [InlineData("miconic", "p01.pddl")]
    [InlineData("rovers", "p01.pddl")]
    [InlineData("satellite", "p01.pddl")]
    [InlineData("schedule", "p01.pddl")]
    [InlineData("visitall", "p01.pddl")]
    public void ParityTest_CompareWithGroundedGenerator(string domainFolderName, string problemFile)
    {
        string domainPath = Path.Combine(BasePath, domainFolderName, "domain.pddl");
        string problemPath = Path.Combine(BasePath, domainFolderName, problemFile);

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        var grounder = new RpgGrounder();
        var groundedGenerator = new GroundedApplicableActionGenerator(problem, problem.InitialState, grounder);
        var cliqueGenerator = new CliqueApplicableActionGenerator(problem);

        ExtendedState state = problem.InitialState.Expand();
        var expectedActions = groundedGenerator.GetApplicableActions(state).ToList();
        var actualActions = cliqueGenerator.GetApplicableActions(state).ToList();

        Assert.Equal(expectedActions.Count, actualActions.Count);

        var expectedNames = expectedActions
            .Select(a => $"{a.Schema.Name} {string.Join(" ", a.Arguments.Select(arg => arg.Name))}").OrderBy(n => n)
            .ToList();
        var actualNames = actualActions
            .Select(a => $"{a.Schema.Name} {string.Join(" ", a.Arguments.Select(arg => arg.Name))}").OrderBy(n => n)
            .ToList();

        for (int i = 0; i < expectedNames.Count; i++)
        {
            Assert.Equal(expectedNames[i], actualNames[i]);
        }
    }

    [Theory]
    [InlineData("blocks_4", "p01.pddl")]
    [InlineData("gripper", "p01.pddl")]
    [InlineData("logistics", "p01.pddl")]
    [InlineData("ferry", "p01.pddl")]
    [InlineData("miconic", "p01.pddl")]
    [InlineData("rovers", "p01.pddl")]
    [InlineData("satellite", "p01.pddl")]
    [InlineData("driverlog", "p01.pddl")]
    [InlineData("grid", "p01.pddl")]
    [InlineData("delivery", "p01.pddl")]
    public void ExhaustiveParityTest_CompareWithGroundedGeneratorAcrossReachableStates(string domainFolderName, string problemFile)
    {
        string domainPath = Path.Combine(BasePath, domainFolderName, "domain.pddl");
        string problemPath = Path.Combine(BasePath, domainFolderName, problemFile);

        var domain = Domain.FromFile(domainPath);
        var problem = Problem.FromFile(domain, problemPath);

        var groundedGenerator = new GroundedApplicableActionGenerator(
            problem,
            problem.InitialState,
            new RpgGrounder());
        var cliqueGenerator = new CliqueApplicableActionGenerator(problem);
        var initialState = problem.InitialState;

        var frontier = new Queue<State>();
        var visited = new HashSet<State>(StateEqualityComparer.Instance);
        frontier.Enqueue(initialState);
        visited.Add(initialState);

        while (frontier.Count > 0)
        {
            var state = frontier.Dequeue();
            ExtendedState extendedState = state.Expand();
            var groundedActions = groundedGenerator.GetApplicableActions(extendedState).ToList();
            var cliqueActions = cliqueGenerator.GetApplicableActions(extendedState).ToList();

            var groundedSignatures = groundedActions
                .Select(FormatActionSignature)
                .OrderBy(signature => signature)
                .ToArray();
            var cliqueSignatures = cliqueActions
                .Select(FormatActionSignature)
                .OrderBy(signature => signature)
                .ToArray();

            Assert.Equal(
                groundedSignatures,
                cliqueSignatures);

            foreach (var successor in groundedActions.Select(extendedState.Apply)
                         .Concat(cliqueActions.Select(extendedState.Apply)))
            {
                if (visited.Add(successor))
                    frontier.Enqueue(successor);
            }
        }
    }

    private static string FormatActionSignature(Mimir.Core.Grounding.Action action)
        => $"{action.Schema.Name} {string.Join(" ", action.Arguments.Select(arg => arg.Name))} @ {action.Cost:G17}";

}
