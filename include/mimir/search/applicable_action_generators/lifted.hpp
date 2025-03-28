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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_LIFTED_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/action.hpp"

namespace mimir::search
{

/// @brief `LiftedApplicableActionGenerator` implements lifted applicable action generation
/// using maximum clique enumeration by Stahlberg (ECAI2023).
/// Source: https://mrlab.ai/papers/stahlberg-ecai2023.pdf
class LiftedApplicableActionGenerator : public IApplicableActionGenerator
{
public:
    class Statistics
    {
    private:
        uint64_t m_num_ground_action_cache_hits;
        uint64_t m_num_ground_action_cache_misses;

        std::vector<uint64_t> m_num_ground_action_cache_hits_per_search_layer;
        std::vector<uint64_t> m_num_ground_action_cache_misses_per_search_layer;
        std::vector<uint64_t> m_num_inapplicable_grounded_actions_per_search_layer;

    public:
        Statistics() :
            m_num_ground_action_cache_hits(0),
            m_num_ground_action_cache_misses(0),
            m_num_ground_action_cache_hits_per_search_layer(),
            m_num_ground_action_cache_misses_per_search_layer(),
            m_num_inapplicable_grounded_actions_per_search_layer()
        {
        }

        /// @brief Store information for the layer
        void on_finish_search_layer()
        {
            m_num_ground_action_cache_hits_per_search_layer.push_back(m_num_ground_action_cache_hits);
            m_num_ground_action_cache_misses_per_search_layer.push_back(m_num_ground_action_cache_misses);
        }

        void increment_num_ground_action_cache_hits() { ++m_num_ground_action_cache_hits; }
        void increment_num_ground_action_cache_misses() { ++m_num_ground_action_cache_misses; }

        uint64_t get_num_ground_action_cache_hits() const { return m_num_ground_action_cache_hits; }
        uint64_t get_num_ground_action_cache_misses() const { return m_num_ground_action_cache_misses; }

        const std::vector<uint64_t>& get_num_ground_action_cache_hits_per_search_layer() const { return m_num_ground_action_cache_hits_per_search_layer; }
        const std::vector<uint64_t>& get_num_ground_action_cache_misses_per_search_layer() const { return m_num_ground_action_cache_misses_per_search_layer; }
        const std::vector<uint64_t>& get_num_inapplicable_grounded_actions_per_search_layer() const
        {
            return m_num_inapplicable_grounded_actions_per_search_layer;
        }
    };

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void on_start_generating_applicable_actions() = 0;

        virtual void on_ground_action(formalism::GroundAction action) = 0;

        virtual void on_ground_action_cache_hit(formalism::GroundAction action) = 0;

        virtual void on_ground_action_cache_miss(formalism::GroundAction action) = 0;

        virtual void on_end_generating_applicable_actions() = 0;

        virtual void on_end_search() = 0;

        virtual void on_finish_search_layer() = 0;

        virtual const Statistics& get_statistics() const = 0;
    };

    /**
     * Base class
     *
     * Collect statistics and call implementation of derived class.
     */
    template<typename Derived_>
    class EventHandlerBase : public IEventHandler
    {
    protected:
        Statistics m_statistics;
        bool m_quiet;

    private:
        EventHandlerBase() = default;
        friend Derived_;

        /// @brief Helper to cast to Derived_.
        constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
        constexpr auto& self() { return static_cast<Derived_&>(*this); }

    public:
        explicit EventHandlerBase(bool quiet = true) : m_statistics(), m_quiet(quiet) {}

        void on_start_generating_applicable_actions() override
        {
            if (!m_quiet)
                self().on_start_generating_applicable_actions_impl();
        }

        void on_ground_action(formalism::GroundAction action) override
        {
            if (!m_quiet)
                self().on_ground_action_impl(action);
        }

        void on_ground_action_cache_hit(formalism::GroundAction action) override
        {
            m_statistics.increment_num_ground_action_cache_hits();

            if (!m_quiet)
                self().on_ground_action_cache_hit_impl(action);
        }

        void on_ground_action_cache_miss(formalism::GroundAction action) override
        {
            m_statistics.increment_num_ground_action_cache_misses();

            if (!m_quiet)
                self().on_ground_action_cache_miss_impl(action);
        }

        void on_end_generating_applicable_actions() override
        {
            if (!m_quiet)
                self().on_end_generating_applicable_actions_impl();
        }

        void on_finish_search_layer() override
        {
            m_statistics.on_finish_search_layer();

            if (!m_quiet)
                self().on_finish_search_layer_impl();
        }

        void on_end_search() override
        {
            if (!m_quiet)
                self().on_end_search_impl();
        }

        const Statistics& get_statistics() const override { return m_statistics; }
    };

    class DebugEventHandler : public EventHandlerBase<DebugEventHandler>
    {
    private:
        /* Implement EventHandlerBase interface */
        friend class EventHandlerBase<DebugEventHandler>;

        void on_start_generating_applicable_actions_impl() const;

        void on_ground_action_impl(formalism::GroundAction action) const;

        void on_ground_action_cache_hit_impl(formalism::GroundAction action) const;

        void on_ground_action_cache_miss_impl(formalism::GroundAction action) const;

        void on_end_generating_applicable_actions_impl() const;

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DebugEventHandler(bool quiet = true) : EventHandlerBase<DebugEventHandler>(quiet) {}
    };

    class DefaultEventHandler : public EventHandlerBase<DefaultEventHandler>
    {
    private:
        /* Implement EventHandlerBase interface */
        friend class EventHandlerBase<DefaultEventHandler>;

        void on_start_generating_applicable_actions_impl() const;

        void on_ground_action_impl(formalism::GroundAction action) const;

        void on_ground_action_cache_hit_impl(formalism::GroundAction action) const;

        void on_ground_action_cache_miss_impl(formalism::GroundAction action) const;

        void on_end_generating_applicable_actions_impl() const;

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DefaultEventHandler(bool quiet = true) : EventHandlerBase<DefaultEventHandler>(quiet) {}
    };

public:
    /// @brief Simplest construction
    explicit LiftedApplicableActionGenerator(formalism::Problem problem);

    /// @brief Complete construction
    LiftedApplicableActionGenerator(formalism::Problem problem, std::shared_ptr<IEventHandler> event_handler);

    // Uncopyable
    LiftedApplicableActionGenerator(const LiftedApplicableActionGenerator& other) = delete;
    LiftedApplicableActionGenerator& operator=(const LiftedApplicableActionGenerator& other) = delete;
    // Unmovable
    LiftedApplicableActionGenerator(LiftedApplicableActionGenerator&& other) = delete;
    LiftedApplicableActionGenerator& operator=(LiftedApplicableActionGenerator&& other) = delete;

    mimir::generator<formalism::GroundAction> create_applicable_action_generator(State state) override;
    mimir::generator<formalism::GroundAction> create_applicable_action_generator(const DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters
     */

    const formalism::Problem& get_problem() const override;

private:
    formalism::Problem m_problem;
    std::shared_ptr<IEventHandler> m_event_handler;

    ActionSatisficingBindingGeneratorList m_action_grounding_data;

    /* Memory for reuse */
    DenseState m_dense_state;
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::AssignmentSet<formalism::FluentTag> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::DerivedTag> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::FluentTag> m_numeric_assignment_set;
};

/**
 * Pretty printing
 */

inline std::ostream& operator<<(std::ostream& os, const LiftedApplicableActionGenerator::Statistics& statistics)
{
    os << "[LiftedApplicableActionGenerator] Number of grounded action cache hits: " << statistics.get_num_ground_action_cache_hits() << std::endl
       << "[LiftedApplicableActionGenerator] Number of grounded action cache hits until last f-layer: "
       << (statistics.get_num_ground_action_cache_hits_per_search_layer().empty() ? 0 : statistics.get_num_ground_action_cache_hits_per_search_layer().back())
       << std::endl
       << "[LiftedApplicableActionGenerator] Number of grounded action cache misses: " << statistics.get_num_ground_action_cache_misses() << std::endl
       << "[LiftedApplicableActionGenerator] Number of grounded action cache misses until last f-layer: "
       << (statistics.get_num_ground_action_cache_misses_per_search_layer().empty() ? 0 :
                                                                                      statistics.get_num_ground_action_cache_misses_per_search_layer().back())
       << std::endl
       << "[LiftedApplicableActionGenerator] Number of generated inapplicable grounded actions until last f-layer: "
       << (statistics.get_num_inapplicable_grounded_actions_per_search_layer().empty() ?
               0 :
               statistics.get_num_inapplicable_grounded_actions_per_search_layer().back());

    return os;
}

}  // namespace mimir

#endif
