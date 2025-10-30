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

#ifndef MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_FACTORY_HPP_
#define MIMIR_LANGUAGES_GENERAL_POLICIES_GENERAL_POLICY_FACTORY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
class GeneralPolicyFactory
{
public:
    /// @brief Create the `GeneralPolicy` by Frances et al. for the Gripper domain: https://arxiv.org/pdf/2101.00692
    /// @param domain the Gripper domain.
    /// @param repositories the general policies repositories.
    /// @param dl_repositories the description logics repositories.
    /// @return the `GeneralPolicy` by Frances et al. for the Gripper domain.
    static GeneralPolicy
    get_or_create_general_policy_gripper(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);

    /// @brief Create the `GeneralPolicy` by Frances et al. for the Blocks3ops domain: https://arxiv.org/pdf/2101.00692
    ///
    /// Note: We removed the condition bwp>0 in feature conditions to make the policy work in states where there are no well placed blocks.
    /// @param domain the Blocks3ops domain
    /// @param repositories the general policies repositories.
    /// @param dl_repositories the description logics repositories.
    /// @return the `GeneralPolicy` by Frances et al. for the Blocks3ops domain.
    static GeneralPolicy
    get_or_create_general_policy_blocks3ops(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);

    /// @brief Create the `GeneralPolicy` by Frances et al. for the Spanner domain: https://arxiv.org/pdf/2101.00692
    /// @param domain the Spanner domain
    /// @param repositories the general policies repositories.
    /// @param dl_repositories the description logics repositories.
    /// @return the `GeneralPolicy` by Frances et al. for the Spanner domain.
    static GeneralPolicy
    get_or_create_general_policy_spanner(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);

    /// @brief Create the `GeneralPolicy` by Frances et al. for the Delivery domain: https://arxiv.org/pdf/2101.00692
    /// @param domain the Delivery domain
    /// @param repositories the general policies repositories.
    /// @param dl_repositories the description logics repositories.
    /// @return the `GeneralPolicy` by Frances et al. for the Delivery domain.
    static GeneralPolicy
    get_or_create_general_policy_delivery(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);

    /// @brief Create the `GeneralPolicy` by Frances et al. for the Miconic domain: https://arxiv.org/pdf/2101.00692
    /// @param domain the Miconic domain
    /// @param repositories the general policies repositories.
    /// @param dl_repositories the description logics repositories.
    /// @return the `GeneralPolicy` by Frances et al. for the Miconic domain.
    static GeneralPolicy
    get_or_create_general_policy_miconic(const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);
};
}

#endif