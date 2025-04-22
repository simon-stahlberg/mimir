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

#ifndef MIMIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_
#define MIMIR_GRAPHS_BGL_PROPERTY_MAPS_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/graphs/dynamic_graph_interface.hpp"
#include "mimir/graphs/static_graph_interface.hpp"

#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map/property_map.hpp>
#include <concepts>
#include <limits>
#include <ranges>

namespace mimir::graphs::bgl
{

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// BasicMatrix
// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

/// @brief `VectorBasicMatrix` is a 2-dimensional table of values for dense graph structures whose underlying storage are vectors.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/graph/doc/BasicMatrix.html
/// @tparam I is the index type.
/// @tparam V is the value type.
template<std::unsigned_integral I, typename V>
class VectorBasicMatrix
{
public:
    VectorBasicMatrix(std::vector<std::vector<V>>& matrix) : m_matrix(matrix) {}

    auto& operator[](I i) { return m_matrix.get()[i]; }

    const auto& operator[](I i) const { return m_matrix.get()[i]; }

    const auto& get_matrix() const { return m_matrix.get(); }

private:
    std::reference_wrapper<std::vector<std::vector<V>>> m_matrix;
};

/// @brief `UnorderedMapBasicMatrix` is a 2-dimensional table of values for sparse graph structures whose underlying storage are unordered_maps.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/graph/doc/BasicMatrix.html
/// @tparam I is the index type.
/// @tparam V is the value type.
template<std::unsigned_integral I, typename V>
class UnorderedMapBasicMatrix
{
public:
    UnorderedMapBasicMatrix(std::unordered_map<I, std::unordered_map<I, V>>& matrix) : m_matrix(matrix) {}

    auto& operator[](I i) { return m_matrix.get()[i]; }

    const auto& operator[](I i) const { return m_matrix.get()[i]; }

    const auto& get() const { return m_matrix.get(); }

private:
    std::reference_wrapper<std::unordered_map<I, std::unordered_map<I, V>>> m_matrix;
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
inline auto get(const TrivialReadPropertyMap<Key, Value>&, Key key)
{
    return key;
}

/* VectorReadPropertyMap */

/// @brief `VectorReadPropertyMap` provides read-access to the `Value` associated with a given `Key` via a call to the `get()` function for dense graph
/// structures whose underlying storage are vectors.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/property_map/doc/ReadablePropertyMap.html
/// @tparam I is the value type.
/// @tparam Value is the value type.
template<std::unsigned_integral I, typename Value>
class VectorReadPropertyMap
{
public:
    using value_type = Value;
    using key_type = I;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadPropertyMap(const std::vector<Value>& data) : m_data(data) {}

    const auto& get(I key) const { return m_data.get().at(key); }

    const auto& get() const { return m_data.get(); }

private:
    std::reference_wrapper<const std::vector<Value>> m_data;
};

template<std::unsigned_integral I, typename Value>
inline const auto& get(const VectorReadPropertyMap<I, Value>& m, I key)
{
    return m.get(key);
}

/* VectorReadWritePropertyMap */

/// @brief `VectorReadWritePropertyMap` provides read-write-access to the `Value` associated with a given `Key` via a call to the `get()` and `put()` function
/// for dense graph structures whose underlying storage are vectors.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/property_map/doc/ReadWritePropertyMap.html
/// @tparam I is the index type.
/// @tparam Value is the value type.
template<std::unsigned_integral I, typename Value>
class VectorReadWritePropertyMap
{
public:
    using container_type = std::vector<Value>;
    using value_type = Value;
    using key_type = I;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit VectorReadWritePropertyMap(std::vector<Value>& data) : m_data(data) {}

    const auto& get(I key) const { return m_data.get().at(key); }
    void set(I key, Value value) { m_data.get().at(key) = value; }

    auto& get() { return m_data.get(); }
    const auto& get() const { return m_data.get(); }

private:
    std::reference_wrapper<std::vector<Value>> m_data;
};

template<std::unsigned_integral I, typename Value>
inline const auto& get(const VectorReadWritePropertyMap<I, Value>& m, I key)
{
    return m.get(key);
}

template<std::unsigned_integral I, typename Value>
inline void put(VectorReadWritePropertyMap<I, Value>& m, I key, Value value)
{
    m.set(key, value);
}

/* UnorderedMapReadPropertyMap */

/// @brief `UnorderedMapReadPropertyMap` provides read-access to the `Value` associated with a given `Key` via a call to the `get()` function for sparse graph
/// structures whose underlying storage are unordered_maps.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/property_map/doc/ReadablePropertyMap.html
/// @tparam Key is the key type.
/// @tparam Value is the value type.
template<std::unsigned_integral Key, typename Value>
class UnorderedMapReadPropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit UnorderedMapReadPropertyMap(const std::unordered_map<Key, Value>& distances) : m_distances(distances) {}

    const auto& get(Key key) const { return m_distances.get().at(key); }

    const auto& get() const { return m_distances.get(); }

private:
    std::reference_wrapper<const std::unordered_map<Key, Value>> m_distances;
};

template<typename Key, typename Value>
inline const auto& get(const UnorderedMapReadPropertyMap<Key, Value>& m, Key key)
{
    return m.get(key);
}

/* UnorderedMapReadWritePropertyMap */

/// @brief `UnorderedMapReadWritePropertyMap` provides read-write-access to the `Value` associated with a given `Key` via a call to the `get()` and `put()`
/// function for sparse graph structures whose underlying storage are unordered_maps.
///
/// Link to Boost docs: https://www.boost.org/doc/libs/1_82_0/libs/property_map/doc/ReadWritePropertyMap.html
/// @tparam Key is the key type.
/// @tparam Value is the value type.
template<std::unsigned_integral Key, typename Value>
class UnorderedMapReadWritePropertyMap
{
public:
    using value_type = Value;
    using key_type = Key;
    using reference = Value;
    using category = boost::read_write_property_map_tag;

    explicit UnorderedMapReadWritePropertyMap(std::unordered_map<Key, Value>& data) : m_data(data) {}

    const auto& get(Key key) const { return m_data.get()[key]; }
    void set(Key key, Value value) { m_data.get()[key] = value; }

    auto& get() { return m_data.get(); }
    const auto& get() const { return m_data.get(); }

private:
    std::reference_wrapper<std::unordered_map<Key, Value>> m_data;
};

template<typename Key, typename Value>
inline const auto& get(const UnorderedMapReadWritePropertyMap<Key, Value>& m, Key key)
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
