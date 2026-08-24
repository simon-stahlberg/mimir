using System.Collections.Immutable;
using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using PddlAdd = Mimir.Pddl.Ast.Expressions.Add;
using PddlConditionalEffect = Mimir.Pddl.Ast.Effects.ConditionalEffect;
using PddlTerm = Mimir.Pddl.Ast.Models.Term;

namespace Mimir.Core.Schemas;

internal sealed record ProgrammaticDomainCompilation(
    DomainDefinition Definition,
    ProgrammaticDomainInputs Inputs);

internal sealed record ProgrammaticActionInput(string Name, ActionCostSpec Cost);

internal sealed class ProgrammaticDomainInputs
{
    internal IReadOnlyList<ProgrammaticActionInput> Actions { get; }

    internal ProgrammaticDomainInputs(IEnumerable<BuilderActionSpec> actions)
    {
        Actions = Array.AsReadOnly(actions
            .Select(action => new ProgrammaticActionInput(action.Name, action.Cost))
            .ToArray());
    }
}

internal static class ProgrammaticDomainCompiler
{
    internal static ProgrammaticDomainCompilation Compile(DomainBuilder builder)
    {
        ImmutableArray<PddlRequirement> requirements = builder.RequirementSpecs
            .SelectMany(ParseRequirement)
            .ToImmutableArray();
        bool actionCostsEnabled = requirements.HasRequirement(PddlRequirement.ActionCosts);
        Dictionary<string, BuilderDerivedPredicateSpec> definitions = builder.DerivedPredicateSpecs
            .ToDictionary(definition => definition.PredicateName, StringComparer.OrdinalIgnoreCase);

        var definition = new DomainDefinition(
            builder.Name,
            requirements,
            builder.TypeSpecs
                .Select(type => new TypeDeclaration(type.Name, type.Type))
                .ToImmutableArray(),
            builder.ConstantSpecs
                .Select(constant => new TypeDeclaration(constant.Name, constant.Type))
                .ToImmutableArray(),
            builder.PredicateSpecs
                .Select(BuildPredicateDeclaration)
                .ToImmutableArray(),
            builder.FunctionSpecs
                .Select(function => new FunctionDeclaration(function.Name, BuildParameters(function.Parameters)))
                .ToImmutableArray(),
            builder.ActionSpecs
                .Select(action => BuildAction(action, actionCostsEnabled))
                .ToImmutableArray(),
            builder.PredicateSpecs
                .Where(predicate => definitions.ContainsKey(predicate.Name))
                .Select(predicate => new DerivedPredicate(
                    BuildPredicateDeclaration(predicate),
                    BuildLogicalExpression(definitions[predicate.Name].Body.Node)))
                .ToImmutableArray());

        return new ProgrammaticDomainCompilation(
            definition,
            new ProgrammaticDomainInputs(builder.ActionSpecs));
    }

    internal static IReadOnlyList<PddlRequirement> ParseRequirement(string requirement)
    {
        ArgumentException.ThrowIfNullOrWhiteSpace(requirement);
        return requirement.ToLowerInvariant() switch
        {
            ":strips" => [PddlRequirement.Strips],
            ":typing" => [PddlRequirement.Typing],
            ":equality" => [PddlRequirement.Equality],
            ":negative-preconditions" => [PddlRequirement.NegativePreconditions],
            ":disjunctive-preconditions" => [PddlRequirement.DisjunctivePreconditions],
            ":fluents" or ":numeric-fluents" => [PddlRequirement.NumericFluents],
            ":conditional-effects" => [PddlRequirement.ConditionalEffects],
            ":existential-preconditions" => [PddlRequirement.ExistentialPreconditions],
            ":universal-preconditions" => [PddlRequirement.UniversalPreconditions],
            ":quantified-preconditions" =>
                [PddlRequirement.ExistentialPreconditions, PddlRequirement.UniversalPreconditions],
            ":adl" => [PddlRequirement.Adl],
            ":derived-predicates" => [PddlRequirement.DerivedPredicates],
            ":action-costs" => [PddlRequirement.ActionCosts],
            _ => throw new ArgumentException(
                $"Unsupported PDDL requirement '{requirement}'.",
                nameof(requirement))
        };
    }

    internal static ILogicalExpression BuildLogicalExpression(LogicalExpressionNode expression)
        => expression switch
        {
            TrueLogicalExpressionNode => new EmptyLogic(),
            FalseLogicalExpressionNode => new Or(ImmutableArray<ILogicalExpression>.Empty),
            AtomLogicalExpressionNode atom => new PredicateCall(
                atom.PredicateName,
                BuildTerms(atom.Arguments)),
            EqualityLogicalExpressionNode equality => new Equality(
                BuildTerm(equality.Left),
                BuildTerm(equality.Right)),
            NotLogicalExpressionNode not => new Not(BuildLogicalExpression(not.Expression)),
            AndLogicalExpressionNode and => new And(
                and.Expressions.Select(BuildLogicalExpression).ToImmutableArray()),
            OrLogicalExpressionNode or => new Or(
                or.Expressions.Select(BuildLogicalExpression).ToImmutableArray()),
            ImplyLogicalExpressionNode imply => new Imply(
                BuildLogicalExpression(imply.Antecedent),
                BuildLogicalExpression(imply.Consequent)),
            ExistsLogicalExpressionNode exists => new Exists(
                BuildParameters(exists.Parameters),
                BuildLogicalExpression(exists.Body)),
            ForallLogicalExpressionNode forall => new Forall(
                BuildParameters(forall.Parameters),
                BuildLogicalExpression(forall.Body)),
            _ => throw new InvalidOperationException(
                $"Unsupported logical expression specification '{expression.GetType().Name}'.")
        };

    internal static INumericExpression BuildActionCost(ActionCostNode cost)
        => cost switch
        {
            ConstantActionCostNode constant => new NumberLiteral(constant.PddlValue),
            FunctionActionCostNode function => new FluentCall(
                function.FunctionName,
                BuildTerms(function.Arguments)),
            BinaryActionCostNode { Operator: ActionCostBinaryOperator.Add } binary => new PddlAdd(
                BuildActionCost(binary.Left),
                BuildActionCost(binary.Right)),
            BinaryActionCostNode { Operator: ActionCostBinaryOperator.Subtract } binary => new Subtract(
                BuildActionCost(binary.Left),
                BuildActionCost(binary.Right)),
            BinaryActionCostNode { Operator: ActionCostBinaryOperator.Multiply } binary => new Multiply(
                BuildActionCost(binary.Left),
                BuildActionCost(binary.Right)),
            BinaryActionCostNode { Operator: ActionCostBinaryOperator.Divide } binary => new Divide(
                BuildActionCost(binary.Left),
                BuildActionCost(binary.Right)),
            _ => throw new InvalidOperationException(
                $"Unsupported action cost specification '{cost.GetType().Name}'.")
        };

    private static PredicateDeclaration BuildPredicateDeclaration(BuilderPredicateSpec predicate)
        => new(predicate.Name, BuildParameters(predicate.Parameters));

    private static ActionDefinition BuildAction(BuilderActionSpec action, bool actionCostsEnabled)
    {
        var effects = new List<IEffect>(
            action.Effects.Count + action.ConditionalEffects.Count + (actionCostsEnabled ? 1 : 0));
        effects.AddRange(action.Effects.Select(BuildSimpleEffect));
        effects.AddRange(action.ConditionalEffects.Select(BuildConditionalEffect));
        if (actionCostsEnabled)
        {
            effects.Add(new Increase(
                new FluentCall("total-cost", ImmutableArray<PddlTerm>.Empty),
                BuildActionCost(action.Cost.Node)));
        }

        return new ActionDefinition(
            action.Name,
            BuildParameters(action.Parameters),
            BuildCondition(action.Preconditions),
            new AndEffect(effects.ToImmutableArray()));
    }

    private static IEffect BuildConditionalEffect(BuilderConditionalEffectSpec effect)
    {
        IEffect result = BuildSimpleEffect(effect.Effect);
        if (effect.Conditions.Count > 0)
            result = new PddlConditionalEffect(BuildCondition(effect.Conditions), result);
        if (effect.Parameters.Count > 0)
            result = new ForallEffect(BuildParameters(effect.Parameters), result);
        return result;
    }

    private static IEffect BuildSimpleEffect(BuilderLiteralSpec literal)
    {
        var predicate = new PredicateCall(literal.PredicateName, BuildTerms(literal.Arguments));
        return literal.Polarity == Polarity.Positive
            ? new AddEffect(predicate)
            : new DeleteEffect(predicate);
    }

    private static ILogicalExpression BuildCondition(IReadOnlyList<BuilderLiteralSpec> literals)
    {
        ILogicalExpression[] expressions = literals.Select(BuildLiteral).ToArray();
        return expressions.Length switch
        {
            0 => new EmptyLogic(),
            1 => expressions[0],
            _ => new And(expressions.ToImmutableArray())
        };
    }

    private static ILogicalExpression BuildLiteral(BuilderLiteralSpec literal)
    {
        ILogicalExpression expression = literal.PredicateName == "="
            ? new Equality(BuildTerm(literal.Arguments[0]), BuildTerm(literal.Arguments[1]))
            : new PredicateCall(literal.PredicateName, BuildTerms(literal.Arguments));
        return literal.Polarity == Polarity.Positive ? expression : new Not(expression);
    }

    private static ImmutableArray<Parameter> BuildParameters(
        IEnumerable<BuilderParameterSpec> parameters)
        => parameters
            .Select(parameter => new Parameter(parameter.Name, parameter.Type))
            .ToImmutableArray();

    private static ImmutableArray<PddlTerm> BuildTerms(IEnumerable<string> terms)
        => terms.Select(BuildTerm).ToImmutableArray();

    private static PddlTerm BuildTerm(string term)
        => term.StartsWith('?') ? PddlTerm.Variable(term) : PddlTerm.Constant(term);
}

internal static class ProgrammaticProblemCompiler
{
    internal static ProblemDefinition Compile(
        Domain domain,
        string name,
        IReadOnlyList<BuilderTypedNameSpec> objects,
        IReadOnlyList<BuilderProblemFactSpec> initialFacts,
        IReadOnlyList<BuilderProblemNumericSpec> numericInitializations,
        IReadOnlyList<BuilderProblemGoalSpec> goals)
    {
        ImmutableArray<IProblemInitElement>.Builder initialState =
            ImmutableArray.CreateBuilder<IProblemInitElement>(
                initialFacts.Count + numericInitializations.Count);
        foreach (BuilderProblemFactSpec fact in initialFacts)
        {
            initialState.Add(new PredicateCall(
                fact.PredicateName,
                BuildConstantTerms(fact.Arguments)));
        }
        foreach (BuilderProblemNumericSpec numeric in numericInitializations)
        {
            initialState.Add(new NumericInitialization(
                new FluentCall(numeric.FunctionName, BuildConstantTerms(numeric.Arguments)),
                new NumberLiteral(numeric.PddlValue)));
        }

        ILogicalExpression[] goalExpressions = goals.Select(BuildGoal).ToArray();
        ILogicalExpression goal = goalExpressions.Length switch
        {
            0 => new EmptyLogic(),
            1 => goalExpressions[0],
            _ => new And(goalExpressions.ToImmutableArray())
        };

        return new ProblemDefinition(
            name,
            domain.Name,
            ImmutableArray<PddlRequirement>.Empty,
            objects
                .Select(value => new TypeDeclaration(value.Name, value.Type))
                .ToImmutableArray(),
            initialState.ToImmutable(),
            goal,
            Metric: null);
    }

    private static ILogicalExpression BuildGoal(BuilderProblemGoalSpec goal)
    {
        ILogicalExpression expression = goal.PredicateName == "="
            ? new Equality(
                PddlTerm.Constant(goal.Arguments[0]),
                PddlTerm.Constant(goal.Arguments[1]))
            : new PredicateCall(goal.PredicateName, BuildConstantTerms(goal.Arguments));
        return goal.Polarity == Polarity.Positive ? expression : new Not(expression);
    }

    private static ImmutableArray<PddlTerm> BuildConstantTerms(IEnumerable<string> arguments)
        => arguments.Select(PddlTerm.Constant).ToImmutableArray();
}
