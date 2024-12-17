/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "cista/containers/flexible_index_vector.h"

#include "cista/serialization.h"
#include "mimir/common/hash_cista.hpp"
#include "mimir/search/action.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(CistaTests, CistaFlexibleIndexVectorEmptyTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector();
    vec.compress();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(buf.size(), 32);
}

TEST(CistaTests, CistaFlexibleIndexVectorTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_index_vector<uint16_t>;

    auto vec = Vector({ 1, 3, 2, 4 });

    EXPECT_EQ(vec.size(), 4);
    EXPECT_EQ(vec.blocks().size(), 4);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 3);
    EXPECT_EQ(vec[2], 2);
    EXPECT_EQ(vec[3], 4);

    auto vec2 = vec;
    std::sort(vec2.begin(), vec2.end());
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 3);
    EXPECT_EQ(vec2[3], 4);

    vec.compress();

    const auto& const_vec = vec;

    EXPECT_EQ(const_vec.type(), cista::FlexibleIndexVectorDataType::UINT8_T);
    EXPECT_EQ(const_vec.size(), 4);
    EXPECT_EQ(const_vec.blocks().size(), 2);
    EXPECT_EQ(const_vec[0], 1);
    EXPECT_EQ(const_vec[1], 3);
    EXPECT_EQ(const_vec[2], 2);
    EXPECT_EQ(const_vec[3], 4);

    auto it = const_vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(it, const_vec.end());

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    std::cout << buf.size() << std::endl;

    EXPECT_EQ(deserialized.size(), 4);
    EXPECT_EQ(deserialized.blocks().size(), 2);
    EXPECT_EQ(deserialized[0], 1);
    EXPECT_EQ(deserialized[1], 3);
    EXPECT_EQ(deserialized[2], 2);
    EXPECT_EQ(deserialized[3], 4);

    it = const_vec.begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 3);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(it, const_vec.end());

    vec.clear();
    vec.push_back(4);
    EXPECT_EQ(vec[0], 4);
    EXPECT_EQ(vec.size(), 1);
}

}
