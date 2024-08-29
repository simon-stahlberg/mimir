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

#include "cista/containers/tuple.h"
#include "cista/containers/vector.h"  // Example Cista++ container
#include "cista/serialization.h"      // Serialization functions

#include <gtest/gtest.h>

namespace mimir::tests
{

void print_buffer(const std::vector<unsigned char>& buf)
{
    std::cout << "Serialized buffer (" << buf.size() << " bytes):" << std::endl;
    for (size_t i = 0; i < buf.size(); ++i)
    {
        if (i % 16 == 0)
            std::cout << std::endl;  // new line every 16 bytes
        std::cout << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(buf[i]) << " ";
    }
    std::cout << std::dec << std::endl;  // switch back to decimal output
}

TEST(MimirTests, CistaTest)
{
    namespace data = cista::offset;

    using CustomTuple = cista::tuple<int, data::string, double>;
    // Define a cista::tuple with different types.
    auto obj = CustomTuple { 42, data::string { "hello" }, 3.14 };

    std::vector<unsigned char> buf;
    {  // Serialize.
        buf = cista::serialize(obj);
    }

    print_buffer(buf);

    // Deserialize.
    auto deserialized = cista::deserialize<CustomTuple>(buf.begin().base(), buf.end().base());
    EXPECT_EQ(cista::get<0>(*deserialized), 42);
    EXPECT_EQ(cista::get<1>(*deserialized), data::string { "hello" });
    EXPECT_EQ(cista::get<2>(*deserialized), 3.14);

    cista::get<0>(*deserialized) = 43;
    EXPECT_EQ(cista::get<0>(*deserialized), 43);
}

}
