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
    public IReadOnlyList<NumericComparison> NumericGoals { get; }

    internal PddlProblemTranslator(
        Domain domain,
        Problem problem,
        ProblemDefinition astProblem)
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
        var numericValues = new Dictionary<NumericFunctionKey, double>();
        foreach (NumericInitialization initialization in astProblem.Init.OfType<NumericInitialization>())
        {
            ProblemNumericInit.RegisterNumericInitialization(numericValues, objectLookup,
                initialization, allFunctions, actionCostsEnabled, ref totalCostInitialized);
        }

        AddUndefinedAssignTargets(domain, objectsList, numericValues);
        Context = new InstanceContext(problem, objectsList, numericValues, domain.ChangingFunctions);

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
            else if (initExpr is NumericInitialization or NegativePredicateInitialization)
            {
                continue;
            }
            else
            {
                throw new NotSupportedException($"Core cannot construct initial element '{initExpr.GetType().Name}'.");
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

        Context.SetStaticBitboardWords(staticBitboardWords);

        var goalList = new List<Literal<Fact>>();
        var comparisons = new List<NumericComparison>();
        ProblemGoalExtractor.ExtractGoalLiterals(
            astProblem.Goal,
            AllPredicates,
            allFunctions,
            objectLookup,
            Context,
            goalList,
            comparisons);
        NumericGoals = comparisons;
        Goal = goalList;
        Context.InitializeDerivedClosure();
    }

    // PDDL 2.1 lets assign define a fluent that the initial state leaves undefined, so every type-correct
    // instance of an assigned function needs a state slot even without an initial value.
    private static void AddUndefinedAssignTargets(Domain domain, IReadOnlyList<Constant> objects,
        Dictionary<NumericFunctionKey, double> numericValues)
    {
        var assignedFunctions = new HashSet<NumericFunction>();
        foreach (ActionSchema action in domain.Actions)
        {
            foreach (ConditionalNumericEffect effect in action.NumericEffects)
            {
                if (effect.Effect.Operator == NumericUpdateOperator.Assign)
                    assignedFunctions.Add(effect.Effect.Target.Function);
            }
        }

        foreach (NumericFunction function in assignedFunctions)
        {
            Constant[][] candidates = function.Parameters
                .Select(parameter => objects.Where(candidate => domain.IsCompatible(candidate.Type, parameter.Type)).ToArray())
                .ToArray();
            var arguments = new Constant[candidates.Length];
            AddInstances(0);

            void AddInstances(int position)
            {
                if (position == arguments.Length)
                {
                    numericValues.TryAdd(new NumericFunctionKey(function, arguments.ToArray()), NumericEvaluation.Undefined);
                    return;
                }
                foreach (Constant candidate in candidates[position])
                {
                    arguments[position] = candidate;
                    AddInstances(position + 1);
                }
            }
        }
    }
}
