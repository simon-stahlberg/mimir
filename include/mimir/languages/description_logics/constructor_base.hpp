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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_BASE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_BASE_HPP_

#include "mimir/languages/description_logics/constructor_interface.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * Concept
 */

template<IsConceptOrRole D, typename Derived_>
class ConstructorEvaluatorBase : public Constructor<D>
{
private:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

public:
    bool is_equal(const Constructor<D>& other) const override { return self().is_equal_impl(other); }
    size_t hash() const override { return self().hash_impl(); }

    Denotation<D> evaluate(EvaluationContext& context) const override
    {
        // Try to access cached result
        auto denotation = context.get_denotation_repository<D>().get_if(this->get_id(), context.get_state_id());
        if (denotation.has_value())
        {
            return denotation.value();
        }

        // Compute the result in the derived class
        self().evaluate_impl(context);

        // Store and return result;
        auto& builder = context.get_denotation_builder<D>();
        builder.get_flatmemory_builder().finish();
        return context.get_denotation_repository<D>().insert(this->get_id(), context.get_state_id(), builder);
    };

    bool accept(const grammar::Visitor<D>& visitor) const override { return self().accept_impl(visitor); }

    size_t get_id() const override { return self().get_id_impl(); }
};

}

#endif
