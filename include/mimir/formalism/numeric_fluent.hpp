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

#ifndef MIMIR_FORMALISM_NUMERIC_FLUENT_HPP_
#define MIMIR_FORMALISM_NUMERIC_FLUENT_HPP_

#include "mimir/formalism/ground_function.hpp"

#include <loki/loki.hpp>
#include <string>
#include <vector>

namespace mimir
{
class NumericFluentImpl : public loki::Base<NumericFluentImpl>
{
private:
    GroundFunction m_function;
    double m_number;

    // Below: add additional members if needed and initialize them in the constructor

    NumericFluentImpl(int identifier, GroundFunction function, double number);

    // Give access to the constructor.
    friend class loki::PDDLFactory<NumericFluentImpl, loki::Hash<NumericFluentImpl*>, loki::EqualTo<NumericFluentImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const NumericFluentImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<NumericFluentImpl>;

public:
    const GroundFunction& get_function() const;
    double get_number() const;
};

/**
 * Type aliases
 */

using NumericFluent = const NumericFluentImpl*;
using NumericFluentList = std::vector<NumericFluent>;
}

#endif
