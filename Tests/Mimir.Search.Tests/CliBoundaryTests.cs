using System.Diagnostics;
using System.Text.Json;
using System.Text.Json.Nodes;
using Mimir.Search.Planning;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class CliBoundaryTests
{
    [Fact]
    public void JsonOutput_EmitsSingleSummaryObjectWithStableShape()
    {
        string domainPath = ExamplePath("blocks_3", "domain.pddl");
        string problemPath = ExamplePath("blocks_3", "p01.pddl");

        CliRun run = RunCli(domainPath, problemPath, "grounded", "bfs", "--json");

        Assert.Equal(0, run.ExitCode);
        Assert.Equal(string.Empty, run.StandardError);

        string[] lines = run.StandardOutput.Split(Environment.NewLine, StringSplitOptions.RemoveEmptyEntries);
        Assert.Single(lines);

        JsonObject summary = JsonNode.Parse(lines[0])!.AsObject();
        Assert.Equal(
            new[]
            {
                "Algorithm",
                "DomainPath",
                "Generator",
                "Heuristic",
                "MemoryAllocatedMb",
                "MemoryRetainedMb",
                "NodesExpanded",
                "NodesGenerated",
                "PlanCost",
                "PlanLength",
                "PlanSteps",
                "ProblemPath",
                "SearchTimeMs",
                "SetupTimeMs",
                "Status",
                "Success",
                "TimeMs"
            },
            summary.Select(pair => pair.Key).Order(StringComparer.Ordinal).ToArray());

        Assert.Equal(Path.GetFullPath(domainPath), summary["DomainPath"]!.GetValue<string>());
        Assert.Equal(Path.GetFullPath(problemPath), summary["ProblemPath"]!.GetValue<string>());
        Assert.Equal("grounded", summary["Generator"]!.GetValue<string>());
        Assert.Equal("bfs", summary["Algorithm"]!.GetValue<string>());
        Assert.Null(summary["Heuristic"]);
        Assert.Equal("Succeeded", summary["Status"]!.GetValue<string>());
        Assert.True(summary["Success"]!.GetValue<bool>());
        Assert.Null(summary["PlanSteps"]);

        double totalTimeMs = summary["TimeMs"]!.GetValue<double>();
        double setupTimeMs = summary["SetupTimeMs"]!.GetValue<double>();
        double searchTimeMs = summary["SearchTimeMs"]!.GetValue<double>();
        Assert.True(totalTimeMs >= 0d);
        Assert.True(setupTimeMs >= 0d);
        Assert.True(searchTimeMs >= 0d);
        Assert.InRange(Math.Abs(totalTimeMs - setupTimeMs - searchTimeMs), 0d, 0.000001d);
        Assert.True(summary["MemoryAllocatedMb"]!.GetValue<double>() >= 0d);
        Assert.True(summary["MemoryRetainedMb"]!.GetValue<double>() >= 0d);
        Assert.True(summary["NodesExpanded"]!.GetValue<int>() >= 0);
        Assert.True(summary["NodesGenerated"]!.GetValue<int>() >= 0);
        Assert.True(summary["PlanLength"]!.GetValue<int>() > 0);
        Assert.True(summary["PlanCost"]!.GetValue<double>() >= 0d);
    }

    [Fact]
    public void TextOutput_ReportsStatusAndSplitTiming()
    {
        string domainPath = ExamplePath("blocks_3", "domain.pddl");
        string problemPath = ExamplePath("blocks_3", "p01.pddl");

        CliRun run = RunCli(domainPath, problemPath, "grounded", "bfs", "--no-plan");

        Assert.Equal(0, run.ExitCode);
        Assert.Equal(string.Empty, run.StandardError);
        Assert.Contains("Result: Succeeded", run.StandardOutput, StringComparison.Ordinal);
        Assert.Contains("Time: ", run.StandardOutput, StringComparison.Ordinal);
        Assert.Contains(" ms (setup ", run.StandardOutput, StringComparison.Ordinal);
        Assert.Contains(" ms, search ", run.StandardOutput, StringComparison.Ordinal);
    }

    [Fact]
    public void InvalidArguments_ReportBoundaryErrorsOnExpectedStreams()
    {
        string domainPath = ExamplePath("blocks_3", "domain.pddl");
        string problemPath = ExamplePath("blocks_3", "p01.pddl");
        string missingDomainPath = Path.Combine(Path.GetTempPath(), "mimir-missing-domain.pddl");
        string missingProblemPath = Path.Combine(Path.GetTempPath(), "mimir-missing-problem.pddl");

        var cases = new[]
        {
            new InvalidCliCase(
                Array.Empty<string>(),
                0,
                "Usage: Mimir.Cli <domain.pddl> <problem.pddl> <generator>",
                string.Empty),
            new InvalidCliCase(
                new[] { "--help" },
                0,
                "Usage: Mimir.Cli <domain.pddl> <problem.pddl> <generator>",
                string.Empty),
            new InvalidCliCase(
                new[] { "a", "b", "c", "d", "e", "f" },
                1,
                "Usage: Mimir.Cli <domain.pddl> <problem.pddl> <generator>",
                "Too many positional arguments (6)."),
            new InvalidCliCase(
                new[] { missingDomainPath, problemPath, "grounded" },
                1,
                string.Empty,
                $"Error: Domain file not found: {missingDomainPath}"),
            new InvalidCliCase(
                new[] { domainPath, missingProblemPath, "grounded" },
                1,
                string.Empty,
                $"Error: Problem file not found: {missingProblemPath}"),
            new InvalidCliCase(
                new[] { domainPath, problemPath, "unknown" },
                1,
                string.Empty,
                "Error: Unknown generator type 'unknown'."),
            new InvalidCliCase(
                new[] { domainPath, problemPath, "grounded", "unknown" },
                1,
                string.Empty,
                "Error: Unknown algorithm type 'unknown'."),
            new InvalidCliCase(
                new[] { domainPath, problemPath, "grounded", "astar", "unknown" },
                1,
                string.Empty,
                "Error: Unknown heuristic type 'unknown'."),
            new InvalidCliCase(
                new[] { domainPath, problemPath, "grounded", "gbfs", "unknown" },
                1,
                string.Empty,
                "Error: Unknown heuristic type 'unknown'.")
        };

        foreach (InvalidCliCase testCase in cases)
        {
            CliRun run = RunCli(testCase.Arguments);

            Assert.Equal(testCase.ExpectedExitCode, run.ExitCode);
            Assert.Contains(testCase.ExpectedStdout, run.StandardOutput, StringComparison.Ordinal);
            Assert.Contains(testCase.ExpectedStderr, run.StandardError, StringComparison.Ordinal);
        }
    }

    [Theory]
    [InlineData("bfs", 7d)]
    [InlineData("ucs", 7d)]
    public void NonHeuristicAlgorithms_IgnoreHeuristicArgumentInCli(string algorithm, double expectedPlanCost)
    {
        string domainPath = ExamplePath("ferry", "domain.pddl");
        string problemPath = ExamplePath("ferry", "p01.pddl");

        CliRun run = RunCli(domainPath, problemPath, "grounded", algorithm, "unknown", "--json");

        Assert.Equal(0, run.ExitCode);
        Assert.Equal(string.Empty, run.StandardError);

        JsonObject summary = JsonNode.Parse(run.StandardOutput)!.AsObject();
        Assert.True(summary["Success"]!.GetValue<bool>());
        Assert.Equal(algorithm, summary["Algorithm"]!.GetValue<string>());
        Assert.Null(summary["Heuristic"]);
        Assert.Equal(expectedPlanCost, summary["PlanCost"]!.GetValue<double>());
    }

    [Theory]
    [InlineData("grounded:astar:goal-count", "grounded", "astar", "goal-count")]
    [InlineData("grounded:ucs", "grounded", "ucs", null)]
    [InlineData("grounded", "grounded", "bfs", null)]
    public void PlannerSpecAndCliPositionalParsing_ProduceSamePlanCost(
        string spec,
        string generator,
        string algorithm,
        string? heuristic)
    {
        string domainPath = ExamplePath("ferry", "domain.pddl");
        string problemPath = ExamplePath("ferry", "p01.pddl");
        var cliArgs = new List<string> { domainPath, problemPath, generator, algorithm };
        if (heuristic != null)
            cliArgs.Add(heuristic);
        cliArgs.Add("--json");

        CliRun cliRun = RunCli(cliArgs.ToArray());
        var directResult = PlannerFactory.CreateFromSpec(spec)
            .Solve(SearchTestHelpers.LoadProblem("ferry"));

        Assert.Equal(0, cliRun.ExitCode);
        Assert.Equal(string.Empty, cliRun.StandardError);

        JsonObject summary = JsonNode.Parse(cliRun.StandardOutput)!.AsObject();
        Assert.Equal(directResult.PlanLength, summary["PlanLength"]!.GetValue<int>());
        Assert.Equal(directResult.PlanCost, summary["PlanCost"]!.GetValue<double>());
    }

    [Fact]
    public void UserArgumentErrors_DoNotPrintStackTraces()
    {
        string domainPath = ExamplePath("blocks_3", "domain.pddl");
        string problemPath = ExamplePath("blocks_3", "p01.pddl");
        string missingDomainPath = Path.Combine(Path.GetTempPath(), "mimir-missing-domain.pddl");

        var invalidCalls = new[]
        {
            new[] { missingDomainPath, problemPath, "grounded" },
            new[] { domainPath, problemPath, "unknown" },
            new[] { domainPath, problemPath, "grounded", "unknown" },
            new[] { domainPath, problemPath, "grounded", "gbfs", "unknown" }
        };

        foreach (string[] args in invalidCalls)
        {
            CliRun run = RunCli(args);

            Assert.Equal(1, run.ExitCode);
            Assert.Contains("Error:", run.StandardError, StringComparison.Ordinal);
            Assert.DoesNotContain("Unhandled exception", run.StandardError, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("Stack trace", run.StandardError, StringComparison.OrdinalIgnoreCase);
            Assert.DoesNotContain("System.", run.StandardError, StringComparison.Ordinal);
        }
    }

    [Fact]
    public void JsonModeArgumentFailures_DoNotEmitPartialJson()
    {
        string domainPath = ExamplePath("blocks_3", "domain.pddl");
        string problemPath = ExamplePath("blocks_3", "p01.pddl");
        var invalidCalls = new[]
        {
            new[] { domainPath, problemPath, "unknown", "--json" },
            new[] { domainPath, problemPath, "grounded", "unknown", "--json" },
            new[] { domainPath, problemPath, "grounded", "astar", "unknown", "--json" }
        };

        foreach (string[] args in invalidCalls)
        {
            CliRun run = RunCli(args);

            Assert.Equal(1, run.ExitCode);
            Assert.Contains("Error:", run.StandardError, StringComparison.Ordinal);
            Assert.DoesNotContain("{", run.StandardOutput, StringComparison.Ordinal);
            Assert.DoesNotContain("}", run.StandardOutput, StringComparison.Ordinal);
        }
    }

    [Theory]
    [InlineData("bfs", null, 3, 102d)]
    [InlineData("ucs", null, 5, 32d)]
    [InlineData("astar", "blind", 5, 32d)]
    public void WeightedPlannerJsonOutput_MatchesDirectCostExpectations(
        string algorithm,
        string? heuristic,
        int expectedPlanLength,
        double expectedPlanCost)
    {
        string domainPath = ExamplePath("transport", "domain.pddl");
        string problemPath = ExamplePath("transport", "p02.pddl");
        var args = new List<string> { domainPath, problemPath, "grounded", algorithm };
        if (heuristic != null)
            args.Add(heuristic);
        args.Add("--json");

        CliRun run = RunCli(args.ToArray());

        Assert.Equal(0, run.ExitCode);
        Assert.Equal(string.Empty, run.StandardError);

        JsonObject summary = JsonNode.Parse(run.StandardOutput)!.AsObject();
        Assert.True(summary["Success"]!.GetValue<bool>());
        Assert.Equal(algorithm, summary["Algorithm"]!.GetValue<string>());
        Assert.Equal(expectedPlanLength, summary["PlanLength"]!.GetValue<int>());
        Assert.Equal(expectedPlanCost, summary["PlanCost"]!.GetValue<double>());
    }

    private static CliRun RunCli(params string[] args)
    {
        string repoRoot = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../../"));
        string cliProjectPath = Path.Combine(repoRoot, "src", "Mimir.Cli", "Mimir.Cli.csproj");
#if DEBUG
        const string configuration = "Debug";
#else
        const string configuration = "Release";
#endif

        var startInfo = new ProcessStartInfo
        {
            FileName = "dotnet",
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false
        };
        startInfo.ArgumentList.Add("run");
        startInfo.ArgumentList.Add("--no-build");
        startInfo.ArgumentList.Add("--configuration");
        startInfo.ArgumentList.Add(configuration);
        startInfo.ArgumentList.Add("--project");
        startInfo.ArgumentList.Add(cliProjectPath);
        startInfo.ArgumentList.Add("--");
        foreach (string arg in args)
            startInfo.ArgumentList.Add(arg);

        using var process = Process.Start(startInfo) ?? throw new InvalidOperationException("Failed to start Mimir.Cli.");
        string standardOutput = process.StandardOutput.ReadToEnd();
        string standardError = process.StandardError.ReadToEnd();

        if (!process.WaitForExit(30_000))
        {
            process.Kill(entireProcessTree: true);
            throw new TimeoutException("Mimir.Cli did not exit within 30 seconds.");
        }

        return new CliRun(process.ExitCode, standardOutput.Trim(), standardError.Trim());
    }

    private static string ExamplePath(string domainDir, string fileName)
        => Path.Combine(SearchTestHelpers.BasePath, domainDir, fileName);

    private sealed record CliRun(int ExitCode, string StandardOutput, string StandardError);

    private sealed record InvalidCliCase(
        string[] Arguments,
        int ExpectedExitCode,
        string ExpectedStdout,
        string ExpectedStderr);
}
