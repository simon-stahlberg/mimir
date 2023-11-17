#ifndef MIMIR_PLANNERS_SEARCH_RESULT_HPP_
#define MIMIR_PLANNERS_SEARCH_RESULT_HPP_

namespace mimir::planners
{
    enum SearchResult
    {
        ABORTED,
        SOLVED,
        UNSOLVABLE
    };
}  // namespace planners

#endif  // MIMIR_PLANNERS_SEARCH_RESULT_HPP_
