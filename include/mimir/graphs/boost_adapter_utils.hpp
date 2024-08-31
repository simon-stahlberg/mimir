/*
 * Copyright (C) 2024 Dominik Drexler and Till Hofmann
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

#ifndef MIMIR_GRAPHS_BOOST_ADAPTER_HPP_
#define MIMIR_GRAPHS_BOOST_ADAPTER_HPP_

#include "mimir/common/concepts.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace boost
{

/// A tag for a graph that is both a vertex list graph and an incidence graph.
struct vertex_list_and_incidence_and_edge_list_and_adjacency_and_bidirectional_graph_tag :
    public vertex_list_graph_tag,
    // public incidence_graph_tag,  // is included in bidirectional_graph_tag
    public edge_list_graph_tag,
    public adjacency_graph_tag,
    public bidirectional_graph_tag
{
};

}

namespace mimir
{

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// BasicMatrix
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

template<typename I, typename V>
class VectorBasicMatrix
{
public:
    VectorBasicMatrix(std::size_t num_vertices, V init_value = std::numeric_limits<V>::max()) : m_matrix(num_vertices, std::vector<V>(num_vertices, init_value))
    {
    }

    std::vector<V>& operator[](I i) { return m_matrix[i]; }

    const std::vector<V>& operator[](I i) const { return m_matrix[i]; }

    const std::vector<std::vector<V>>& get_matrix() const { return m_matrix; }

private:
    std::vector<std::vector<V>> m_matrix;
};

template<typename I, typename V>
class UnorderedMapBasicMatrix
{
public:
    std::unordered_map<I, V>& operator[](I i) { return m_matrix[i]; }

    const std::unordered_map<I, V>& operator[](I i) const { return m_matrix[i]; }

    const std::unordered_map<I, std::unordered_map<I, V>>& get_matrix() const { return m_matrix; }

private:
    std::unordered_map<I, std::unordered_map<I, V>> m_matrix;
};

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// PropertyMaps
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/* TrivialReadPropertyMap */

/// @brief A property map that returns its key as value.
///
/// boost requires a vertex_index that translates vertex->index, where the index is an index into the vertex list.
/// In our case, the vertex is described with a Index and so vertex and index are the same.
/// To avoid storing a map of the size of the graph, we use TrivialReadPropertyMap that just returns the key.
template<typename Key, typename Value>
struct TrivialReadPropertyMap
{
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;
};

/// @brief Get the index of a state.
/// @param key the state.
/// @return the index of the state, which is just the input key.
template<typename Key, typename Value>
inline Value get(const TrivialReadPropertyMap<Key, Value>&, Key key)
{
    return key;
}

/* VectorReadPropertyMap */

template<IsUnsignedIntegral Key, typename Value>
class VectorReadPropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadPropertyMap(const std::vector<Value>& distances) : m_distances(distances) {}

    const Value& get(Key key) const { return m_distances.get().at(key); }

private:
    std::reference_wrapper<const std::vector<Value>> m_distances;
};

template<typename Key, typename Value>
inline const Value& get(const VectorReadPropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

/* VectorReadWritePropertyMap */

template<IsUnsignedIntegral Key, typename Value>
class VectorReadWritePropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadWritePropertyMap(std::vector<Value>& distances) : m_distances(distances) {}

    const Value& get(Key key) const { return m_distances.get().at(key); }
    void set(Key key, Value value) { m_distances.get().at(key) = value; }

private:
    std::reference_wrapper<std::vector<Value>> m_distances;
};

template<typename Key, typename Value>
inline const Value& get(const VectorReadWritePropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

template<typename Key, typename Value>
inline void put(VectorReadWritePropertyMap<Key, Value>& m, Key key, Value value)
{
    m.set(key, value);
}

/* UnorderedMapReadPropertyMap */

template<IsUnsignedIntegral Key, typename Value>
class UnorderedMapReadPropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit UnorderedMapReadPropertyMap(const std::unordered_map<Key, Value>& distances) : m_distances(distances) {}

    const Value& get(Key key) const { return m_distances.get().at(key); }

private:
    std::reference_wrapper<const std::unordered_map<Key, Value>> m_distances;
};

template<typename Key, typename Value>
inline const Value& get(const UnorderedMapReadPropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

/* UnorderedMapReadWritePropertyMap */

template<IsUnsignedIntegral Key, typename Value>
class UnorderedMapReadWritePropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit UnorderedMapReadWritePropertyMap(std::unordered_map<Key, Value>& distances) : m_distances(distances) {}

    const Value& get(Key key) const { return m_distances.get()[key]; }
    void set(Key key, Value value) { m_distances.get()[key] = value; }

private:
    std::reference_wrapper<std::unordered_map<Key, Value>> m_distances;
};

template<typename Key, typename Value>
inline const Value& get(const UnorderedMapReadWritePropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

template<typename Key, typename Value>
inline void put(UnorderedMapReadWritePropertyMap<Key, Value>& m, Key key, Value value)
{
    m.set(key, value);
}

}

#endif
