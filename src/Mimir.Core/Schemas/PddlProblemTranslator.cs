using Mimir.Core.Grounding;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Ast.Models;

namespace Mimir.Core.Schemas;

internal sealed class PddlProblemTranslator
{
    public InstanceContext Context { get; }
    public IReadOnlyList<Constant> DeclaredObjects { get; }
    public IReadOnlyList<Constant> AllObjects { get; }
    public IReadOnlyDictionary<string, Constant> ObjectLookup { get; }
    public IReadOnlyDictionary<string, Predicate> AllPredicates { get; }
    public IReadOnlyList<Literal<Fact>> Goal { get; }

    internal PddlProblemTranslator(
        Domain domain,
        Problem problem,
        ProblemDefinition astProblem,
        ProgrammaticProblemInputs? programmaticInputs = null)
    {
        var objectsList = new List<Constant>(domain.Constants);
        var declaredObjectsList = new List<Constant>();
        var objectLookup = objectsList.ToDictionary(o => o.Name, o => o, StringComparer.OrdinalIgnoreCase);
        Constant GetOrCreateObject(string name, string type)
        {
            if (objectLookup.TryGetValue(name, out var existing))
                return existing;

            var constant = new Constant(name, type);
            objectLookup[name] = constant;
            objectsList.Add(constant);
            return constant;
        }

        foreach (var obj in astProblem.Objects)
            declaredObjectsList.Add(GetOrCreateObject(obj.Name, obj.ParentType));
        DeclaredObjects = declaredObjectsList;
        AllObjects = objectsList;
        ObjectLookup = objectLookup;
        Context = new InstanceContext(problem, objectsList);

        // 2. Identify all predicates
        var allPredicates = new Dictionary<string, Predicate>(StringComparer.OrdinalIgnoreCase);
        foreach (var p in domain.Fluents) allPredicates[p.Name] = p;
        foreach (var p in domain.Statics) allPredicates[p.Name] = p;
        foreach (var p in domain.Derived) allPredicates[p.Name] = p;
        AllPredicates = allPredicates;
        var allFunctions = domain.Functions.ToDictionary(function => function.Name, StringComparer.OrdinalIgnoreCase);
        bool actionCostsEnabled =
            domain.PddlDefinition.Requirements.HasRequirement(PddlRequirement.ActionCosts);
        bool totalCostInitialized = false;

        // 3. Process Initial State
        int projectedNumericInitializationCount = 0;
        foreach (var initExpr in astProblem.Init)
        {
            if (initExpr is PredicateCall pCall)
            {
                var args = pCall.Arguments.Select(a => objectLookup[a.Name]).ToList();
                if (!allPredicates.TryGetValue(pCall.Name, out var pred))
                    throw new InvalidOperationException($"Predicate '{pCall.Name}' is not declared in the domain.");

                if (pred is Predicate<Fluent> pf)
                {
                    var fact = Context.RegisterFact(pf, args);
                    problem._initialFluentFacts.Add(fact);
                }
                else if (pred is Predicate<Static> ps)
                {
                    var fact = Context.RegisterFact(ps, args);
                    problem._initialStaticFacts.Add(fact);
                }
                else
                    throw new InvalidOperationException(
                        $"Derived predicate '{pCall.Name}' cannot appear in the initial state.");
            }
            else if (initExpr is NumericInitialization numericInitialization)
            {
                if (programmaticInputs is null)
                {
                    ProblemNumericInit.RegisterNumericInitialization(
                        problem._numericFunctionValues,
                        objectLookup,
                        numericInitialization,
                        allFunctions,
                        actionCostsEnabled,
                        ref totalCostInitialized);
                }
                else
                {
                    projectedNumericInitializationCount++;
                }
            }
            else if (initExpr is NegativePredicateInitialization)
            {
                // Negative initial facts are already false under closed-world semantics.
                continue;
            }
            else
            {
                throw new NotSupportedException(
                    $"Core cannot construct initial element '{initExpr.GetType().Name}'.");
            }
        }

        // StaticBitboard will be built on demand or when requested
        int staticWordCount = (int)Math.Ceiling(Context.StaticCount / 64.0);
        var staticBitboardWords = new ulong[staticWordCount];
        foreach (var fact in problem._initialStaticFacts)
        {
            int arrayIndex = fact.LocalIndex / 64;
            int bitIndex = fact.LocalIndex % 64;
            staticBitboardWords[arrayIndex] |= 1UL << bitIndex;
        }

        if (programmaticInputs is not null)
        {
            if (projectedNumericInitializationCount != programmaticInputs.NumericInitializations.Count)
                throw new InvalidOperationException("Programmatic numeric initialization projection is inconsistent.");

            foreach (BuilderProblemNumericSpec numeric in programmaticInputs.NumericInitializations)
            {
                NumericFunction function = allFunctions[numeric.FunctionName];
                Constant[] arguments = numeric.Arguments
                    .Select(argument => objectLookup[argument])
                    .ToArray();
                var key = new NumericFunctionKey(function, arguments);
                if (!problem._numericFunctionValues.TryAdd(key, numeric.Value))
                {
                    throw new InvalidOperationException(
                        $"Numeric function '{function.Name}' was initialized more than once for the same arguments.");
                }
            }
        }
        Context.SetStaticBitboardWords(staticBitboardWords);

        var goalList = new List<Literal<Fact>>();
        ProblemGoalExtractor.ExtractGoalLiterals(
            astProblem.Goal,
            AllPredicates,
            objectLookup,
            Context,
            goalList);
        Goal = goalList;
        Context.InitializeDerivedClosure();
    }
}
