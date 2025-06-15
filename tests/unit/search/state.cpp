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

#include "mimir/search/state.hpp"

#include <gtest/gtest.h>

using namespace mimir::search;

namespace mimir::tests
{

TEST(MimirTests, SearchStateBuilderTest)
{
    auto builder = buffering::PackedStateBuilder();

    auto in_data = buffering::PackedStateData { 4, 5, 6, 7, 8, 9 };
    builder.serialize(in_data);
    EXPECT_EQ(builder.get_buffer_writer().get_buffer().size(), 7);

    auto view = buffering::PackedStateView(builder);
    auto [out_data, size] = view.deserialize();
    EXPECT_EQ(in_data.index, out_data.index);
    EXPECT_EQ(in_data.fluent_atoms_index, out_data.fluent_atoms_index);
    EXPECT_EQ(in_data.fluent_atoms_size, out_data.fluent_atoms_size);
    EXPECT_EQ(in_data.derived_atoms_index, out_data.derived_atoms_index);
    EXPECT_EQ(in_data.derived_atoms_size, out_data.derived_atoms_size);
    EXPECT_EQ(in_data.numeric_variables, out_data.numeric_variables);
}

TEST(MimirTests, SearchStateSetBuilderReuseTest)
{
    auto builder = buffering::PackedStateBuilder();
    auto states = buffering::PackedStateSet();

    auto s0_in_data = buffering::PackedStateData { 4, 5, 6, 7, 8, 9 };
    builder.serialize(s0_in_data);
    EXPECT_EQ(builder.get_buffer_writer().get_buffer().size(), 7);
    auto s0_state = *states.insert(builder).first;

    auto s1_in_data = buffering::PackedStateData { 5, 4, 7, 6, 900, 800 };
    builder.serialize(s1_in_data);
    EXPECT_EQ(builder.get_buffer_writer().get_buffer().size(), 9);
    auto s1_state = *states.insert(builder).first;

    EXPECT_EQ(states.size(), 2);

    EXPECT_EQ(s1_state.get_buffer() - s0_state.get_buffer(), 7);

    auto [s0_out_data, s0_size] = s0_state.deserialize();
    EXPECT_EQ(s0_in_data.index, s0_out_data.index);
    EXPECT_EQ(s0_in_data.fluent_atoms_index, s0_out_data.fluent_atoms_index);
    EXPECT_EQ(s0_in_data.fluent_atoms_size, s0_out_data.fluent_atoms_size);
    EXPECT_EQ(s0_in_data.derived_atoms_index, s0_out_data.derived_atoms_index);
    EXPECT_EQ(s0_in_data.derived_atoms_size, s0_out_data.derived_atoms_size);
    EXPECT_EQ(s0_in_data.numeric_variables, s0_out_data.numeric_variables);

    auto [s1_out_data, s1_size] = s1_state.deserialize();
    EXPECT_EQ(s1_in_data.index, s1_out_data.index);
    EXPECT_EQ(s1_in_data.fluent_atoms_index, s1_out_data.fluent_atoms_index);
    EXPECT_EQ(s1_in_data.fluent_atoms_size, s1_out_data.fluent_atoms_size);
    EXPECT_EQ(s1_in_data.derived_atoms_index, s1_out_data.derived_atoms_index);
    EXPECT_EQ(s1_in_data.derived_atoms_size, s1_out_data.derived_atoms_size);
    EXPECT_EQ(s1_in_data.numeric_variables, s1_out_data.numeric_variables);
}

// TEST(MimirTests, SearchStateSetHashAndEqualToTest)
//{
//     auto s0_builder = InternalStateImpl();
//     auto s0_in_data = InternalStateImpl::UnpackedData { valla::make_slot(4, 5), valla::make_slot(6, 7), 8, 9 };
//     s0_builder.set_data(s0_in_data);
//     auto s0_hash = loki::Hash<InternalStateImpl> {}(s0_builder);
//     EXPECT_EQ(s0_hash, 175247764240);
//
//     auto s1_builder = InternalStateImpl();
//     auto s1_in_data = InternalStateImpl::UnpackedData { valla::make_slot(5, 4), valla::make_slot(7, 6), 900, 800 };
//     s1_builder.set_data(s1_in_data);
//     auto s1_hash = loki::Hash<InternalStateImpl> {}(s1_builder);
//     EXPECT_EQ(s1_hash, 175247764317);
//
//     auto s2_builder = InternalStateImpl();
//     auto s2_in_data = InternalStateImpl::UnpackedData { valla::make_slot(4, 5), valla::make_slot(6, 7), 8, 9 };
//     s2_builder.set_data(s2_in_data);
//     auto s2_hash = loki::Hash<InternalStateImpl> {}(s2_builder);
//     EXPECT_EQ(s2_hash, 175247764240);
//
//     EXPECT_FALSE(loki::EqualTo<InternalStateImpl> {}(s0_builder, s1_builder));
//     EXPECT_TRUE(loki::EqualTo<InternalStateImpl> {}(s0_builder, s2_builder));
// }

}
