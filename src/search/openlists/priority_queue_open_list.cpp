#include "../../../include/mimir/search/openlists/priority_queue_open_list.hpp"

namespace mimir::planners
{
    template<typename T>
    PriorityQueueOpenList<T>::PriorityQueueOpenList() : priority_queue_()
    {
    }

    template<typename T>
    void PriorityQueueOpenList<T>::insert(const T& item, double priority)
    {
        priority_queue_.emplace(priority, item);
    }

    template<typename T>
    T PriorityQueueOpenList<T>::pop()
    {
        const std::pair<double, T> entry = priority_queue_.top();
        priority_queue_.pop();
        return entry.second;
    }

    template<typename T>
    std::size_t PriorityQueueOpenList<T>::size()
    {
        return priority_queue_.size();
    }

    std::shared_ptr<PriorityQueueOpenList<int32_t>> create_priority_queue_open_list() { return std::make_shared<PriorityQueueOpenList<int32_t>>(); }
}  // namespace planners
