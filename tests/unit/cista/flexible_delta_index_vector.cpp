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

#include "cista/containers/flexible_delta_index_vector.h"

#include "cista/serialization.h"
#include "mimir/common/hash.hpp"
#include "mimir/formalism/ground_action.hpp"

#include <bitset>
#include <gtest/gtest.h>
#include <iostream>

namespace mimir::tests
{

void print_bits(const uint16_t* vec, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        std::bitset<16> bits(vec[i]);  // 16 bits for uint8_t
        std::cout << bits << " ";
    }
    std::cout << std::endl;
}

TEST(CistaTests, CistaFlexibleIndexVectorEmptyTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint16_t>;

    auto vec = Vector();
    vec.compress();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(buf.size(), 32);
    EXPECT_EQ(deserialized.size(), 0);
}

TEST(CistaTests, CistaFlexibleIndexVectorSize1Test)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint16_t>;

    auto vec = Vector();
    vec.push_back(5);
    vec.compress();

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(buf.size(), 34);
    EXPECT_EQ(deserialized.size(), 1);
    EXPECT_EQ(*deserialized.compressed_begin(), 5);
}

TEST(CistaTests, CistaFlexibleIndexVectorIterateUncompressedTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint16_t>;

    auto vec = Vector({ 1, 2, 4, 9, 16 });

    // test default vector iterator
    auto it = vec.uncompressed_begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(it, vec.uncompressed_end());

    // test our custom const_iterator
    auto cit = vec.uncompressed_begin();
    EXPECT_EQ(*cit, 1);
    ++cit;
    EXPECT_EQ(*cit, 2);
    ++cit;
    EXPECT_EQ(*cit, 4);
    ++cit;
    EXPECT_EQ(*cit, 9);
    ++cit;
    EXPECT_EQ(*cit, 16);
    ++cit;
    EXPECT_EQ(cit, vec.uncompressed_end());
}

TEST(CistaTests, CistaFlexibleIndexVectorIterateCompressedTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint32_t>;

    auto vec = Vector({ 0, std::numeric_limits<uint32_t>::max() });

    {
        // test default vector iterator
        auto it = vec.uncompressed_begin();
        EXPECT_EQ(*it, 0);
        ++it;
        EXPECT_EQ(*it, std::numeric_limits<uint32_t>::max());
        ++it;
        EXPECT_EQ(it, vec.uncompressed_end());

        // test our custom const_iterator
        auto cit = vec.uncompressed_begin();
        EXPECT_EQ(*cit, 0);
        ++cit;
        EXPECT_EQ(*cit, std::numeric_limits<uint32_t>::max());
        ++cit;
        EXPECT_EQ(cit, vec.uncompressed_end());
    }

    vec.compress();

    {
        std::vector<uint8_t> buf;
        {  // Serialize.
            buf = cista::serialize(vec);
        }

        // Deserialize.
        const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

        auto it = deserialized.compressed_begin();
        EXPECT_EQ(*it, 0);
        ++it;
        EXPECT_EQ(*it, std::numeric_limits<uint32_t>::max());
        ++it;
        EXPECT_EQ(it, deserialized.compressed_end());
    }
}

TEST(CistaTests, CistaFlexibleIndexVectorTest)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint16_t>;

    auto vec = Vector({ 1, 2, 4, 9, 16 });

    EXPECT_EQ(vec.bit_width(), 0);
    EXPECT_EQ(vec.bit_width_log2(), 0);
    EXPECT_EQ(vec.elements_per_block(), 0);
    EXPECT_EQ(vec.elements_per_block_log2(), 0);
    EXPECT_EQ(vec.size(), 5);
    EXPECT_EQ(vec.blocks().size(), 5);
    EXPECT_EQ(vec[0], 1);
    EXPECT_EQ(vec[1], 2);
    EXPECT_EQ(vec[2], 4);
    EXPECT_EQ(vec[3], 9);
    EXPECT_EQ(vec[4], 16);

    auto vec2 = vec;
    std::sort(vec2.uncompressed_begin(), vec2.uncompressed_begin());
    EXPECT_EQ(vec2[0], 1);
    EXPECT_EQ(vec2[1], 2);
    EXPECT_EQ(vec2[2], 4);
    EXPECT_EQ(vec2[3], 9);
    EXPECT_EQ(vec2[4], 16);

    vec.compress();

    // print_bits(reinterpret_cast<const uint16_t*>(vec.blocks().data()), vec.blocks().size());

    const auto& const_vec = vec;

    EXPECT_EQ(const_vec.bit_width(), 4);
    EXPECT_EQ(const_vec.bit_width_log2(), 2);
    EXPECT_EQ(const_vec.elements_per_block(), 4);
    EXPECT_EQ(const_vec.elements_per_block_log2(), 2);
    EXPECT_EQ(const_vec.size(), 5);
    EXPECT_EQ(const_vec.blocks().size(), 2);

    auto it = const_vec.compressed_begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(it, const_vec.compressed_end());

    std::vector<uint8_t> buf;
    {  // Serialize.
        buf = cista::serialize(vec);
    }

    // Deserialize.
    const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

    EXPECT_EQ(deserialized.bit_width(), 4);
    EXPECT_EQ(deserialized.bit_width_log2(), 2);
    EXPECT_EQ(deserialized.elements_per_block(), 4);
    EXPECT_EQ(deserialized.elements_per_block_log2(), 2);
    EXPECT_EQ(deserialized.size(), 5);
    EXPECT_EQ(deserialized.blocks().size(), 2);

    it = const_vec.compressed_begin();
    EXPECT_EQ(*it, 1);
    ++it;
    EXPECT_EQ(*it, 2);
    ++it;
    EXPECT_EQ(*it, 4);
    ++it;
    EXPECT_EQ(*it, 9);
    ++it;
    EXPECT_EQ(*it, 16);
    ++it;
    EXPECT_EQ(it, const_vec.compressed_end());

    vec.clear();
    vec.push_back(4);
    EXPECT_EQ(vec[0], 4);
    EXPECT_EQ(vec.size(), 1);
}

TEST(CistaTests, CistaFlexibleIndexVectorIterateCompressed2Test)
{
    namespace data = cista::offset;

    using Vector = data::flexible_delta_index_vector<uint32_t>;

    auto vec =
        Vector({ 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,   12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,
                 28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,   40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,  54,  55,
                 56,  57,  58,  59,  60,  61,  62,  63,  64,  65,  66,  67,   68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,  81,  82,  83,
                 84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,   96,  97,  98,  99,  101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
                 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,  125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140,
                 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152,  153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
                 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180,  181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196,
                 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,  209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223, 224,
                 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236,  237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252,
                 253, 254, 255, 256, 257, 258, 259, 260, 261, 262, 263, 264,  265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 280,
                 281, 282, 283, 284, 285, 286, 287, 288, 289, 290, 291, 292,  293, 294, 295, 296, 297, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308,
                 309, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319, 320,  321, 322, 323, 324, 325, 326, 327, 328, 329, 330, 331, 332, 333, 334, 335, 336,
                 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348,  349, 350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364,
                 365, 366, 367, 368, 369, 370, 371, 372, 373, 374, 375, 376,  377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 391, 392,
                 393, 394, 395, 396, 397, 398, 399, 400, 401, 402, 403, 404,  405, 406, 407, 408, 409, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420,
                 421, 422, 423, 424, 425, 426, 427, 428, 429, 430, 431, 432,  433, 434, 435, 436, 437, 438, 439, 440, 441, 442, 443, 444, 445, 446, 447, 448,
                 449, 450, 451, 452, 453, 454, 455, 456, 457, 458, 459, 460,  461, 462, 463, 464, 465, 466, 467, 468, 469, 470, 471, 472, 473, 474, 475, 476,
                 477, 478, 479, 480, 481, 482, 483, 484, 485, 486, 487, 488,  489, 490, 491, 492, 493, 494, 495, 496, 497, 498, 499, 500, 501, 502, 503, 504,
                 505, 506, 507, 508, 509, 510, 511, 512, 513, 514, 515, 516,  517, 518, 519, 520, 521, 522, 523, 524, 525, 526, 527, 528, 529, 530, 531, 532,
                 533, 534, 535, 536, 537, 538, 539, 540, 541, 542, 543, 544,  545, 546, 547, 548, 549, 550, 551, 552, 553, 554, 555, 556, 557, 558, 559, 560,
                 561, 562, 563, 564, 565, 566, 567, 568, 569, 570, 571, 572,  573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588,
                 589, 590, 591, 592, 593, 594, 596, 597, 598, 599, 600, 66728 });

    /*
    Wrong output: [0, 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 66, 78, 91, 105, 120, 136, 153, 171, 190, 210, 231, 253, 276, 300, 325, 351, 378, 406, 435, 465, 496,
    528, 561, 595, 630, 666, 703, 741, 780, 820, 861, 903, 946, 990, 1035, 1081, 1128, 1176, 1225, 1275, 1326, 1378, 1431, 1485, 1540, 1596, 1653, 1711, 1770,
    1830, 1891, 1953, 2016, 2080, 2145, 2211, 2278, 2346, 2415, 2485, 2556, 2628, 2701, 2775, 2850, 2926, 3003, 3081, 3160, 3240, 3321, 3403, 3486, 3570, 3655,
    3741, 3828, 3916, 4005, 4095, 4186, 4278, 4371, 4465, 4560, 4656, 4753, 4851, 4950, 5051, 5153, 5256, 5360, 5465, 5571, 5678, 5786, 5895, 6005, 6116, 6228,
    6341, 6455, 6570, 6686, 6803, 6921, 7040, 7160, 7281, 7403, 7526, 7650, 7775, 7901, 8028, 8156, 8285, 8415, 8546, 8678, 8811, 8945, 9080, 9216, 9353, 9491,
    9630, 9770, 9911, 10053, 10196, 10340, 10485, 10631, 10778, 10926, 11075, 11225, 11376, 11528, 11681, 11835, 11990, 12146, 12303, 12461, 12620, 12780,
    12941, 13103, 13266, 13430, 13595, 13761, 13928, 14096, 14265, 14435, 14606, 14778, 14951, 15125, 15300, 15476, 15653, 15831, 16010, 16190, 16371, 16553,
    16736, 16920, 17105, 17291, 17478, 17666, 17855, 18045, 18236, 18428, 18621, 18815, 19010, 19206, 19403, 19601, 19800, 20000, 20201, 20403, 20606, 20810,
    21015, 21221, 21428, 21636, 21845, 22055, 22266, 22478, 22691, 22905, 23120, 23336, 23553, 23771, 23990, 24210, 24431, 24653, 24876, 25100, 25325, 25551,
    25778, 26006, 26235, 26465, 26696, 26928, 27161, 27395, 27630, 27866, 28103, 28341, 28580, 28820, 29061, 29303, 29546, 29790, 30035, 30281, 30528, 30776,
    31025, 31275, 31526, 31778, 32031, 32285, 32540, 32796, 33053, 33311, 33570, 33830, 34091, 34353, 34616, 34880, 35145, 35411, 35678, 35946, 36215, 36485,
    36756, 37028, 37301, 37575, 37850, 38126, 38403, 38681, 38960, 39240, 39521, 39803, 40086, 40370, 40655, 40941, 41228, 41516, 41805, 42095, 42386, 42678,
    42971, 43265, 43560, 43856, 44153, 44451, 44750, 45050, 45351, 45653, 45956, 46260, 46565, 46871, 47178, 47486, 47795, 48105, 48416, 48728, 49041, 49355,
    49670, 49986, 50303, 50621, 50940, 51260, 51581, 51903, 52226, 52550, 52875, 53201, 53528, 53856, 54185, 54515, 54846, 55178, 55511, 55845, 56180, 56516,
    56853, 57191, 57530, 57870, 58211, 58553, 58896, 59240, 59585, 59931, 60278, 60626, 60975, 61325, 61676, 62028, 62381, 62735, 63090, 63446, 63803, 64161,
    64520, 64880, 65241, 65603, 65966, 66330, 66695, 67061, 67428, 67796, 68165, 68535, 68906, 69278, 69651, 70025, 70400, 70776, 71153, 71531, 71910, 72290,
    72671, 73053, 73436, 73820, 74205, 74591, 74978, 75366, 75755, 76145, 76536, 76928, 77321, 77715, 78110, 78506, 78903, 79301, 79700, 80100, 80501, 80903,
    81306, 81710, 82115, 82521, 82928, 83336, 83745, 84155, 84566, 84978, 85391, 85805, 86220, 86636, 87053, 87471, 87890, 88310, 88731, 89153, 89576, 90000,
    90425, 90851, 91278, 91706, 92135, 92565, 92996, 93428, 93861, 94295, 94730, 95166, 95603, 96041, 96480, 96920, 97361, 97803, 98246, 98690, 99135, 99581,
    100028, 100476, 100925, 101375, 101826, 102278, 102731, 103185, 103640, 104096, 104553, 105011, 105470, 105930, 106391, 106853, 107316, 107780, 108245,
    108711, 109178, 109646, 110115, 110585, 111056, 111528, 112001, 112475, 112950, 113426, 113903, 114381, 114860, 115340, 115821, 116303, 116786, 117270,
    117755, 118241, 118728, 119216, 119705, 120195, 120686, 121178, 121671, 122165, 122660, 123156, 123653, 124151, 124650, 125150, 125651, 126153, 126656,
    127160, 127665, 128171, 128678, 129186, 129695, 130205, 130716, 131228, 131741, 132255, 132770, 133286, 133803, 134321, 134840, 135360, 135881, 136403,
    136926, 137450, 137975, 138501, 139028, 139556, 140085, 140615, 141146, 141678, 142211, 142745, 143280, 143816, 144353, 144891, 145430, 145970, 146511,
    147053, 147596, 148140, 148685, 149231, 149778, 150326, 150875, 151425, 151976, 152528, 153081, 153635, 154190, 154746, 155303, 155861, 156420, 156980,
    157541, 158103, 158666, 159230, 159795, 160361, 160928, 161496, 162065, 162635, 163206, 163778, 164351, 164925, 165500, 166076, 166653, 167231, 167810,
    168390, 168971, 169553, 170136, 170720, 171305, 171891, 172478, 173066, 173655, 174245, 174836, 175428, 176021, 176615, 177211, 177808, 178406, 179005,
    179605, 246333]
    */

    {
        // test default vector iterator
        auto it = vec.uncompressed_begin();
        EXPECT_EQ(*it, 0);
        ++it;
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 2);

        // test our custom const_iterator
        auto cit = vec.uncompressed_begin();
        EXPECT_EQ(*cit, 0);
        ++cit;
        EXPECT_EQ(*cit, 1);
        ++cit;
        EXPECT_EQ(*cit, 2);
    }

    vec.compress();

    {
        std::vector<uint8_t> buf;
        {  // Serialize.
            buf = cista::serialize(vec);
        }

        // Deserialize.
        const auto& deserialized = *cista::deserialize<Vector>(buf.begin().base(), buf.end().base());

        auto it = deserialized.compressed_begin();
        EXPECT_EQ(*it, 0);
        ++it;
        EXPECT_EQ(*it, 1);
        ++it;
        EXPECT_EQ(*it, 2);
    }
}

}
