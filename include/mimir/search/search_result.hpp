#ifndef PLANNERS_SEARCH_RESULT_HPP_
#define PLANNERS_SEARCH_RESULT_HPP_

namespace mimir::planners
{
    enum SearchResult
    {
        ABORTED,
        SOLVED,
        UNSOLVABLE
    };
}  // namespace planners

#endif  // PLANNERS_SEARCH_RESULT_HPP_
