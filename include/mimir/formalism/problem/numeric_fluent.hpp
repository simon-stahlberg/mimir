/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_PROBLEM_NUMERIC_FLUENT_HPP_
#define MIMIR_FORMALISM_PROBLEM_NUMERIC_FLUENT_HPP_

#include "declarations.hpp"

#include <loki/problem/pddl/numeric_fluent.hpp>
#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class NumericFluentImpl : public loki::Base<NumericFluentImpl>
{
private:
    Function m_function;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    NumericFluentImpl(int identifier, Function function, double number);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const NumericFluentImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<NumericFluentImpl>;

public:
    const Function& get_function() const;
    double get_number() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::NumericFluent>
{
    bool operator()(const mimir::NumericFluent& left_numeric_fluent, const mimir::NumericFluent& right_numeric_fluent) const;
};

template<>
struct hash<mimir::NumericFluentImpl>
{
    std::size_t operator()(const mimir::NumericFluentImpl& numeric_fluent) const;
};
}

#endif
