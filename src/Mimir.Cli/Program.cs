using System.Text.Json;
using Mimir.Core.Schemas;
using Mimir.Search.Planning;

namespace Mimir.Cli;

class Program
{
    private sealed class CliOptions
    {
        public required string DomainPath { get; init; }
        public required string ProblemPath { get; init; }
        public required string GeneratorType { get; init; }
        public required string AlgorithmType { get; init; }
        public required string HeuristicType { get; init; }
        public required bool JsonOutput { get; init; }
        public required bool IncludePlanSteps { get; init; }
    }

    static int Main(string[] args)
    {
        if (!TryParseArguments(args, out var options, out var errorMessage))
        {
            if (!string.IsNullOrEmpty(errorMessage))
                Console.Error.WriteLine(errorMessage);

            PrintUsage();
            return string.IsNullOrEmpty(errorMessage) ? 0 : 1;
        }

        if (options is null)
        {
            Console.Error.WriteLine($"Error: {nameof(options)} is null");
            return 1;
        }

        if (!File.Exists(options.DomainPath))
        {
            Console.Error.WriteLine($"Error: Domain file not found: {options.DomainPath}");
            return 1;
        }

        if (!File.Exists(options.ProblemPath))
        {
            Console.Error.WriteLine($"Error: Problem file not found: {options.ProblemPath}");
            return 1;
        }

        if (!options.JsonOutput)
            Console.WriteLine($"Loading domain: {options.DomainPath}");
        var domain = Domain.FromFile(options.DomainPath);

        if (!options.JsonOutput)
            Console.WriteLine($"Loading problem: {options.ProblemPath}");
        var problem = Problem.FromFile(domain, options.ProblemPath);

        IPlanner planner;
        try
        {
            planner = PlannerFactory.Create(options.GeneratorType, options.AlgorithmType, options.HeuristicType);
        }
        catch (ArgumentException ex)
        {
            Console.Error.WriteLine($"Error: {ex.Message}");
            return 1;
        }

        if (!options.JsonOutput)
        {
            if (options.AlgorithmType is "astar" or "gbfs")
                Console.WriteLine($"Starting {options.AlgorithmType.ToUpperInvariant()} using '{options.GeneratorType}' generator with '{options.HeuristicType}' heuristic...");
            else
                Console.WriteLine($"Starting {options.AlgorithmType.ToUpperInvariant()} using '{options.GeneratorType}' generator...");
        }

        GC.Collect();
        GC.WaitForPendingFinalizers();
        GC.Collect();
        long baselineHeap = GC.GetTotalMemory(false);
        long startAllocated = GC.GetAllocatedBytesForCurrentThread();

        var result = planner.Solve(problem);

        long endAllocated = GC.GetAllocatedBytesForCurrentThread();
        long afterHeap = GC.GetTotalMemory(false);

        double nonNegativeMemoryUsedMB = (endAllocated - startAllocated) / (1024.0 * 1024.0);
        double retainedMb = Math.Max(0, afterHeap - baselineHeap) / (1024.0 * 1024.0);

        var summary = new BenchmarkRunSummary(
            DomainPath: Path.GetFullPath(options.DomainPath),
            ProblemPath: Path.GetFullPath(options.ProblemPath),
            Generator: options.GeneratorType,
            Algorithm: options.AlgorithmType,
            Heuristic: options.AlgorithmType is "astar" or "gbfs" ? options.HeuristicType : null,
            Status: result.Status.ToString(),
            Success: result.IsSuccess,
            TimeMs: result.ElapsedTime.TotalMilliseconds,
            SetupTimeMs: result.SetupTime.TotalMilliseconds,
            SearchTimeMs: result.SearchTime.TotalMilliseconds,
            MemoryAllocatedMb: nonNegativeMemoryUsedMB,
            MemoryRetainedMb: retainedMb,
            NodesExpanded: result.Statistics.NodesExpanded,
            NodesGenerated: result.Statistics.NodesGenerated,
            PlanLength: result.IsSuccess ? result.PlanLength : 0,
            PlanCost: result.IsSuccess ? result.PlanCost : 0d,
            PlanSteps: result.IsSuccess && options.IncludePlanSteps ? result.Plan.Select(action => action.ToString()).ToArray() : null);

        if (options.JsonOutput)
        {
            Console.WriteLine(JsonSerializer.Serialize(summary, BenchmarkRunSummaryJsonContext.Default.BenchmarkRunSummary));
            return 0;
        }

        Console.WriteLine("--------------------------------------------------");
        Console.WriteLine($"Result: {summary.Status}");
        Console.WriteLine($"Time: {summary.TimeMs.ToString(System.Globalization.CultureInfo.InvariantCulture)} ms " +
            $"(setup {summary.SetupTimeMs.ToString(System.Globalization.CultureInfo.InvariantCulture)} ms, " +
            $"search {summary.SearchTimeMs.ToString(System.Globalization.CultureInfo.InvariantCulture)} ms)");
        Console.WriteLine($"Memory Allocated (GC pressure): {summary.MemoryAllocatedMb.ToString("F2", System.Globalization.CultureInfo.InvariantCulture)} MB");
        Console.WriteLine($"Memory Retained (peak approx): {summary.MemoryRetainedMb.ToString("F2", System.Globalization.CultureInfo.InvariantCulture)} MB");
        Console.WriteLine($"Nodes Expanded: {summary.NodesExpanded}");
        Console.WriteLine($"Nodes Generated: {summary.NodesGenerated}");

        if (summary.Success)
        {
            Console.WriteLine($"Plan Length: {summary.PlanLength}");
            Console.WriteLine($"Plan Cost: {summary.PlanCost.ToString(System.Globalization.CultureInfo.InvariantCulture)}");
            if (summary.PlanSteps != null)
            {
                foreach (var action in summary.PlanSteps)
                    Console.WriteLine($"  {action}");
            }
        }

        return 0;
    }

    private static bool TryParseArguments(string[] args, out CliOptions? options, out string? errorMessage)
    {
        options = null;
        errorMessage = null;

        if (args.Any(arg => string.Equals(arg, "--help", StringComparison.OrdinalIgnoreCase) || string.Equals(arg, "-h", StringComparison.OrdinalIgnoreCase)))
            return false;

        bool jsonOutput = false;
        bool includePlanSteps = true;
        var positionalArgs = new List<string>();

        foreach (string arg in args)
        {
            switch (arg.ToLowerInvariant())
            {
                case "--json":
                    jsonOutput = true;
                    includePlanSteps = false;
                    break;
                case "--no-plan":
                    includePlanSteps = false;
                    break;
                default:
                    positionalArgs.Add(arg);
                    break;
            }
        }

        if (positionalArgs.Count < 3)
            return false;

        if (positionalArgs.Count > 5)
        {
            errorMessage = $"Too many positional arguments ({positionalArgs.Count}).";
            return false;
        }

        options = new CliOptions
        {
            DomainPath = positionalArgs[0],
            ProblemPath = positionalArgs[1],
            GeneratorType = positionalArgs[2].ToLowerInvariant(),
            AlgorithmType = positionalArgs.Count >= 4 ? positionalArgs[3].ToLowerInvariant() : "bfs",
            HeuristicType = positionalArgs.Count >= 5 ? positionalArgs[4].ToLowerInvariant() : "blind",
            JsonOutput = jsonOutput,
            IncludePlanSteps = includePlanSteps
        };

        return true;
    }

    private static void PrintUsage()
    {
        Console.WriteLine("Usage: Mimir.Cli <domain.pddl> <problem.pddl> <generator> [algorithm] [heuristic] [--json] [--no-plan]");
        Console.WriteLine("Generators: grounded, lifted");
        Console.WriteLine("Algorithms: bfs, ucs, astar, gbfs");
        Console.WriteLine("Heuristics: blind, goal-count, lifted-ff");
        Console.WriteLine("Options: --json emits a single JSON summary row; --no-plan suppresses plan steps in text output.");
    }
}
