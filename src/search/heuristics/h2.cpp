#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/heuristics/h2.hpp"
#include "mimir/search/state.hpp"

#include <algorithm>
#include <boost/hana.hpp>
#include <cassert>
#include <limits>

namespace mimir::search
{

H2HeuristicImpl::H2HeuristicImpl(const IGrounder& grounder) : m_problem(grounder.get_problem())
{
    const auto& repositories = m_problem->get_repositories();
    const auto& fluent_atoms = repositories.get_ground_atoms<formalism::FluentTag>();
    m_num_fluent_atoms = std::distance(fluent_atoms.begin(), fluent_atoms.end());

    const auto& derived_atoms = repositories.get_ground_atoms<formalism::DerivedTag>();
    m_num_derived_atoms = std::distance(derived_atoms.begin(), derived_atoms.end());

    uint32_t num_vars = m_num_fluent_atoms + m_num_derived_atoms;
    m_num_atoms = 2 * num_vars;

    m_h1_table.resize(m_num_atoms);
    m_h2_table.resize(m_num_atoms * m_num_atoms);

    // Build internal ground actions
    for (const auto& action : grounder.create_ground_actions())
    {
        InternalGroundAction internal_action;
        internal_action.cost = 1.0;  // Unit costs are assumed

        // Preconditions
        const auto& preconds = action->get_conjunctive_condition();

        const auto& pos_fluents = preconds->get_precondition<formalism::PositiveTag, formalism::FluentTag>();
        for (auto idx : pos_fluents)
        {
            internal_action.precondition.push_back(2 * idx);
        }

        const auto& neg_fluents = preconds->get_precondition<formalism::NegativeTag, formalism::FluentTag>();
        for (auto idx : neg_fluents)
        {
            internal_action.precondition.push_back(2 * idx + 1);
        }

        const auto& pos_derived = preconds->get_precondition<formalism::PositiveTag, formalism::DerivedTag>();
        for (auto idx : pos_derived)
        {
            internal_action.precondition.push_back(2 * (m_num_fluent_atoms + idx));
        }

        const auto& neg_derived = preconds->get_precondition<formalism::NegativeTag, formalism::DerivedTag>();
        for (auto idx : neg_derived)
        {
            internal_action.precondition.push_back(2 * (m_num_fluent_atoms + idx) + 1);
        }

        std::sort(internal_action.precondition.begin(), internal_action.precondition.end());

        // Effects
        const auto& effects = action->get_conditional_effects();
        for (const auto& cond_eff : effects)
        {
            const auto& eff = cond_eff->get_conjunctive_effect();

            for (auto idx : eff->get_propositional_effects<formalism::PositiveTag>())
            {
                internal_action.add_effect.push_back(2 * idx);
            }

            for (auto idx : eff->get_propositional_effects<formalism::NegativeTag>())
            {
                internal_action.delete_effect.push_back(2 * idx + 1);
            }
        }

        std::sort(internal_action.add_effect.begin(), internal_action.add_effect.end());
        std::sort(internal_action.delete_effect.begin(), internal_action.delete_effect.end());

        for (uint32_t atom_idx = 0; atom_idx < m_num_atoms; ++atom_idx)
        {
            if (!std::binary_search(internal_action.delete_effect.begin(), internal_action.delete_effect.end(), atom_idx))
            {
                internal_action.delete_effect_complement.push_back(atom_idx);
            }
        }

        m_internal_actions.push_back(std::move(internal_action));
    }

    update_goal(nullptr);
}

std::shared_ptr<H2HeuristicImpl> H2HeuristicImpl::create(const IGrounder& grounder) { return std::make_shared<H2HeuristicImpl>(grounder); }

void H2HeuristicImpl::update_goal(formalism::GroundConjunctiveCondition goal) const
{
    m_goal.clear();
    if (goal == nullptr)
    {
        const auto& goal_literals = m_problem->get_goal_literals();

        const auto& fluent_goals = boost::hana::at_key(goal_literals, boost::hana::type<formalism::FluentTag> {});
        for (const auto& lit : fluent_goals)
        {
            uint32_t idx = lit->get_atom()->get_index();
            m_goal.push_back(lit->get_polarity() ? 2 * idx : 2 * idx + 1);
        }

        const auto& derived_goals = boost::hana::at_key(goal_literals, boost::hana::type<formalism::DerivedTag> {});
        for (const auto& lit : derived_goals)
        {
            uint32_t idx = lit->get_atom()->get_index();
            m_goal.push_back(lit->get_polarity() ? 2 * (m_num_fluent_atoms + idx) : 2 * (m_num_fluent_atoms + idx) + 1);
        }
    }
    else
    {
        const auto& pos_fluents = goal->get_precondition<formalism::PositiveTag, formalism::FluentTag>();
        for (auto idx : pos_fluents)
        {
            m_goal.push_back(2 * idx);
        }

        const auto& neg_fluents = goal->get_precondition<formalism::NegativeTag, formalism::FluentTag>();
        for (auto idx : neg_fluents)
        {
            m_goal.push_back(2 * idx + 1);
        }

        const auto& pos_derived = goal->get_precondition<formalism::PositiveTag, formalism::DerivedTag>();
        for (auto idx : pos_derived)
        {
            m_goal.push_back(2 * (m_num_fluent_atoms + idx));
        }

        const auto& neg_derived = goal->get_precondition<formalism::NegativeTag, formalism::DerivedTag>();
        for (auto idx : neg_derived)
        {
            m_goal.push_back(2 * (m_num_fluent_atoms + idx) + 1);
        }
    }
    std::sort(m_goal.begin(), m_goal.end());
    m_last_goal = goal;
}

void H2HeuristicImpl::initialize_tables(const State& state) const
{
    std::fill(m_h1_table.begin(), m_h1_table.end(), std::numeric_limits<double>::infinity());
    std::fill(m_h2_table.begin(), m_h2_table.end(), std::numeric_limits<double>::infinity());

    std::vector<uint32_t> true_literals;
    true_literals.reserve(m_num_atoms);

    const auto& fluent_atoms = state.get_atoms<formalism::FluentTag>();
    auto it = fluent_atoms.begin();
    auto end = fluent_atoms.end();

    for (uint32_t i = 0; i < m_num_fluent_atoms; ++i)
    {
        bool is_true = false;
        if (it != end && (*it == i))
        {
            is_true = true;
            ++it;
        }
        true_literals.push_back(is_true ? (2 * i) : (2 * i + 1));
    }

    const auto& derived_atoms = state.get_atoms<formalism::DerivedTag>();
    auto it_d = derived_atoms.begin();
    auto end_d = derived_atoms.end();

    for (uint32_t i = 0; i < m_num_derived_atoms; ++i)
    {
        bool is_true = false;
        if (it_d != end_d && (*it_d == i))
        {
            is_true = true;
            ++it_d;
        }
        true_literals.push_back(is_true ? (2 * (m_num_fluent_atoms + i)) : (2 * (m_num_fluent_atoms + i) + 1));
    }

    for (size_t i = 0; i < true_literals.size(); ++i)
    {
        uint32_t u = true_literals[i];
        m_h1_table[u] = 0;

        for (size_t j = i; j < true_literals.size(); ++j)
        {
            uint32_t v = true_literals[j];
            m_h2_table[u * m_num_atoms + v] = 0;
            m_h2_table[v * m_num_atoms + u] = 0;
        }
    }
}

double H2HeuristicImpl::evaluate(const std::vector<uint32_t>& indices) const
{
    double v = 0;

    for (std::size_t i = 0; i < indices.size(); i++)
    {
        v = std::max(v, m_h1_table[indices[i]]);

        if (v == std::numeric_limits<double>::infinity())
        {
            return std::numeric_limits<double>::infinity();
        }

        for (std::size_t j = i + 1; j < indices.size(); j++)
        {
            v = std::max(v, m_h2_table[indices[i] * m_num_atoms + indices[j]]);

            if (v == std::numeric_limits<double>::infinity())
            {
                return std::numeric_limits<double>::infinity();
            }
        }
    }

    return v;
}

double H2HeuristicImpl::evaluate(const std::vector<uint32_t>& indices, uint32_t index) const
{
    double v = m_h1_table[index];

    if (v == std::numeric_limits<double>::infinity())
    {
        return std::numeric_limits<double>::infinity();
    }

    for (std::size_t i = 0; i < indices.size(); i++)
    {
        if (index == indices[i])
        {
            continue;
        }

        v = std::max(v, m_h2_table[index * m_num_atoms + indices[i]]);

        if (v == std::numeric_limits<double>::infinity())
        {
            return std::numeric_limits<double>::infinity();
        }
    }

    return v;
}

void H2HeuristicImpl::update(uint32_t index, double value, bool& changed) const
{
    if (value < m_h1_table[index])
    {
        m_h1_table[index] = value;
        m_h2_table[index * m_num_atoms + index] = value;
        changed = true;
    }
}

void H2HeuristicImpl::update(uint32_t u, uint32_t v, double value, bool& changed) const
{
    if (value < m_h2_table[u * m_num_atoms + v])
    {
        m_h2_table[u * m_num_atoms + v] = value;
        m_h2_table[v * m_num_atoms + u] = value;
        changed = true;
    }
}

void H2HeuristicImpl::fill_tables(const State& state, formalism::GroundConjunctiveCondition goal) const
{
    initialize_tables(state);

    bool changed;

    do
    {
        changed = false;

        for (const auto& internal_action : m_internal_actions)
        {
            const auto c1 = evaluate(internal_action.precondition);

            if (c1 == std::numeric_limits<double>::infinity())
            {
                continue;
            }

            for (std::size_t i = 0; i < internal_action.add_effect.size(); i++)
            {
                const auto p = internal_action.add_effect[i];
                update(p, c1 + internal_action.cost, changed);

                for (std::size_t j = i + 1; j < internal_action.add_effect.size(); j++)
                {
                    const auto q = internal_action.add_effect[j];
                    if (p != q)
                    {
                        update(p, q, c1 + internal_action.cost, changed);
                    }
                }

                for (const auto r : internal_action.delete_effect_complement)
                {
                    const auto c2 = std::max(c1, evaluate(internal_action.precondition, r));

                    if (c2 != std::numeric_limits<double>::infinity())
                    {
                        update(p, r, c2 + internal_action.cost, changed);
                    }
                }
            }
        }
    } while (changed);
}

ContinuousCost H2HeuristicImpl::compute_heuristic(const State& state, formalism::GroundConjunctiveCondition goal)
{
    if (goal != m_last_goal)
    {
        update_goal(goal);
    }

    if (m_goal.empty())
    {
        return 0;
    }

    fill_tables(state, goal);
    return evaluate(m_goal);
}

}
