#ifndef MIMIR_PLANNERS_PRIORITY_QUEUE_OPEN_LIST_HPP_
#define MIMIR_PLANNERS_PRIORITY_QUEUE_OPEN_LIST_HPP_

#include "open_list_base.hpp"

#include <functional>
#include <queue>

namespace mimir::planners
{
    template<typename T>
    class PriorityQueueOpenList : public OpenListBase<T>
    {
      private:
        std::priority_queue<std::pair<double, T>, std::vector<std::pair<double, T>>, std::greater<std::pair<double, T>>> priority_queue_;

      public:
        PriorityQueueOpenList();

        void insert(const T& item, double priority) override;

        T pop() override;

        std::size_t size() override;
    };

    std::shared_ptr<PriorityQueueOpenList<int32_t>> create_priority_queue_open_list();
}  // namespace planners

#endif  // MIMIR_PLANNERS_PRIORITY_QUEUE_OPEN_LIST_HPP_
