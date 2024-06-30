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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_INTERFACE_HPP_

#include "mimir/languages/description_logics/visitors_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{
struct EvaluationContext;
class ConceptVisitor;
class RoleVisitor;

/**
 * Concepts
 */

class Concept
{
protected:
    bool type_equal(const Concept& other) const { return typeid(*this) == typeid(other); }

    Concept() = default;
    Concept(Concept&& other) = default;
    Concept& operator=(Concept&& other) = default;

public:
    // Uncopieable
    Concept(const Concept& other) = delete;
    Concept& operator=(const Concept& other) = delete;
    virtual ~Concept() = default;

    bool operator==(const Concept& other) const { return is_equal(other); }
    virtual bool is_equal(const Concept& other) const = 0;
    virtual size_t hash() const = 0;

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    /// Uses double dispatch.
    virtual bool accept(const ConceptVisitor& visitor) const = 0;

    virtual size_t get_id() const = 0;
};

using ConceptList = std::vector<const Concept*>;

/**
 * Roles
 */

class Role
{
protected:
    bool type_equal(const Role& other) const { return typeid(*this) == typeid(other); }

    Role() = default;
    Role(Role&& other) = default;
    Role& operator=(Role&& other) = default;

public:
    // Uncopieable
    Role(const Role& other) = delete;
    Role& operator=(const Role& other) = delete;
    virtual ~Role() = default;

    bool operator==(const Role& other) const { return is_equal(other); }
    virtual bool is_equal(const Role& other) const = 0;
    virtual size_t hash() const = 0;

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    virtual bool accept(const RoleVisitor& visitor) const = 0;

    virtual size_t get_id() const = 0;
};

using RoleList = std::vector<const Role*>;

/**
 * Concepts
 */

template<typename T>
concept IsConceptOrRole = std::is_same<T, Concept>::value || std::is_same<T, Role>::value;

template<typename T>
concept IsConcreteConceptOrRole = std::derived_from<T, Concept> || std::derived_from<T, Role>;

}

#endif
