using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using CoreConditionalEffect = Mimir.Core.Schemas.ConditionalEffect;
using DerivedPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Derived>;
using FluentPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Fluent>;
using StaticPredicate = Mimir.Core.Schemas.Predicate<Mimir.Core.Schemas.Static>;

namespace Mimir.Core.Tests;

public sealed class ProgrammaticDomainValidationTests
{
    [Fact]
    public void ManualDomainRejectsUndeclaredPredicateInActionPrecondition()
    {
        var declared = new FluentPredicate("declared", Array.Empty<Variable>());
        var foreign = new FluentPredicate("foreign", Array.Empty<Variable>());
        var precondition = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(foreign, Array.Empty<ITerm>()),
            Polarity.Positive);
        ActionSchema action = CreateAction("act", fluentPreconditions: [precondition]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(fluents: [declared], actions: [action]));

        Assert.Contains("Undeclared predicate 'foreign'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNamePredicateSubstituteInActionPrecondition()
    {
        var declared = new FluentPredicate("ready", Array.Empty<Variable>());
        var referenced = new FluentPredicate("READY", Array.Empty<Variable>());
        var precondition = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(referenced, Array.Empty<ITerm>()),
            Polarity.Positive);
        ActionSchema action = CreateAction("act", fluentPreconditions: [precondition]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(fluents: [declared], actions: [action]));

        Assert.Contains("Predicate 'READY' must use the exact declared instance", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsPredicateWithDeclaredNameButWrongType()
    {
        var declared = new StaticPredicate("ready", Array.Empty<Variable>());
        var referenced = new FluentPredicate("READY", Array.Empty<Variable>());
        var precondition = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(referenced, Array.Empty<ITerm>()),
            Polarity.Positive);
        ActionSchema action = CreateAction("act", fluentPreconditions: [precondition]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(statics: [declared], actions: [action]));

        Assert.Contains("Predicate 'READY' type mismatch", exception.Message);
        Assert.Contains("Expected static, got fluent", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsUnboundVariableInActionEffect()
    {
        var predicateParameter = new Variable("?value");
        var effectVariable = new Variable("?missing");
        var predicate = new FluentPredicate("marked", [predicateParameter]);
        var effectLiteral = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(predicate, [effectVariable]),
            Polarity.Positive);
        var effect = new CoreConditionalEffect(
            Array.Empty<Variable>(),
            Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            effectLiteral);
        ActionSchema action = CreateAction("mark", effects: [effect]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(fluents: [predicate], actions: [action]));

        Assert.Contains("Undeclared variable '?missing'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameActionParameterSubstituteInEffect()
    {
        var predicateParameter = new Variable("?value");
        var actionParameter = new Variable("?item");
        var referencedParameter = new Variable("?ITEM");
        var predicate = new FluentPredicate("marked", [predicateParameter]);
        var effectLiteral = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(predicate, [referencedParameter]),
            Polarity.Positive);
        var effect = new CoreConditionalEffect(
            Array.Empty<Variable>(),
            Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            effectLiteral);
        ActionSchema action = CreateAction("mark", effects: [effect], parameters: [actionParameter]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(fluents: [predicate], actions: [action]));

        Assert.Contains(
            "Variable '?ITEM' must use the exact variable instance from the active scope",
            exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameQuantifierVariableSubstituteInEffect()
    {
        var predicateParameter = new Variable("?value");
        var quantifiedVariable = new Variable("?item");
        var referencedVariable = new Variable("?ITEM");
        var predicate = new FluentPredicate("marked", [predicateParameter]);
        var effectLiteral = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(predicate, [referencedVariable]),
            Polarity.Positive);
        var effect = new CoreConditionalEffect(
            [quantifiedVariable],
            Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            effectLiteral);
        ActionSchema action = CreateAction("mark", effects: [effect]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(fluents: [predicate], actions: [action]));

        Assert.Contains(
            "Variable '?ITEM' must use the exact variable instance from the active scope",
            exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsUndeclaredNumericFunctionInActionCost()
    {
        var foreignFunction = new NumericFunction("foreign-cost", Array.Empty<Variable>());
        ActionSchema action = CreateAction(
            "costly",
            costExpression: new FunctionCall(
                foreignFunction,
                Array.Empty<ITerm>()));
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(actions: [action], requirements: [":action-costs"]));

        Assert.Contains("Undeclared numeric function 'foreign-cost'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsActionCostDependingOnTotalCost()
    {
        var totalCost = new NumericFunction("total-cost", Array.Empty<Variable>());
        ActionSchema action = CreateAction(
            "costly",
            costExpression: new FunctionCall(
                totalCost,
                Array.Empty<ITerm>()));

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(
                actions: [action],
                requirements: [":action-costs"]));

        Assert.Contains("cannot be referenced in numeric expressions", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsReservedTotalCostFunction()
    {
        var totalCost = new NumericFunction("TOTAL-COST", Array.Empty<Variable>());

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(functions: [totalCost]));

        Assert.Equal("functions", exception.ParamName);
        Assert.Contains("reserved for planner bookkeeping", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameNumericFunctionSubstituteInActionCost()
    {
        var declaredFunction = new NumericFunction("fare", Array.Empty<Variable>());
        var referencedFunction = new NumericFunction("FARE", Array.Empty<Variable>());
        ActionSchema action = CreateAction(
            "costly",
            costExpression: new FunctionCall(
                referencedFunction,
                Array.Empty<ITerm>()));
        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(
                functions: [declaredFunction],
                actions: [action],
                requirements: [":action-costs"]));

        Assert.Contains("Numeric function 'FARE' must use the exact declared instance", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameActionParameterSubstituteInActionCostTerm()
    {
        var functionParameter = new Variable("?value");
        var actionParameter = new Variable("?item");
        var referencedParameter = new Variable("?ITEM");
        var function = new NumericFunction("fare", [functionParameter]);
        ActionSchema action = CreateAction(
            "costly",
            costExpression: new FunctionCall(
                function,
                [referencedParameter]),
            parameters: [actionParameter]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(
                functions: [function],
                actions: [action],
                requirements: [":action-costs"]));

        Assert.Contains(
            "Variable '?ITEM' must use the exact variable instance from the active scope",
            exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsUndeclaredPredicateInDerivedBody()
    {
        var derived = new DerivedPredicate("ready", Array.Empty<Variable>());
        var foreign = new FluentPredicate("foreign", Array.Empty<Variable>());
        var definition = new DerivedPredicateDefinition(
            derived,
            new GroundedAtom(foreign, Array.Empty<ITerm>()));

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(derivedDefinitions: [definition]));

        Assert.Contains("Undeclared predicate 'foreign'", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNamePredicateSubstituteBeforeDerivedCycleValidation()
    {
        var declared = new DerivedPredicate("ready", Array.Empty<Variable>());
        var referenced = new DerivedPredicate("READY", Array.Empty<Variable>());
        var definition = new DerivedPredicateDefinition(
            declared,
            new GroundedAtom(referenced, Array.Empty<ITerm>()));

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(derivedDefinitions: [definition]));

        Assert.Contains("Predicate 'READY' must use the exact declared instance", exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameParameterSubstituteInDerivedAtom()
    {
        var declaredParameter = new Variable("?item");
        var referencedParameter = new Variable("?ITEM");
        var staticPredicate = new StaticPredicate("same-value", [declaredParameter]);
        var derived = new DerivedPredicate("same", [declaredParameter]);
        var definition = new DerivedPredicateDefinition(
            derived,
            new GroundedAtom(staticPredicate, [referencedParameter]));

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(
                statics: [staticPredicate],
                derivedDefinitions: [definition],
                requirements: [":derived-predicates"]));

        Assert.Contains(
            "Variable '?ITEM' must use the exact variable instance from the active scope",
            exception.Message);
    }

    [Fact]
    public void ManualDomainRejectsSameNameConstantSubstituteInActionPrecondition()
    {
        var parameter = new Variable("?place");
        var predicate = new FluentPredicate("at", [parameter]);
        var declaredConstant = new Constant("depot", "object");
        var referencedConstant = new Constant("DEPOT", "object");
        var precondition = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(predicate, [referencedConstant]),
            Polarity.Positive);
        ActionSchema action = CreateAction("act", fluentPreconditions: [precondition]);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => CreateDomain(
                fluents: [predicate],
                constants: [declaredConstant],
                actions: [action]));

        Assert.Contains("Constant 'DEPOT' must use the exact declared instance", exception.Message);
    }

    [Fact]
    public void ManualDomainPreservesCanonicalSymbolInstances()
    {
        var predicateParameter = new Variable("?place");
        var declaredPredicate = new FluentPredicate("at", [predicateParameter]);
        var functionParameter = new Variable("?place");
        var declaredFunction = new NumericFunction("fare", [functionParameter]);
        var declaredConstant = new Constant("depot", "object");
        var precondition = new Literal<Atom<Fluent>>(
            new Atom<Fluent>(declaredPredicate, [declaredConstant]),
            Polarity.Positive);
        ActionSchema action = CreateAction(
            "act",
            fluentPreconditions: [precondition],
            costExpression: new FunctionCall(
                declaredFunction,
                [declaredConstant]));

        Domain domain = CreateDomain(
            fluents: [declaredPredicate],
            functions: [declaredFunction],
            constants: [declaredConstant],
            actions: [action],
            requirements: [":action-costs"]);

        Assert.Same(declaredPredicate, Assert.Single(domain.Fluents));
        Assert.Same(declaredFunction, Assert.Single(domain.Functions));
        Assert.Same(declaredConstant, Assert.Single(domain.Constants));
        Assert.Same(action, Assert.Single(domain.Actions));
        Assert.Single(domain.PddlDefinition.Actions);
    }

    private static ActionSchema CreateAction(
        string name,
        IReadOnlyList<Literal<Atom<Fluent>>>? fluentPreconditions = null,
        IReadOnlyList<CoreConditionalEffect>? effects = null,
        NumericExpression? costExpression = null,
        IReadOnlyList<Variable>? parameters = null)
    {
        return new ActionSchema(
            name,
            parameters ?? Array.Empty<Variable>(),
            fluentPreconditions ?? Array.Empty<Literal<Atom<Fluent>>>(),
            Array.Empty<Literal<Atom<Static>>>(),
            Array.Empty<Literal<Atom<Derived>>>(),
            [],
            effects ?? Array.Empty<CoreConditionalEffect>(),
            [],
            costExpression ?? new NumericConstant(1d));
    }

    private static Domain CreateDomain(
        IReadOnlyList<FluentPredicate>? fluents = null,
        IReadOnlyList<StaticPredicate>? statics = null,
        IReadOnlyList<DerivedPredicateDefinition>? derivedDefinitions = null,
        IReadOnlyList<NumericFunction>? functions = null,
        IReadOnlyList<Constant>? constants = null,
        IReadOnlyList<ActionSchema>? actions = null,
        IReadOnlyList<string>? requirements = null)
    {
        return new Domain(
            "manual",
            fluents ?? Array.Empty<FluentPredicate>(),
            statics ?? Array.Empty<StaticPredicate>(),
            derivedDefinitions ?? Array.Empty<DerivedPredicateDefinition>(),
            functions ?? Array.Empty<NumericFunction>(),
            constants ?? Array.Empty<Constant>(),
            actions ?? Array.Empty<ActionSchema>(),
            new Dictionary<string, string>(),
            requirements);
    }
}
