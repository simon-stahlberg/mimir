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


#include "help_functions.hpp"
#include "problem.hpp"

#include <cassert>
#include <cmath>

namespace formalism
{
    ProblemImpl::ProblemImpl(const std::string& name,
                             const formalism::DomainDescription& domain,
                             const formalism::ObjectList& objects,
                             const formalism::AtomSet& static_atoms,
                             const formalism::State& initial,
                             const formalism::LiteralList& goal) :
        static_atoms_(static_atoms),
        predicate_id_to_offset_(),
        predicate_id_to_predicate_(),
        predicate_id_to_static_(),
        object_id_to_object_(),
        rank_to_arity_(),
        rank_to_predicate_id_(),
        name(name),
        domain(domain),
        objects(objects),
        initial(initial),
        goal(goal)
    {
        // The predicates must be sorted by their id for offsets and indices to match.
        auto predicates = domain->predicates;
        std::sort(predicates.begin(), predicates.end(), [](const formalism::Predicate& lhs, const formalism::Predicate& rhs) { return lhs->id < rhs->id; });

        const auto num_objects = static_cast<uint32_t>(objects.size());
        predicate_id_to_offset_.push_back(0);
        uint32_t predicate_offset = 0;

        for (const auto& predicate : predicates)
        {
            assert(predicate->id == (predicate_id_to_offset_.size() - 1));
            predicate_offset += static_cast<uint32_t>(std::pow(num_objects, predicate->arity));
            predicate_id_to_offset_.push_back(predicate_offset);
        }

        predicate_id_to_predicate_.resize(predicates.size());
        for (const auto& predicate : predicates)
        {
            predicate_id_to_predicate_[predicate->id] = predicate;
        }

        object_id_to_object_.resize(objects.size());
        for (const auto& object : objects)
        {
            object_id_to_object_[object->id] = object;
        }

        predicate_id_to_static_.resize(predicates.size(), false);
        for (const auto& predicate : domain->static_predicates)
        {
            predicate_id_to_static_[predicate->id] = true;
        }

        const auto num_ranks = predicate_id_to_offset_[predicate_id_to_offset_.size() - 1];
        rank_to_arity_.resize(num_ranks);
        rank_to_predicate_id_.resize(num_ranks);

        for (std::size_t index = 0; index < predicates.size(); ++index)
        {
            const auto start_offset = predicate_id_to_offset_[index];
            const auto end_offset = predicate_id_to_offset_[index + 1];
            const auto arity = predicate_id_to_predicate_[index]->arity;
            const auto id = predicate_id_to_predicate_[index]->id;

            assert(id == static_cast<uint32_t>(index));

            std::fill(rank_to_arity_.begin() + start_offset, rank_to_arity_.begin() + end_offset, arity);
            std::fill(rank_to_predicate_id_.begin() + start_offset, rank_to_predicate_id_.begin() + end_offset, id);
        }
    }

    const formalism::AtomSet& ProblemImpl::get_static_atoms() const { return static_atoms_; }

    uint32_t ProblemImpl::num_ranks() const { return predicate_id_to_offset_[predicate_id_to_offset_.size() - 1]; }

    uint32_t ProblemImpl::get_rank(const formalism::Atom& atom) const
    {
        uint32_t rank = predicate_id_to_offset_[atom->predicate->id];

        for (std::size_t argument_index = 0; argument_index < atom->arguments.size(); ++argument_index)
        {
            const auto argument_offset = static_cast<uint32_t>(std::pow(objects.size(), argument_index));
            rank += argument_offset * atom->arguments[argument_index]->id;
        }

        return rank;
    }

    bool ProblemImpl::is_static(uint32_t rank) const
    {
        assert(rank < num_ranks());
        const std::size_t predicate_index = get_predicate_id(rank);
        return predicate_id_to_static_[predicate_index];
    }

    bool ProblemImpl::is_dynamic(uint32_t rank) const { return !is_static(rank); }

    uint32_t ProblemImpl::get_arity(uint32_t rank) const
    {
        assert(rank < num_ranks());
        return rank_to_arity_[rank];
    }

    uint32_t ProblemImpl::get_predicate_id(uint32_t rank) const
    {
        assert(rank < num_ranks());
        return rank_to_predicate_id_[rank];
    }

    std::vector<uint32_t> ProblemImpl::get_argument_ids(uint32_t rank) const
    {
        assert(rank < num_ranks());
        const auto predicate_index = rank_to_predicate_id_[rank];
        const auto arity = rank_to_arity_[rank];
        const auto num_objects = objects.size();
        rank -= predicate_id_to_offset_[predicate_index];

        std::vector<uint32_t> object_ids;

        for (std::size_t argument_index = 0; argument_index < arity; ++argument_index)
        {
            const auto object_id = rank % num_objects;
            rank /= num_objects;
            object_ids.push_back(object_id);
        }

        return object_ids;
    }

    formalism::Atom ProblemImpl::get_atom(uint32_t rank) const
    {
        assert(rank < num_ranks());
        const auto predicate_index = get_predicate_id(rank);
        const auto predicate = predicate_id_to_predicate_[predicate_index];
        const auto arity = predicate->arity;
        const auto num_objects = objects.size();
        rank -= predicate_id_to_offset_[predicate_index];

        formalism::ObjectList arguments;

        for (std::size_t argument_index = 0; argument_index < arity; ++argument_index)
        {
            const auto object_id = rank % num_objects;
            rank /= num_objects;
            arguments.push_back(object_id_to_object_[object_id]);
        }

        return formalism::create_atom(predicate, std::move(arguments));
    }

    uint32_t ProblemImpl::num_objects() const { return static_cast<uint32_t>(objects.size()); }

    ProblemDescription create_problem(const std::string& name,
                                      const formalism::DomainDescription& domain,
                                      const formalism::ObjectList& objects,
                                      const formalism::AtomList& initial,
                                      const formalism::LiteralList& goal)
    {
        formalism::PredicateSet static_predicate_set(domain->static_predicates.begin(), domain->static_predicates.end());
        formalism::AtomSet static_atoms;

        for (const auto& atom : initial)
        {
            if (static_predicate_set.find(atom->predicate) != static_predicate_set.end())
            {
                static_atoms.insert(atom);
            }
        }

        auto problem = std::make_shared<formalism::ProblemImpl>(name, domain, objects, static_atoms, nullptr, goal);
        problem->initial = create_state(initial, problem);
        return problem;
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
        return equal_to_combine(
            std::make_tuple(left_problem->name, left_problem->domain, left_problem->objects, left_problem->initial, left_problem->goal),
            std::make_tuple(right_problem->name, right_problem->domain, right_problem->objects, right_problem->initial, right_problem->goal));
    }
}  // namespace std
