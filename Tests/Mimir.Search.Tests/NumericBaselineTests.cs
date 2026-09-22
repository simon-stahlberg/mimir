using System.Collections.Immutable;
using Mimir.Pddl.Ast;
using Mimir.Pddl.Ast.Models;
using Mimir.Pddl.Ast.Expressions;
using Mimir.Pddl.Parsing;
using System.Diagnostics;
using Mimir.Core.Engines;
using Mimir.Core.Grounding;
using Mimir.Core.Schemas;
using Xunit;
using Xunit.Abstractions;
using Action = Mimir.Core.Grounding.Action;

namespace Mimir.Search.Tests;

public class NumericBaselineTests(ITestOutputHelper output)
{
    private static string DataPath => Path.Combine(AppContext.BaseDirectory, "../../../../../Benchmark/numeric");

    // Complete layers previously checked against C++ c664ba695b7117055e3a7ade5e968a55a86003f9. Hydropower's last
    // layer has one state fewer than C++: two funds values that differ only by floating-point noise snap to the
    // same epsilon grid point, while C++ compares doubles exactly. TPP is excluded because its purchase costs
    // depend on changing fluents, which Mimir rejects at load.
    private static readonly IReadOnlyDictionary<string, (int Expanded, long Generated, int NewStates)[]> ExpectedLayers =
        new Dictionary<string, (int, long, int)[]>
        {
            ["block-grouping"] = [(1, 19, 19), (19, 361, 180), (180, 3422, 1141), (1141, 21712, 5476),
                (5476, 104315, 21328), (21328, 406722, 70494)],
            ["counters"] = [(1, 7, 7), (7, 50, 25), (25, 180, 61), (61, 442, 117),
                (117, 852, 191), (191, 1396, 278), (278, 2035, 370), (370, 2712, 459)],
            ["delivery"] = [(1, 20, 20), (20, 284, 150), (150, 1664, 648), (648, 6536, 2091),
                (2091, 19980, 5332), (5332, 48684, 11167), (11167, 100172, 20188), (20188, 177700, 31735)],
            ["drone"] = [(1, 5, 4), (4, 16, 11), (11, 45, 23), (23, 88, 38),
                (38, 144, 52), (52, 191, 59), (59, 211, 56), (56, 195, 48)],
            ["expedition"] = [(1, 6, 6), (6, 26, 13), (13, 48, 22), (22, 96, 39),
                (39, 148, 54), (54, 246, 86), (86, 338, 110), (110, 498, 158)],
            ["ext-plant-watering"] = [(1, 16, 16), (16, 256, 96), (96, 1506, 290), (290, 4372, 564),
                (564, 8289, 874), (874, 12745, 1255), (1255, 18253, 1787), (1787, 25846, 2545)],
            ["farmland"] = [(1, 3, 3), (3, 8, 4), (4, 14, 8), (8, 28, 12),
                (12, 40, 13), (13, 49, 17), (17, 64, 21), (21, 76, 22)],
            ["hydropower"] = [(1, 2, 2), (2, 5, 4), (4, 10, 6), (6, 15, 9),
                (9, 23, 15), (15, 38, 24), (24, 61, 38), (38, 97, 61)],
            ["sailing"] = [(1, 14, 14), (14, 196, 97), (97, 1358, 448), (448, 6272, 1562),
                (1562, 21868, 4420), (4420, 61880, 10670), (10670, 149380, 22774), (22774, 318836, 44063)],
        };

    public static IEnumerable<object[]> Instances()
    {
        foreach (string domainName in ExpectedLayers.Keys)
        {
            yield return [domainName, ApplicableActionGeneratorType.Lifted];
            yield return [domainName, ApplicableActionGeneratorType.Grounded];
        }
    }

    [Theory]
    [MemberData(nameof(Instances))]
    [Trait("Category", "NumericBaseline")]
    public void BreadthFirstLayers_MatchCppBaseline(string domainName, ApplicableActionGeneratorType generatorType)
    {
        (int Expanded, long Generated, int NewStates)[] expected = ExpectedLayers[domainName];
        long totalAllocated = GC.GetAllocatedBytesForCurrentThread();
        int gen0 = GC.CollectionCount(0);
        var total = Stopwatch.StartNew();
        Problem problem = LoadTraversalProblem(domainName,
            "pfile1.pddl", generatorType);
        Assert.True(problem.HasNumericPlanning);
        State initial = problem.InitialState;
        IApplicableActionGenerator generator = problem.GetApplicableActionGenerator(initial);
        var visited = new HashSet<State> { initial };
        var frontier = new List<State> { initial };
        output.WriteLine($"Setup: {total.Elapsed.TotalMilliseconds:F3} ms");

        for (int depth = 0; depth < expected.Length; depth++)
        {
            (int expectedExpanded, long expectedGenerated, int expectedNewStates) = expected[depth];
            long allocated = GC.GetAllocatedBytesForCurrentThread();
            var timer = Stopwatch.StartNew();
            var next = new List<State>();
            long generated = 0;
            foreach (State state in frontier)
            {
                ExtendedState expanded = state.Expand();
                foreach (Action action in generator.GetApplicableActions(expanded))
                {
                    State successor = expanded.Apply(action);
                    generated++;
                    if (visited.Add(successor))
                        next.Add(successor);
                }
            }
            timer.Stop();
            allocated = GC.GetAllocatedBytesForCurrentThread() - allocated;
            output.WriteLine($"Depth {depth}: expanded={frontier.Count}, generated={generated}, " +
                $"new={next.Count}, elapsed={timer.Elapsed.TotalMilliseconds:F3} ms, " +
                $"allocated={allocated} bytes");
            Assert.Equal(expectedExpanded, frontier.Count);
            Assert.Equal(expectedGenerated, generated);
            Assert.Equal(expectedNewStates, next.Count);
            frontier = next;
        }

        Assert.Equal(1 + expected.Sum(level => level.NewStates), visited.Count);
        output.WriteLine($"Total: {total.Elapsed.TotalMilliseconds:F3} ms, " +
            $"allocated={GC.GetAllocatedBytesForCurrentThread() - totalAllocated} bytes, gen0={GC.CollectionCount(0) - gen0}");
    }
    [Theory]
    [InlineData("block-grouping")]
    [InlineData("delivery")]
    public void OriginalUnsupportedGoalAndMetricRemainRejected(string domainName)
    {
        Domain domain = Domain.FromFile(Path.Combine(DataPath, domainName, "domain.pddl"));
        string file = Path.Combine(DataPath, domainName, "pfile1.pddl");
        Assert.Equal(PddlLoadErrorCode.UnsupportedFeature,
            Assert.Throws<PddlLoadException>(() => Problem.FromFile(domain, file)).ErrorCode);
    }

    [Fact]
    public void OriginalTppStateDependentCostsAreRejected()
    {
        Domain domain = Domain.FromFile(Path.Combine(DataPath, "tpp", "domain.pddl"));
        PddlLoadException exception = Assert.Throws<PddlLoadException>(() =>
            Problem.FromFile(domain, Path.Combine(DataPath, "tpp", "pfile1.pddl")));
        Assert.IsType<NotSupportedException>(exception.InnerException);
    }

    private static Problem LoadTraversalProblem(string domainName, string problemFile,
        ApplicableActionGeneratorType generatorType)
    {
        string domainText = File.ReadAllText(Path.Combine(DataPath, domainName, "domain.pddl"));
        string problemText = File.ReadAllText(Path.Combine(DataPath, domainName, problemFile));
        if (domainName == "block-grouping")
        {
            // Complete-frontier traversal never consults the goal.
            ProblemDefinition ast = PddlParser.ParseProblem(problemText);
            problemText = (ast with { Goal = new And(ImmutableArray<ILogicalExpression>.Empty) }).ToPddlString();
        }
        if (domainName == "delivery")
        {
            // C++ retains this ordinary numeric fluent in state identity, even though it is the metric.
            ProblemDefinition problem = PddlParser.ParseProblem(problemText);
            problemText = (problem with { Metric = null }).ToPddlString();
        }
        Domain loaded = Domain.FromText(domainText);
        return Problem.FromText(loaded, problemText, generatorType);
    }
}
