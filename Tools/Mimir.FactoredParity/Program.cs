using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Pddl.Analysis;
using GroundAction = Mimir.Core.Grounding.Action;

// Compares FactoredApplicableActionGenerator against CliqueApplicableActionGenerator over the benchmark set.
// Both generators run on the same Problem, so states and constants are reference-comparable.

if (args.Length > 2 && args[0] == "--iwprofile")
{
    Mimir.FactoredParity.IwProfile.Run(args[1], args[2], args.Length > 3 ? int.Parse(args[3]) : 200);
    return 0;
}

if (args.Length > 2 && args[0] == "--iw")
{
    Mimir.FactoredParity.IwComparison.Run(
        Path.GetFullPath(args[1]),
        int.Parse(args[2]),
        args.Length > 3 ? int.Parse(args[3]) : 10,
        args.Length > 4 ? int.Parse(args[4]) : 1);
    return 0;
}

if (args.Length > 3 && args[0] == "--novelty")
{
    Mimir.FactoredParity.Novelty.Run(args[1], args[2], int.Parse(args[3]), args.Length > 4 ? int.Parse(args[4]) : 2000);
    return 0;
}

if (args.Length > 1 && args[0] == "--scale")
{
    Mimir.FactoredParity.Scaling.Run(Path.GetFullPath(args[1]), TimeSpan.FromSeconds(60));
    return 0;
}

string benchmarkRoot = Path.GetFullPath(args.Length > 0 ? args[0] : "Benchmark");
int stateLimit = args.Length > 1 ? int.Parse(args[1]) : 200;
string? only = args.Length > 2 ? args[2] : null;

if (!Directory.Exists(benchmarkRoot))
    throw new DirectoryNotFoundException($"Benchmark directory not found: {benchmarkRoot}");

int instances = 0;
int mismatches = 0;
int unsupported = 0;
var unsupportedDomains = new SortedSet<string>();
long totalActions = 0;
long totalGroups = 0;
var cliqueTime = new System.Diagnostics.Stopwatch();
var factoredTime = new System.Diagnostics.Stopwatch();
var expandTime = new System.Diagnostics.Stopwatch();

foreach ((string domainName, string domainPath, string problemPath) in Discover(benchmarkRoot))
{
    if (only is not null && !domainName.Contains(only, StringComparison.OrdinalIgnoreCase)) continue;

    Domain domain;
    Problem problem;
    try
    {
        domain = Domain.FromFile(domainPath);
        problem = Problem.FromFile(domain, problemPath, ApplicableActionGeneratorType.Factored);
    }
    catch (Exception exception)
        when (exception is NotSupportedException or PddlValidationException or PddlLoadException)
    {
        unsupported++;
        unsupportedDomains.Add($"{domainName}: {Shorten(exception.Message)}");
        continue;
    }

    FactoredApplicableActionGenerator factored;
    try
    {
        factored = new FactoredApplicableActionGenerator(problem);
    }
    catch (NotSupportedException exception)
    {
        unsupported++;
        unsupportedDomains.Add($"{domainName}: {Shorten(exception.Message)}");
        continue;
    }

    var clique = new CliqueApplicableActionGenerator(problem);
    instances++;

    TimeSpan beforeFactored = factoredTime.Elapsed;
    TimeSpan beforeClique = cliqueTime.Elapsed;
    string? failure = Compare(
        problem, clique, factored, stateLimit, ref totalActions, ref totalGroups, cliqueTime, factoredTime, expandTime);
    TimeSpan spent = factoredTime.Elapsed - beforeFactored;
    if (spent > TimeSpan.FromSeconds(0.5))
    {
        Console.WriteLine($"SLOW {domainName,-45} factored {spent.TotalSeconds,7:F2}s "
            + $"clique {(cliqueTime.Elapsed - beforeClique).TotalSeconds,7:F2}s");
    }

    if (failure is null) continue;

    mismatches++;
    Console.WriteLine($"MISMATCH {Path.GetRelativePath(benchmarkRoot, problemPath)}");
    Console.WriteLine($"  {failure}");
    if (mismatches >= 25) break;
}

Console.WriteLine();
Console.WriteLine($"instances compared : {instances}");
Console.WriteLine($"mismatches         : {mismatches}");
Console.WriteLine($"unsupported        : {unsupported}");
Console.WriteLine($"ground actions     : {totalActions}");
Console.WriteLine($"factored groups    : {totalGroups}");
if (totalGroups > 0)
    Console.WriteLine($"compression        : {(double)totalActions / totalGroups:F2}x");
Console.WriteLine($"clique time        : {cliqueTime.Elapsed.TotalSeconds:F2}s");
Console.WriteLine($"factored groups ms : {factoredTime.Elapsed.TotalSeconds:F2}s");
Console.WriteLine($"factored expand ms : {expandTime.Elapsed.TotalSeconds:F2}s");
if (unsupportedDomains.Count > 0)
{
    Console.WriteLine();
    Console.WriteLine("unsupported reasons:");
    foreach (string reason in unsupportedDomains) Console.WriteLine($"  {reason}");
}

Console.WriteLine($"distinctness excl  : {Counters.Distinct}");
Console.WriteLine($"forbidden-tuple    : {Counters.Forbidden}");

return mismatches == 0 ? 0 : 1;


static string Shorten(string message)
{
    int stop = message.IndexOf(';');
    return stop < 0 ? message : message[..stop];
}

// Breadth-first over the shared Problem; the clique generator drives traversal so both see identical states.
static string? Compare(
    Problem problem,
    CliqueApplicableActionGenerator clique,
    FactoredApplicableActionGenerator factored,
    int stateLimit,
    ref long totalActions,
    ref long totalGroups,
    Stopwatch cliqueTime,
    Stopwatch factoredTime,
    Stopwatch expandTime)
{
    var open = new Queue<State>();
    var closed = new HashSet<State> { problem.InitialState };
    open.Enqueue(problem.InitialState);

    var cliqueActions = new List<GroundAction>();
    var stopwatch = Stopwatch.StartNew();
    int visited = 0;

    while (open.Count > 0 && visited < stateLimit && stopwatch.Elapsed < TimeSpan.FromSeconds(30))
    {
        State state = open.Dequeue();
        ExtendedState extended = state.Expand();
        visited++;

        cliqueTime.Start();
        clique.CollectApplicableActions(extended, cliqueActions);
        cliqueTime.Stop();

        var factoredActions = new List<GroundAction>();
        long groups = 0;
        factoredTime.Start();
        var groupList = factored.GetFactoredApplicableActions(extended).ToList();
        factoredTime.Stop();
        groups = groupList.Count;
        foreach (FactoredAction g in groupList)
        {
            foreach (FactoredExclusion e in g.Exclusions)
            {
                if (e.IsDistinctness) Counters.Distinct++; else Counters.Forbidden++;
            }
        }
        expandTime.Start();
        foreach (FactoredAction group in groupList) factoredActions.AddRange(group.Expand());
        expandTime.Stop();

        totalActions += cliqueActions.Count;
        totalGroups += groups;

        string? difference = Difference(cliqueActions, factoredActions, problem);
        if (difference is not null) return $"state {visited}: {difference}";

        foreach (GroundAction action in cliqueActions)
        {
            State successor = extended.Apply(action);
            if (closed.Add(successor)) open.Enqueue(successor);
        }
    }

    return null;
}

// Witness parameters carry an arbitrary representative in both generators, so compare the projection onto the
// parameters that actually determine the transition.
static string? Difference(List<GroundAction> expected, List<GroundAction> actual, Problem problem)
{
    Dictionary<string, int> expectedKeys = KeyCounts(expected, problem);
    Dictionary<string, int> actualKeys = KeyCounts(actual, problem);

    foreach ((string key, int count) in expectedKeys)
    {
        int actualCount = actualKeys.GetValueOrDefault(key);
        if (actualCount != count)
            return $"'{key}' expected {count}, factored produced {actualCount}";
    }

    foreach ((string key, int count) in actualKeys)
    {
        if (!expectedKeys.ContainsKey(key))
            return $"'{key}' produced {count} times by the factored generator only";
    }

    return null;
}

static Dictionary<string, int> KeyCounts(List<GroundAction> actions, Problem problem)
{
    var counts = new Dictionary<string, int>(StringComparer.Ordinal);
    foreach (GroundAction action in actions)
    {
        string key = TransitionKey(action, problem);
        counts[key] = counts.GetValueOrDefault(key) + 1;
    }

    return counts;
}

static string TransitionKey(GroundAction action, Problem problem)
{
    bool[] relevant = problem.Context.GetActionGroundingPlan(action.Schema).TransitionRelevantParameters;
    IEnumerable<string> arguments = action.Arguments
        .Select((argument, index) => relevant[index] ? argument.Name : "_");
    return $"{action.Schema.Name}({string.Join(",", arguments)})";
}

// Several benchmark directories ship a second domain file next to the instances.
static bool IsDomainFile(string path)
    => File.ReadLines(path)
        .Select(line => line.Trim())
        .Where(line => line.Length > 0 && !line.StartsWith(';'))
        .Take(3)
        .Any(line => line.Replace(" ", "").Contains("(define(domain", StringComparison.OrdinalIgnoreCase));

static IEnumerable<(string Domain, string DomainPath, string ProblemPath)> Discover(string root)
{
    foreach (string category in new[] { "strips", "adl" })
    {
        string categoryPath = Path.Combine(root, category);
        if (!Directory.Exists(categoryPath)) continue;

        foreach (string directory in Directory.EnumerateDirectories(categoryPath).OrderBy(path => path, StringComparer.Ordinal))
        {
            string domainPath = Path.Combine(directory, "domain.pddl");
            if (!File.Exists(domainPath)) continue;

            string[] problems = Directory
                .EnumerateFiles(directory, "*.pddl", SearchOption.TopDirectoryOnly)
                .Where(path => !string.Equals(path, domainPath, StringComparison.OrdinalIgnoreCase))
                .Where(path => !IsDomainFile(path))
                .OrderBy(path => path, StringComparer.Ordinal)
                .ToArray();

            // A few directories pair each instance with its own domain file.
            foreach (string problemPath in problems.Take(2))
            {
                string paired = Path.Combine(
                    directory,
                    $"{Path.GetFileNameWithoutExtension(problemPath)}-domain.pddl");
                yield return (
                    $"{category}/{Path.GetFileName(directory)}",
                    File.Exists(paired) ? paired : domainPath,
                    problemPath);
            }
        }
    }
}

static class Counters
{
    public static long Distinct;
    public static long Forbidden;
}
