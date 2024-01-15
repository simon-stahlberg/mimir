#ifndef MIMIR_SEARCH_OPEN_LISTS_PRIORITY_QUEUE_HPP_
#define MIMIR_SEARCH_OPEN_LISTS_PRIORITY_QUEUE_HPP_

#include "../open_list_base.hpp"

#include <functional>
#include <queue>

namespace mimir::search
{

template<typename T>
class PriorityQueue : public OpenListBase<PriorityQueue, T> {
private:
    std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>>, std::greater<std::pair<double, T>>> priority_queue_;

    void insert_impl(const T& item, double priority) {
        priority_queue_.emplace(priority, item);
    }

    T pop_impl() {
        T item = priority_queue_.top().second;
        priority_queue_.pop();
        return item;
    }

    std::size_t size_impl() {
        return priority_queue_.size();
    }
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_OPEN_LISTS_PRIORITY_QUEUE_HPP_
