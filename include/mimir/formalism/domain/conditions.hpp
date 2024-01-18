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


#ifndef MIMIR_FORMALISM_DOMAIN_CONDITIONS_HPP_
#define MIMIR_FORMALISM_DOMAIN_CONDITIONS_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/conditions.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {

/* Literal */
class ConditionLiteralImpl : public loki::Base<ConditionLiteralImpl> {
private:
    loki::pddl::Condition external_;

    Literal m_literal;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionLiteralImpl(int identifier, loki::pddl::Condition external, Literal literal);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionLiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class Base<ConditionLiteralImpl>;

public:
    const Literal& get_literal() const;
};


/* And */
class ConditionAndImpl : public loki::Base<ConditionAndImpl> {
private:
    loki::pddl::Condition external_;

    ConditionList m_conditions;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionAndImpl(int identifier, loki::pddl::Condition external, ConditionList conditions);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionAndImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionAndImpl>;

public:
    const ConditionList& get_conditions() const;
};


/* Or */
class ConditionOrImpl : public loki::Base<ConditionOrImpl> {
private:
    loki::pddl::Condition external_;

    ConditionList m_conditions;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionOrImpl(int identifier, loki::pddl::Condition external, ConditionList conditions);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionOrImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionOrImpl>;

public:
    const ConditionList& get_conditions() const;
};


/* Not */
class ConditionNotImpl : public loki::Base<ConditionNotImpl> {
private:
    loki::pddl::Condition external_;

    Condition m_condition;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionNotImpl(int identifier, loki::pddl::Condition external, Condition condition);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionNotImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionNotImpl>;

public:
    const Condition& get_condition() const;
};


/* Imply */
class ConditionImplyImpl : public loki::Base<ConditionNotImpl> {
private:
    loki::pddl::Condition external_;

    Condition m_condition_left;
    Condition m_condition_right;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionImplyImpl(int identifier, loki::pddl::Condition external, Condition condition_left, Condition condition_right);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionImplyImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionImplyImpl>;

public:
    const Condition& get_condition_left() const;
    const Condition& get_condition_right() const;
};


/* Exists */
class ConditionExistsImpl : public loki::Base<ConditionExistsImpl> {
private:
    loki::pddl::Condition external_;

    ParameterList m_parameters;
    Condition m_condition;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionExistsImpl(int identifier, loki::pddl::Condition external, ParameterList parameters, Condition condition);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionExistsImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionExistsImpl>;

public:
    const ParameterList& get_parameters() const;
    const Condition& get_condition() const;
};


/* Forall */
class ConditionForallImpl : public loki::Base<ConditionForallImpl> {
private:
    loki::pddl::Condition external_;

    ParameterList m_parameters;
    Condition m_condition;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    ConditionForallImpl(int identifier, loki::pddl::Condition external, ParameterList parameters, Condition condition);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const ConditionForallImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ConditionForallImpl>;

public:
    const ParameterList& get_parameters() const;
    const Condition& get_condition() const;
};

}


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Condition>
    {
        bool operator()(const loki::pddl::Condition& left_condition, const loki::pddl::Condition& right_condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionLiteralImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionLiteralImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionAndImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionAndImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionOrImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionOrImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionNotImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionNotImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionImplyImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionImplyImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionExistsImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionExistsImpl& condition) const;
    };

    template<>
    struct hash<loki::pddl::ConditionForallImpl>
    {
        std::size_t operator()(const loki::pddl::ConditionForallImpl& condition) const;
    };
}


#endif
