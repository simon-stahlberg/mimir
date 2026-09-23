namespace Mimir.Search;

public interface ISearchAlgorithm
{
    SearchResult Search(
        CancellationToken cancellationToken = default,
        int? maxExpandedStates = null);

    event Action<SearchNode>? NodeExpanded;
    event Action<SearchNode>? NodeGenerated;
    event Action<SearchTransition>? TransitionGenerated;
    event Action<SearchTransition>? TransitionDiscovered;
    event Action<SearchTransition>? TransitionPruned;
}
