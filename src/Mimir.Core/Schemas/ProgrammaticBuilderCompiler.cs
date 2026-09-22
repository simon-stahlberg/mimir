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

internal sealed record ProgrammaticNumberLiteral(double RuntimeValue, decimal Projection) : NumberLiteral(Projection);

internal static class ProgrammaticDomainCompiler
{
    internal static DomainDefinition Compile(DomainBuilder builder)
    {
        ImmutableArray<PddlRequirement> requirements = builder.RequirementSpecs
            .SelectMany(ParseRequirement)
            .ToImmutableArray();
        bool actionCostsEnabled = requirements.HasRequirement(PddlRequirement.ActionCosts);
        Dictionary<string, BuilderDerivedPredicateSpec> definitions = builder.DerivedPredicateSpecs
            .ToDictionary(definition => definition.PredicateName, StringComparer.OrdinalIgnoreCase);

        return new DomainDefinition(
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
            ComparisonLogicalExpressionNode comparison => new Comparison((Mimir.Pddl.Ast.Expressions.ComparisonOperator)comparison.Operator,
                BuildNumericExpression(comparison.Left), BuildNumericExpression(comparison.Right)),
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

    internal static INumericExpression BuildNumericExpression(NumericExpressionNode cost)
        => cost switch
        {
            ConstantNumericNode constant => new ProgrammaticNumberLiteral(constant.Value, constant.PddlValue),
            FunctionNumericNode function => new FluentCall(
                function.FunctionName,
                BuildTerms(function.Arguments)),
            BinaryNumericNode { Operator: NumericOperator.Add } binary => new PddlAdd(
                BuildNumericExpression(binary.Left),
                BuildNumericExpression(binary.Right)),
            BinaryNumericNode { Operator: NumericOperator.Subtract } binary => new Subtract(
                BuildNumericExpression(binary.Left),
                BuildNumericExpression(binary.Right)),
            BinaryNumericNode { Operator: NumericOperator.Multiply } binary => new Multiply(
                BuildNumericExpression(binary.Left),
                BuildNumericExpression(binary.Right)),
            BinaryNumericNode { Operator: NumericOperator.Divide } binary => new Divide(
                BuildNumericExpression(binary.Left),
                BuildNumericExpression(binary.Right)),
            _ => throw new InvalidOperationException(
                $"Unsupported numeric expression specification '{cost.GetType().Name}'.")
        };

    private static PredicateDeclaration BuildPredicateDeclaration(BuilderPredicateSpec predicate)
        => new(predicate.Name, BuildParameters(predicate.Parameters));

    private static ActionDefinition BuildAction(BuilderActionSpec action, bool actionCostsEnabled)
    {
        var effects = new List<IEffect>();
        effects.AddRange(action.Effects.Select(BuildSimpleEffect));
        effects.AddRange(action.NumericUpdates.Select(BuildNumericUpdate));
        effects.AddRange(action.ConditionalEffects.Select(BuildConditionalEffect));
        // Without :action-costs the only permitted explicit cost is the implicit unit cost.
        if (actionCostsEnabled && action.Cost is not null)
        {
            effects.Add(new Increase(
                new FluentCall(NumericFunction.TotalCostName, ImmutableArray<PddlTerm>.Empty),
                BuildNumericExpression(action.Cost.Node)));
        }

        return new ActionDefinition(
            action.Name,
            BuildParameters(action.Parameters),
            BuildCondition(action.Preconditions, action.Expressions),
            new AndEffect(effects.ToImmutableArray()));
    }

    private static IEffect BuildConditionalEffect(BuilderConditionalEffectSpec effect)
    {
        IEffect result = effect.NumericUpdate is { } numeric ? BuildNumericUpdate(numeric)
            : BuildSimpleEffect(effect.Effect ?? throw new InvalidOperationException("Missing effect."));
        if (effect.Conditions.Count > 0 || effect.Expressions.Count > 0)
            result = new PddlConditionalEffect(BuildCondition(effect.Conditions, effect.Expressions), result);
        if (effect.Parameters.Count > 0)
            result = new ForallEffect(BuildParameters(effect.Parameters), result);
        return result;
    }

    private static IEffect BuildNumericUpdate(BuilderNumericUpdateSpec update)
    {
        var target = (FluentCall)BuildNumericExpression(update.Target.Node);
        INumericExpression value = BuildNumericExpression(update.Expression.Node);
        return update.Operator switch
        {
            NumericUpdateOperator.Assign => new Assign(target, value),
            NumericUpdateOperator.Increase => new Increase(target, value),
            NumericUpdateOperator.Decrease => new Decrease(target, value),
            NumericUpdateOperator.ScaleUp => new ScaleUp(target, value),
            NumericUpdateOperator.ScaleDown => new ScaleDown(target, value),
            _ => throw new ArgumentOutOfRangeException(nameof(update))
        };
    }

    private static IEffect BuildSimpleEffect(BuilderLiteralSpec literal)
    {
        var predicate = new PredicateCall(literal.PredicateName, BuildTerms(literal.Arguments));
        return literal.Polarity == Polarity.Positive
            ? new AddEffect(predicate)
            : new DeleteEffect(predicate);
    }

    private static ILogicalExpression BuildCondition(IReadOnlyList<BuilderLiteralSpec> literals, IReadOnlyList<LogicalExpressionSpec> conditions)
    {
        ILogicalExpression[] expressions = literals.Select(BuildLiteral).Concat(conditions.Select(condition => BuildLogicalExpression(condition.Node))).ToArray();
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
        IReadOnlyList<BuilderProblemGoalSpec> goals, IReadOnlyList<LogicalExpressionSpec> expressions)
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
                new ProgrammaticNumberLiteral(numeric.Value, numeric.PddlValue)));
        }

        ILogicalExpression[] goalExpressions = goals.Select(BuildGoal).Concat(expressions.Select(expression => ProgrammaticDomainCompiler.BuildLogicalExpression(expression.Node))).ToArray();
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
