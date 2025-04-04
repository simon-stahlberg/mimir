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

#include "cista/containers/optional.h"

#include "cista/serialization.h"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_action.hpp"

#include <gtest/gtest.h>

namespace mimir::tests
{

TEST(CistaTests, CistaFlexibleIndexVectorEmptyTest)
{
    using Optional = cista::optional<uint16_t>;

    auto optional = Optional();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(optional);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Optional>(buf.begin().base(), buf.end().base());

    // Only 4 bytes :^)
    EXPECT_EQ(buf.size(), 4);
}

}
