/*
 * Copyright (C) 2023 Simon Stahlberg
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

#include "../../include/mimir/algorithms/murmurhash3.hpp"
#include "../../include/mimir/formalism/problem.hpp"
#include "../../include/mimir/formalism/state.hpp"
#include "help_functions.hpp"

#include <algorithm>
#include <cassert>

namespace mimir::formalism
{
    inline std::size_t compute_state_hash(const mimir::formalism::Bitset& bitset, const mimir::formalism::ProblemDescription& problem)
    {
        // TODO: Add problem to hash
        return std::hash<mimir::formalism::Bitset>()(bitset);
    }

    StateImpl::StateImpl(mimir::formalism::Bitset&& bitset, const mimir::formalism::ProblemDescription& problem) :
        bitset_(std::move(bitset)),
        problem_(problem),
        hash_(compute_state_hash(bitset_, problem))
    {
    }

    StateImpl::StateImpl(const std::vector<uint32_t>& ranks, const mimir::formalism::ProblemDescription& problem) :
        bitset_(*std::max_element(ranks.cbegin(), ranks.cend()) + 1),
        problem_(problem),
        hash_(0)
    {
        for (auto rank : ranks)
        {
            bitset_.set(rank);
        }

        hash_ = compute_state_hash(bitset_, problem);
    }

    StateImpl::StateImpl() : bitset_(0), problem_(nullptr), hash_(0) {}

    StateImpl::StateImpl(const mimir::formalism::AtomList& atoms, const mimir::formalism::ProblemDescription& problem) :
        bitset_(problem->num_ranks()),
        problem_(problem),
        hash_(0)
    {
        for (const auto& atom : atoms)
        {
            const auto rank = problem->get_rank(atom);
            bitset_.set(rank);
        }

        hash_ = compute_state_hash(bitset_, problem_);
    }

    StateImpl::StateImpl(const mimir::formalism::AtomSet& atoms, const mimir::formalism::ProblemDescription& problem) : bitset_(0), problem_(problem), hash_(0)
    {
        for (const auto& atom : atoms)
        {
            const auto rank = problem->get_rank(atom);
            bitset_.set(rank);
        }

        hash_ = compute_state_hash(bitset_, problem_);
    }

    State create_state() { return std::make_shared<mimir::formalism::StateImpl>(); }

    State create_state(const mimir::formalism::AtomList& atoms, mimir::formalism::ProblemDescription problem)
    {
        return std::make_shared<mimir::formalism::StateImpl>(atoms, problem);
    }

    State create_state(const mimir::formalism::AtomSet& atoms, mimir::formalism::ProblemDescription problem)
    {
        return std::make_shared<mimir::formalism::StateImpl>(atoms, problem);
    }

    bool StateImpl::operator<(const StateImpl& other) const
    {
        if (problem_ != other.problem_)
        {
            return problem_ < other.problem_;
        }

        return bitset_ < other.bitset_;
    }

    bool StateImpl::operator==(const StateImpl& other) const
    {
        if (hash_ != other.hash_)
        {
            return false;
        }

        if (problem_.get() != other.problem_.get())
        {
            return false;
        }

        return bitset_ == other.bitset_;
    }

    bool StateImpl::operator!=(const StateImpl& other) const { return !(this->operator==(other)); }

    mimir::formalism::AtomList StateImpl::get_atoms() const
    {
        mimir::formalism::AtomList atoms;
        std::size_t position = 0;

        while (true)
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);
            atoms.emplace_back(problem_->get_atom(rank));
            ++position;
        }

        return atoms;
    }

    mimir::formalism::AtomList StateImpl::get_static_atoms() const
    {
        mimir::formalism::AtomList atoms;
        const auto& static_atoms = problem_->get_static_atoms();
        std::size_t position = 0;

        while (position < static_atoms.size())
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);
            assert(problem_->is_static(rank));
            atoms.emplace_back(problem_->get_atom(rank));
            ++position;
        }

        return atoms;
    }

    mimir::formalism::AtomList StateImpl::get_dynamic_atoms() const
    {
        mimir::formalism::AtomList atoms;
        const auto& static_atoms = problem_->get_static_atoms();
        std::size_t position = static_atoms.size();

        while (true)
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);
            assert(problem_->is_dynamic(rank));
            atoms.emplace_back(problem_->get_atom(rank));
            ++position;
        }

        return atoms;
    }

    std::vector<uint32_t> StateImpl::get_ranks() const
    {
        std::vector<uint32_t> ranks;
        std::size_t position = 0;

        while (true)
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);
            ranks.emplace_back(rank);
            ++position;
        }

        return ranks;
    }

    std::vector<uint32_t> StateImpl::get_static_ranks() const
    {
        std::vector<uint32_t> static_ranks;
        std::size_t position = 0;

        while (true)
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);

            if (problem_->is_static(rank))
            {
                static_ranks.emplace_back(rank);
            }

            ++position;
        }

        return static_ranks;
    }

    std::vector<uint32_t> StateImpl::get_dynamic_ranks() const
    {
        std::vector<uint32_t> static_ranks;
        std::size_t position = 0;

        while (true)
        {
            position = bitset_.next_set_bit(position);

            if (position == bitset_.no_position)
            {
                break;
            }

            const auto rank = static_cast<uint32_t>(position);

            if (problem_->is_dynamic(rank))
            {
                static_ranks.emplace_back(rank);
            }

            ++position;
        }

        return static_ranks;
    }

    mimir::formalism::ProblemDescription StateImpl::get_problem() const { return problem_; }

    std::map<mimir::formalism::Predicate, mimir::formalism::AtomList> StateImpl::get_atoms_grouped_by_predicate() const
    {
        std::map<mimir::formalism::Predicate, mimir::formalism::AtomList> grouped_atoms;

        for (const auto& atom : get_atoms())
        {
            const auto& predicate = atom->predicate;

            if (grouped_atoms.find(predicate) == grouped_atoms.end())
            {
                grouped_atoms.insert(std::make_pair(predicate, mimir::formalism::AtomList()));
            }

            auto& atom_list = grouped_atoms.at(predicate);
            atom_list.push_back(atom);
        }

        return grouped_atoms;
    }

    std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
    StateImpl::pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const
    {
        std::map<uint32_t, std::vector<uint32_t>> packed_ids;
        std::map<uint32_t, std::pair<std::string, uint32_t>> id_to_name_arity;

        for (const auto& atom : get_atoms())
        {
            const auto& predicate = atom->predicate;

            if (packed_ids.find(predicate->id) == packed_ids.end())
            {
                packed_ids.insert(std::make_pair(predicate->id, std::vector<uint32_t>()));
                id_to_name_arity.insert(std::make_pair(predicate->id, std::make_pair(predicate->name, predicate->arity)));
            }

            auto& atom_list = packed_ids.at(predicate->id);

            for (const auto& object : atom->arguments)
            {
                atom_list.push_back(object->id);
            }
        }

        auto num_predicates = problem_->domain->predicates.size();

        if (include_types)
        {
            std::map<mimir::formalism::Type, uint32_t> type_ids;
            const auto& types = problem_->domain->types;

            for (const auto& type : types)
            {
                type_ids.emplace(type, static_cast<uint32_t>(num_predicates + type_ids.size()));
            }

            for (const auto& object : problem_->objects)
            {
                const auto object_id = object->id;
                auto type = object->type;

                while (type != nullptr)
                {
                    const auto predicate_id = type_ids[type];

                    if (packed_ids.find(predicate_id) == packed_ids.end())
                    {
                        packed_ids.insert(std::make_pair(predicate_id, std::vector<uint32_t>()));
                        id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(type->name + "_type", 1)));
                    }

                    auto& atom_list = packed_ids.at(predicate_id);
                    atom_list.push_back(object_id);
                    type = type->base;
                }
            }

            num_predicates += types.size();
        }

        if (include_goal)
        {
            for (const auto& literal : problem_->goal)
            {
                if (literal->negated)
                {
                    throw std::invalid_argument("negated literal in the goal");
                }

                const auto atom = literal->atom;
                const auto predicate_id = static_cast<uint32_t>(num_predicates) + atom->predicate->id;

                if (packed_ids.find(predicate_id) == packed_ids.end())
                {
                    packed_ids.emplace(predicate_id, std::vector<uint32_t>());
                    id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(atom->predicate->name + "_goal", atom->predicate->arity)));
                }

                auto& atom_list = packed_ids.at(predicate_id);

                for (const auto& object : atom->arguments)
                {
                    atom_list.push_back(object->id);
                }
            }
        }

        return std::make_pair(packed_ids, id_to_name_arity);
    }

    std::size_t StateImpl::hash() const { return hash_; }

    bool is_in_state(uint32_t rank, const mimir::formalism::State& state) { return state->bitset_.get(rank); }

    bool is_in_state(const mimir::formalism::Atom& atom, const mimir::formalism::State& state)
    {
        return is_in_state(state->get_problem()->get_rank(atom), state);
    }

    bool subset_of_state(const std::vector<uint32_t>& ranks, const mimir::formalism::State& state)
    {
        for (auto rank : ranks)
        {
            if (!is_in_state(rank, state))
            {
                return false;
            }
        }

        return true;
    }

    bool subset_of_state(const mimir::formalism::AtomList& atoms, const mimir::formalism::State& state)
    {
        for (const auto& atom : atoms)
        {
            if (!is_in_state(atom, state))
            {
                return false;
            }
        }

        return true;
    }

    bool is_applicable(const mimir::formalism::Action& action, const mimir::formalism::State& state)
    {
        if (static_cast<int32_t>(action->get_arguments().size()) != action->schema->arity)
        {
            throw std::runtime_error("is_applicable: action is not ground");
        }

        auto bitset = state->bitset_;
        bitset |= action->applicability_positive_precondition_bitset_;
        bitset &= action->applicability_negative_precondition_bitset_;
        return state->bitset_ == bitset;
    }

    mimir::formalism::State apply(const mimir::formalism::Action& action, const mimir::formalism::State& state)
    {
        // We first apply the delete lists, followed by the add lists to prevent actions from simultaneously negating and establishing the same condition.

        auto bitset = state->bitset_;
        const auto num_conditional_effects = action->conditional_positive_precondition_bitsets_.size();

        if (num_conditional_effects > 0)
        {
            std::vector<std::size_t> applicable_conditional_effects;

            for (std::size_t index = 0; index < num_conditional_effects; ++index)
            {
                if ((bitset == (bitset | action->conditional_positive_precondition_bitsets_[index])) &&  // positive preconditions
                    (bitset == (bitset & action->conditional_negative_precondition_bitsets_[index])))    // negative preconditions
                {
                    applicable_conditional_effects.emplace_back(index);
                }
            }

            // Apply the delete lists

            bitset &= action->unconditional_negative_effect_bitset_;

            for (const auto& index : applicable_conditional_effects)
            {
                bitset &= action->conditional_negative_effect_bitsets_[index];
            }

            // Apply the add lists

            bitset |= action->unconditional_positive_effect_bitset_;

            for (const auto& index : applicable_conditional_effects)
            {
                bitset |= action->conditional_positive_effect_bitsets_[index];
            }
        }
        else
        {
            // Apply the delete list, then the add list

            bitset &= action->unconditional_negative_effect_bitset_;
            bitset |= action->unconditional_positive_effect_bitset_;
        }

        return std::make_shared<mimir::formalism::StateImpl>(std::move(bitset), state->problem_);
    }

    bool atoms_hold(const AtomList& atoms, const mimir::formalism::State& state)
    {
        for (const auto& atom : atoms)
        {
            if (!mimir::formalism::is_in_state(atom, state))
            {
                return false;
            }
        }

        return true;
    }

    bool literal_holds(const mimir::formalism::Literal& literal, const mimir::formalism::State& state)
    {
        return mimir::formalism::is_in_state(literal->atom, state) != literal->negated;
    }

    bool literals_hold(const LiteralList& literal_list, const mimir::formalism::State& state, std::size_t min_arity)
    {
        for (const auto& literal : literal_list)
        {
            const auto predicate_arity = literal->atom->predicate->arity;

            if ((predicate_arity >= min_arity) && !mimir::formalism::literal_holds(literal, state))
            {
                return false;
            }
        }

        return true;
    }

    bool matches_any_in_state(const mimir::formalism::Atom& atom, const mimir::formalism::State& state)
    {
        for (const auto& state_atom : state->get_atoms())
        {
            if (mimir::formalism::matches(atom, state_atom))
            {
                return true;
            }
        }

        return false;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::State& state)
    {
        os << state->get_atoms();
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::StateList& states)
    {
        print_vector(os, states);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructu1
    std::size_t hash<mimir::formalism::State>::operator()(const mimir::formalism::State& state) const { return !state ? 0 : state->hash_; }

    bool less<mimir::formalism::State>::operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const
    {
        return *left_state < *right_state;
    }

    bool equal_to<mimir::formalism::State>::operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const
    {
        return *left_state == *right_state;
    }
}  // namespace std
