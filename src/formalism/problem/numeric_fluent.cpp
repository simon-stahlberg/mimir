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

#include <mimir/formalism/problem/numeric_fluent.hpp>

#include <mimir/formalism/domain/function.hpp>

#include <loki/common/hash.hpp>


namespace mimir {
NumericFluentImpl::NumericFluentImpl(int identifier, Function function, double number)
    : Base(identifier), m_function(std::move(function)), m_number(number) { }

bool NumericFluentImpl::is_structurally_equivalent_to_impl(const NumericFluentImpl& other) const {
    return (m_function == other.m_function)
        && (m_number == other.m_number);
}

size_t NumericFluentImpl::hash_impl() const {
    return hash_combine(m_function, m_number);
}

void NumericFluentImpl::str_impl(std::ostringstream& out, const loki::FormattingOptions& /*options*/) const {
    out << "(= " << *m_function << " " << m_number << ")";
}

const Function& NumericFluentImpl::get_function() const {
    return m_function;
}

double NumericFluentImpl::get_number() const {
    return m_number;
}

}



namespace std {
    bool less<mimir::NumericFluent>::operator()(
        const mimir::NumericFluent& left_numeric_fluent,
        const mimir::NumericFluent& right_numeric_fluent) const {
        return *left_numeric_fluent < *right_numeric_fluent;
    }

    std::size_t hash<mimir::NumericFluentImpl>::operator()(const mimir::NumericFluentImpl& numeric_fluent) const {
        return numeric_fluent.hash();
    }
}
