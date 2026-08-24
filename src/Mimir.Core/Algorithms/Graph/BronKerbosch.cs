using System.Buffers;
using System.Numerics;

namespace Mimir.Core.Algorithms.Graph;

/// <summary>
/// Implements the Bron-Kerbosch algorithm for finding all maximal cliques in an undirected graph.
/// This implementation is highly optimized using bitboards, pivoting, and zero-allocation recursion.
/// </summary>
public static class BronKerbosch
{
    /// <summary>
    /// Finds all maximal cliques in the given graph.
    /// Returns a list of arrays, where each array contains the vertex IDs of a maximal clique.
    /// </summary>
    public static List<int[]> FindMaximalCliques(BitGraph graph)
    {
        var results = new List<int[]>();
        FindMaximalCliques(graph, clique =>
        {
            results.Add(clique);
            return true; // continue searching
        });
        return results;
    }

    /// <summary>
    /// Finds maximal cliques in the given graph and invokes the provided callback for each clique.
    /// Return 'true' from the callback to continue searching, or 'false' to immediately terminate the search.
    /// </summary>
    public static void FindMaximalCliques(BitGraph graph, Func<int[], bool> onMaximalCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(graph);
        ArgumentNullException.ThrowIfNull(onMaximalCliqueFound);

        int words = graph.WordsPerVertex;
        if (words == 0) return;

        // R tracks the vertices in the current clique being explored.
        int[] R = new int[graph.VertexCount];

        // We use a single flat array from the ArrayPool to hold the P and X sets for every depth level.
        int requiredSize = 2 * words * (graph.VertexCount + 1);
        ulong[] pool = ArrayPool<ulong>.Shared.Rent(requiredSize);

        try
        {
            Array.Clear(pool, 0, requiredSize);

            // Initial P: all vertices in the graph
            Span<ulong> initialP = new Span<ulong>(pool, 0, words);
            for (int i = 0; i < graph.VertexCount; i++)
            {
                initialP[i / 64] |= (1UL << (i % 64));
            }

            Span<ulong> initialX = new Span<ulong>(pool, words, words);

            BronKerboschRecursive(graph, R, 0, initialP, initialX, pool, 1, onMaximalCliqueFound);
        }
        finally
        {
            ArrayPool<ulong>.Shared.Return(pool);
        }
    }

    private static bool BronKerboschRecursive(
        BitGraph graph,
        int[] R,
        int rCount,
        Span<ulong> P,
        Span<ulong> X,
        ulong[] pool,
        int depth,
        Func<int[], bool> onMaximalCliqueFound)
    {
        int words = graph.WordsPerVertex;

        if (IsZero(P) && IsZero(X))
        {
            // Found a maximal clique
            int[] clique = new int[rCount];
            Array.Copy(R, clique, rCount);
            return onMaximalCliqueFound(clique);
        }

        int pivot = ChoosePivot(graph, P, X);
        ReadOnlySpan<ulong> pivotNeighbors = pivot >= 0 ? graph.GetNeighbors(pivot) : default;
        int pivotWord = pivot >= 0 ? pivot / 64 : -1;
        ulong pivotMask = pivot >= 0 ? 1UL << (pivot % 64) : 0UL;

        for (int w = 0; w < words; w++)
        {
            ulong pWord = P[w];
            if (pivot >= 0)
            {
                ulong pivotNeighborWord = pivotNeighbors[w];
                if (w == pivotWord)
                    pivotNeighborWord &= ~pivotMask;

                pWord &= ~pivotNeighborWord; // P \ N(pivot)
            }

            while (pWord != 0)
            {
                int bit = BitOperations.TrailingZeroCount(pWord);
                int v = w * 64 + bit;
                pWord &= ~(1UL << bit); // Clear the bit we are processing

                // Rent memory slice for the next recursion depth
                int nextOffset = depth * 2 * words;
                Span<ulong> nextP = new Span<ulong>(pool, nextOffset, words);
                Span<ulong> nextX = new Span<ulong>(pool, nextOffset + words, words);

                ReadOnlySpan<ulong> vNeighbors = graph.GetNeighbors(v);

                for (int i = 0; i < words; i++)
                {
                    nextP[i] = P[i] & vNeighbors[i];
                    nextX[i] = X[i] & vNeighbors[i];
                }

                // Self-loops do not contribute to clique adjacency.
                int vertexWord = v / 64;
                ulong vertexMask = 1UL << (v % 64);
                nextP[vertexWord] &= ~vertexMask;
                nextX[vertexWord] &= ~vertexMask;

                R[rCount] = v;

                // If the callback returned false, we bubble it up to abort the entire search instantly.
                if (!BronKerboschRecursive(graph, R, rCount + 1, nextP, nextX, pool, depth + 1, onMaximalCliqueFound))
                {
                    return false;
                }

                // P := P \ {v}
                P[w] &= ~(1UL << bit);

                // X := X ∪ {v}
                X[w] |= (1UL << bit);
            }
        }

        return true;
    }

    private static bool IsZero(Span<ulong> span)
    {
        for (int i = 0; i < span.Length; i++)
        {
            if (span[i] != 0) return false;
        }

        return true;
    }

    private static int ChoosePivot(BitGraph graph, Span<ulong> P, Span<ulong> X)
    {
        int bestPivot = -1;
        int maxDegreeInP = -1;
        int words = graph.WordsPerVertex;

        for (int w = 0; w < words; w++)
        {
            ulong puX = P[w] | X[w];
            while (puX != 0)
            {
                int bit = BitOperations.TrailingZeroCount(puX);
                int u = w * 64 + bit;
                puX &= ~(1UL << bit);

                ReadOnlySpan<ulong> neighbors = graph.GetNeighbors(u);
                int vertexWord = u / 64;
                ulong vertexMask = 1UL << (u % 64);
                int degreeInP = 0;
                for (int i = 0; i < words; i++)
                {
                    ulong neighborsInP = P[i] & neighbors[i];
                    if (i == vertexWord)
                        neighborsInP &= ~vertexMask;

                    degreeInP += BitOperations.PopCount(neighborsInP);
                }

                if (degreeInP > maxDegreeInP)
                {
                    maxDegreeInP = degreeInP;
                    bestPivot = u;
                }
            }
        }

        return bestPivot;
    }
}
