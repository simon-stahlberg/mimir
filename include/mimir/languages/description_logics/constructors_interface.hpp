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
    size_t m_id;

public:
    explicit Concept(size_t id) : m_id(id) {}
    virtual ~Concept() {}

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    /// Uses double dispatch.
    virtual bool accept(const ConceptVisitor& visitor) const = 0;
};

using ConceptList = std::vector<const Concept*>;

/**
 * Roles
 */

class Role
{
protected:
    size_t m_id;

public:
    explicit Role(size_t id) : m_id(id) {}
    virtual ~Role() {}

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    virtual bool accept(const RoleVisitor& visitor) const = 0;
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
