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

#include "../generators/lifted_successor_generator.hpp"
#include "help_functions.hpp"
#include "problem.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>

namespace formalism
{
    ProblemImpl::ProblemImpl(const std::string& name,
                             const formalism::DomainDescription& domain,
                             const formalism::ObjectList& objects,
                             const formalism::AtomList& initial,
                             const formalism::LiteralList& goal,
                             const std::unordered_map<formalism::Atom, double>& atom_costs) :
        static_atoms_(),
        predicate_id_to_static_(),
        atom_ranks_(),
        rank_to_atom_(),
        rank_to_predicate_id_(),
        rank_to_arity_(),
        rank_to_argument_ids_(),
        name(name),
        domain(domain),
        objects(objects),
        initial(initial),
        goal(goal),
        atom_costs(atom_costs)
    {
        const auto& static_predicates = domain->static_predicates;
        formalism::PredicateSet static_predicate_set(static_predicates.begin(), static_predicates.end());

        for (const auto& atom : initial)
        {
            if (static_predicate_set.find(atom->predicate) != static_predicate_set.end())
            {
                static_atoms_.insert(atom);
            }
        }

        predicate_id_to_static_.resize(domain->predicates.size(), false);

        for (const auto& static_predicate : static_predicates)
        {
            predicate_id_to_static_[static_predicate->id] = true;
        }

        std::sort(this->objects.begin(), this->objects.end(), [](const formalism::Object& lhs, const formalism::Object& rhs) { return lhs->id < rhs->id; });
    }

    const formalism::AtomSet& ProblemImpl::get_static_atoms() const { return static_atoms_; }

    uint32_t ProblemImpl::get_rank(const formalism::Atom& atom) const
    {
        auto& rank_with_offset = atom_ranks_[atom];

        if (rank_with_offset == 0)
        {
            // Indexing creates a new item if none exists, thus the size is at least 1.
            rank_with_offset = static_cast<uint32_t>(atom_ranks_.size());
            const auto rank = rank_with_offset - 1;

            assert(rank_to_atom_.size() == rank);
            assert(rank_to_predicate_id_.size() == rank);
            assert(rank_to_arity_.size() == rank);
            assert(rank_to_argument_ids_.size() == rank);

            rank_to_atom_.emplace_back(atom);
            rank_to_predicate_id_.emplace_back(atom->predicate->id);
            rank_to_arity_.emplace_back(atom->predicate->arity);

            std::vector<uint32_t> predicate_ids;
            predicate_ids.reserve(atom->arguments.size());
            std::transform(atom->arguments.cbegin(),
                           atom->arguments.cend(),
                           std::back_insert_iterator(predicate_ids),
                           [](const formalism::Object& object) { return object->id; });
            rank_to_argument_ids_.emplace_back(std::move(predicate_ids));
        }

        return rank_with_offset - 1;
    }

    uint32_t ProblemImpl::num_ranks() const { return static_cast<uint32_t>(atom_ranks_.size()); }

    bool ProblemImpl::is_static(uint32_t rank) const { return predicate_id_to_static_.at(get_predicate_id(rank)); }

    bool ProblemImpl::is_dynamic(uint32_t rank) const { return !is_static(rank); }

    uint32_t ProblemImpl::get_arity(uint32_t rank) const
    {
        assert(rank < rank_to_arity_.size());
        return rank_to_arity_.at(rank);
    }

    uint32_t ProblemImpl::get_predicate_id(uint32_t rank) const
    {
        assert(rank < rank_to_predicate_id_.size());
        return rank_to_predicate_id_.at(rank);
    }

    const std::vector<uint32_t>& ProblemImpl::get_argument_ids(uint32_t rank) const
    {
        assert(rank < rank_to_argument_ids_.size());
        return rank_to_argument_ids_.at(rank);
    }

    formalism::Atom ProblemImpl::get_atom(uint32_t rank) const
    {
        assert(rank < rank_to_atom_.size());
        return rank_to_atom_.at(rank);
    }

    formalism::AtomList ProblemImpl::get_encountered_atoms() const
    {
        formalism::AtomList atoms;

        for (const auto& [key, value] : atom_ranks_)
        {
            atoms.emplace_back(key);
        }

        return atoms;
    }

    formalism::Object ProblemImpl::get_object(uint32_t object_id) const
    {
        assert(object_id < objects.size());
        assert(objects.at(object_id)->id == object_id);
        return objects.at(object_id);
    }

    uint32_t ProblemImpl::num_objects() const { return static_cast<uint32_t>(objects.size()); }

    ProblemDescription create_problem(const std::string& name,
                                      const formalism::DomainDescription& domain,
                                      const formalism::ObjectList& objects,
                                      const formalism::AtomList& initial,
                                      const formalism::LiteralList& goal,
                                      const std::unordered_map<formalism::Atom, double>& atom_costs)
    {
        return std::shared_ptr<formalism::ProblemImpl>(new ProblemImpl(name, domain, objects, initial, goal, atom_costs));
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ProblemDescription& problem)
    {
        os << "Name: " << problem->name << std::endl;
        os << "Objects: " << problem->objects << std::endl;
        os << "Goal: " << problem->goal << std::endl;
        os << "Initial: " << problem->initial << std::endl;
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const formalism::ProblemDescriptionList& problems)
    {
        print_vector(os, problems);
        return os;
    }
}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    std::size_t hash<formalism::ProblemDescription>::operator()(const formalism::ProblemDescription& problem) const
    {
        return hash_combine(problem->name, problem->domain, problem->objects, problem->initial, problem->goal);
    }

    bool less<formalism::ProblemDescription>::operator()(const formalism::ProblemDescription& left_problem,
                                                         const formalism::ProblemDescription& right_problem) const
    {
        return less_combine(std::make_tuple(left_problem->name, left_problem->domain, left_problem->objects, left_problem->initial, left_problem->goal),
                            std::make_tuple(right_problem->name, right_problem->domain, right_problem->objects, right_problem->initial, right_problem->goal));
    }

    bool equal_to<formalism::ProblemDescription>::operator()(const formalism::ProblemDescription& left_problem,
                                                             const formalism::ProblemDescription& right_problem) const
    {
        if (left_problem.get() == right_problem.get())
        {
            return true;
        }

        return equal_to_combine(
            std::make_tuple(left_problem->name, left_problem->domain, left_problem->objects, left_problem->initial, left_problem->goal),
            std::make_tuple(right_problem->name, right_problem->domain, right_problem->objects, right_problem->initial, right_problem->goal));
    }
}  // namespace std
