using System.Numerics;

namespace Mimir.Core.Algorithms.Graph;


/// <summary>
/// A high-performance, undirected graph represented as an adjacency matrix of bitboards.
/// Optimized for clique finding and dense graph operations.
/// </summary>
public class BitGraph
{
    private readonly int _vertexCount;
    private readonly int _wordsPerVertex;

    // A single, flattened 1D array storing the adjacency matrix in row-major order.
    // Using a 1D array guarantees contiguous memory allocation for maximum cache locality.
    private readonly ulong[] _matrix;

    public int VertexCount => _vertexCount;
    public int WordsPerVertex => _wordsPerVertex;

    public BitGraph(int vertexCount)
    {
        if (vertexCount < 0)
            throw new ArgumentOutOfRangeException(nameof(vertexCount));

        long wordsPerVertex = ((long)vertexCount + 63L) / 64L;
        long matrixLength = (long)vertexCount * wordsPerVertex;
        if (matrixLength > Array.MaxLength)
        {
            throw new ArgumentOutOfRangeException(
                nameof(vertexCount),
                vertexCount,
                "The graph's adjacency matrix would exceed the maximum supported array length.");
        }

        _vertexCount = vertexCount;
        _wordsPerVertex = (int)wordsPerVertex;
        _matrix = new ulong[(int)matrixLength];
    }

    /// <summary>
    /// Adds an undirected edge between u and v.
    /// </summary>
    public void AddEdge(int u, int v)
    {
        ValidateVertex(u, nameof(u));
        ValidateVertex(v, nameof(v));

        SetBit(u, v);
        SetBit(v, u);
    }

    private void SetBit(int row, int col)
    {
        int wordOffset = row * _wordsPerVertex + (col / 64);
        int bitIndex = col % 64;
        _matrix[wordOffset] |= (1UL << bitIndex);
    }

    public void RemoveEdge(int u, int v)
    {
        ValidateVertex(u, nameof(u));
        ValidateVertex(v, nameof(v));

        ClearBit(u, v);
        ClearBit(v, u);
    }

    private void ClearBit(int row, int col)
    {
        int wordOffset = row * _wordsPerVertex + (col / 64);
        int bitIndex = col % 64;
        _matrix[wordOffset] &= ~(1UL << bitIndex);
    }

    public BitGraph Clone()
    {
        var clone = new BitGraph(_vertexCount);
        Array.Copy(_matrix, clone._matrix, _matrix.Length);
        return clone;
    }

    /// <summary>
    /// Checks if there is an edge between u and v.
    /// </summary>
    public bool HasEdge(int u, int v)
    {
        ValidateVertex(u, nameof(u));
        ValidateVertex(v, nameof(v));

        int wordOffset = u * _wordsPerVertex + (v / 64);
        int bitIndex = v % 64;
        return (_matrix[wordOffset] & (1UL << bitIndex)) != 0;
    }

    /// <summary>
    /// Retrieves the neighbors of a vertex as a ReadOnlySpan of 64-bit words.
    /// This allows zero-allocation, extremely fast bitwise operations (like intersection).
    /// </summary>
    public ReadOnlySpan<ulong> GetNeighbors(int vertex)
    {
        ValidateVertex(vertex, nameof(vertex));

        int start = vertex * _wordsPerVertex;
        return new ReadOnlySpan<ulong>(_matrix, start, _wordsPerVertex);
    }

    private void ValidateVertex(int vertex, string parameterName)
    {
        if (vertex < 0 || vertex >= _vertexCount)
            throw new ArgumentOutOfRangeException(parameterName, vertex, "The vertex is outside the graph.");
    }

    /// <summary>
    /// Helper to intersect two span bitsets and return the number of set bits (shared neighbors).
    /// </summary>
    public static int IntersectCount(ReadOnlySpan<ulong> a, ReadOnlySpan<ulong> b)
    {
        int count = 0;
        int length = Math.Min(a.Length, b.Length);
        for (int i = 0; i < length; i++)
        {
            count += BitOperations.PopCount(a[i] & b[i]);
        }
        return count;
    }
}
