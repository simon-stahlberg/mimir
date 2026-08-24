using System;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Linq;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Effects;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Pddl.Analysis;

public sealed class PddlValidationException : Exception
{
    public PddlValidationException(string message) : base(message) { }
}

public static class SemanticValidator
{
    public static DomainDefinition ValidateDomain(DomainDefinition domain)
    {
        ArgumentNullException.ThrowIfNull(domain);
        return new Validator(domain, null).ValidateDomain();
    }

    public static ProblemDefinition ValidateProblem(DomainDefinition domain, ProblemDefinition problem)
    {
        ArgumentNullException.ThrowIfNull(domain);
        ArgumentNullException.ThrowIfNull(problem);

        DomainDefinition validatedDomain = ValidateDomain(domain);
        return new Validator(validatedDomain, problem).ValidateProblem();
    }

    private sealed class Validator
    {
        private readonly DomainDefinition _domain;
        private readonly ProblemDefinition? _problem;
        private readonly Dictionary<string, PredicateDeclaration> _predicates;
        private readonly Dictionary<string, FunctionDeclaration> _functions;
        private readonly Dictionary<string, string> _typeHierarchy;
        private readonly Dictionary<string, string> _constantsAndObjects;
        private readonly HashSet<string> _derivedPredicates;
        private readonly HashSet<string> _declaredTypes;

        public Validator(DomainDefinition domain, ProblemDefinition? problem)
        {
            _domain = domain;
            _problem = problem;

            _predicates = new Dictionary<string, PredicateDeclaration>(StringComparer.OrdinalIgnoreCase);
            foreach (PredicateDeclaration predicate in domain.Predicates)
            {
                _predicates.TryAdd(predicate.Name, predicate);
            }

            _derivedPredicates = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (DerivedPredicate derivedPredicate in domain.DerivedPredicates)
            {
                _derivedPredicates.Add(derivedPredicate.Signature.Name);
                _predicates.TryAdd(derivedPredicate.Signature.Name, derivedPredicate.Signature);
            }

            _functions = new Dictionary<string, FunctionDeclaration>(StringComparer.OrdinalIgnoreCase);
            foreach (FunctionDeclaration function in domain.Functions)
            {
                _functions.TryAdd(function.Name, function);
            }

            _declaredTypes = new HashSet<string>(StringComparer.OrdinalIgnoreCase) { "object" };
            foreach (TypeDeclaration type in domain.Types)
            {
                _declaredTypes.Add(type.Name);
            }

            _typeHierarchy = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (TypeDeclaration type in domain.Types)
            {
                _typeHierarchy.TryAdd(type.Name, type.ParentType);
            }

            _constantsAndObjects = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (TypeDeclaration constant in domain.Constants)
            {
                _constantsAndObjects.TryAdd(constant.Name, constant.ParentType);
            }

            if (problem is null)
            {
                return;
            }

            foreach (TypeDeclaration obj in problem.Objects)
            {
                _constantsAndObjects.TryAdd(obj.Name, obj.ParentType);
            }
        }

        public DomainDefinition ValidateDomain()
        {
            ValidateDeclarations(includeProblem: false);

            ImmutableArray<ActionDefinition>.Builder actions = ImmutableArray.CreateBuilder<ActionDefinition>(_domain.Actions.Length);
            foreach (ActionDefinition action in _domain.Actions)
            {
                actions.Add(ValidateAction(action));
            }

            ImmutableArray<DerivedPredicate>.Builder derived = ImmutableArray.CreateBuilder<DerivedPredicate>(_domain.DerivedPredicates.Length);
            foreach (DerivedPredicate predicate in _domain.DerivedPredicates)
            {
                derived.Add(ValidateDerivedPredicate(predicate));
            }

            return _domain with
            {
                Actions = actions.MoveToImmutable(),
                DerivedPredicates = derived.MoveToImmutable()
            };
        }

        public ProblemDefinition ValidateProblem()
        {
            if (_problem is null)
            {
                throw new InvalidOperationException("A problem is required for problem validation.");
            }

            ValidateDeclarations(includeProblem: true);
            if (!_problem.DomainName.Equals(_domain.Name, StringComparison.OrdinalIgnoreCase))
            {
                throw new PddlValidationException($"Problem '{_problem.Name}' references domain '{_problem.DomainName}', but validator was given domain '{_domain.Name}'.");
            }

            var scope = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            ValidateInitialState(_problem.Init, scope);
            ILogicalExpression goal = ValidateLogicalExpression(_problem.Goal, scope, "problem goal");
            if (_problem.Metric is not null)
            {
                ValidateMetric(_problem.Metric, scope);
            }

            return _problem with { Goal = goal };
        }

        private void ValidateDeclarations(bool includeProblem)
        {
            foreach (TypeDeclaration type in _domain.Types)
            {
                if (type.Name.Equals("object", StringComparison.OrdinalIgnoreCase))
                {
                    throw new PddlValidationException("The built-in type 'object' cannot be declared or reparented.");
                }
            }

            ValidateDuplicateNames(_domain.Types.Select(type => type.Name), "type");
            ValidateDuplicateNames(_domain.Constants.Select(constant => constant.Name), "constant");
            ValidateDuplicateNames(_domain.Predicates.Select(predicate => predicate.Name), "predicate");
            ValidateDuplicateNames(_domain.Functions.Select(function => function.Name), "numeric fluent");
            ValidateDuplicateNames(_domain.Actions.Select(action => action.Name), "action");
            foreach (string predicateName in _predicates.Keys)
            {
                if (_functions.ContainsKey(predicateName))
                {
                    throw new PddlValidationException($"Name '{predicateName}' is declared as both predicate and numeric fluent.");
                }
            }

            foreach (TypeDeclaration type in _domain.Types)
            {
                ValidateTypeIsDeclared(type.ParentType, $"type declaration '{type.Name}'");
            }
            ValidateTypeCycles();

            foreach (TypeDeclaration constant in _domain.Constants)
            {
                ValidateTypeIsDeclared(constant.ParentType, $"constant '{constant.Name}'");
            }
            foreach (PredicateDeclaration predicate in _domain.Predicates)
            {
                ValidateParameters(predicate.Parameters, $"predicate '{predicate.Name}'");
            }
            foreach (FunctionDeclaration function in _domain.Functions)
            {
                ValidateParameters(function.Parameters, $"numeric fluent '{function.Name}'");
                if (IsBuiltInTotalCost(function.Name) && !function.Parameters.IsDefaultOrEmpty)
                {
                    throw new PddlValidationException(
                        $"Arity mismatch for fluent '{function.Name}' in its declaration. Expected 0, got {function.Parameters.Length}.");
                }
            }
            foreach (ActionDefinition action in _domain.Actions)
            {
                ValidateParameters(action.Parameters, $"action '{action.Name}'");
            }
            foreach (DerivedPredicate derivedPredicate in _domain.DerivedPredicates)
            {
                ValidateParameters(derivedPredicate.Signature.Parameters, $"derived predicate '{derivedPredicate.Signature.Name}'");
            }
            ValidateDerivedSignatures();

            if (!includeProblem || _problem is null)
            {
                return;
            }

            ValidateDuplicateNames(_problem.Objects.Select(obj => obj.Name), "object");
            var constantNames = new HashSet<string>(
                _domain.Constants.Select(constant => constant.Name),
                StringComparer.OrdinalIgnoreCase);
            foreach (TypeDeclaration obj in _problem.Objects)
            {
                if (constantNames.Contains(obj.Name))
                {
                    throw new PddlValidationException(
                        $"Problem object '{obj.Name}' conflicts with a domain constant of the same name.");
                }
                ValidateTypeIsDeclared(obj.ParentType, $"object '{obj.Name}'");
            }
        }

        private static void ValidateDuplicateNames(IEnumerable<string> names, string declarationType)
        {
            var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (string name in names)
            {
                if (!seen.Add(name))
                {
                    throw new PddlValidationException($"Duplicate {declarationType} declaration '{name}'.");
                }
            }
        }

        private void ValidateParameters(IEnumerable<Parameter> parameters, string context)
        {
            var names = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (Parameter parameter in parameters)
            {
                if (!names.Add(parameter.Name))
                {
                    throw new PddlValidationException($"Duplicate parameter '{parameter.Name}' in {context}.");
                }
                ValidateTypeIsDeclared(parameter.TypeName, context);
            }
        }

        private void ValidateDerivedSignatures()
        {
            foreach (DerivedPredicate derivedPredicate in _domain.DerivedPredicates)
            {
                PredicateDeclaration signature = derivedPredicate.Signature;
                PredicateDeclaration canonicalSignature = _predicates[signature.Name];
                if (signature.Parameters.Length != canonicalSignature.Parameters.Length)
                {
                    throw new PddlValidationException(
                        $"Arity mismatch for derived predicate '{signature.Name}'. Expected {canonicalSignature.Parameters.Length}, got {signature.Parameters.Length}.");
                }

                for (int index = 0; index < signature.Parameters.Length; index++)
                {
                    string expectedType = canonicalSignature.Parameters[index].TypeName;
                    string actualType = signature.Parameters[index].TypeName;
                    if (!actualType.Equals(expectedType, StringComparison.OrdinalIgnoreCase))
                    {
                        throw new PddlValidationException(
                            $"Type mismatch for parameter {index + 1} of derived predicate '{signature.Name}'. Expected '{expectedType}', got '{actualType}'.");
                    }
                }
            }
        }

        private void ValidateTypeIsDeclared(string typeName, string context)
        {
            if (string.IsNullOrWhiteSpace(typeName))
            {
                throw new PddlValidationException($"A type name is required in {context}.");
            }
            if (typeName.Equals("object", StringComparison.OrdinalIgnoreCase))
            {
                return;
            }
            if (!_declaredTypes.Contains(typeName))
            {
                throw new PddlValidationException($"Undeclared type '{typeName}' in {context}.");
            }
        }

        private void ValidateTypeCycles()
        {
            foreach (TypeDeclaration type in _domain.Types)
            {
                var path = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
                string current = type.Name;
                while (_typeHierarchy.TryGetValue(current, out string? parent))
                {
                    if (!path.Add(current))
                    {
                        throw new PddlValidationException($"Type hierarchy contains a cycle involving '{current}'.");
                    }
                    if (parent.Equals("object", StringComparison.OrdinalIgnoreCase))
                    {
                        break;
                    }
                    current = parent;
                }
            }
        }

        private ActionDefinition ValidateAction(ActionDefinition action)
        {
            Dictionary<string, string> scope = CreateScope(action.Parameters, $"action {action.Name}");
            ILogicalExpression? precondition = action.Precondition is null
                ? null
                : ValidateLogicalExpression(action.Precondition, scope, $"action {action.Name}");
            IEffect? effect = action.Effect is null
                ? null
                : ValidateEffect(action.Effect, scope, $"action {action.Name}");
            return action with { Precondition = precondition, Effect = effect };
        }

        private DerivedPredicate ValidateDerivedPredicate(DerivedPredicate derivedPredicate)
        {
            Dictionary<string, string> scope = CreateScope(
                derivedPredicate.Signature.Parameters,
                $"derived predicate {derivedPredicate.Signature.Name}");
            ILogicalExpression body = ValidateLogicalExpression(
                derivedPredicate.Body,
                scope,
                $"derived predicate {derivedPredicate.Signature.Name}");
            return derivedPredicate with { Body = body };
        }

        private static Dictionary<string, string> CreateScope(IEnumerable<Parameter> parameters, string context)
        {
            var scope = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
            foreach (Parameter parameter in parameters)
            {
                if (!scope.TryAdd(parameter.Name, parameter.TypeName))
                {
                    throw new PddlValidationException($"Duplicate parameter '{parameter.Name}' in {context}.");
                }
            }
            return scope;
        }

        private void ValidateMetric(Metric metric, Dictionary<string, string> scope)
        {
            ValidateNumericExpression(metric.Expression, scope, "problem metric");
        }

        private ILogicalExpression ValidateLogicalExpression(
            ILogicalExpression expression,
            Dictionary<string, string> scope,
            string context)
        {
            switch (expression)
            {
                case And andExpression:
                    return new And(andExpression.Expressions
                        .Select(child => ValidateLogicalExpression(child, scope, context))
                        .ToImmutableArray());
                case Or orExpression:
                    return new Or(orExpression.Expressions
                        .Select(child => ValidateLogicalExpression(child, scope, context))
                        .ToImmutableArray());
                case Not notExpression:
                    return new Not(ValidateLogicalExpression(notExpression.Expression, scope, context));
                case Imply implyExpression:
                    return new Imply(
                        ValidateLogicalExpression(implyExpression.Antecedent, scope, context),
                        ValidateLogicalExpression(implyExpression.Consequent, scope, context));
                case Forall forallExpression:
                    return new Forall(
                        forallExpression.Variables,
                        ValidateQuantifier(forallExpression.Variables, forallExpression.Body, scope, context));
                case Exists existsExpression:
                    return new Exists(
                        existsExpression.Variables,
                        ValidateQuantifier(existsExpression.Variables, existsExpression.Body, scope, context));
                case PredicateCall predicateCall:
                    ValidatePredicateCall(predicateCall, scope, context);
                    return predicateCall;
                case Equality equality:
                    ValidateTerm(equality.Left, scope, context);
                    ValidateTerm(equality.Right, scope, context);
                    return equality;
                case AmbiguousEquality ambiguousEquality:
                    return ResolveAmbiguousEquality(ambiguousEquality, scope, context);
                case Comparison comparison:
                    ValidateNumericExpression(comparison.Left, scope, context);
                    ValidateNumericExpression(comparison.Right, scope, context);
                    return comparison;
                case EmptyLogic:
                    return expression;
                default:
                    throw new PddlValidationException($"Unknown logical expression {expression.GetType().Name} in {context}.");
            }
        }

        private ILogicalExpression ResolveAmbiguousEquality(
            AmbiguousEquality equality,
            Dictionary<string, string> scope,
            string context)
        {
            bool leftTerm = _constantsAndObjects.ContainsKey(equality.LeftName);
            bool rightTerm = _constantsAndObjects.ContainsKey(equality.RightName);
            bool leftFunction = IsZeroArityFunction(equality.LeftName);
            bool rightFunction = IsZeroArityFunction(equality.RightName);

            if ((leftTerm && leftFunction) || (rightTerm && rightFunction))
            {
                throw new PddlValidationException($"Ambiguous equality in {context} contains an operand declared as both an object term and a numeric fluent.");
            }

            if (leftFunction && rightFunction)
            {
                var left = new FluentCall(equality.LeftName, ImmutableArray<Term>.Empty);
                var right = new FluentCall(equality.RightName, ImmutableArray<Term>.Empty);
                ValidateFluentCall(left, scope, context);
                ValidateFluentCall(right, scope, context);
                return new Comparison(ComparisonOperator.Equal, left, right);
            }

            if (leftTerm && rightTerm)
            {
                Term left = Term.Constant(equality.LeftName);
                Term right = Term.Constant(equality.RightName);
                ValidateTerm(left, scope, context);
                ValidateTerm(right, scope, context);
                return new Equality(left, right);
            }

            bool hasKnownOperand = leftTerm
                || rightTerm
                || _functions.ContainsKey(equality.LeftName)
                || _functions.ContainsKey(equality.RightName)
                || IsBuiltInTotalCost(equality.LeftName)
                || IsBuiltInTotalCost(equality.RightName);
            string reason = hasKnownOperand ? "mixes incompatible or non-zero-arity operands" : "contains unresolved operands";
            throw new PddlValidationException($"Ambiguous equality in {context} {reason}: '{equality.LeftName}' and '{equality.RightName}'.");
        }

        private bool IsZeroArityFunction(string name)
        {
            return (_functions.TryGetValue(name, out FunctionDeclaration? declaration) && declaration.Parameters.IsDefaultOrEmpty)
                || IsBuiltInTotalCost(name);
        }

        private bool IsBuiltInTotalCost(string name)
        {
            return name.Equals("total-cost", StringComparison.OrdinalIgnoreCase)
                && _domain.Requirements.HasRequirement(PddlRequirement.ActionCosts);
        }

        private void ValidateNumericExpression(
            INumericExpression expression,
            Dictionary<string, string> scope,
            string context)
        {
            switch (expression)
            {
                case NumberLiteral:
                    return;
                case FluentCall fluentCall:
                    ValidateFluentCall(fluentCall, scope, context);
                    return;
                case Negate negate:
                    ValidateNumericExpression(negate.Operand, scope, context);
                    return;
                case Add addExpression:
                    ValidateNumericExpression(addExpression.Left, scope, context);
                    ValidateNumericExpression(addExpression.Right, scope, context);
                    return;
                case Subtract subtractExpression:
                    ValidateNumericExpression(subtractExpression.Left, scope, context);
                    ValidateNumericExpression(subtractExpression.Right, scope, context);
                    return;
                case Multiply multiplyExpression:
                    ValidateNumericExpression(multiplyExpression.Left, scope, context);
                    ValidateNumericExpression(multiplyExpression.Right, scope, context);
                    return;
                case Divide divideExpression:
                    ValidateNumericExpression(divideExpression.Left, scope, context);
                    ValidateNumericExpression(divideExpression.Right, scope, context);
                    return;
                default:
                    throw new PddlValidationException($"Unknown numeric expression {expression.GetType().Name} in {context}.");
            }
        }

        private void ValidateInitialState(
            ImmutableArray<IProblemInitElement> initialState,
            Dictionary<string, string> scope)
        {
            var positiveFacts = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var negativeFacts = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            var initializedNumericFluents = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

            foreach (IProblemInitElement element in initialState)
            {
                switch (element)
                {
                    case PredicateCall predicateCall:
                        ValidateInitialPredicate(predicateCall, scope);
                        string positiveKey = CreateFactKey(predicateCall);
                        if (negativeFacts.Contains(positiveKey))
                        {
                            throw new PddlValidationException($"Problem init contains contradictory positive and negative facts for '{predicateCall.ToPddlString()}'.");
                        }
                        positiveFacts.Add(positiveKey);
                        break;
                    case NegativePredicateInitialization negative:
                        ValidateInitialPredicate(negative.Predicate, scope);
                        string negativeKey = CreateFactKey(negative.Predicate);
                        if (positiveFacts.Contains(negativeKey))
                        {
                            throw new PddlValidationException($"Problem init contains contradictory positive and negative facts for '{negative.Predicate.ToPddlString()}'.");
                        }
                        negativeFacts.Add(negativeKey);
                        break;
                    case NumericInitialization numericInitialization:
                        ValidateFluentCall(numericInitialization.Fluent, scope, "problem init");
                        if (numericInitialization.Fluent.Name.Equals("total-cost", StringComparison.OrdinalIgnoreCase))
                        {
                            break;
                        }

                        string numericFluentKey = CreateFluentKey(numericInitialization.Fluent);
                        if (!initializedNumericFluents.Add(numericFluentKey))
                        {
                            throw new PddlValidationException(
                                $"Numeric fluent '{numericInitialization.Fluent.ToPddlString()}' is initialized more than once in problem init.");
                        }
                        break;
                    default:
                        throw new PddlValidationException($"Unknown init element {element.GetType().Name} in problem init.");
                }
            }
        }

        private void ValidateInitialPredicate(PredicateCall predicateCall, Dictionary<string, string> scope)
        {
            if (_derivedPredicates.Contains(predicateCall.Name))
            {
                throw new PddlValidationException($"Derived predicate '{predicateCall.Name}' cannot appear in problem init.");
            }
            ValidatePredicateCall(predicateCall, scope, "problem init");
        }

        private static string CreateFactKey(PredicateCall predicateCall)
        {
            return $"{predicateCall.Name}\u001f{string.Join("\u001f", predicateCall.Arguments.Select(argument => argument.Name))}";
        }

        private static string CreateFluentKey(FluentCall fluentCall)
        {
            return $"{fluentCall.Name}\u001f{string.Join("\u001f", fluentCall.Arguments.Select(argument => argument.Name))}";
        }

        private IEffect ValidateEffect(IEffect effect, Dictionary<string, string> scope, string context)
        {
            switch (effect)
            {
                case AndEffect andEffect:
                    return new AndEffect(andEffect.Effects
                        .Select(child => ValidateEffect(child, scope, context))
                        .ToImmutableArray());
                case ForallEffect forallEffect:
                    return new ForallEffect(
                        forallEffect.Variables,
                        ValidateQuantifierEffect(forallEffect.Variables, forallEffect.Effect, scope, context));
                case ConditionalEffect conditionalEffect:
                    return new ConditionalEffect(
                        ValidateLogicalExpression(conditionalEffect.Condition, scope, context),
                        ValidateEffect(conditionalEffect.Effect, scope, context));
                case AddEffect addEffect:
                    ValidateMutablePredicateCall(addEffect.Predicate, context);
                    ValidatePredicateCall(addEffect.Predicate, scope, context);
                    return addEffect;
                case DeleteEffect deleteEffect:
                    ValidateMutablePredicateCall(deleteEffect.Predicate, context);
                    ValidatePredicateCall(deleteEffect.Predicate, scope, context);
                    return deleteEffect;
                case Assign assign:
                    ValidateFluentCall(assign.Fluent, scope, context);
                    ValidateNumericExpression(assign.Value, scope, context);
                    return assign;
                case Increase increase:
                    ValidateFluentCall(increase.Fluent, scope, context);
                    ValidateNumericExpression(increase.Value, scope, context);
                    return increase;
                case Decrease decrease:
                    ValidateFluentCall(decrease.Fluent, scope, context);
                    ValidateNumericExpression(decrease.Value, scope, context);
                    return decrease;
                case ScaleUp scaleUp:
                    ValidateFluentCall(scaleUp.Fluent, scope, context);
                    ValidateNumericExpression(scaleUp.Value, scope, context);
                    return scaleUp;
                case ScaleDown scaleDown:
                    ValidateFluentCall(scaleDown.Fluent, scope, context);
                    ValidateNumericExpression(scaleDown.Value, scope, context);
                    return scaleDown;
                default:
                    throw new PddlValidationException($"Unknown effect {effect.GetType().Name} in {context}.");
            }
        }

        private void ValidateMutablePredicateCall(PredicateCall predicateCall, string context)
        {
            if (_derivedPredicates.Contains(predicateCall.Name))
            {
                throw new PddlValidationException($"Derived predicate '{predicateCall.Name}' cannot be modified in {context}.");
            }
        }

        private ILogicalExpression ValidateQuantifier(
            IEnumerable<Parameter> variables,
            ILogicalExpression body,
            Dictionary<string, string> scope,
            string context)
        {
            Dictionary<string, string> innerScope = CreateQuantifiedScope(variables, scope, context);
            return ValidateLogicalExpression(body, innerScope, context);
        }

        private IEffect ValidateQuantifierEffect(
            IEnumerable<Parameter> variables,
            IEffect effect,
            Dictionary<string, string> scope,
            string context)
        {
            Dictionary<string, string> innerScope = CreateQuantifiedScope(variables, scope, context);
            return ValidateEffect(effect, innerScope, context);
        }

        private Dictionary<string, string> CreateQuantifiedScope(
            IEnumerable<Parameter> variables,
            Dictionary<string, string> scope,
            string context)
        {
            var innerScope = new Dictionary<string, string>(scope, StringComparer.OrdinalIgnoreCase);
            var localNames = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            foreach (Parameter variable in variables)
            {
                ValidateTypeIsDeclared(variable.TypeName, context);
                if (!localNames.Add(variable.Name))
                {
                    throw new PddlValidationException($"Duplicate quantified variable '{variable.Name}' in {context}.");
                }
                if (scope.ContainsKey(variable.Name))
                {
                    throw new PddlValidationException($"Quantified variable '{variable.Name}' shadows an existing variable in {context}.");
                }
                innerScope[variable.Name] = variable.TypeName;
            }
            return innerScope;
        }

        private void ValidatePredicateCall(
            PredicateCall predicateCall,
            Dictionary<string, string> scope,
            string context)
        {
            if (!_predicates.TryGetValue(predicateCall.Name, out PredicateDeclaration? declaration))
            {
                throw new PddlValidationException($"Undeclared predicate '{predicateCall.Name}' in {context}.");
            }
            if (declaration.Parameters.Length != predicateCall.Arguments.Length)
            {
                throw new PddlValidationException($"Arity mismatch for predicate '{predicateCall.Name}' in {context}. Expected {declaration.Parameters.Length}, got {predicateCall.Arguments.Length}.");
            }
            for (int index = 0; index < predicateCall.Arguments.Length; index++)
            {
                ValidateTypeMatch(
                    predicateCall.Arguments[index],
                    declaration.Parameters[index].TypeName,
                    scope,
                    context);
            }
        }

        private void ValidateFluentCall(
            FluentCall fluentCall,
            Dictionary<string, string> scope,
            string context)
        {
            if (IsBuiltInTotalCost(fluentCall.Name))
            {
                if (!fluentCall.Arguments.IsDefaultOrEmpty)
                {
                    throw new PddlValidationException($"Arity mismatch for fluent '{fluentCall.Name}' in {context}. Expected 0, got {fluentCall.Arguments.Length}.");
                }
                return;
            }

            if (!_functions.TryGetValue(fluentCall.Name, out FunctionDeclaration? declaration))
            {
                throw new PddlValidationException($"Undeclared numeric fluent '{fluentCall.Name}' in {context}.");
            }
            if (declaration.Parameters.Length != fluentCall.Arguments.Length)
            {
                throw new PddlValidationException($"Arity mismatch for fluent '{fluentCall.Name}' in {context}. Expected {declaration.Parameters.Length}, got {fluentCall.Arguments.Length}.");
            }
            for (int index = 0; index < fluentCall.Arguments.Length; index++)
            {
                ValidateTypeMatch(
                    fluentCall.Arguments[index],
                    declaration.Parameters[index].TypeName,
                    scope,
                    context);
            }
        }

        private void ValidateTypeMatch(
            Term argument,
            string expectedType,
            Dictionary<string, string> scope,
            string context)
        {
            string actualType;
            if (argument.IsVariable)
            {
                if (!scope.TryGetValue(argument.Name, out string? scopedType))
                {
                    throw new PddlValidationException($"Undeclared variable '{argument.Name}' in {context}.");
                }
                actualType = scopedType;
            }
            else if (!_constantsAndObjects.TryGetValue(argument.Name, out actualType!))
            {
                throw new PddlValidationException($"Undeclared constant or object '{argument.Name}' in {context}.");
            }

            if (!IsAssignable(actualType, expectedType))
            {
                throw new PddlValidationException($"Type mismatch for '{argument.Name}' in {context}. Expected '{expectedType}', got '{actualType}'.");
            }
        }

        private void ValidateTerm(Term term, Dictionary<string, string> scope, string context)
        {
            if (term.IsVariable)
            {
                if (!scope.ContainsKey(term.Name))
                {
                    throw new PddlValidationException($"Undeclared variable '{term.Name}' in {context}.");
                }
                return;
            }
            if (!_constantsAndObjects.ContainsKey(term.Name))
            {
                throw new PddlValidationException($"Undeclared constant or object '{term.Name}' in {context}.");
            }
        }

        private bool IsAssignable(string childType, string parentType)
        {
            if (childType.Equals(parentType, StringComparison.OrdinalIgnoreCase)
                || parentType.Equals("object", StringComparison.OrdinalIgnoreCase))
            {
                return true;
            }

            var visited = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
            string current = childType;
            while (visited.Add(current) && _typeHierarchy.TryGetValue(current, out string? nextParent))
            {
                if (nextParent.Equals(parentType, StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
                current = nextParent;
            }
            return false;
        }
    }
}
