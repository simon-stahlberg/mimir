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

#ifndef MIMIR_FORMALISM_COMMON_BASE_HPP_
#define MIMIR_FORMALISM_COMMON_BASE_HPP_

#include "printer.hpp"

#include <algorithm>
#include <sstream>
#include <vector>


namespace mimir {
/// @brief Implements a common base class for PDDL objects.
///
///        Each PDDL object has an identifier.
///        Identifiers are used to describe and detect semantically equivalent PDDL object.
///        Detecting semantic equivalence is important for
///            - reducing the required memory by detecting duplicates
///            - reducing comparison and hashing to pointer level instead of traversing the whole structure
///        For the following type of PDDL objects, loki detects semantic equivalence:
///            * Type
///            * Object
///            * Variable
///            * Term
///            * Atom
///            * NumericFluent
///            * Literal
///            * Parameter
///            * Function
///            * FunctionSkeleton
///            * Requirements
///        For the remaining type of PDDL objects, loki approximates semantic equivalence
///        with structural equivalence where collections of objects are sorted by the identifier.
///        For example, loki detects semantic equivalence of a conjunction of atoms
///        but loki does not detect semantic equivalence of an arbitrary formula of atoms.
template<typename Derived>
class Base {
protected:
    int m_identifier;

    explicit Base(int identifier) : m_identifier(identifier) { }
    friend Derived;

public:
    // moveable but not copyable
    Base(const Base& other) = delete;
    Base& operator=(const Base& other) = delete;
    Base(Base&& other) = default;
    Base& operator=(Base&& other) = default;

    constexpr const auto& self() const { return static_cast<Derived const&>(*this); }

    bool operator==(const Base& other) const {
        return self().is_structurally_equivalent_to_impl(other.self());
    }

    bool operator!=(const Base& other) const {
        return !(*this == other);
    }

    bool operator<(const Base& other) const {
        return m_identifier < other.m_identifier;
    }

    bool operator>(const Base& other) const {
        return m_identifier > other.m_identifier;
    }

    size_t hash() const {
        return self().hash_impl();
    }

    /// @brief Overload of the output stream insertion operator (operator<<).
    friend std::ostream& operator<<(std::ostream& os, const Base& element) {
        os << element.str();
        return os;
    }

    /// @brief Compute a string representation of this object.
    void str(std::ostringstream& out, const FormattingOptions& options) const {
        self().str_impl(out, options);
    }

    /// @brief Compute a string representation of this object.
    std::string str() const {
        std::ostringstream out;
        FormattingOptions options{0, 4};
        str(out, options);
        return out.str();
    }

    /// @brief Returns the identifier
    int get_identifier() const {
        return m_identifier;
    }
};

}  // namespace mimir

#endif  // MIMIR_FORMALISM_COMMON_BASE_HPP_
