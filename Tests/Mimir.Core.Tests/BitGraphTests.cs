using System;
using Mimir.Core.Grounding;
using Mimir.Core.Engines;
using Mimir.Core.Schemas;
using Mimir.Core.Algorithms.Graph;
using Xunit;

namespace Mimir.Core.Tests;

public class BitGraphTests
{
    [Theory]
    [InlineData(0, 0)]
    [InlineData(63, 1)]
    [InlineData(64, 1)]
    [InlineData(65, 2)]
    [InlineData(130, 3)]
    public void Constructor_ValidVertexCount_CalculatesWordsCorrectly(int vertexCount, int expectedWords)
    {
        var graph = new BitGraph(vertexCount);
        Assert.Equal(expectedWords, graph.WordsPerVertex);
    }

    [Fact]
    public void Constructor_NegativeVertexCount_ThrowsException()
    {
        Assert.Throws<ArgumentOutOfRangeException>(() => new BitGraph(-1));
    }

    [Fact]
    public void Constructor_VertexCountWhoseWordRoundingWouldOverflowInt_ThrowsException()
    {
        ArgumentOutOfRangeException exception = Assert.Throws<ArgumentOutOfRangeException>(
            () => new BitGraph(int.MaxValue));

        Assert.Equal("vertexCount", exception.ParamName);
    }

    [Fact]
    public void Constructor_MatrixLengthThatWouldWrapToZero_ThrowsException()
    {
        ArgumentOutOfRangeException exception = Assert.Throws<ArgumentOutOfRangeException>(
            () => new BitGraph(524_288));

        Assert.Equal("vertexCount", exception.ParamName);
    }

    [Fact]
    public void AddEdge_HasEdge_IsSymmetric()
    {
        var graph = new BitGraph(10);
        graph.AddEdge(2, 5);

        Assert.True(graph.HasEdge(2, 5));
        Assert.True(graph.HasEdge(5, 2));
    }

    [Fact]
    public void HasEdge_WithoutEdge_ReturnsFalse()
    {
        var graph = new BitGraph(10);
        graph.AddEdge(0, 1);

        Assert.False(graph.HasEdge(0, 2));
        Assert.False(graph.HasEdge(2, 0));
    }

    [Fact]
    public void AddEdge_OutOfBounds_ThrowsException()
    {
        var graph = new BitGraph(5);

        Assert.Throws<ArgumentOutOfRangeException>(() => graph.AddEdge(-1, 0));
        Assert.Throws<ArgumentOutOfRangeException>(() => graph.AddEdge(0, 5));
    }

    [Fact]
    public void HasEdge_OutOfBounds_ThrowsException()
    {
        var graph = new BitGraph(5);

        ArgumentOutOfRangeException uException = Assert.Throws<ArgumentOutOfRangeException>(
            () => graph.HasEdge(-1, 0));
        ArgumentOutOfRangeException vException = Assert.Throws<ArgumentOutOfRangeException>(
            () => graph.HasEdge(0, 5));

        Assert.Equal("u", uException.ParamName);
        Assert.Equal("v", vException.ParamName);
    }

    [Fact]
    public void GetNeighbors_ReturnsCorrectBitmask()
    {
        var graph = new BitGraph(100);
        
        // Add edges to vertex 0
        graph.AddEdge(0, 2);
        graph.AddEdge(0, 65);

        var neighbors = graph.GetNeighbors(0);

        // Expected: 
        // neighbors[0] should have bit 2 set
        // neighbors[1] should have bit 1 set (65 - 64)
        
        Assert.Equal(1UL << 2, neighbors[0]);
        Assert.Equal(1UL << 1, neighbors[1]);
    }

    [Fact]
    public void GetNeighbors_ReturnsCorrectLength()
    {
        var graph = new BitGraph(150);
        var neighbors = graph.GetNeighbors(5);

        // (150 + 63) / 64 = 3
        Assert.Equal(3, neighbors.Length);
    }

    [Fact]
    public void GetNeighbors_OutOfBounds_ThrowsException()
    {
        var graph = new BitGraph(5);
        var emptyGraph = new BitGraph(0);

        ArgumentOutOfRangeException negativeException = Assert.Throws<ArgumentOutOfRangeException>(
            () => graph.GetNeighbors(-1));
        ArgumentOutOfRangeException upperException = Assert.Throws<ArgumentOutOfRangeException>(
            () => graph.GetNeighbors(5));
        ArgumentOutOfRangeException emptyException = Assert.Throws<ArgumentOutOfRangeException>(
            () => emptyGraph.GetNeighbors(0));

        Assert.Equal("vertex", negativeException.ParamName);
        Assert.Equal("vertex", upperException.ParamName);
        Assert.Equal("vertex", emptyException.ParamName);
    }

    [Fact]
    public void IntersectCount_WithSharedNeighbors_ReturnsCorrectCount()
    {
        var graph = new BitGraph(100);

        // Node A (index 0) connected to 1, 10, 66
        graph.AddEdge(0, 1);
        graph.AddEdge(0, 10);
        graph.AddEdge(0, 66);

        // Node B (index 1) connected to 1, 66, 99
        graph.AddEdge(1, 1);
        graph.AddEdge(1, 66);
        graph.AddEdge(1, 99);

        var neighborsA = graph.GetNeighbors(0);
        var neighborsB = graph.GetNeighbors(1);

        int sharedCount = BitGraph.IntersectCount(neighborsA, neighborsB);

        // Should share neighbors 1 and 66
        Assert.Equal(2, sharedCount);
    }

    [Fact]
    public void IntersectCount_DisjointSets_ReturnsZero()
    {
        var graph = new BitGraph(10);

        // Node A connected to evens
        graph.AddEdge(0, 2);
        graph.AddEdge(0, 4);

        // Node B connected to odds
        graph.AddEdge(1, 3);
        graph.AddEdge(1, 5);

        var neighborsA = graph.GetNeighbors(0);
        var neighborsB = graph.GetNeighbors(1);

        int sharedCount = BitGraph.IntersectCount(neighborsA, neighborsB);

        Assert.Equal(0, sharedCount);
    }

    [Fact]
    public void RemoveEdge_AfterAddEdge_RemovesEdgeSymmetrically()
    {
        var graph = new BitGraph(10);
        graph.AddEdge(2, 5);
        graph.RemoveEdge(2, 5);

        Assert.False(graph.HasEdge(2, 5));
        Assert.False(graph.HasEdge(5, 2));
    }

    [Fact]
    public void RemoveEdge_OutOfBounds_ThrowsException()
    {
        var graph = new BitGraph(5);

        Assert.Throws<ArgumentOutOfRangeException>(() => graph.RemoveEdge(-1, 0));
        Assert.Throws<ArgumentOutOfRangeException>(() => graph.RemoveEdge(0, 5));
    }

    [Fact]
    public void Clone_ProducesIndependentCopy()
    {
        var graph = new BitGraph(10);
        graph.AddEdge(1, 3);

        var clone = graph.Clone();
        clone.AddEdge(2, 4);

        Assert.True(graph.HasEdge(1, 3));
        Assert.False(graph.HasEdge(2, 4));
        Assert.True(clone.HasEdge(1, 3));
        Assert.True(clone.HasEdge(2, 4));
    }
}
