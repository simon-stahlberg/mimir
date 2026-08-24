using System.Diagnostics;
using System.Text.Json;
using Xunit;

namespace Mimir.Search.Tests;

public sealed class CliCostTests : IDisposable
{
    private readonly string _tempDir = Path.Combine(Path.GetTempPath(), "Mimir.Cli.Tests", Guid.NewGuid().ToString("N"));

    [Fact]
    public void Cli_ReportsFractionalPlanCostInJsonAndTextOutput()
    {
        Directory.CreateDirectory(_tempDir);
        var domainPath = Path.Combine(_tempDir, "domain.pddl");
        var problemPath = Path.Combine(_tempDir, "problem.pddl");
        File.WriteAllText(domainPath, FractionalCostDomain);
        File.WriteAllText(problemPath, FractionalCostProblem);

        var jsonRun = RunCli(domainPath, problemPath, "--json");
        using var document = JsonDocument.Parse(GetJsonSummaryLine(jsonRun.StandardOutput));

        Assert.Equal(0, jsonRun.ExitCode);
        Assert.Equal(3.75, document.RootElement.GetProperty("PlanCost").GetDouble());

        var textRun = RunCli(domainPath, problemPath, "--no-plan");

        Assert.Equal(0, textRun.ExitCode);
        Assert.Contains("Plan Cost: 3.75", textRun.StandardOutput, StringComparison.Ordinal);
    }

    public void Dispose()
    {
        if (!Directory.Exists(_tempDir))
            return;

        Directory.Delete(_tempDir, recursive: true);
    }

    private static CliRun RunCli(string domainPath, string problemPath, string extraArgument)
    {
        var repoRoot = Path.GetFullPath(Path.Combine(AppContext.BaseDirectory, "../../../../../"));
        var cliProjectPath = Path.Combine(repoRoot, "src", "Mimir.Cli", "Mimir.Cli.csproj");
        var startInfo = new ProcessStartInfo
        {
            FileName = "dotnet",
            Arguments = string.Join(
                " ",
                "run",
                "--no-build",
                "--project",
                Quote(cliProjectPath),
                "--",
                Quote(domainPath),
                Quote(problemPath),
                "grounded",
                "ucs",
                extraArgument),
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            UseShellExecute = false
        };

        using var process = Process.Start(startInfo) ?? throw new InvalidOperationException("Failed to start Mimir.Cli.");
        var standardOutput = process.StandardOutput.ReadToEnd();
        var standardError = process.StandardError.ReadToEnd();

        if (!process.WaitForExit(30_000))
        {
            process.Kill(entireProcessTree: true);
            throw new TimeoutException("Mimir.Cli did not exit within 30 seconds.");
        }

        if (process.ExitCode != 0)
            throw new InvalidOperationException($"Mimir.Cli failed with exit code {process.ExitCode}: {standardError}");

        return new CliRun(process.ExitCode, standardOutput, standardError);
    }

    private static string Quote(string value) => $"\"{value}\"";

    private static string GetJsonSummaryLine(string standardOutput)
    {
        foreach (string line in standardOutput.Split(Environment.NewLine, StringSplitOptions.RemoveEmptyEntries).Reverse())
        {
            string trimmed = line.Trim();
            if (trimmed.StartsWith('{'))
                return trimmed;
        }

        throw new InvalidOperationException($"Mimir.Cli did not emit a JSON summary line. Output: {standardOutput}");
    }

    private sealed record CliRun(int ExitCode, string StandardOutput, string StandardError);

    private const string FractionalCostDomain = """
(define (domain cli-fractional-cost)
  (:requirements :strips :action-costs)
  (:predicates (start) (middle) (goal))

  (:action first
    :parameters ()
    :precondition (start)
    :effect (and (not (start)) (middle) (increase (total-cost) 1.25)))

  (:action second
    :parameters ()
    :precondition (middle)
    :effect (and (not (middle)) (goal) (increase (total-cost) 2.5))))
""";

    private const string FractionalCostProblem = """
(define (problem cli-fractional-cost-problem)
  (:domain cli-fractional-cost)
  (:init (start) (= (total-cost) 0))
  (:goal (goal))
  (:metric minimize (total-cost)))
""";
}
