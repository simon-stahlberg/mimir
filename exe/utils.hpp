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

#ifndef MIMIR_EXE_UTILS_HPP_
#define MIMIR_EXE_UTILS_HPP_

#include <mimir/mimir.hpp>
#include <stdexcept>
#include <string>

namespace mimir
{
enum class HeuristicType
{
    BLIND,
    PERFECT,
    MAX,
    ADD,
    SETADD,
    FF
};

inline HeuristicType get_heuristic_type(const std::string& name)
{
    if (name == "blind")
        return HeuristicType::BLIND;
    else if (name == "perfect")
        return HeuristicType::PERFECT;
    else if (name == "max")
        return HeuristicType::MAX;
    else if (name == "add")
        return HeuristicType::ADD;
    else if (name == "setadd")
        return HeuristicType::SETADD;
    else if (name == "ff")
        return HeuristicType::FF;
    else
        throw std::runtime_error("Undefined mapping from name to heuristic type.");
}

inline search::SearchContextImpl::SymmetryPruning get_symmetry_pruning(const std::string& symmetry_pruning_mode)
{
    if (symmetry_pruning_mode == "off")
        return search::SearchContextImpl::SymmetryPruning::OFF;
    else if (symmetry_pruning_mode == "gi")
        return search::SearchContextImpl::SymmetryPruning::GI;
    else if (symmetry_pruning_mode == "1-wl")
        return search::SearchContextImpl::SymmetryPruning::WL1;
    else
        throw std::runtime_error("Undefined symmetry pruning mode.");
}

inline search::SearchContextImpl::LiftedOptions get_lifted_options(const std::string& lifted_mode, const std::string& symmetry_pruning_mode)
{
    if (lifted_mode == "exhaustive")
        return search::SearchContextImpl::LiftedOptions(search::SearchContextImpl::LiftedOptions::ExhaustiveOptions());
    else if (lifted_mode == "kpkc")
        return search::SearchContextImpl::LiftedOptions(search::SearchContextImpl::LiftedOptions::KPKCOptions(get_symmetry_pruning(symmetry_pruning_mode)));
    else
        throw std::runtime_error("Undefined lifted mode.");
}

inline search::SearchContextImpl::SearchMode get_search_mode(const std::string& mode, const std::string& lifted_mode, const std::string& symmetry_pruning_mode)
{
    if (mode == "grounded")
        return search::SearchContextImpl::GroundedOptions();
    else if (mode == "lifted")
        return get_lifted_options(lifted_mode, symmetry_pruning_mode);
    else
        throw std::runtime_error("Undefined mode");
}
}

#endif
