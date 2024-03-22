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

#ifndef MIMIR_FORMALISM_CONDITIONS_HPP_
#define MIMIR_FORMALISM_CONDITIONS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/conditions.hpp>
#include <loki/pddl/factory.hpp>
#include <string>

namespace mimir
{
/* Literal */
class ConditionLiteralImpl : public loki::Base<ConditionLiteralImpl>
{
private:
    Literal m_literal;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionLiteralImpl(int identifier, Literal literal);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionLiteralImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class Base<ConditionLiteralImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const Literal& get_literal() const;
};

/* And */
class ConditionAndImpl : public loki::Base<ConditionAndImpl>
{
private:
    ConditionList m_conditions;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionAndImpl(int identifier, ConditionList conditions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionAndImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionAndImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const ConditionList& get_conditions() const;
};

/* Or */
class ConditionOrImpl : public loki::Base<ConditionOrImpl>
{
private:
    ConditionList m_conditions;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionOrImpl(int identifier, ConditionList conditions);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionOrImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionOrImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const ConditionList& get_conditions() const;
};

/* Not */
class ConditionNotImpl : public loki::Base<ConditionNotImpl>
{
private:
    Condition m_condition;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionNotImpl(int identifier, Condition condition);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionNotImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionNotImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const Condition& get_condition() const;
};

/* Imply */
class ConditionImplyImpl : public loki::Base<ConditionImplyImpl>
{
private:
    Condition m_condition_left;
    Condition m_condition_right;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionImplyImpl(int identifier, Condition condition_left, Condition condition_right);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionImplyImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionImplyImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const Condition& get_condition_left() const;
    const Condition& get_condition_right() const;
};

/* Exists */
class ConditionExistsImpl : public loki::Base<ConditionExistsImpl>
{
private:
    ParameterList m_parameters;
    Condition m_condition;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionExistsImpl(int identifier, ParameterList parameters, Condition condition);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionExistsImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionExistsImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const ParameterList& get_parameters() const;
    const Condition& get_condition() const;
};

/* Forall */
class ConditionForallImpl : public loki::Base<ConditionForallImpl>
{
private:
    ParameterList m_parameters;
    Condition m_condition;

    // Below: add additional members if needed and initialize them in the constructor

    ConditionForallImpl(int identifier, ParameterList parameters, Condition condition);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ConditionImpl, loki::Hash<ConditionImpl*>, loki::EqualTo<ConditionImpl*>>;

    bool is_structurally_equivalent_to_impl(const ConditionForallImpl& other) const;
    size_t hash_impl() const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionForallImpl>;

public:
    void str(std::ostream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const ParameterList& get_parameters() const;
    const Condition& get_condition() const;
};
}

#endif
