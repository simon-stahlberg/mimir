using System.Collections.Immutable;
using System.Diagnostics;
using System.Text.Json;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Mimir.Search;
using Mimir.Search.Evaluation;
using Mimir.Search.Planning;

if (args.Length != 0)
    throw new ArgumentException("Expected no arguments.");

string benchmarkRoot = Path.GetFullPath("Benchmark");
if (!Directory.Exists(benchmarkRoot))
    throw new DirectoryNotFoundException($"Benchmark directory not found: {benchmarkRoot}");

BenchmarkInstance[] benchmarks = DiscoverBenchmarks(benchmarkRoot);
var jsonOptions = new JsonSerializerOptions { WriteIndented = false };

foreach (BenchmarkInstance benchmark in benchmarks)
{
    BenchmarkMeasurement measurement = RunBenchmark(benchmark);
    Console.WriteLine(JsonSerializer.Serialize(measurement, jsonOptions));
}

static BenchmarkInstance[] DiscoverBenchmarks(string benchmarkRoot)
{
    var benchmarks = new List<BenchmarkInstance>();
    string[] directories = Directory
        .EnumerateDirectories(benchmarkRoot)
        .OrderBy(path => path, StringComparer.Ordinal)
        .ToArray();

    foreach (string directory in directories)
    {
        string domainPath = Path.Combine(directory, "domain.pddl");
        if (!File.Exists(domainPath))
            throw new FileNotFoundException($"Benchmark domain not found: {domainPath}", domainPath);

        string[] problemPaths = Directory
            .EnumerateFiles(directory, "*.pddl", SearchOption.TopDirectoryOnly)
            .Where(path => !string.Equals(path, domainPath, StringComparison.OrdinalIgnoreCase))
            .OrderBy(path => path, StringComparer.Ordinal)
            .ToArray();
        if (problemPaths.Length == 0)
            throw new InvalidOperationException($"Benchmark directory has no problem instances: {directory}");

        foreach (string problemPath in problemPaths)
        {
            string name = Path
                .GetRelativePath(benchmarkRoot, problemPath)
                .Replace(Path.DirectorySeparatorChar, '/');
            benchmarks.Add(new BenchmarkInstance(name, domainPath, problemPath));
        }
    }

    if (benchmarks.Count == 0)
        throw new InvalidOperationException($"No benchmark instances found in: {benchmarkRoot}");

    return benchmarks.ToArray();
}

static BenchmarkMeasurement RunBenchmark(BenchmarkInstance benchmark)
{
    Domain domain = Domain.FromFile(benchmark.DomainPath);
    Problem problem = Problem.FromFile(domain, benchmark.ProblemPath, ApplicableActionGeneratorType.Lifted);
    State initialState = problem.InitialState;
    var layers = new List<LayerCounter>();

    GC.Collect();
    GC.WaitForPendingFinalizers();
    GC.Collect();

    long retainedBefore = GC.GetTotalMemory(forceFullCollection: false);
    long totalAllocatedBefore = GC.GetAllocatedBytesForCurrentThread();
    int gen0Before = GC.CollectionCount(0);
    int gen1Before = GC.CollectionCount(1);
    int gen2Before = GC.CollectionCount(2);
    long totalStarted = Stopwatch.GetTimestamp();

    long generatorAllocatedBefore = GC.GetAllocatedBytesForCurrentThread();
    long generatorStarted = Stopwatch.GetTimestamp();
    var generator = problem.GetApplicableActionGenerator(initialState);
    double generatorMilliseconds = Stopwatch.GetElapsedTime(generatorStarted).TotalMilliseconds;
    long generatorAllocatedBytes = GC.GetAllocatedBytesForCurrentThread() - generatorAllocatedBefore;

    var search = new SearchBuilder()
        .WithInitialState(initialState)
        .WithGoal(CreateContradictoryGoal(problem))
        .OnNodeGenerated(node => GetLayer(layers, node.Depth).NodesGenerated++)
        .OnNodeExpanded(node => GetLayer(layers, node.Depth).NodesExpanded++)
        .BuildBfs();

    long searchAllocatedBefore = GC.GetAllocatedBytesForCurrentThread();
    long searchStarted = Stopwatch.GetTimestamp();
    SearchResult result = search.Search();
    double searchMilliseconds = Stopwatch.GetElapsedTime(searchStarted).TotalMilliseconds;
    long searchAllocatedBytes = GC.GetAllocatedBytesForCurrentThread() - searchAllocatedBefore;

    double totalMilliseconds = Stopwatch.GetElapsedTime(totalStarted).TotalMilliseconds;
    long totalAllocatedBytes = GC.GetAllocatedBytesForCurrentThread() - totalAllocatedBefore;
    int gen0Collections = GC.CollectionCount(0) - gen0Before;
    int gen1Collections = GC.CollectionCount(1) - gen1Before;
    int gen2Collections = GC.CollectionCount(2) - gen2Before;

    ValidateExhaustiveSearch(benchmark.Name, result, layers);

    long retainedAfter = GC.GetTotalMemory(forceFullCollection: true);
    var layerMeasurements = layers
        .Select(layer => new LayerMeasurement(
            layer.Depth,
            layer.NodesExpanded,
            layer.NodesGenerated))
        .ToArray();

    var measurement = new BenchmarkMeasurement(
        benchmark.Name,
        "lifted:bfs:never-goal",
        result.Statistics.NodesExpanded,
        result.Statistics.NodesGenerated,
        result.Statistics.MaxDepth,
        layerMeasurements,
        generatorMilliseconds,
        searchMilliseconds,
        totalMilliseconds,
        generatorAllocatedBytes,
        searchAllocatedBytes,
        totalAllocatedBytes,
        retainedAfter - retainedBefore,
        gen0Collections,
        gen1Collections,
        gen2Collections);

    GC.KeepAlive(problem);
    GC.KeepAlive(generator);
    GC.KeepAlive(search);
    GC.KeepAlive(result);
    return measurement;
}

static GoalCondition CreateContradictoryGoal(Problem problem)
{
    Fact fact = problem.Goal.FirstOrDefault()?.Value
        ?? problem.Context.AllFacts.FirstOrDefault()
        ?? throw new InvalidOperationException(
            $"Benchmark problem '{problem.Name}' has no ground fact for an exhaustive-search goal.");
    var atom = new GroundedAtom(fact.Predicate, fact.Arguments);
    var expression = new GroundedAnd(
        ImmutableArray.Create<IGroundedExpression>(atom, new GroundedNot(atom)));
    return GoalCondition.FromExpression(problem, expression);
}

static LayerCounter GetLayer(List<LayerCounter> layers, int depth)
{
    while (layers.Count <= depth)
        layers.Add(new LayerCounter(layers.Count));

    return layers[depth];
}

static void ValidateExhaustiveSearch(
    string benchmark,
    SearchResult result,
    IReadOnlyList<LayerCounter> layers)
{
    if (result.IsSuccess)
        throw new InvalidOperationException($"Benchmark '{benchmark}' unexpectedly reached its never goal.");

    SearchStatistics statistics = result.Statistics;
    if (statistics.NodesExpanded != statistics.NodesGenerated)
    {
        throw new InvalidOperationException(
            $"Benchmark '{benchmark}' did not expand every generated state.");
    }

    if (layers.Count != statistics.MaxDepth + 1)
        throw new InvalidOperationException($"Benchmark '{benchmark}' produced non-contiguous BFS layers.");

    if (layers[0].NodesExpanded != 1 || layers[0].NodesGenerated != 1)
        throw new InvalidOperationException($"Benchmark '{benchmark}' produced an invalid root layer.");

    int expanded = 0;
    int generated = 0;
    foreach (LayerCounter layer in layers)
    {
        if (layer.NodesExpanded != layer.NodesGenerated)
        {
            throw new InvalidOperationException(
                $"Benchmark '{benchmark}' did not exhaust BFS layer {layer.Depth}.");
        }

        expanded = checked(expanded + layer.NodesExpanded);
        generated = checked(generated + layer.NodesGenerated);
    }

    if (expanded != statistics.NodesExpanded || generated != statistics.NodesGenerated)
    {
        throw new InvalidOperationException(
            $"Benchmark '{benchmark}' layer totals do not match the search totals.");
    }
}

internal sealed record BenchmarkInstance(string Name, string DomainPath, string ProblemPath);

internal sealed class LayerCounter(int depth)
{
    internal int Depth { get; } = depth;
    internal int NodesExpanded { get; set; }
    internal int NodesGenerated { get; set; }
}

internal sealed record LayerMeasurement(
    int Depth,
    int NodesExpanded,
    int NodesGenerated);

internal sealed record BenchmarkMeasurement(
    string Instance,
    string Search,
    int NodesExpanded,
    int NodesGenerated,
    int MaxDepth,
    IReadOnlyList<LayerMeasurement> Layers,
    double GeneratorMilliseconds,
    double SearchMilliseconds,
    double TotalMilliseconds,
    long GeneratorAllocatedBytes,
    long SearchAllocatedBytes,
    long TotalAllocatedBytes,
    long RetainedBytes,
    int Gen0Collections,
    int Gen1Collections,
    int Gen2Collections);
