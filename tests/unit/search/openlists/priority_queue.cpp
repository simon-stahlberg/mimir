#include <mimir/search/openlists.hpp>

#include <gtest/gtest.h>


namespace mimir::tests
{

TEST(MimirTests, SearchOpenListsPriorityQueueTest) {
    // Instantiate grounded version
    auto priority_queue = OpenList<PriorityQueue<int>>();
    priority_queue.insert(42, 1.1);
    priority_queue.insert(5, 2.2);
    auto element = priority_queue.pop();
    EXPECT_EQ(element, 42);
}


}