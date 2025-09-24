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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATORS_EVENT_HANDLERS_STATISTICS_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATORS_EVENT_HANDLERS_STATISTICS_HPP_

#include <cstdint>
#include <ostream>
#include <vector>

namespace mimir::search::satisficing_binding_generator
{
class Statistics
{
private:
    uint64_t m_num_valid_base_bindings;
    uint64_t m_num_valid_derived_bindings;

    uint64_t m_num_invalid_base_bindings;
    uint64_t m_num_invalid_derived_bindings;

    std::vector<uint64_t> m_num_valid_base_bindings_per_search_layer;
    std::vector<uint64_t> m_num_valid_derived_bindings_per_search_layer;

    std::vector<uint64_t> m_num_invalid_base_bindings_per_search_layer;
    std::vector<uint64_t> m_num_invalid_derived_bindings_per_search_layer;

public:
    Statistics() :
        m_num_valid_base_bindings(0),
        m_num_valid_derived_bindings(0),
        m_num_invalid_base_bindings(0),
        m_num_invalid_derived_bindings(0),
        m_num_valid_base_bindings_per_search_layer(),
        m_num_valid_derived_bindings_per_search_layer(),
        m_num_invalid_base_bindings_per_search_layer(),
        m_num_invalid_derived_bindings_per_search_layer()
    {
    }

    /// @brief Store information for the layer
    void on_finish_search_layer()
    {
        m_num_valid_base_bindings_per_search_layer.push_back(m_num_valid_base_bindings);
        m_num_valid_derived_bindings_per_search_layer.push_back(m_num_valid_derived_bindings);

        m_num_invalid_base_bindings_per_search_layer.push_back(m_num_invalid_base_bindings);
        m_num_invalid_derived_bindings_per_search_layer.push_back(m_num_invalid_derived_bindings);
    }

    void increment_num_valid_base_bindings() { ++m_num_valid_base_bindings; }
    void increment_num_valid_derived_bindings() { ++m_num_valid_derived_bindings; }

    void increment_num_invalid_base_bindings() { ++m_num_invalid_base_bindings; }
    void increment_num_invalid_derived_bindings() { ++m_num_invalid_derived_bindings; }

    uint64_t get_num_valid_base_bindings() const { return m_num_valid_base_bindings; }
    uint64_t get_num_valid_derived_bindings() const { return m_num_valid_derived_bindings; }

    uint64_t get_num_invalid_base_bindings() const { return m_num_invalid_base_bindings; }
    uint64_t get_num_invalid_derived_bindings() const { return m_num_invalid_derived_bindings; }

    const std::vector<uint64_t>& get_num_valid_base_bindings_per_search_layer() const { return m_num_valid_base_bindings_per_search_layer; }
    const std::vector<uint64_t>& get_num_valid_derived_bindings_per_search_layer() const { return m_num_valid_derived_bindings_per_search_layer; }

    const std::vector<uint64_t>& get_num_invalid_base_bindings_per_search_layer() const { return m_num_invalid_base_bindings_per_search_layer; }
    const std::vector<uint64_t>& get_num_invalid_derived_bindings_per_search_layer() const { return m_num_invalid_derived_bindings_per_search_layer; }
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const Statistics& statistics)
{
    os << "[SatisficingBindingGenerator] Number of generated valid base bindings: " << statistics.get_num_valid_base_bindings() << std::endl
       << "[SatisficingBindingGenerator] Number of generated valid base bindings until last f-layer: "
       << (statistics.get_num_valid_base_bindings_per_search_layer().empty() ? 0 : statistics.get_num_valid_base_bindings_per_search_layer().back())
       << std::endl
       << "[SatisficingBindingGenerator] Number of generated valid derived bindings: " << statistics.get_num_valid_derived_bindings() << std::endl

       << "[SatisficingBindingGenerator] Number of generated valid derived bindings until last f-layer: "
       << (statistics.get_num_valid_derived_bindings_per_search_layer().empty() ? 0 : statistics.get_num_valid_derived_bindings_per_search_layer().back())
       << std::endl
       << "[SatisficingBindingGenerator] Number of generated invalid base bindings: " << statistics.get_num_invalid_base_bindings() << std::endl
       << "[SatisficingBindingGenerator] Number of generated invalid base bindings until last f-layer: "
       << (statistics.get_num_invalid_base_bindings_per_search_layer().empty() ? 0 : statistics.get_num_invalid_base_bindings_per_search_layer().back())
       << std::endl
       << "[SatisficingBindingGenerator] Number of generated invalid derived bindings: " << statistics.get_num_invalid_derived_bindings() << std::endl
       << "[SatisficingBindingGenerator] Number of generated invalid derived bindings until last f-layer: "
       << (statistics.get_num_invalid_derived_bindings_per_search_layer().empty() ? 0 : statistics.get_num_invalid_derived_bindings_per_search_layer().back());
    ;

    return os;
}

}

#endif
