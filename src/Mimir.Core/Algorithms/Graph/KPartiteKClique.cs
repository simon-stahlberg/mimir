using System.Numerics;

namespace Mimir.Core.Algorithms.Graph;

/// <summary>
/// Implements a highly optimized branch-and-bound algorithm for finding k-cliques in k-partite graphs.
/// This leverages the fail-first heuristic and bit-parallel constraint propagation to achieve
/// significantly better performance than generalized clique solvers.
/// </summary>
public static class KPartiteKClique
{
    public delegate bool CliqueVisitor(ReadOnlySpan<int> clique);
    public delegate bool CliqueVisitor<TState>(
        ReadOnlySpan<int> clique,
        ref TState state);
    internal delegate bool PartialCliqueConstraint(
        ReadOnlySpan<int> clique,
        ReadOnlySpan<int> assignedPartitions);
    internal delegate bool PartialCliqueConstraint<TState>(
        ReadOnlySpan<int> clique,
        ReadOnlySpan<int> assignedPartitions,
        ref TState state);

    private struct DelegateState
    {
        public required CliqueVisitor Visitor { get; init; }
        public PartialCliqueConstraint? PartialConstraint { get; init; }
    }

    private static bool VisitWithDelegate(
        ReadOnlySpan<int> clique,
        ref DelegateState state)
        => state.Visitor(clique);

    private static bool CheckWithDelegate(
        ReadOnlySpan<int> clique,
        ReadOnlySpan<int> assignedPartitions,
        ref DelegateState state)
        => state.PartialConstraint is null
            || state.PartialConstraint(clique, assignedPartitions);

    public interface ICompatibilityProvider
    {
        bool HasCompatibility(int sourcePartition, int targetPartition);
        ReadOnlySpan<ulong> GetCompatibilityMask(int sourcePartition, int sourceVertex, int targetPartition);
    }

    internal sealed class SearchWorkspace
    {
        private ulong[] _candidateSets = Array.Empty<ulong>();
        private int[] _clique = Array.Empty<int>();
        private int[] _assignedPartitions = Array.Empty<int>();
        private int[] _remainingPartitions = Array.Empty<int>();

        internal ulong[] CandidateSets => _candidateSets;
        internal int[] Clique => _clique;
        internal int[] AssignedPartitions => _assignedPartitions;
        internal int[] RemainingPartitions => _remainingPartitions;

        internal void Prepare(int partitionCount, int wordsPerVertex)
        {
            int initialSetSize = checked(partitionCount * wordsPerVertex);
            int requiredSetSize = checked(partitionCount * initialSetSize);
            if (_candidateSets.Length < requiredSetSize)
                _candidateSets = new ulong[requiredSetSize];

            if (_clique.Length < partitionCount)
            {
                _clique = new int[partitionCount];
                _assignedPartitions = new int[partitionCount];
                _remainingPartitions = new int[partitionCount];
            }

            Array.Clear(_candidateSets, 0, initialSetSize);
            for (int partition = 0; partition < partitionCount; partition++)
                _remainingPartitions[partition] = partition;
        }
    }

    /// <summary>
    /// Finds all k-cliques where each clique contains exactly one vertex from each of the provided partitions.
    /// Returns a list of arrays, where each array contains the vertex IDs of a k-clique ordered by partition.
    /// </summary>
    public static List<int[]> FindCliques(BitGraph graph, int[][] partitions)
    {
        var results = new List<int[]>();
        FindCliques(graph, partitions, clique =>
        {
            results.Add(clique);
            return true;
        });
        return results;
    }

    /// <summary>
    /// Finds all k-cliques in a k-partite graph and invokes a callback for each.
    /// Return 'true' from the callback to continue searching, or 'false' to immediately terminate.
    /// </summary>
    public static void FindCliques(BitGraph graph, int[][] partitions, Func<int[], bool> onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);

        VisitCliques(graph, partitions, clique =>
        {
            int[] result = new int[clique.Length];
            clique.CopyTo(result);
            return onCliqueFound(result);
        });
    }

    /// <summary>
    /// Finds all k-cliques in a k-partite graph and invokes the provided visitor with a span over the current clique.
    /// The span is only valid for the duration of the callback and must not be stored.
    /// Return 'true' from the callback to continue searching, or 'false' to immediately terminate.
    /// </summary>
    public static void VisitCliques(BitGraph graph, int[][] partitions, CliqueVisitor onCliqueFound)
    {
        VisitCliques(graph, partitions, compatibilityProvider: null, onCliqueFound);
    }

    public static void VisitCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
        => VisitCliques(
            graph,
            partitions,
            compatibilityProvider: null,
            ref state,
            onCliqueFound);

    /// <summary>
    /// Finds all k-cliques in a k-partite graph and applies optional additional compatibility masks
    /// while propagating constraints. The compatibility provider can supply state-specific masks
    /// without materializing a mutated graph.
    /// </summary>
    public static void VisitCliques(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        CliqueVisitor onCliqueFound)
        => VisitCliques(
            graph,
            partitions,
            compatibilityProvider,
            partialConstraint: null,
            onCliqueFound);

    public static void VisitCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
        => VisitCliques(
            graph,
            partitions,
            compatibilityProvider,
            partialConstraint: null,
            ref state,
            onCliqueFound);

    internal static void VisitCliques(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitCliques(
            graph,
            partitions,
            compatibilityProvider,
            statefulConstraint,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ValidateInputs(graph, partitions, onCliqueFound);

        VisitCliquesCore(
            graph,
            partitions,
            partitionCounts: null,
            partitions.Length,
            new SearchWorkspace(),
            compatibilityProvider,
            partialConstraint,
            ref state,
            onCliqueFound);
    }

    internal static void VisitCliques(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitCliques(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            compatibilityProvider,
            statefulConstraint,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ValidateInputs(graph, partitions, onCliqueFound, partitionCounts, partitionCount);

        VisitCliquesCore(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            new SearchWorkspace(),
            compatibilityProvider,
            partialConstraint,
            ref state,
            onCliqueFound);
    }

    internal static void VisitCliquesTrusted(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitCliquesTrusted(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            workspace,
            compatibilityProvider,
            statefulConstraint,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitCliquesTrusted<TState>(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(workspace);
        VisitCliquesCore(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            workspace,
            compatibilityProvider,
            partialConstraint,
            ref state,
            onCliqueFound);
    }

    private static void VisitCliquesCore<TState>(
        BitGraph graph,
        int[][] partitions,
        int[]? partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        int k = partitionCount;
        if (k == 0) return;
        int wordsPerVertex = graph.WordsPerVertex;
        if (wordsPerVertex == 0) return;

        workspace.Prepare(k, wordsPerVertex);
        ulong[] candidateSetsByDepth = workspace.CandidateSets;
        int[] clique = workspace.Clique;
        int[] assignedPartitions = workspace.AssignedPartitions;
        ReadOnlySpan<int> initialRemainingPartitions =
            workspace.RemainingPartitions.AsSpan(0, k);
        Span<ulong> initialCandidateSets =
            candidateSetsByDepth.AsSpan(0, k * wordsPerVertex);

        // Populate the initial bitset for each partition
        for (int partition = 0; partition < k; partition++)
        {
            Span<ulong> partitionCandidates = initialCandidateSets.Slice(
                partition * wordsPerVertex,
                wordsPerVertex);
            int valueCount = partitionCounts?[partition] ?? partitions[partition].Length;
            for (int valueIndex = 0; valueIndex < valueCount; valueIndex++)
            {
                int vertex = partitions[partition][valueIndex];
                partitionCandidates[vertex / 64] |= 1UL << (vertex % 64);
            }
        }

        // Primitive flags keep the recursive frame smaller; an outcome enum
        // measurably regressed clique-heavy lifted searches.
        bool continueVisiting = true;
        Search(
            graph,
            candidateSetsByDepth,
            0,
            k,
            wordsPerVertex,
            initialCandidateSets,
            initialRemainingPartitions,
            clique,
            assignedPartitions,
            compatibilityProvider,
            partialConstraint,
            ref state,
            onCliqueFound,
            stopAfterFirst: false,
            ref continueVisiting);
    }

    internal static void VisitRepresentativeCliques(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        bool[] retainedPartitions,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitRepresentativeCliques(
            graph,
            partitions,
            compatibilityProvider,
            statefulConstraint,
            retainedPartitions,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitRepresentativeCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        bool[] retainedPartitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ValidateInputs(graph, partitions, onCliqueFound);
        ArgumentNullException.ThrowIfNull(retainedPartitions);
        if (retainedPartitions.Length != partitions.Length)
            throw new ArgumentException(
                "The retained-partition mask must have one entry per partition.",
                nameof(retainedPartitions));

        VisitRepresentativeCliquesCore(
            graph,
            partitions,
            partitionCounts: null,
            partitions.Length,
            new SearchWorkspace(),
            compatibilityProvider,
            partialConstraint,
            retainedPartitions,
            ref state,
            onCliqueFound);
    }

    internal static void VisitRepresentativeCliques(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        bool[] retainedPartitions,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitRepresentativeCliques(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            compatibilityProvider,
            statefulConstraint,
            retainedPartitions,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitRepresentativeCliques<TState>(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        bool[] retainedPartitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ValidateInputs(graph, partitions, onCliqueFound, partitionCounts, partitionCount);
        ArgumentNullException.ThrowIfNull(retainedPartitions);
        if (retainedPartitions.Length < partitionCount)
            throw new ArgumentException(nameof(retainedPartitions));

        VisitRepresentativeCliquesCore(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            new SearchWorkspace(),
            compatibilityProvider,
            partialConstraint,
            retainedPartitions,
            ref state,
            onCliqueFound);
    }

    internal static void VisitRepresentativeCliquesTrusted(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint? partialConstraint,
        bool[] retainedPartitions,
        CliqueVisitor onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        var state = new DelegateState
        {
            Visitor = onCliqueFound,
            PartialConstraint = partialConstraint,
        };
        PartialCliqueConstraint<DelegateState>? statefulConstraint =
            partialConstraint is null ? null : CheckWithDelegate;
        VisitRepresentativeCliquesTrusted(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            workspace,
            compatibilityProvider,
            statefulConstraint,
            retainedPartitions,
            ref state,
            VisitWithDelegate);
    }

    internal static void VisitRepresentativeCliquesTrusted<TState>(
        BitGraph graph,
        int[][] partitions,
        int[] partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        bool[] retainedPartitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(workspace);
        VisitRepresentativeCliquesCore(
            graph,
            partitions,
            partitionCounts,
            partitionCount,
            workspace,
            compatibilityProvider,
            partialConstraint,
            retainedPartitions,
            ref state,
            onCliqueFound);
    }

    private static void VisitRepresentativeCliquesCore<TState>(
        BitGraph graph,
        int[][] partitions,
        int[]? partitionCounts,
        int partitionCount,
        SearchWorkspace workspace,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        bool[] retainedPartitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        int k = partitionCount;
        if (k == 0) return;
        int wordsPerVertex = graph.WordsPerVertex;
        if (wordsPerVertex == 0) return;

        workspace.Prepare(k, wordsPerVertex);
        ulong[] candidateSetsByDepth = workspace.CandidateSets;
        int[] clique = workspace.Clique;
        int[] assignedPartitions = workspace.AssignedPartitions;
        ReadOnlySpan<int> initialRemainingPartitions =
            workspace.RemainingPartitions.AsSpan(0, k);
        Span<ulong> initialCandidateSets =
            candidateSetsByDepth.AsSpan(0, k * wordsPerVertex);

        for (int partition = 0; partition < k; partition++)
        {
            Span<ulong> candidates = initialCandidateSets.Slice(
                partition * wordsPerVertex,
                wordsPerVertex);
            int valueCount = partitionCounts?[partition] ?? partitions[partition].Length;
            for (int valueIndex = 0; valueIndex < valueCount; valueIndex++)
            {
                int vertex = partitions[partition][valueIndex];
                candidates[vertex / 64] |= 1UL << (vertex % 64);
            }
        }

        SearchRepresentatives(
            graph,
            candidateSetsByDepth,
            depth: 0,
            k,
            wordsPerVertex,
            initialCandidateSets,
            initialRemainingPartitions,
            clique,
            assignedPartitions,
            compatibilityProvider,
            partialConstraint,
            retainedPartitions,
            ref state,
            onCliqueFound);
    }

    private static void ValidateInputs<TState>(
        BitGraph graph,
        int[][] partitions,
        CliqueVisitor<TState> onCliqueFound)
    {
        ArgumentNullException.ThrowIfNull(graph);
        ArgumentNullException.ThrowIfNull(partitions);
        ArgumentNullException.ThrowIfNull(onCliqueFound);

        for (int partitionIndex = 0; partitionIndex < partitions.Length; partitionIndex++)
        {
            int[] partition = partitions[partitionIndex]
                ?? throw new ArgumentException("Partitions cannot contain null values.", nameof(partitions));
            foreach (int vertex in partition)
            {
                if (vertex < 0 || vertex >= graph.VertexCount)
                {
                    throw new ArgumentOutOfRangeException(
                        nameof(partitions),
                        vertex,
                        $"Partition {partitionIndex} contains a vertex outside the graph.");
                }
            }
        }
    }

    private static void ValidateInputs<TState>(
        BitGraph graph,
        int[][] partitions,
        CliqueVisitor<TState> onCliqueFound,
        int[] partitionCounts,
        int partitionCount)
    {
        ArgumentNullException.ThrowIfNull(graph);
        ArgumentNullException.ThrowIfNull(partitions);
        ArgumentNullException.ThrowIfNull(onCliqueFound);
        ArgumentNullException.ThrowIfNull(partitionCounts);

        if (partitionCount < 0 || partitionCount > partitions.Length)
            throw new ArgumentOutOfRangeException(nameof(partitionCount));
        if (partitionCounts.Length < partitionCount)
            throw new ArgumentException(nameof(partitionCounts));

        for (int partitionIndex = 0; partitionIndex < partitionCount; partitionIndex++)
        {
            int[] partition = partitions[partitionIndex]
                ?? throw new ArgumentException(nameof(partitions));
            int valueCount = partitionCounts[partitionIndex];
            if (valueCount < 0 || valueCount > partition.Length)
                throw new ArgumentOutOfRangeException(nameof(partitionCounts));

            for (int valueIndex = 0; valueIndex < valueCount; valueIndex++)
            {
                int vertex = partition[valueIndex];
                if (vertex < 0 || vertex >= graph.VertexCount)
                    throw new ArgumentOutOfRangeException(nameof(partitions), vertex, null);
            }
        }
    }

    private static bool Search<TState>(
        BitGraph graph,
        ulong[] candidateSetsByDepth,
        int depth,
        int k,
        int wordsPerVertex,
        Span<ulong> candidateSetsAtDepth,
        ReadOnlySpan<int> remainingPartitions,
        int[] clique,
        int[] assignedPartitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound,
        bool stopAfterFirst,
        ref bool continueVisiting)
    {
        int remainingCount = remainingPartitions.Length;
        if (remainingCount == 0)
        {
            continueVisiting = onCliqueFound(clique.AsSpan(0, k), ref state);
            return continueVisiting && !stopAfterFirst;
        }

        // Branching Step: Find the partition with the minimum number of candidates (Fail-First principle)
        int bestPartition = -1;
        int minimumCandidateCount = int.MaxValue;
        int bestRemainingIndex = -1;

        for (int remainingIndex = 0; remainingIndex < remainingCount; remainingIndex++)
        {
            int partition = remainingPartitions[remainingIndex];
            Span<ulong> partitionCandidates = candidateSetsAtDepth.Slice(
                partition * wordsPerVertex,
                wordsPerVertex);

            int candidateCount = 0;
            for (int wordIndex = 0; wordIndex < wordsPerVertex; wordIndex++)
            {
                candidateCount += BitOperations.PopCount(partitionCandidates[wordIndex]);
            }

            if (candidateCount < minimumCandidateCount)
            {
                minimumCandidateCount = candidateCount;
                bestPartition = partition;
                bestRemainingIndex = remainingIndex;
            }
        }

        // If any partition has 0 candidates, it's impossible to form a k-clique. Prune this branch.
        if (minimumCandidateCount == 0) return true;

        // Allocate the remaining partitions list for the next depth on the stack (zero GC allocation)
        Span<int> nextRemainingPartitions = stackalloc int[remainingCount - 1];
        int nextIndex = 0;
        for (int remainingIndex = 0; remainingIndex < remainingCount; remainingIndex++)
        {
            if (remainingIndex != bestRemainingIndex)
            {
                nextRemainingPartitions[nextIndex++] = remainingPartitions[remainingIndex];
            }
        }

        Span<ulong> chosenCandidates = candidateSetsAtDepth.Slice(
            bestPartition * wordsPerVertex,
            wordsPerVertex);
        // For the last partition, the recursive leaf does not read another depth slice.
        Span<ulong> candidateSetsAtNextDepth = default;
        if (remainingCount > 1)
        {
            int nextDepthOffset = (depth + 1) * k * wordsPerVertex;
            candidateSetsAtNextDepth = new Span<ulong>(
                candidateSetsByDepth,
                nextDepthOffset,
                k * wordsPerVertex);
        }

        // Ascending bit enumeration preserves deterministic action prefixes.
        for (int wordIndex = 0; wordIndex < wordsPerVertex; wordIndex++)
        {
            ulong candidateWord = chosenCandidates[wordIndex];
            while (candidateWord != 0)
            {
                int bitIndex = BitOperations.TrailingZeroCount(candidateWord);
                int vertex = wordIndex * 64 + bitIndex;
                candidateWord &= ~(1UL << bitIndex);

                clique[bestPartition] = vertex;
                assignedPartitions[depth] = bestPartition;
                if (partialConstraint != null
                    && !partialConstraint(
                        clique,
                        assignedPartitions.AsSpan(0, depth + 1),
                        ref state))
                {
                    continue;
                }

                ReadOnlySpan<ulong> neighbors = graph.GetNeighbors(vertex);
                bool allDomainsRemainNonempty = true;

                for (int remainingIndex = 0;
                     remainingIndex < nextRemainingPartitions.Length;
                     remainingIndex++)
                {
                    int partition = nextRemainingPartitions[remainingIndex];
                    Span<ulong> currentCandidates = candidateSetsAtDepth.Slice(
                        partition * wordsPerVertex,
                        wordsPerVertex);
                    Span<ulong> nextCandidates = candidateSetsAtNextDepth.Slice(
                        partition * wordsPerVertex,
                        wordsPerVertex);
                    bool hasCompatibility = compatibilityProvider != null
                        && compatibilityProvider.HasCompatibility(bestPartition, partition);
                    ReadOnlySpan<ulong> compatibilityMask = hasCompatibility
                        ? compatibilityProvider!.GetCompatibilityMask(
                            bestPartition,
                            vertex,
                            partition)
                        : default;

                    bool hasAny = false;
                    for (int maskWordIndex = 0;
                         maskWordIndex < wordsPerVertex;
                         maskWordIndex++)
                    {
                        ulong compatibleCandidates =
                            currentCandidates[maskWordIndex] & neighbors[maskWordIndex];
                        if (hasCompatibility)
                            compatibleCandidates &= compatibilityMask[maskWordIndex];
                        nextCandidates[maskWordIndex] = compatibleCandidates;
                        if (compatibleCandidates != 0) hasAny = true;
                    }

                    if (!hasAny)
                    {
                        allDomainsRemainNonempty = false;
                        break;
                    }
                }

                if (allDomainsRemainNonempty)
                {
                    if (!Search(
                            graph,
                            candidateSetsByDepth,
                            depth + 1,
                            k,
                            wordsPerVertex,
                            candidateSetsAtNextDepth,
                            nextRemainingPartitions,
                            clique,
                            assignedPartitions,
                            compatibilityProvider,
                            partialConstraint,
                            ref state,
                            onCliqueFound,
                            stopAfterFirst,
                            ref continueVisiting))
                    {
                        // Callback requested termination
                        return false;
                    }
                }
            }
        }

        return true;
    }

    private static bool SearchRepresentatives<TState>(
        BitGraph graph,
        ulong[] candidateSetsByDepth,
        int depth,
        int k,
        int wordsPerVertex,
        Span<ulong> candidateSetsAtDepth,
        ReadOnlySpan<int> remainingPartitions,
        int[] clique,
        int[] assignedPartitions,
        ICompatibilityProvider? compatibilityProvider,
        PartialCliqueConstraint<TState>? partialConstraint,
        bool[] retainedPartitions,
        ref TState state,
        CliqueVisitor<TState> onCliqueFound)
    {
        int remainingCount = remainingPartitions.Length;
        int bestPartition = -1;
        int minimumCandidates = int.MaxValue;
        int bestRemainingIndex = -1;

        for (int i = 0; i < remainingCount; i++)
        {
            int partition = remainingPartitions[i];
            Span<ulong> candidates = candidateSetsAtDepth.Slice(
                partition * wordsPerVertex,
                wordsPerVertex);
            int candidateCount = 0;
            for (int wordIndex = 0; wordIndex < wordsPerVertex; wordIndex++)
                candidateCount += BitOperations.PopCount(candidates[wordIndex]);

            if (candidateCount == 0)
                return true;
            if (!retainedPartitions[partition] || candidateCount >= minimumCandidates)
                continue;

            minimumCandidates = candidateCount;
            bestPartition = partition;
            bestRemainingIndex = i;
        }

        if (bestPartition < 0)
        {
            bool continueVisiting = true;
            Search(
                graph,
                candidateSetsByDepth,
                depth,
                k,
                wordsPerVertex,
                candidateSetsAtDepth,
                remainingPartitions,
                clique,
                assignedPartitions,
                compatibilityProvider,
                partialConstraint,
                ref state,
                onCliqueFound,
                stopAfterFirst: true,
                ref continueVisiting);
            return continueVisiting;
        }

        Span<int> nextRemainingPartitions = stackalloc int[remainingCount - 1];
        int nextIndex = 0;
        for (int i = 0; i < remainingCount; i++)
        {
            if (i != bestRemainingIndex)
                nextRemainingPartitions[nextIndex++] = remainingPartitions[i];
        }

        Span<ulong> chosenCandidates = candidateSetsAtDepth.Slice(
            bestPartition * wordsPerVertex,
            wordsPerVertex);
        Span<ulong> candidateSetsAtNextDepth = default;
        if (remainingCount > 1)
        {
            int nextDepthOffset = (depth + 1) * k * wordsPerVertex;
            candidateSetsAtNextDepth = new Span<ulong>(
                candidateSetsByDepth,
                nextDepthOffset,
                k * wordsPerVertex);
        }

        for (int wordIndex = 0; wordIndex < wordsPerVertex; wordIndex++)
        {
            ulong candidates = chosenCandidates[wordIndex];
            while (candidates != 0)
            {
                int bit = BitOperations.TrailingZeroCount(candidates);
                int vertex = wordIndex * 64 + bit;
                candidates &= ~(1UL << bit);

                clique[bestPartition] = vertex;
                assignedPartitions[depth] = bestPartition;
                if (partialConstraint != null
                    && !partialConstraint(
                        clique,
                        assignedPartitions.AsSpan(0, depth + 1),
                        ref state))
                {
                    continue;
                }

                ReadOnlySpan<ulong> neighbors = graph.GetNeighbors(vertex);
                bool allDomainsRemainNonempty = true;
                for (int i = 0; i < nextRemainingPartitions.Length; i++)
                {
                    int partition = nextRemainingPartitions[i];
                    Span<ulong> currentCandidates = candidateSetsAtDepth.Slice(
                        partition * wordsPerVertex,
                        wordsPerVertex);
                    Span<ulong> nextCandidates = candidateSetsAtNextDepth.Slice(
                        partition * wordsPerVertex,
                        wordsPerVertex);
                    bool hasCompatibility = compatibilityProvider != null
                        && compatibilityProvider.HasCompatibility(bestPartition, partition);
                    ReadOnlySpan<ulong> compatibilityMask = hasCompatibility
                        ? compatibilityProvider!.GetCompatibilityMask(bestPartition, vertex, partition)
                        : default;

                    bool hasAny = false;
                    for (int candidateWord = 0;
                         candidateWord < wordsPerVertex;
                         candidateWord++)
                    {
                        ulong compatible = currentCandidates[candidateWord] & neighbors[candidateWord];
                        if (hasCompatibility)
                            compatible &= compatibilityMask[candidateWord];
                        nextCandidates[candidateWord] = compatible;
                        if (compatible != 0) hasAny = true;
                    }

                    if (!hasAny)
                    {
                        allDomainsRemainNonempty = false;
                        break;
                    }
                }

                if (allDomainsRemainNonempty
                    && !SearchRepresentatives(
                        graph,
                        candidateSetsByDepth,
                        depth + 1,
                        k,
                        wordsPerVertex,
                        candidateSetsAtNextDepth,
                        nextRemainingPartitions,
                        clique,
                        assignedPartitions,
                        compatibilityProvider,
                        partialConstraint,
                        retainedPartitions,
                        ref state,
                        onCliqueFound))
                {
                    return false;
                }
            }
        }

        return true;
    }
}
