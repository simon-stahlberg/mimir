using System.Text.Json.Serialization;

namespace Mimir.Cli;

internal sealed record BenchmarkRunSummary(
    string DomainPath,
    string ProblemPath,
    string Generator,
    string Algorithm,
    string? Heuristic,
    string Status,
    bool Success,
    double TimeMs,
    double SetupTimeMs,
    double SearchTimeMs,
    double MemoryAllocatedMb,
    double MemoryRetainedMb,
    int NodesExpanded,
    int NodesGenerated,
    int PlanLength,
    double PlanCost,
    string[]? PlanSteps
);

[JsonSourceGenerationOptions(WriteIndented = false)]
[JsonSerializable(typeof(BenchmarkRunSummary))]
internal partial class BenchmarkRunSummaryJsonContext : JsonSerializerContext
{
}
