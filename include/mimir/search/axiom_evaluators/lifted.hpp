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

#ifndef MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_HPP_
#define MIMIR_SEARCH_AXIOM_EVALUATOR_LIFTED_HPP_

#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/axiom.hpp"

namespace mimir::search
{

class LiftedAxiomEvaluator : public IAxiomEvaluator
{
public:
    class Statistics
    {
    private:
        uint64_t m_num_ground_axiom_cache_hits;
        uint64_t m_num_ground_axiom_cache_misses;

        std::vector<uint64_t> m_num_ground_axiom_cache_hits_per_search_layer;
        std::vector<uint64_t> m_num_ground_axiom_cache_misses_per_search_layer;

    public:
        Statistics() :
            m_num_ground_axiom_cache_hits(0),
            m_num_ground_axiom_cache_misses(0),
            m_num_ground_axiom_cache_hits_per_search_layer(),
            m_num_ground_axiom_cache_misses_per_search_layer()
        {
        }

        /// @brief Store information for the layer
        void on_finish_search_layer()
        {
            m_num_ground_axiom_cache_hits_per_search_layer.push_back(m_num_ground_axiom_cache_hits);
            m_num_ground_axiom_cache_misses_per_search_layer.push_back(m_num_ground_axiom_cache_misses);
        }

        void increment_num_ground_axiom_cache_hits() { ++m_num_ground_axiom_cache_hits; }
        void increment_num_ground_axiom_cache_misses() { ++m_num_ground_axiom_cache_misses; }

        uint64_t get_num_ground_axiom_cache_hits() const { return m_num_ground_axiom_cache_hits; }
        uint64_t get_num_ground_axiom_cache_misses() const { return m_num_ground_axiom_cache_misses; }

        const std::vector<uint64_t>& get_num_ground_axiom_cache_hits_per_search_layer() const { return m_num_ground_axiom_cache_hits_per_search_layer; }
        const std::vector<uint64_t>& get_num_ground_axiom_cache_misses_per_search_layer() const { return m_num_ground_axiom_cache_misses_per_search_layer; }
    };

    class IEventHandler
    {
    public:
        virtual ~IEventHandler() = default;

        virtual void on_start_generating_applicable_axioms() = 0;

        virtual void on_ground_axiom(formalism::GroundAxiom axiom) = 0;

        virtual void on_ground_axiom_cache_hit(formalism::GroundAxiom axiom) = 0;

        virtual void on_ground_axiom_cache_miss(formalism::GroundAxiom axiom) = 0;

        virtual void on_end_generating_applicable_axioms() = 0;

        virtual void on_end_search() = 0;

        virtual void on_finish_search_layer() = 0;

        virtual const Statistics& get_statistics() const = 0;
    };

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

        void on_start_generating_applicable_axioms() override
        {
            if (!m_quiet)
                self().on_start_generating_applicable_axioms_impl();
        }

        void on_ground_axiom(formalism::GroundAxiom axiom) override
        {
            if (!m_quiet)
                self().on_ground_axiom_impl(axiom);
        }

        void on_ground_axiom_cache_hit(formalism::GroundAxiom axiom) override
        {
            m_statistics.increment_num_ground_axiom_cache_hits();

            if (!m_quiet)
                self().on_ground_axiom_cache_hit_impl(axiom);
        }

        void on_ground_axiom_cache_miss(formalism::GroundAxiom axiom) override
        {
            m_statistics.increment_num_ground_axiom_cache_misses();

            if (!m_quiet)
                self().on_ground_axiom_cache_miss_impl(axiom);
        }

        void on_end_generating_applicable_axioms() override
        {
            if (!m_quiet)
                self().on_end_generating_applicable_axioms_impl();
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

        void on_start_generating_applicable_axioms_impl() const;

        void on_ground_axiom_impl(formalism::GroundAxiom axiom) const;

        void on_ground_axiom_cache_hit_impl(formalism::GroundAxiom axiom) const;

        void on_ground_axiom_cache_miss_impl(formalism::GroundAxiom axiom) const;

        void on_end_generating_applicable_axioms_impl() const;

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

        void on_start_generating_applicable_axioms_impl() const;

        void on_ground_axiom_impl(formalism::GroundAxiom axiom) const;

        void on_ground_axiom_cache_hit_impl(formalism::GroundAxiom axiom) const;

        void on_ground_axiom_cache_miss_impl(formalism::GroundAxiom axiom) const;

        void on_end_generating_applicable_axioms_impl() const;

        void on_finish_search_layer_impl() const;

        void on_end_search_impl() const;

    public:
        explicit DefaultEventHandler(bool quiet = true) : EventHandlerBase<DefaultEventHandler>(quiet) {}
    };

public:
    explicit LiftedAxiomEvaluator(formalism::Problem problem);

    LiftedAxiomEvaluator(formalism::Problem problem, std::shared_ptr<IEventHandler> event_handler);

    // Uncopyable
    LiftedAxiomEvaluator(const LiftedAxiomEvaluator& other) = delete;
    LiftedAxiomEvaluator& operator=(const LiftedAxiomEvaluator& other) = delete;
    // Unmovable
    LiftedAxiomEvaluator(LiftedAxiomEvaluator&& other) = delete;
    LiftedAxiomEvaluator& operator=(LiftedAxiomEvaluator&& other) = delete;

    void generate_and_apply_axioms(DenseState& dense_state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    /**
     * Getters.
     */

    const formalism::Problem& get_problem() const override;
    const std::shared_ptr<IEventHandler>& get_event_handler() const;

private:
    formalism::Problem m_problem;
    std::shared_ptr<IEventHandler> m_event_handler;

    AxiomSatisficingBindingGeneratorList m_condition_grounders;

    /* Memory for reuse */
    formalism::GroundAtomList<formalism::FluentTag> m_fluent_atoms;
    formalism::GroundAtomList<formalism::DerivedTag> m_derived_atoms;
    formalism::GroundFunctionList<formalism::FluentTag> m_fluent_functions;
    formalism::AssignmentSet<formalism::FluentTag> m_fluent_assignment_set;
    formalism::AssignmentSet<formalism::DerivedTag> m_derived_assignment_set;
    formalism::NumericAssignmentSet<formalism::FluentTag> m_numeric_assignment_set;
};

inline std::ostream& operator<<(std::ostream& os, const LiftedAxiomEvaluator::Statistics& statistics)
{
    os << "[LiftedAxiomEvaluator] Number of grounded axiom cache hits: " << statistics.get_num_ground_axiom_cache_hits() << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache hits until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_hits_per_search_layer().empty() ? 0 : statistics.get_num_ground_axiom_cache_hits_per_search_layer().back())
       << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache misses: " << statistics.get_num_ground_axiom_cache_misses() << std::endl
       << "[LiftedAxiomEvaluator] Number of grounded axiom cache misses until last f-layer: "
       << (statistics.get_num_ground_axiom_cache_misses_per_search_layer().empty() ? 0 :
                                                                                     statistics.get_num_ground_axiom_cache_misses_per_search_layer().back());

    return os;
}

}  // namespace mimir

#endif
