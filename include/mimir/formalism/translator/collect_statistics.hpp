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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_TRANSLATOR_COLLECT_STATISTICS_HPP_
#define MIMIR_FORMALISM_TRANSLATOR_COLLECT_STATISTICS_HPP_

#include "mimir/formalism/translator/recursive_base.hpp"

namespace mimir::formalism
{

class CollectStatisticsTranslator : public RecursiveBaseTranslator<CollectStatisticsTranslator>
{
private:
    // There can be a one to many relationship between relaxed and unrelaxed.
    std::vector<size_t> m_num_predicates_by_arity;
    std::vector<size_t> m_num_functions_by_arity;
    std::vector<size_t> m_num_constraints_by_arity;

    /* Implement RecursiveBaseTranslator interface. */
    friend class RecursiveBaseTranslator<CollectStatisticsTranslator>;

    using RecursiveBaseTranslator<CollectStatisticsTranslator>::prepare_level_2;

    void prepare_level_2(NumericConstraint condition);

    void prepare_level_2(const DomainImpl* domain);

    // Provide default implementations
    using RecursiveBaseTranslator<CollectStatisticsTranslator>::translate_level_2;

public:
    CollectStatisticsTranslator();

    const std::vector<size_t>& get_num_predicates_by_arity() const;
    const std::vector<size_t>& get_num_functions_by_arity() const;
    const std::vector<size_t>& get_num_constraints_by_arity() const;
};

extern std::ostream& operator<<(std::ostream& out, const CollectStatisticsTranslator& translator);

}

#endif