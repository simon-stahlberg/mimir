using System.Collections.Immutable;
using System.Globalization;
using System.Linq;
using System.Numerics;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;
using Superpower;
using Superpower.Model;
using Superpower.Parsers;

namespace Mimir.Pddl.Parsing;

internal static class PddlGrammar
{
    private static readonly HashSet<string> UnsupportedExpressionNames = new(StringComparer.OrdinalIgnoreCase)
    {
        "preference", "always", "sometime", "within", "at-most-once",
        "sometime-after", "sometime-before", "always-within", "hold-during",
        "hold-after", "probabilistic", "oneof", "is-violated"
    };

    private static TokenListParser<PddlToken, Token<PddlToken>> Keyword(string value) =>
        Token.EqualTo(PddlToken.Identifier)
            .Where(t => string.Equals(t.ToStringValue(), value, StringComparison.OrdinalIgnoreCase));

    private static readonly TokenListParser<PddlToken, Token<PddlToken>> NonReservedIdentifier =
        Token.EqualTo(PddlToken.Identifier)
            .Where(token => !token.ToStringValue().StartsWith(':')
                && !PddlName.IsReserved(token.ToStringValue()));

    private static TokenListParser<PddlToken, T> UnsupportedExpression<T>() =>
        from open in Token.EqualTo(PddlToken.OpenParen)
        from keyword in Token.EqualTo(PddlToken.Identifier)
            .Where(token => UnsupportedExpressionNames.Contains(token.ToStringValue()))
        select Throw<T>(
            PddlParseErrorCode.UnsupportedFeature,
            $"Unsupported PDDL expression '{keyword.ToStringValue()}'.",
            keyword);

    private static readonly TokenListParser<PddlToken, Term> Variable =
        Token.EqualTo(PddlToken.Variable)
            .Select(t => Term.Variable(t.ToStringValue()));

    private static readonly TokenListParser<PddlToken, Term> Constant =
        NonReservedIdentifier
            .Select(t => Term.Constant(t.ToStringValue()));

    private static readonly TokenListParser<PddlToken, Term> TermParser = Variable.Or(Constant);

    internal static readonly TokenListParser<PddlToken, PredicateCall> PredicateCall =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from args in TermParser.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new PredicateCall(name.ToStringValue(), args.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, PredicateCall> GroundPredicateCall =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from args in Constant.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new PredicateCall(name.ToStringValue(), args.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, FluentCall> ParenthesizedFluentCall =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from args in TermParser.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new FluentCall(name.ToStringValue(), args.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, FluentCall> BareFluentCall =
        NonReservedIdentifier.Select(name => new FluentCall(name.ToStringValue(), ImmutableArray<Term>.Empty));

    private static readonly TokenListParser<PddlToken, FluentCall> FluentCall =
        ParenthesizedFluentCall.Try().Or(BareFluentCall);

    private static readonly TokenListParser<PddlToken, FluentCall> GroundParenthesizedFluentCall =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from args in Constant.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new FluentCall(name.ToStringValue(), args.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, FluentCall> GroundBareFluentCall =
        NonReservedIdentifier.Select(name => new FluentCall(name.ToStringValue(), ImmutableArray<Term>.Empty));

    private static readonly TokenListParser<PddlToken, FluentCall> GroundFluentCall =
        GroundParenthesizedFluentCall.Try().Or(GroundBareFluentCall);

    private static readonly TokenListParser<PddlToken, string> UnsupportedEitherType =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from either in Keyword("either")
        select Throw<string>(
            PddlParseErrorCode.UnsupportedFeature,
            "Union types using '(either ...)' are not supported.",
            either);

    private static readonly TokenListParser<PddlToken, string> TypeName =
        UnsupportedEitherType.Try()
            .Or(NonReservedIdentifier.Select(type => type.ToStringValue()));

    private static readonly TokenListParser<PddlToken, ImmutableArray<Parameter>> TypedParameterGroup =
        from vars in Token.EqualTo(PddlToken.Variable).AtLeastOnce()
        from dash in Token.EqualTo(PddlToken.Dash)
        from typeName in TypeName
        select vars.Select(v => new Parameter(v.ToStringValue(), typeName)).ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<Parameter>> UntypedParameterGroup =
        from vars in Token.EqualTo(PddlToken.Variable).AtLeastOnce()
        select vars.Select(v => new Parameter(v.ToStringValue())).ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<Parameter>> ParameterGroup =
        TypedParameterGroup.Try().Or(UntypedParameterGroup);

    private static readonly TokenListParser<PddlToken, ImmutableArray<Parameter>> ParametersList =
        ParameterGroup.Many().Select(groups => groups.SelectMany(g => g).ToImmutableArray());

    // PDDL expressions are recursive and intentionally have no artificial depth cap. Pathological
    // caller-supplied nesting may exhaust the process stack; guarding that input is out of scope.
    private static TokenListParser<PddlToken, ILogicalExpression> LogicalExpressionInternal { get; set; } = null!;
    internal static readonly TokenListParser<PddlToken, ILogicalExpression> LogicalExpression = Parse.Ref(() => LogicalExpressionInternal);

    private static readonly TokenListParser<PddlToken, EqualityOperand> EqualityOperandParser =
        Variable.Select(term => EqualityOperand.FromTerm(term))
            .Or(NonReservedIdentifier
                .Select(token => EqualityOperand.FromBareName(token.ToStringValue())))
            .Or(Parse.Ref(() => NumericExpressionInternal!)
                .Select(expression => EqualityOperand.FromNumeric(expression)));

    private static readonly TokenListParser<PddlToken, ILogicalExpression> EqualityExpression =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from eq in Token.EqualTo(PddlToken.Equals)
        from left in EqualityOperandParser
        from right in EqualityOperandParser
        from close in Token.EqualTo(PddlToken.CloseParen)
        select BuildEquality(left, right, eq);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> And =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("and")
        from exprs in LogicalExpression.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new And(exprs.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, ILogicalExpression> Or =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("or")
        from exprs in LogicalExpression.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Or(exprs.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, ILogicalExpression> Not =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("not")
        from expr in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Not(expr);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> Imply =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("imply")
        from ant in LogicalExpression
        from cons in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Imply(ant, cons);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> Forall =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("forall")
        from pOpen in Token.EqualTo(PddlToken.OpenParen)
        from vars in ParametersList
        from pClose in Token.EqualTo(PddlToken.CloseParen)
        from body in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Forall(vars, body);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> Exists =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("exists")
        from pOpen in Token.EqualTo(PddlToken.OpenParen)
        from vars in ParametersList
        from pClose in Token.EqualTo(PddlToken.CloseParen)
        from body in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Exists(vars, body);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> NumericComparison =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.GreaterThan)
            .Or(Token.EqualTo(PddlToken.LessThan))
            .Or(Token.EqualTo(PddlToken.GreaterThanOrEqual))
            .Or(Token.EqualTo(PddlToken.LessThanOrEqual))
        from left in Parse.Ref(() => NumericExpressionInternal!)
        from right in Parse.Ref(() => NumericExpressionInternal!)
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new Comparison(ToComparisonOperator(op.Kind), left, right);

    private static readonly TokenListParser<PddlToken, ILogicalExpression> EmptyLogic =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (ILogicalExpression)new EmptyLogic();

    private static TokenListParser<PddlToken, INumericExpression> NumericExpressionInternal { get; set; } = null!;
    internal static readonly TokenListParser<PddlToken, INumericExpression> NumericExpression = Parse.Ref(() => NumericExpressionInternal);

    private static readonly TokenListParser<PddlToken, NumberLiteral> NumberLiteralValue =
        Token.EqualTo(PddlToken.Number)
            .Select(ParseNumberLiteral);

    private static readonly TokenListParser<PddlToken, INumericExpression> NumberLiteralExpression =
        NumberLiteralValue.Select(literal => (INumericExpression)literal);

    private static readonly TokenListParser<PddlToken, INumericExpression> NumericAdd =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Plus)
        from first in NumericExpression
        from remaining in NumericExpression.AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select remaining.Aggregate(
            first,
            static (left, right) => (INumericExpression)new Add(left, right));

    private static readonly TokenListParser<PddlToken, INumericExpression> NumericMinus =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Dash)
        from left in NumericExpression
        from right in NumericExpression.AsNullable().OptionalOrDefault()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select right is null
            ? (INumericExpression)new Negate(left)
            : new Subtract(left, right);

    private static readonly TokenListParser<PddlToken, INumericExpression> NumericMultiply =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Asterisk)
        from first in NumericExpression
        from remaining in NumericExpression.AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select remaining.Aggregate(
            first,
            static (left, right) => (INumericExpression)new Multiply(left, right));

    private static readonly TokenListParser<PddlToken, INumericExpression> NumericDivide =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Slash)
        from left in NumericExpression
        from right in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (INumericExpression)new Divide(left, right);

    private static TokenListParser<PddlToken, INumericExpression> GroundNumericExpressionInternal { get; set; } = null!;
    private static readonly TokenListParser<PddlToken, INumericExpression> GroundNumericExpression =
        Parse.Ref(() => GroundNumericExpressionInternal);

    private static readonly TokenListParser<PddlToken, INumericExpression> GroundNumericAdd =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Plus)
        from first in GroundNumericExpression
        from remaining in GroundNumericExpression.AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select remaining.Aggregate(
            first,
            static (left, right) => (INumericExpression)new Add(left, right));

    private static readonly TokenListParser<PddlToken, INumericExpression> GroundNumericMinus =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Dash)
        from left in GroundNumericExpression
        from right in GroundNumericExpression.AsNullable().OptionalOrDefault()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select right is null
            ? (INumericExpression)new Negate(left)
            : new Subtract(left, right);

    private static readonly TokenListParser<PddlToken, INumericExpression> GroundNumericMultiply =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Asterisk)
        from first in GroundNumericExpression
        from remaining in GroundNumericExpression.AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select remaining.Aggregate(
            first,
            static (left, right) => (INumericExpression)new Multiply(left, right));

    private static readonly TokenListParser<PddlToken, INumericExpression> GroundNumericDivide =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from op in Token.EqualTo(PddlToken.Slash)
        from left in GroundNumericExpression
        from right in GroundNumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (INumericExpression)new Divide(left, right);

    private static readonly TokenListParser<PddlToken, INumericExpression> UnsupportedGroundMetricTotalTime =
        Keyword("total-time")
            .Select(token => Throw<INumericExpression>(
                PddlParseErrorCode.UnsupportedFeature,
                "Unsupported PDDL metric expression 'total-time'.",
                token));

    private static TokenListParser<PddlToken, IEffect> EffectInternal { get; set; } = null!;
    internal static readonly TokenListParser<PddlToken, IEffect> Effect = Parse.Ref(() => EffectInternal);

    private static readonly TokenListParser<PddlToken, IEffect> AddEffect =
        PredicateCall.Select(p => (IEffect)new AddEffect(p));

    private static readonly TokenListParser<PddlToken, IEffect> DeleteEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("not")
        from pred in PredicateCall
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new DeleteEffect(pred);

    private static readonly TokenListParser<PddlToken, IEffect> AssignEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("assign")
        from fluent in FluentCall
        from val in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new Assign(fluent, val);

    private static readonly TokenListParser<PddlToken, IEffect> IncreaseEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("increase")
        from fluent in FluentCall
        from val in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new Increase(fluent, val);

    private static readonly TokenListParser<PddlToken, IEffect> DecreaseEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("decrease")
        from fluent in FluentCall
        from val in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new Decrease(fluent, val);

    private static readonly TokenListParser<PddlToken, IEffect> ScaleUpEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("scale-up")
        from fluent in FluentCall
        from val in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new ScaleUp(fluent, val);

    private static readonly TokenListParser<PddlToken, IEffect> ScaleDownEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("scale-down")
        from fluent in FluentCall
        from val in NumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new ScaleDown(fluent, val);

    private static readonly TokenListParser<PddlToken, IEffect> ConditionalEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("when")
        from cond in LogicalExpression
        from eff in Effect
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new ConditionalEffect(cond, eff);

    private static readonly TokenListParser<PddlToken, IEffect> AndEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("and")
        from effs in Effect.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new AndEffect(effs.ToImmutableArray());

    private static readonly TokenListParser<PddlToken, IEffect> EmptyEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new AndEffect(ImmutableArray<IEffect>.Empty);

    private static readonly TokenListParser<PddlToken, IEffect> ForallEffect =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in Keyword("forall")
        from pOpen in Token.EqualTo(PddlToken.OpenParen)
        from vars in ParametersList
        from pClose in Token.EqualTo(PddlToken.CloseParen)
        from eff in Effect
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IEffect)new ForallEffect(vars, eff);

    private static readonly TokenListParser<PddlToken, PredicateDeclaration> PredicateDecl =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from parameters in ParametersList
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new PredicateDeclaration(name.ToStringValue(), parameters);

    private static readonly TokenListParser<PddlToken, FunctionDeclaration> FunctionDecl =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from name in NonReservedIdentifier
        from parameters in ParametersList
        from close in Token.EqualTo(PddlToken.CloseParen)
        from optionalType in (
            from dash in Token.EqualTo(PddlToken.Dash)
            from typeName in NonReservedIdentifier
                .Or(UnsupportedEitherType.Select(_ => Token<PddlToken>.Empty))
            select typeName
        ).OptionalOrDefault()
        select BuildFunctionDeclaration(name, parameters, optionalType);

    private static readonly TokenListParser<PddlToken, ImmutableArray<TypeDeclaration>> TypedIdentifierGroup =
        from ids in NonReservedIdentifier.AtLeastOnce()
        from dash in Token.EqualTo(PddlToken.Dash)
        from parentType in TypeName
        select ids.Select(id => new TypeDeclaration(id.ToStringValue(), parentType)).ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<TypeDeclaration>> UntypedIdentifierGroup =
        from ids in NonReservedIdentifier.AtLeastOnce()
        select ids.Select(id => new TypeDeclaration(id.ToStringValue())).ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<TypeDeclaration>> IdentifierGroup =
        TypedIdentifierGroup.Try().Or(UntypedIdentifierGroup);

    private static readonly TokenListParser<PddlToken, ImmutableArray<PddlRequirement>> RequirementsTail =
        from reqs in Token.EqualTo(PddlToken.Identifier).AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select ParseRequirements(reqs);

    private static readonly TokenListParser<PddlToken, ImmutableArray<TypeDeclaration>> TypeListTail =
        from groups in IdentifierGroup.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select groups.SelectMany(g => g).ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<PredicateDeclaration>> PredicatesTail =
        from preds in PredicateDecl.AtLeastOnce()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select preds.ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<FunctionDeclaration>> FunctionsTail =
        from funcs in FunctionDecl.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select funcs.ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ImmutableArray<Parameter>> ActionParametersTail =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from parameters in ParametersList
        from close in Token.EqualTo(PddlToken.CloseParen)
        select parameters;

    private static readonly TokenListParser<PddlToken, ActionPart> ActionPartParser =
        from keyword in Token.EqualTo(PddlToken.Identifier)
        from part in ActionPartTail(keyword)
        select part;

    private static TokenListParser<PddlToken, ActionPart> ActionPartTail(Token<PddlToken> keyword)
    {
        PddlSourceSpan span = ToSpan(keyword);
        return keyword.ToStringValue().ToLowerInvariant() switch
        {
            ":parameters" => ActionParametersTail
                .Select(parameters => (ActionPart)new ActionPart.Parameters(parameters, span)),
            ":precondition" => LogicalExpression
                .Select(expression => (ActionPart)new ActionPart.Precondition(expression, span)),
            ":effect" => Effect.Select(effect => (ActionPart)new ActionPart.Effect(effect, span)),
            ":duration" or ":condition" => Fail<ActionPart>(
                PddlParseErrorCode.UnsupportedFeature,
                $"Unsupported PDDL action construct '{keyword.ToStringValue()}'.",
                keyword),
            _ => Fail<ActionPart>(
                PddlParseErrorCode.UnknownConstruct,
                $"Unknown PDDL action construct '{keyword.ToStringValue()}'.",
                keyword)
        };
    }

    private static readonly TokenListParser<PddlToken, ActionDefinition> ActionTail =
        from name in NonReservedIdentifier
        from parts in ActionPartParser.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select BuildAction(name, parts, close);

    private static readonly TokenListParser<PddlToken, DerivedPredicate> DerivedPredicateTail =
        from sig in PredicateDecl
        from body in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new DerivedPredicate(sig, body);

    private static readonly TokenListParser<PddlToken, IProblemInitElement> NumericInitialization =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from eq in Token.EqualTo(PddlToken.Equals)
        from fluent in GroundFluentCall
        from value in NumberLiteralValue
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IProblemInitElement)new NumericInitialization(fluent, value);

    private static readonly TokenListParser<PddlToken, IProblemInitElement> NegativeInitialization =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from not in Keyword("not")
        from predicate in GroundPredicateCall
        from close in Token.EqualTo(PddlToken.CloseParen)
        select (IProblemInitElement)new NegativePredicateInitialization(predicate);

    private static readonly TokenListParser<PddlToken, IProblemInitElement> InitElement =
        NumericInitialization.Try()
            .Or(NegativeInitialization.Try())
            .Or(GroundPredicateCall.Select(predicate => (IProblemInitElement)predicate));

    private static readonly TokenListParser<PddlToken, ImmutableArray<IProblemInitElement>> InitTail =
        from exprs in InitElement.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select exprs.ToImmutableArray();

    private static readonly TokenListParser<PddlToken, ILogicalExpression> GoalTail =
        from expr in LogicalExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select expr;

    private static readonly TokenListParser<PddlToken, Metric> MetricTail =
        from dir in Keyword("minimize").Select(_ => MetricDirection.Minimize)
            .Or(Keyword("maximize").Select(_ => MetricDirection.Maximize))
        from expr in GroundNumericExpression
        from close in Token.EqualTo(PddlToken.CloseParen)
        select new Metric(dir, expr);

    private static readonly TokenListParser<PddlToken, DomainSection> DomainSectionParser =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from keyword in Token.EqualTo(PddlToken.Identifier)
        from section in DomainSectionTail(keyword)
        select section;

    private static TokenListParser<PddlToken, DomainSection> DomainSectionTail(Token<PddlToken> keyword)
    {
        PddlSourceSpan span = ToSpan(keyword);
        return keyword.ToStringValue().ToLowerInvariant() switch
        {
            ":requirements" => RequirementsTail.Select(items => (DomainSection)new DomainSection.Requirements(items, span)),
            ":types" => TypeListTail.Select(items => (DomainSection)new DomainSection.Types(items, span)),
            ":constants" => TypeListTail.Select(items => (DomainSection)new DomainSection.Constants(items, span)),
            ":predicates" => PredicatesTail.Select(items => (DomainSection)new DomainSection.Predicates(items, span)),
            ":functions" => FunctionsTail.Select(items => (DomainSection)new DomainSection.Functions(items, span)),
            ":action" => ActionTail.Select(action => (DomainSection)new DomainSection.Action(action, span)),
            ":derived" => DerivedPredicateTail.Select(derived => (DomainSection)new DomainSection.Derived(derived, span)),
            ":durative-action" or ":constraints" or ":timeless" or ":safety" =>
                Fail<DomainSection>(PddlParseErrorCode.UnsupportedFeature,
                    $"Unsupported PDDL domain construct '{keyword.ToStringValue()}'.", keyword),
            _ => Fail<DomainSection>(PddlParseErrorCode.UnknownConstruct,
                $"Unknown PDDL domain construct '{keyword.ToStringValue()}'.", keyword)
        };
    }

    internal static readonly TokenListParser<PddlToken, DomainDefinition> DomainDecl =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from def in Keyword("define")
        from dOpen in Token.EqualTo(PddlToken.OpenParen)
        from dom in Keyword("domain")
        from name in NonReservedIdentifier
        from dClose in Token.EqualTo(PddlToken.CloseParen)
        from sections in DomainSectionParser.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select BuildDomain(name.ToStringValue(), sections);

    private static readonly TokenListParser<PddlToken, string> ProblemDomainNameTail =
        from domainName in NonReservedIdentifier
        from close in Token.EqualTo(PddlToken.CloseParen)
        select domainName.ToStringValue();

    private static readonly TokenListParser<PddlToken, ProblemSection> ProblemSectionParser =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from keyword in Token.EqualTo(PddlToken.Identifier)
        from section in ProblemSectionTail(keyword)
        select section;

    private static TokenListParser<PddlToken, ProblemSection> ProblemSectionTail(Token<PddlToken> keyword)
    {
        PddlSourceSpan span = ToSpan(keyword);
        return keyword.ToStringValue().ToLowerInvariant() switch
        {
            ":domain" => ProblemDomainNameTail.Select(name => (ProblemSection)new ProblemSection.DomainName(name, span)),
            ":requirements" => RequirementsTail.Select(items => (ProblemSection)new ProblemSection.Requirements(items, span)),
            ":objects" => TypeListTail.Select(items => (ProblemSection)new ProblemSection.Objects(items, span)),
            ":init" => InitTail.Select(items => (ProblemSection)new ProblemSection.Init(items, span)),
            ":goal" => GoalTail.Select(goal => (ProblemSection)new ProblemSection.Goal(goal, span)),
            ":metric" => MetricTail.Select(metric => (ProblemSection)new ProblemSection.Metric(metric, span)),
            ":constraints" or ":preferences" =>
                Fail<ProblemSection>(PddlParseErrorCode.UnsupportedFeature,
                    $"Unsupported PDDL problem construct '{keyword.ToStringValue()}'.", keyword),
            _ => Fail<ProblemSection>(PddlParseErrorCode.UnknownConstruct,
                $"Unknown PDDL problem construct '{keyword.ToStringValue()}'.", keyword)
        };
    }

    internal static readonly TokenListParser<PddlToken, ProblemDefinition> ProblemDecl =
        from open in Token.EqualTo(PddlToken.OpenParen)
        from def in Keyword("define")
        from pOpen in Token.EqualTo(PddlToken.OpenParen)
        from prob in Keyword("problem")
        from name in NonReservedIdentifier
        from pClose in Token.EqualTo(PddlToken.CloseParen)
        from sections in ProblemSectionParser.Many()
        from close in Token.EqualTo(PddlToken.CloseParen)
        select BuildProblem(name.ToStringValue(), sections, close);

    static PddlGrammar()
    {
        LogicalExpressionInternal =
            And.Try()
            .Or(Or.Try())
            .Or(Not.Try())
            .Or(Imply.Try())
            .Or(Forall.Try())
            .Or(Exists.Try())
            .Or(EqualityExpression.Try())
            .Or(NumericComparison.Try())
            .Or(UnsupportedExpression<ILogicalExpression>().Try())
            .Or(PredicateCall.Select(p => (ILogicalExpression)p).Try())
            .Or(EmptyLogic);

        NumericExpressionInternal =
            NumericAdd.Try()
            .Or(NumericMinus.Try())
            .Or(NumericMultiply.Try())
            .Or(NumericDivide.Try())
            .Or(UnsupportedExpression<INumericExpression>().Try())
            .Or(FluentCall.Select(f => (INumericExpression)f).Try())
            .Or(NumberLiteralExpression);

        GroundNumericExpressionInternal =
            GroundNumericAdd.Try()
            .Or(GroundNumericMinus.Try())
            .Or(GroundNumericMultiply.Try())
            .Or(GroundNumericDivide.Try())
            .Or(UnsupportedExpression<INumericExpression>().Try())
            .Or(UnsupportedGroundMetricTotalTime.Try())
            .Or(GroundFluentCall.Select(f => (INumericExpression)f).Try())
            .Or(NumberLiteralExpression);

        EffectInternal =
            AndEffect.Try()
            .Or(ForallEffect.Try())
            .Or(ConditionalEffect.Try())
            .Or(AssignEffect.Try())
            .Or(IncreaseEffect.Try())
            .Or(DecreaseEffect.Try())
            .Or(ScaleUpEffect.Try())
            .Or(ScaleDownEffect.Try())
            .Or(DeleteEffect.Try())
            .Or(UnsupportedExpression<IEffect>().Try())
            .Or(EmptyEffect.Try())
            .Or(AddEffect);
    }

    private static TokenListParser<PddlToken, T> Fail<T>(
        PddlParseErrorCode errorCode,
        string message,
        Token<PddlToken> token)
    {
        return Token.EqualTo(PddlToken.OpenParen)
            .Try()
            .OptionalOrDefault()
            .Select(_ => Throw<T>(errorCode, message, token));
    }

    private static T Throw<T>(
        PddlParseErrorCode errorCode,
        string message,
        Token<PddlToken> token)
    {
        throw new PddlGrammarException(errorCode, message, ToSpan(token));
    }

    private static PddlSourceSpan ToSpan(Token<PddlToken> token)
    {
        return new PddlSourceSpan(
            token.Position.Absolute,
            token.Span.Length,
            token.Position.Line,
            token.Position.Column);
    }

    private static ComparisonOperator ToComparisonOperator(PddlToken token) => token switch
    {
        PddlToken.Equals => ComparisonOperator.Equal,
        PddlToken.LessThan => ComparisonOperator.LessThan,
        PddlToken.LessThanOrEqual => ComparisonOperator.LessThanOrEqual,
        PddlToken.GreaterThan => ComparisonOperator.GreaterThan,
        PddlToken.GreaterThanOrEqual => ComparisonOperator.GreaterThanOrEqual,
        _ => throw new InvalidOperationException($"Token '{token}' is not a comparison operator.")
    };

    private static ILogicalExpression BuildEquality(
        EqualityOperand left,
        EqualityOperand right,
        Token<PddlToken> equalsToken)
    {
        if (left.BareName != null && right.BareName != null)
            return new AmbiguousEquality(left.BareName, right.BareName);

        if (left.Numeric != null || right.Numeric != null)
        {
            if (left.Term != null || right.Term != null)
            {
                throw new PddlGrammarException(
                    PddlParseErrorCode.Syntax,
                    "Equality operands cannot mix variables with numeric expressions.",
                    ToSpan(equalsToken));
            }

            return new Comparison(
                ComparisonOperator.Equal,
                ToNumericEqualityOperand(left),
                ToNumericEqualityOperand(right));
        }

        return new Equality(ToTermEqualityOperand(left), ToTermEqualityOperand(right));
    }

    private static INumericExpression ToNumericEqualityOperand(EqualityOperand operand)
    {
        return operand.Numeric
            ?? new FluentCall(operand.BareName!, ImmutableArray<Term>.Empty);
    }

    private static Term ToTermEqualityOperand(EqualityOperand operand)
    {
        return operand.Term ?? Term.Constant(operand.BareName!);
    }

    private static NumberLiteral ParseNumberLiteral(Token<PddlToken> token)
    {
        string text = token.ToStringValue();
        bool negative = text[0] == '-';
        int start = text[0] is '-' or '+' ? 1 : 0;
        int decimalPoint = text.IndexOf('.', start);
        int scale = decimalPoint < 0 ? 0 : text.Length - decimalPoint - 1;
        string digits = decimalPoint < 0
            ? text[start..]
            : string.Concat(text.AsSpan(start, decimalPoint - start), text.AsSpan(decimalPoint + 1));

        int coefficientLength = digits.Length;
        while (scale > 0 && digits[coefficientLength - 1] == '0')
        {
            coefficientLength--;
            scale--;
        }

        digits = digits[..coefficientLength].TrimStart('0');
        if (digits.Length == 0)
            digits = "0";

        BigInteger coefficient = BigInteger.Parse(digits, CultureInfo.InvariantCulture);
        BigInteger maximumCoefficient = BigInteger.Parse(
            "79228162514264337593543950335",
            CultureInfo.InvariantCulture);

        if (scale > 28 || coefficient > maximumCoefficient)
        {
            throw new PddlGrammarException(
                PddlParseErrorCode.InvalidNumber,
                $"Numeric literal '{text}' is outside the exact decimal range.",
                ToSpan(token));
        }

        int low = unchecked((int)(uint)(coefficient & uint.MaxValue));
        int middle = unchecked((int)(uint)((coefficient >> 32) & uint.MaxValue));
        int high = unchecked((int)(uint)((coefficient >> 64) & uint.MaxValue));
        return new NumberLiteral(new decimal(low, middle, high, negative, (byte)scale));
    }

    private static PddlRequirement ParseRequirement(Token<PddlToken> token)
    {
        return token.ToStringValue().ToLowerInvariant() switch
        {
            ":strips" => PddlRequirement.Strips,
            ":typing" => PddlRequirement.Typing,
            ":equality" => PddlRequirement.Equality,
            ":negative-preconditions" => PddlRequirement.NegativePreconditions,
            ":disjunctive-preconditions" => PddlRequirement.DisjunctivePreconditions,
            ":fluents" or ":numeric-fluents" => PddlRequirement.NumericFluents,
            ":conditional-effects" => PddlRequirement.ConditionalEffects,
            ":existential-preconditions" => PddlRequirement.ExistentialPreconditions,
            ":universal-preconditions" => PddlRequirement.UniversalPreconditions,
            ":adl" => PddlRequirement.Adl,
            ":derived-predicates" => PddlRequirement.DerivedPredicates,
            ":action-costs" => PddlRequirement.ActionCosts,
            _ => Throw<PddlRequirement>(
                PddlParseErrorCode.UnsupportedFeature,
                $"Unsupported PDDL requirement '{token.ToStringValue()}'.",
                token)
        };
    }

    private static ImmutableArray<PddlRequirement> ParseRequirements(Token<PddlToken>[] tokens)
    {
        var requirements = ImmutableArray.CreateBuilder<PddlRequirement>();

        foreach (Token<PddlToken> token in tokens)
        {
            if (token.ToStringValue().Equals(
                ":quantified-preconditions",
                StringComparison.OrdinalIgnoreCase))
            {
                requirements.Add(PddlRequirement.ExistentialPreconditions);
                requirements.Add(PddlRequirement.UniversalPreconditions);
                continue;
            }

            requirements.Add(ParseRequirement(token));
        }

        return requirements.ToImmutable();
    }

    private static FunctionDeclaration BuildFunctionDeclaration(
        Token<PddlToken> name,
        ImmutableArray<Parameter> parameters,
        Token<PddlToken> resultType)
    {
        if (resultType.HasValue
            && !resultType.ToStringValue().Equals("number", StringComparison.OrdinalIgnoreCase))
        {
            throw new PddlGrammarException(
                PddlParseErrorCode.UnsupportedFeature,
                $"Object-valued function result type '{resultType.ToStringValue()}' is not supported.",
                ToSpan(resultType));
        }

        return new FunctionDeclaration(name.ToStringValue(), parameters);
    }

    private static ActionDefinition BuildAction(
        Token<PddlToken> name,
        IEnumerable<ActionPart> parts,
        Token<PddlToken> close)
    {
        ImmutableArray<Parameter> parameters = ImmutableArray<Parameter>.Empty;
        ILogicalExpression? precondition = null;
        IEffect? effect = null;
        bool hasParameters = false;
        bool hasPrecondition = false;
        bool hasEffect = false;

        foreach (ActionPart part in parts)
        {
            switch (part)
            {
                case ActionPart.Parameters value:
                    RejectDuplicate(ref hasParameters, ":parameters", value.Span);
                    parameters = value.Items;
                    break;
                case ActionPart.Precondition value:
                    RejectDuplicate(ref hasPrecondition, ":precondition", value.Span);
                    precondition = value.Item;
                    break;
                case ActionPart.Effect value:
                    RejectDuplicate(ref hasEffect, ":effect", value.Span);
                    effect = value.Item;
                    break;
            }
        }

        if (!hasParameters)
        {
            throw new PddlGrammarException(
                PddlParseErrorCode.MissingSection,
                $"Action '{name.ToStringValue()}' is missing required section ':parameters'.",
                ToSpan(close));
        }

        return new ActionDefinition(name.ToStringValue(), parameters, precondition, effect);
    }

    private static void RejectDuplicate(ref bool present, string sectionName, PddlSourceSpan span)
    {
        if (present)
        {
            throw new PddlGrammarException(
                PddlParseErrorCode.DuplicateSection,
                $"Section '{sectionName}' may only appear once.",
                span);
        }

        present = true;
    }

    private static DomainDefinition BuildDomain(string name, IEnumerable<DomainSection> sections)
    {
        ImmutableArray<PddlRequirement> requirements = ImmutableArray<PddlRequirement>.Empty;
        ImmutableArray<TypeDeclaration> types = ImmutableArray<TypeDeclaration>.Empty;
        ImmutableArray<TypeDeclaration> constants = ImmutableArray<TypeDeclaration>.Empty;
        ImmutableArray<PredicateDeclaration> predicates = ImmutableArray<PredicateDeclaration>.Empty;
        ImmutableArray<FunctionDeclaration> functions = ImmutableArray<FunctionDeclaration>.Empty;
        var actions = ImmutableArray.CreateBuilder<ActionDefinition>();
        var derivedPredicates = ImmutableArray.CreateBuilder<DerivedPredicate>();
        bool hasRequirements = false;
        bool hasTypes = false;
        bool hasConstants = false;
        bool hasPredicates = false;
        bool hasFunctions = false;

        foreach (DomainSection section in sections)
        {
            switch (section)
            {
                case DomainSection.Requirements r:
                    RejectDuplicate(ref hasRequirements, ":requirements", r.Span);
                    requirements = r.Items;
                    break;
                case DomainSection.Types t:
                    RejectDuplicate(ref hasTypes, ":types", t.Span);
                    types = t.Items;
                    break;
                case DomainSection.Constants c:
                    RejectDuplicate(ref hasConstants, ":constants", c.Span);
                    constants = c.Items;
                    break;
                case DomainSection.Predicates p:
                    RejectDuplicate(ref hasPredicates, ":predicates", p.Span);
                    predicates = p.Items;
                    break;
                case DomainSection.Functions f:
                    RejectDuplicate(ref hasFunctions, ":functions", f.Span);
                    functions = f.Items;
                    break;
                case DomainSection.Action a:
                    actions.Add(a.Item);
                    break;
                case DomainSection.Derived d:
                    derivedPredicates.Add(d.Item);
                    break;
            }
        }

        return new DomainDefinition(
            name,
            requirements,
            types,
            constants,
            predicates,
            functions,
            actions.ToImmutable(),
            derivedPredicates.ToImmutable());
    }

    private static ProblemDefinition BuildProblem(
        string name,
        IEnumerable<ProblemSection> sections,
        Token<PddlToken> close)
    {
        string? domainName = null;
        ImmutableArray<PddlRequirement> requirements = ImmutableArray<PddlRequirement>.Empty;
        ImmutableArray<TypeDeclaration> objects = ImmutableArray<TypeDeclaration>.Empty;
        ImmutableArray<IProblemInitElement> init = ImmutableArray<IProblemInitElement>.Empty;
        ILogicalExpression? goal = null;
        Metric? metric = null;
        bool hasDomain = false;
        bool hasRequirements = false;
        bool hasObjects = false;
        bool hasInit = false;
        bool hasGoal = false;
        bool hasMetric = false;

        foreach (ProblemSection section in sections)
        {
            switch (section)
            {
                case ProblemSection.DomainName d:
                    RejectDuplicate(ref hasDomain, ":domain", d.Span);
                    domainName = d.Item;
                    break;
                case ProblemSection.Requirements r:
                    RejectDuplicate(ref hasRequirements, ":requirements", r.Span);
                    requirements = r.Items;
                    break;
                case ProblemSection.Objects o:
                    RejectDuplicate(ref hasObjects, ":objects", o.Span);
                    objects = o.Items;
                    break;
                case ProblemSection.Init i:
                    RejectDuplicate(ref hasInit, ":init", i.Span);
                    init = i.Items;
                    break;
                case ProblemSection.Goal g:
                    RejectDuplicate(ref hasGoal, ":goal", g.Span);
                    goal = g.Item;
                    break;
                case ProblemSection.Metric m:
                    RejectDuplicate(ref hasMetric, ":metric", m.Span);
                    metric = m.Item;
                    break;
            }
        }

        PddlSourceSpan missingSpan = ToSpan(close);
        RequireProblemSection(hasDomain, ":domain", missingSpan);
        RequireProblemSection(hasInit, ":init", missingSpan);
        RequireProblemSection(hasGoal, ":goal", missingSpan);

        return new ProblemDefinition(name, domainName!, requirements, objects, init, goal!, metric);
    }

    private static void RequireProblemSection(bool present, string sectionName, PddlSourceSpan span)
    {
        if (!present)
        {
            throw new PddlGrammarException(
                PddlParseErrorCode.MissingSection,
                $"Problem is missing required section '{sectionName}'.",
                span);
        }
    }

    private sealed record EqualityOperand(
        Term? Term,
        string? BareName,
        INumericExpression? Numeric)
    {
        internal static EqualityOperand FromTerm(Term term) => new(term, null, null);
        internal static EqualityOperand FromBareName(string name) => new(null, name, null);
        internal static EqualityOperand FromNumeric(INumericExpression expression) => new(null, null, expression);
    }

    private abstract record ActionPart(PddlSourceSpan Span)
    {
        internal sealed record Parameters(
            ImmutableArray<Parameter> Items,
            PddlSourceSpan Span) : ActionPart(Span);

        internal sealed record Precondition(
            ILogicalExpression Item,
            PddlSourceSpan Span) : ActionPart(Span);

        internal sealed record Effect(
            IEffect Item,
            PddlSourceSpan Span) : ActionPart(Span);
    }

    private abstract record DomainSection(PddlSourceSpan Span)
    {
        public sealed record Requirements(
            ImmutableArray<PddlRequirement> Items,
            PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Types(ImmutableArray<TypeDeclaration> Items, PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Constants(ImmutableArray<TypeDeclaration> Items, PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Predicates(ImmutableArray<PredicateDeclaration> Items, PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Functions(ImmutableArray<FunctionDeclaration> Items, PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Action(ActionDefinition Item, PddlSourceSpan Span) : DomainSection(Span);
        public sealed record Derived(DerivedPredicate Item, PddlSourceSpan Span) : DomainSection(Span);
    }

    private abstract record ProblemSection(PddlSourceSpan Span)
    {
        public sealed record DomainName(string Item, PddlSourceSpan Span) : ProblemSection(Span);
        public sealed record Requirements(
            ImmutableArray<PddlRequirement> Items,
            PddlSourceSpan Span) : ProblemSection(Span);
        public sealed record Objects(ImmutableArray<TypeDeclaration> Items, PddlSourceSpan Span) : ProblemSection(Span);
        public sealed record Init(ImmutableArray<IProblemInitElement> Items, PddlSourceSpan Span) : ProblemSection(Span);
        public sealed record Goal(ILogicalExpression Item, PddlSourceSpan Span) : ProblemSection(Span);
        public sealed record Metric(Mimir.Pddl.Ast.Models.Metric Item, PddlSourceSpan Span) : ProblemSection(Span);
    }
}
