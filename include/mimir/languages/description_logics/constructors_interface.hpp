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

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{
struct EvaluationContext;

template<typename T>
concept IsDLConstructor = requires(const T a, EvaluationContext& context) {
    {
        a.evaluate(context)
    };
};

/**
 * Concept
 */

class ConceptVisitor;

class Concept
{
protected:
    size_t m_identifier;

public:
    virtual ~Concept() {}

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    /// Uses double dispatch.
    virtual bool accept(const ConceptVisitor& visitor) const = 0;
};

using ConceptList = std::vector<const Concept*>;

/**
 * Role
 */

class RoleVisitor;

class Role
{
protected:
    size_t m_identifier;

public:
    virtual ~Role() {}

    /// @brief Evaluate the dl constructor on the evaluation context.
    virtual void evaluate(EvaluationContext& context) const = 0;

    /// @brief Return true iff the symbols match the data in the dl constructor.
    virtual bool accept(const RoleVisitor& visitor) const = 0;
};

using RoleList = std::vector<const Role*>;

}

#endif
