#include "mimir/search/openlists.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(MimirTests, SearchOpenListsPriorityQueueTest)
{
    // Instantiate grounded version
    auto priority_queue = OpenList<OpenListDispatcher<PriorityQueueTag<int>>>();
    priority_queue.insert(1.1, 42);
    priority_queue.insert(2.2, 5);
    auto element = priority_queue.top();
    priority_queue.pop();
    EXPECT_EQ(element, 42);
}

}
