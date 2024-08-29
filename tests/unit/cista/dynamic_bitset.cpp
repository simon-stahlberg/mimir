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

#include "cista/containers/dynamic_bitset.h"

#include "cista/serialization.h"

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

TEST(MimirTests, CistaDynamicBitsetTest)
{
    namespace data = cista::raw;

    using Bitset = data::dynamic_bitset<uint64_t>;

    auto obj = Bitset(1000);
    for (size_t i = 0; i < 1000; ++i)
    {
        obj.set(i);
    }

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(obj);
    }

    print_buffer(buf);

    // Deserialize.
    auto deserialized = cista::deserialize<Bitset>(buf.begin().base(), buf.end().base());
}

}
