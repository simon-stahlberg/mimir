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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/constructor_ids.hpp"
#include "mimir/languages/description_logics/denotations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{
struct EvaluationContext;
class ConceptVisitor;
class RoleVisitor;

template<IsConceptOrRole D>
class Constructor
{
};

/**
 * Concept
 */

template<>
class Constructor<Concept>
{
protected:
    bool type_equal(const Constructor& other) const { return typeid(*this) == typeid(other); }

    Constructor() = default;
    // Move constructor and move assignment operator are protected
    // to restrict their usage to derived classes only.
    Constructor(Constructor&& other) = default;
    Constructor& operator=(Constructor&& other) = default;

public:
    // Uncopieable
    Constructor(const Constructor& other) = delete;
    Constructor& operator=(const Constructor& other) = delete;
    virtual ~Constructor() = default;

    bool operator==(const Constructor& other) const { return is_equal(other); }
    virtual bool is_equal(const Constructor& other) const = 0;
    virtual size_t hash() const = 0;

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual Denotation<Concept> evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    /// Uses double dispatch.
    virtual bool accept(const ConceptVisitor& visitor) const = 0;

    virtual size_t get_id() const = 0;
};

using ConceptConstructorList = std::vector<std::reference_wrapper<const Constructor<Concept>>>;

/**
 * Role
 */

template<>
class Constructor<Role>
{
protected:
    bool type_equal(const Constructor& other) const { return typeid(*this) == typeid(other); }

    Constructor() = default;
    Constructor(Constructor&& other) = default;
    Constructor& operator=(Constructor&& other) = default;

public:
    // Uncopieable
    Constructor(const Constructor& other) = delete;
    Constructor& operator=(const Constructor& other) = delete;
    virtual ~Constructor() = default;

    bool operator==(const Constructor& other) const { return is_equal(other); }
    virtual bool is_equal(const Constructor& other) const = 0;
    virtual size_t hash() const = 0;

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual Denotation<Role> evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    virtual bool accept(const RoleVisitor& visitor) const = 0;

    virtual size_t get_id() const = 0;
};

using RoleConstructorList = std::vector<std::reference_wrapper<const Constructor<Role>>>;

}

#endif
