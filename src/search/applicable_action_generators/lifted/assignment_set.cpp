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

#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"

namespace mimir
{

/*
 * Assignment
 */

Assignment::Assignment(size_t index, size_t object) :
    first_index(index),
    first_object(object),
    second_index(MAX_VALUE),
    second_object(MAX_VALUE)
{
}

Assignment::Assignment(size_t first_index, size_t first_object, size_t second_index, size_t second_object) :
    first_index(first_index),
    first_object(first_object),
    second_index(second_index),
    second_object(second_object)
{
}

size_t Assignment::size() const { return (first_object != MAX_VALUE ? 1 : 0) + (second_object != MAX_VALUE ? 1 : 0); }

/*
 * VertexAssignmentIterator
 */

size_t VertexAssignmentIterator::get_object_if_overlap(const Term& term)
{
    if (const auto term_object = std::get_if<TermObjectImpl>(term))
    {
        return term_object->get_object()->get_identifier();
    }

    if (const auto term_variable = std::get_if<TermVariableImpl>(term))
    {
        if (m_vertex.get_parameter_index() == term_variable->get_variable()->get_parameter_index())
        {
            return m_vertex.get_object_id();
        }
    }

    return UNDEFINED;
}

void VertexAssignmentIterator::find_next_binding()
{
    auto found = false;

    for (size_t index = m_index + 1; index < m_terms.size(); ++index)
    {
        assert(index < m_terms.size());

        auto object = get_object_if_overlap(m_terms[index]);

        if (object != UNDEFINED)
        {
            m_index = index;
            m_object = object;
            found = true;
            break;
        }
    }

    if (!found)
    {
        m_index = UNDEFINED;
        m_object = UNDEFINED;
    }
}

VertexAssignmentIterator::VertexAssignmentIterator(const TermList& terms, const consistency_graph::Vertex& vertex) :
    m_terms(terms),
    m_vertex(vertex),
    m_index(UNDEFINED),
    m_object(UNDEFINED)
{
    find_next_binding();
}

bool VertexAssignmentIterator::has_next() const { return m_index < m_terms.size(); }

Assignment VertexAssignmentIterator::next()
{
    if (!has_next())
    {
        throw std::out_of_range("No more bindings available");
    }

    auto binding = Assignment(m_index, m_object, UNDEFINED, UNDEFINED);
    find_next_binding();
    return binding;
}

/*
 * EdgeAssignmentIterator
 */

size_t EdgeAssignmentIterator::get_object_if_overlap(const Term& term)
{
    if (const auto term_object = std::get_if<TermObjectImpl>(term))
    {
        return term_object->get_object()->get_identifier();
    }

    if (const auto term_variable = std::get_if<TermVariableImpl>(term))
    {
        const auto parameter_index = term_variable->get_variable()->get_parameter_index();

        if (m_edge.get_src().get_parameter_index() == parameter_index)
        {
            return m_edge.get_src().get_object_id();
        }
        else if (m_edge.get_dst().get_parameter_index() == parameter_index)
        {
            return m_edge.get_dst().get_object_id();
        }
    }

    return UNDEFINED;
}

void EdgeAssignmentIterator::find_next_binding()
{
    if (m_second_index == UNDEFINED)
    {
        auto found_first = false;

        for (size_t first_index = m_first_index + 1; first_index < m_terms.size(); ++first_index)
        {
            assert(first_index < m_terms.size());

            auto first_object = get_object_if_overlap(m_terms[first_index]);

            if (first_object != UNDEFINED)
            {
                m_first_index = first_index;
                m_first_object = first_object;
                m_second_index = first_index;
                found_first = true;
                break;
            }
        }

        if (!found_first)
        {
            m_first_index = UNDEFINED;
            m_first_object = UNDEFINED;
            m_second_index = UNDEFINED;
            m_second_object = UNDEFINED;
        }
    }

    if (m_first_index != UNDEFINED)
    {
        auto found_second = false;

        for (size_t second_index = m_second_index + 1; second_index < m_terms.size(); ++second_index)
        {
            assert(second_index < m_terms.size());

            auto second_object = get_object_if_overlap(m_terms[second_index]);

            if (second_object != UNDEFINED)
            {
                m_second_index = second_index;
                m_second_object = second_object;
                found_second = true;
                break;
            }
        }

        if (!found_second)
        {
            m_second_index = UNDEFINED;
            m_second_object = UNDEFINED;
        }
    }
}

EdgeAssignmentIterator::EdgeAssignmentIterator(const TermList& terms, const consistency_graph::Edge& edge) :
    m_terms(terms),
    m_edge(edge),
    m_first_index(UNDEFINED),
    m_second_index(UNDEFINED),
    m_first_object(UNDEFINED),
    m_second_object(UNDEFINED)
{
    find_next_binding();
}

bool EdgeAssignmentIterator::has_next() const { return m_first_index < m_terms.size(); }

Assignment EdgeAssignmentIterator::next()
{
    if (!has_next())
    {
        throw std::out_of_range("No more bindings available");
    }

    auto binding = Assignment(m_first_index, m_first_object, m_second_index, m_second_object);
    find_next_binding();
    return binding;
}

}
