using System;
using System.Collections.Generic;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class KPartiteKCliqueTests
{
    private struct CliqueCollector
    {
        public List<int[]> Cliques { get; }
        public int Limit { get; }
        public int Visits;

        public CliqueCollector(List<int[]> cliques, int limit)
        {
            Cliques = cliques;
            Limit = limit;
            Visits = 0;
        }
    }

    private static bool CollectClique(
        ReadOnlySpan<int> clique,
        ref CliqueCollector collector)
    {
        collector.Cliques.Add(clique.ToArray());
        collector.Visits++;
        return collector.Visits < collector.Limit;
    }

    [Fact]
    public void ZeroPartitions_ReturnsEmpty()
    {
        var graph = new BitGraph(10);
        var partitions = new int[0][];
        var cliques = KPartiteKClique.FindCliques(graph, partitions);
        Assert.Empty(cliques);
    }

    [Fact]
    public void NullInputs_ThrowBeforeZeroPartitionEarlyReturn()
    {
        var graph = new BitGraph(1);
        int[][] noPartitions = Array.Empty<int[]>();

        ArgumentNullException graphException = Assert.Throws<ArgumentNullException>(
            () => KPartiteKClique.FindCliques(null!, noPartitions));
        ArgumentNullException partitionsException = Assert.Throws<ArgumentNullException>(
            () => KPartiteKClique.FindCliques(graph, null!));
        ArgumentNullException arrayCallbackException = Assert.Throws<ArgumentNullException>(
            () => KPartiteKClique.FindCliques(
                graph,
                noPartitions,
                (Func<int[], bool>)null!));
        ArgumentNullException spanCallbackException = Assert.Throws<ArgumentNullException>(
            () => KPartiteKClique.VisitCliques(
                graph,
                noPartitions,
                (KPartiteKClique.CliqueVisitor)null!));

        Assert.Equal("graph", graphException.ParamName);
        Assert.Equal("partitions", partitionsException.ParamName);
        Assert.Equal("onCliqueFound", arrayCallbackException.ParamName);
        Assert.Equal("onCliqueFound", spanCallbackException.ParamName);
    }

    [Fact]
    public void NullPartition_ThrowsBeforeSearch()
    {
        var graph = new BitGraph(1);

        ArgumentException exception = Assert.Throws<ArgumentException>(
            () => KPartiteKClique.FindCliques(graph, [null!]));

        Assert.Equal("partitions", exception.ParamName);
    }

    [Fact]
    public void EmptyPartitions_ReturnsEmpty()
    {
        var graph = new BitGraph(10);
        var partitions = new int[][] { Array.Empty<int>(), Array.Empty<int>() };
        var cliques = KPartiteKClique.FindCliques(graph, partitions);
        Assert.Empty(cliques);
    }

    [Fact]
    public void NoEdges_ReturnsEmpty()
    {
        var graph = new BitGraph(10);
        var partitions = new int[][] { new[] { 0, 1 }, new[] { 2, 3 } };
        var cliques = KPartiteKClique.FindCliques(graph, partitions);
        Assert.Empty(cliques);
    }

    [Fact]
    public void SinglePartition_ReturnsEachVertex()
    {
        var graph = new BitGraph(10);
        var partitions = new int[][] { new[] { 0, 2, 4 } };
        var cliques = KPartiteKClique.FindCliques(graph, partitions);
        
        Assert.Equal(3, cliques.Count);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 0);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 2);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 4);
    }

    [Fact]
    public void EarlyTermination_AbortsCorrectly()
    {
        var graph = new BitGraph(6);
        // Complete bipartite K3,3
        for (int i = 0; i < 3; i++)
            for (int j = 3; j < 6; j++)
                graph.AddEdge(i, j);

        var partitions = new int[][] { new[] { 0, 1, 2 }, new[] { 3, 4, 5 } };
        
        int count = 0;
        KPartiteKClique.FindCliques(graph, partitions, clique => 
        {
            count++;
            return false; // Abort immediately after the first clique
        });

        Assert.Equal(1, count);
    }

    [Fact]
    public void StatefulVisitorPreservesStateAndEarlyTermination()
    {
        var graph = new BitGraph(6);
        for (int left = 0; left < 3; left++)
        {
            for (int right = 3; right < 6; right++)
                graph.AddEdge(left, right);
        }

        int[][] partitions = [[0, 1, 2], [3, 4, 5]];
        var collector = new CliqueCollector(new List<int[]>(), limit: 2);
        KPartiteKClique.VisitCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            ref collector,
            CollectClique);

        Assert.Equal(2, collector.Visits);
        Assert.Equal(2, collector.Cliques.Count);
        Assert.All(collector.Cliques, clique => Assert.Equal(2, clique.Length));
    }

    [Fact]
    public void RepresentativeCliques_EmitOneWitnessPerRetainedBinding()
    {
        var graph = new BitGraph(6);
        int[][] partitions = [[0, 1], [2, 3], [4, 5]];
        for (int left = 0; left < partitions.Length; left++)
        {
            for (int right = left + 1; right < partitions.Length; right++)
            {
                foreach (int leftVertex in partitions[left])
                foreach (int rightVertex in partitions[right])
                    graph.AddEdge(leftVertex, rightVertex);
            }
        }

        var representatives = new List<int[]>();
        KPartiteKClique.VisitRepresentativeCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            partialConstraint: null,
            retainedPartitions: [true, false, true],
            clique =>
            {
                representatives.Add(clique.ToArray());
                return true;
            });

        Assert.Equal(4, representatives.Count);
        Assert.All(representatives, clique => Assert.Equal(2, clique[1]));
        Assert.Equal(4, representatives.Select(clique => (clique[0], clique[2])).Distinct().Count());
    }

    [Fact]
    public void RepresentativeCliques_EmitOneValidWitnessPerRetainedBinding()
    {
        var graph = new BitGraph(4);
        int[][] partitions = [[0, 1], [2, 3]];
        foreach (int retainedVertex in partitions[0])
        foreach (int witnessVertex in partitions[1])
            graph.AddEdge(retainedVertex, witnessVertex);

        var representatives = new List<int[]>();
        KPartiteKClique.VisitRepresentativeCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            partialConstraint: null,
            retainedPartitions: [true, false],
            clique =>
            {
                representatives.Add(clique.ToArray());
                return true;
            });

        Assert.Equal(2, representatives.Count);
        Assert.Equal(new[] { 0, 1 }, representatives.Select(clique => clique[0]).Order());
        Assert.All(representatives, clique => Assert.Contains(clique[1], partitions[1]));
    }

    [Fact]
    public void RepresentativeCliques_SkipBindingWithoutValidWitness()
    {
        var graph = new BitGraph(4);
        int[][] partitions = [[0, 1], [2, 3]];
        foreach (int left in partitions[0])
        foreach (int right in partitions[1])
            graph.AddEdge(left, right);

        var representatives = new List<int[]>();
        KPartiteKClique.VisitRepresentativeCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            (clique, assigned) => assigned.Length < 2 || clique[0] == 0 && clique[1] == 3,
            retainedPartitions: [true, false],
            clique =>
            {
                representatives.Add(clique.ToArray());
                return true;
            });

        int[] representative = Assert.Single(representatives);
        Assert.Equal(new[] { 0, 3 }, representative);
    }

    [Fact]
    public void RepresentativeCliques_NoRetainedPartitionsHonorsEarlyTermination()
    {
        var graph = new BitGraph(4);
        int[][] partitions = [[0, 1], [2, 3]];
        foreach (int left in partitions[0])
        foreach (int right in partitions[1])
            graph.AddEdge(left, right);

        int emitted = 0;
        KPartiteKClique.VisitRepresentativeCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            partialConstraint: null,
            retainedPartitions: [false, false],
            _ =>
            {
                emitted++;
                return false;
            });

        Assert.Equal(1, emitted);
    }

    [Fact]
    public void PartialConstraint_EmitsOnlyValidClique()
    {
        const int partitionCount = 8;
        const int valuesPerPartition = 10;
        var graph = new BitGraph(partitionCount * valuesPerPartition);
        var partitions = new int[partitionCount][];
        for (int partition = 0; partition < partitionCount; partition++)
        {
            partitions[partition] = Enumerable.Range(
                partition * valuesPerPartition,
                valuesPerPartition).ToArray();
        }

        for (int leftPartition = 0; leftPartition < partitionCount; leftPartition++)
        {
            for (int rightPartition = leftPartition + 1; rightPartition < partitionCount; rightPartition++)
            {
                foreach (int leftVertex in partitions[leftPartition])
                {
                    foreach (int rightVertex in partitions[rightPartition])
                        graph.AddEdge(leftVertex, rightVertex);
                }
            }
        }

        int emitted = 0;
        KPartiteKClique.VisitCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            (clique, assignedPartitions) =>
            {
                foreach (int partition in assignedPartitions)
                {
                    int expectedVertex = partitions[partition][partition];
                    if (clique[partition] != expectedVertex)
                        return false;
                }

                return true;
            },
            clique =>
            {
                emitted++;
                for (int partition = 0; partition < partitionCount; partition++)
                    Assert.Equal(partitions[partition][partition], clique[partition]);
                return true;
            });

        Assert.Equal(1, emitted);
    }

    [Theory]
    [InlineData(-1)]
    [InlineData(5)]
    [InlineData(999)]
    public void OutOfBoundsVertices_ThrowException(int invalidVertex)
    {
        var graph = new BitGraph(5);
        var partitions = new int[][] { new[] { 0, invalidVertex }, new[] { 1 } };
        graph.AddEdge(0, 1);

        ArgumentOutOfRangeException exception = Assert.Throws<ArgumentOutOfRangeException>(
            () => KPartiteKClique.FindCliques(graph, partitions));

        Assert.Equal("partitions", exception.ParamName);
        Assert.Equal(invalidVertex, exception.ActualValue);
    }

    [Fact]
    public void UnreachablePartition_PrunesEarly()
    {
        var graph = new BitGraph(10);
        // Fully connect partition 0 and 1
        graph.AddEdge(0, 2);
        graph.AddEdge(1, 2);
        
        // Partition 2 is completely isolated and has no edges
        var partitions = new int[][] { new[] { 0, 1 }, new[] { 2 }, new[] { 3, 4 } };
        var cliques = KPartiteKClique.FindCliques(graph, partitions);
        
        Assert.Empty(cliques);
    }

    public static IEnumerable<object[]> KnownGraphsData => new List<object[]>
    {
        // 8. CompleteBipartite_K33_ReturnsAllCombinations
        new object[] {
            6, // Vertex count
            new int[][] { // Edges
                [0,3], [0,4], [0,5],
                [1,3], [1,4], [1,5],
                [2,3], [2,4], [2,5]
            },
            new int[][] { [0,1,2], [3,4,5] }, // Partitions
            new int[][] { // Expected cliques
                [0,3], [0,4], [0,5],
                [1,3], [1,4], [1,5],
                [2,3], [2,4], [2,5]
            }
        },
        // 9. PerfectMatching_ReturnsIndependentEdges
        new object[] {
            6,
            new int[][] { [0,3], [1,4], [2,5] },
            new int[][] { [0,1,2], [3,4,5] },
            new int[][] { [0,3], [1,4], [2,5] }
        },
        // 10. Tripartite_MinusOneEdge_ReducesCombinations
        new object[] {
            6,
            new int[][] { 
                /* [0,2] is intentionally removed! */ [0,3], [1,2], [1,3], // P0 to P1
                [0,4], [0,5], [1,4], [1,5],                                // P0 to P2
                [2,4], [2,5], [3,4], [3,5]                                 // P1 to P2
            },
            new int[][] { [0,1], [2,3], [4,5] },
            new int[][] { 
                [0,3,4], [0,3,5], 
                [1,2,4], [1,2,5], 
                [1,3,4], [1,3,5] 
            }
        }
    };

    [Theory]
    [MemberData(nameof(KnownGraphsData))]
    public void FindCliques_OnKnownGraphs_ReturnsExpectedCliques(
        int vertexCount, int[][] edges, int[][] partitions, int[][] expectedCliques)
    {
        var graph = new BitGraph(vertexCount);
        foreach (var edge in edges)
        {
            graph.AddEdge(edge[0], edge[1]);
        }

        var cliques = KPartiteKClique.FindCliques(graph, partitions);

        // Normalize actual and expected cliques. 
        // Note: We intentionally do NOT sort vertices inside the cliques here. 
        // KPartiteKClique guarantees that clique[i] is ALWAYS the vertex from partition[i]!
        var normalizedActual = cliques
            .Select(c => string.Join(",", c)) 
            .OrderBy(s => s)
            .ToList();

        var normalizedExpected = expectedCliques
            .Select(c => string.Join(",", c))
            .OrderBy(s => s)
            .ToList();

        Assert.Equal(normalizedExpected.Count, normalizedActual.Count);
        for (int i = 0; i < normalizedExpected.Count; i++)
        {
            Assert.Equal(normalizedExpected[i], normalizedActual[i]);
        }
    }
}
