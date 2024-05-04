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

#ifndef MIMIR_FORMALISM_TRANSFORMERS_COPY_HPP_
#define MIMIR_FORMALISM_TRANSFORMERS_COPY_HPP_

#include "mimir/formalism/transformers/base_cached_recurse.hpp"

namespace mimir
{

/**
 * Recursively copy from one factory to another.
 */
class CopyTransformer : public BaseCachedRecurseTransformer<CopyTransformer>
{
private:
    /* Implement BaseTransformer interface. */
    friend class BaseCachedRecurseTransformer<CopyTransformer>;

    // Provide default implementations
    using BaseCachedRecurseTransformer::prepare_impl;
    using BaseCachedRecurseTransformer::transform_impl;

public:
    explicit CopyTransformer(PDDLFactories& pddl_factories) : BaseCachedRecurseTransformer<CopyTransformer>(pddl_factories) {}
};
}

#endif