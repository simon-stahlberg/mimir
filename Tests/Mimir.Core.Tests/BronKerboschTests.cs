using System;
using System.Collections.Generic;
using System.Linq;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class BronKerboschTests
{
    [Fact]
    public void ZeroVertices_ReturnsEmptyList()
    {
        var graph = new BitGraph(0);
        var cliques = BronKerbosch.FindMaximalCliques(graph);
        Assert.Empty(cliques);
    }

    [Fact]
    public void NullInputs_ThrowBeforeZeroVertexEarlyReturn()
    {
        var graph = new BitGraph(0);

        ArgumentNullException listGraphException = Assert.Throws<ArgumentNullException>(
            () => BronKerbosch.FindMaximalCliques(null!));
        ArgumentNullException callbackGraphException = Assert.Throws<ArgumentNullException>(
            () => BronKerbosch.FindMaximalCliques(null!, _ => true));
        ArgumentNullException callbackException = Assert.Throws<ArgumentNullException>(
            () => BronKerbosch.FindMaximalCliques(graph, null!));

        Assert.Equal("graph", listGraphException.ParamName);
        Assert.Equal("graph", callbackGraphException.ParamName);
        Assert.Equal("onMaximalCliqueFound", callbackException.ParamName);
    }

    [Fact]
    public void EmptyGraph_ReturnsIsolatedVertices()
    {
        var graph = new BitGraph(3);
        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Equal(3, cliques.Count);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 0);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 1);
        Assert.Contains(cliques, c => c.Length == 1 && c[0] == 2);
    }

    [Fact]
    public void SelfLoop_ReturnsVertexAsMaximalClique()
    {
        var graph = new BitGraph(1);
        graph.AddEdge(0, 0);

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Equal(new[] { 0 }, Assert.Single(cliques));
    }

    [Fact]
    public void SelfLoops_DoNotChangeMaximalCliques()
    {
        var graph = new BitGraph(4);
        graph.AddEdge(0, 1);
        graph.AddEdge(0, 2);
        graph.AddEdge(1, 2);
        graph.AddEdge(1, 3);
        graph.AddEdge(2, 3);

        for (int vertex = 0; vertex < graph.VertexCount; vertex++)
            graph.AddEdge(vertex, vertex);

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Equal(2, cliques.Count);
        Assert.Contains(cliques, clique => clique.Order().SequenceEqual(new[] { 0, 1, 2 }));
        Assert.Contains(cliques, clique => clique.Order().SequenceEqual(new[] { 1, 2, 3 }));
    }

    [Fact]
    public void EarlyTermination_AbortsCorrectly()
    {
        var graph = new BitGraph(5);
        for (int i = 0; i < 5; i++)
        {
            for (int j = i + 1; j < 5; j++)
            {
                graph.AddEdge(i, j);
            }
        }

        int count = 0;
        BronKerbosch.FindMaximalCliques(graph, clique =>
        {
            count++;
            return false; // Abort immediately after the first clique
        });

        Assert.Equal(1, count);
    }

    [Theory]
    [InlineData(8)]
    [InlineData(10)]
    public void CompleteGraph_ReturnsOneLargeClique(int n)
    {
        var graph = new BitGraph(n);
        for (int i = 0; i < n; i++)
            for (int j = i + 1; j < n; j++)
                graph.AddEdge(i, j);

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Single(cliques);
        Assert.Equal(n, cliques[0].Length);
    }

    [Fact]
    public void StarGraph_ReturnsCliquesOfSizeTwo()
    {
        int n = 6; // 0 is center, 1..5 are leaves
        var graph = new BitGraph(n);
        for (int i = 1; i < n; i++)
        {
            graph.AddEdge(0, i);
        }

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Equal(5, cliques.Count);
        Assert.All(cliques, c => Assert.Equal(2, c.Length));
        Assert.All(cliques, c => Assert.Contains(0, c));
        for (int i = 1; i < n; i++)
            Assert.Contains(cliques, c => c.Contains(0) && c.Contains(i));
    }

    [Fact]
    public void DisjointCliques_ReturnsDistinctCliques()
    {
        var graph = new BitGraph(6);
        // Triangle 1: 0, 1, 2
        graph.AddEdge(0, 1); graph.AddEdge(1, 2); graph.AddEdge(0, 2);
        // Triangle 2: 3, 4, 5
        graph.AddEdge(3, 4); graph.AddEdge(4, 5); graph.AddEdge(3, 5);

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        Assert.Equal(2, cliques.Count);
        Assert.Contains(cliques, c => c.Length == 3 && c.Contains(0) && c.Contains(1) && c.Contains(2));
        Assert.Contains(cliques, c => c.Length == 3 && c.Contains(3) && c.Contains(4) && c.Contains(5));
    }

    public static IEnumerable<object[]> KnownGraphsData => new List<object[]>
    {
        // Diamond Graph (4 vertices, 5 edges)
        new object[] {
            4,
            new int[][] { [0,1], [0,2], [1,2], [1,3], [2,3] },
            new int[][] { [0,1,2], [1,2,3] }
        },
        // House Graph (5 vertices: roof 0, walls 1,2, floor 3,4)
        new object[] {
            5,
            new int[][] { [0,1], [0,2], [1,2], [1,3], [2,4], [3,4] },
            new int[][] { [0,1,2], [1,3], [2,4], [3,4] }
        },
        // Octahedron Graph (6 vertices, missing edges (0,5), (1,4), (2,3))
        new object[] {
            6,
            new int[][]
            {
                [0,1], [0,2], [0,3], [0,4],
                [1,2], [1,3], [1,5],
                [2,4], [2,5],
                [3,4], [3,5],
                [4,5]
            },
            new int[][] {
                [0,1,2], [0,2,4], [0,3,4], [0,1,3],
                [1,2,5], [2,4,5], [3,4,5], [1,3,5]
            }
        },
        // Cycle Graph C5 (5 vertices, a ring)
        new object[] {
            5,
            new int[][] { [0,1], [1,2], [2,3], [3,4], [4,0] },
            new int[][] { [0,1], [1,2], [2,3], [3,4], [0,4] }
        }
    };

    [Theory]
    [MemberData(nameof(KnownGraphsData))]
    public void FindMaximalCliques_OnKnownGraphs_ReturnsExpectedCliques(
        int vertexCount, int[][] edges, int[][] expectedCliques)
    {
        var graph = new BitGraph(vertexCount);
        foreach (var edge in edges)
        {
            graph.AddEdge(edge[0], edge[1]);
        }

        var cliques = BronKerbosch.FindMaximalCliques(graph);

        // Normalize the actual and expected cliques by sorting them into strings for deterministic comparison
        var normalizedActual = cliques
            .Select(c => string.Join(",", c.OrderBy(v => v)))
            .OrderBy(s => s)
            .ToList();

        var normalizedExpected = expectedCliques
            .Select(c => string.Join(",", c.OrderBy(v => v)))
            .OrderBy(s => s)
            .ToList();

        Assert.Equal(normalizedExpected.Count, normalizedActual.Count);
        for (int i = 0; i < normalizedExpected.Count; i++)
        {
            Assert.Equal(normalizedExpected[i], normalizedActual[i]);
        }
    }
}
