#include "mimir/formalism/formalism.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/algorithms/event_handlers.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/plan.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{
TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth1Test)
{
    const auto atom_indices = AtomIndices({
        0,
        2,
        3,  // placeholder to generate tuples of size less than arity
    });

    const auto tuple_index_mapper = TupleIndexMapper(1, 3);

    const auto tuple_index_generator = SingleStateTupleIndexGenerator(tuple_index_mapper, atom_indices);

    auto iter = tuple_index_generator.begin();

    EXPECT_EQ("(0,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, tuple_index_generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWSingleStateTupleIndexGeneratorWidth2Test)
{
    const auto atom_indices = AtomIndices({
        0,
        2,
        3,  // placeholder to generate tuples of size less than arity
    });

    const auto tuple_index_mapper = TupleIndexMapper(2, 3);

    const auto tuple_index_generator = SingleStateTupleIndexGenerator(tuple_index_mapper, atom_indices);

    auto iter = tuple_index_generator.begin();

    EXPECT_EQ("(0,2,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(0,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("()", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, tuple_index_generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWCombinedStateTupleIndexGeneratorWidth1Test)
{
    const auto atom_indices = AtomIndices({
        0,
        2,
        4,  // placeholder to generate tuples of size less than arity
    });

    const auto add_atom_indices = AtomIndices({
        1,
        3,
    });

    const auto tuple_index_mapper = TupleIndexMapper(1, 4);

    const auto tuple_index_generator = StatePairTupleIndexGenerator(tuple_index_mapper, atom_indices, add_atom_indices);

    auto iter = tuple_index_generator.begin();

    EXPECT_EQ("(1,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, tuple_index_generator.end());
}

TEST(MimirTests, SearchAlgorithmsIWCombinedStateTupleIndexGeneratorWidth2Test)
{
    const auto atom_indices = AtomIndices({
        0,
        2,
        4,  // placeholder to generate tuples of size less than arity
    });

    const auto add_atom_indices = AtomIndices({
        1,
        3,
    });

    const auto tuple_index_mapper = TupleIndexMapper(2, 4);

    const auto tuple_index_generator = StatePairTupleIndexGenerator(tuple_index_mapper, atom_indices, add_atom_indices);

    auto iter = tuple_index_generator.begin();

    EXPECT_EQ("(1,2,)", tuple_index_mapper.tuple_index_to_string(*iter));
    EXPECT_EQ("(1,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,1,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(0,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(2,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));
    EXPECT_EQ("(1,3,)", tuple_index_mapper.tuple_index_to_string(*(++iter)));

    EXPECT_EQ(++iter, tuple_index_generator.end());
}
}
