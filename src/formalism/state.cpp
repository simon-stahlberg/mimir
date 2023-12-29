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
    /*
     * RepositoryImpl Member Functions
     */

    RepositoryImpl::~RepositoryImpl() {}

    Atom RepositoryImpl::create_atom(const Predicate& predicate, TermList&& terms) { throw std::runtime_error("not implemented"); }

    Literal RepositoryImpl::create_literal(const Atom& atom, bool is_negated) { throw std::runtime_error("not implemented"); }

    State RepositoryImpl::create_state(const AtomList& atoms) { throw std::runtime_error("not implemented"); }

    Action RepositoryImpl::create_action(const ActionSchema& schema, TermList&& terms, double cost) { throw std::runtime_error("not implemented"); }

    Action RepositoryImpl::create_action(const ActionSchema& schema,
                                         TermList&& terms,
                                         LiteralList&& precondition,
                                         LiteralList&& unconditional_effect,
                                         ImplicationList&& conditional_effect,
                                         double cost)
    {
        throw std::runtime_error("not implemented");
    }

    Repository RepositoryImpl::delete_relax() { throw std::runtime_error("not implemented"); }

    Domain RepositoryImpl::get_domain() const { throw std::runtime_error("not implemented"); }

    Problem RepositoryImpl::get_problem() const { throw std::runtime_error("not implemented"); }

    uint32_t RepositoryImpl::get_arity(uint32_t atom_id) const { throw std::runtime_error("not implemented"); }

    uint32_t RepositoryImpl::get_predicate_id(uint32_t atom_id) const { throw std::runtime_error("not implemented"); }

    Term RepositoryImpl::get_object(uint32_t object_id) const { throw std::runtime_error("not implemented"); }

    std::vector<Atom> RepositoryImpl::get_encountered_atoms() const
    {
        // TODO: Ensure that the initial and goal atoms are added.
        throw std::runtime_error("not implemented");
    }

    std::vector<uint32_t> RepositoryImpl::get_term_ids(uint32_t atom_id) const { throw std::runtime_error("not implemented"); }

    /*
     * RepositoryImpl Namespace Functions
     */

    Repository create_repository(const Problem& problem) { throw std::runtime_error("not implemented"); }

    /*
     * State Member Functions
     */

    inline static std::size_t compute_state_hash(const Bitset& bitset)  // , const Problem& problem
    {
        // TODO: Add problem to hash
        return std::hash<Bitset>()(bitset);
    }

    State::State() : bitset_(0), hash_(0) { throw std::runtime_error("not implemented"); }

    State::State(Bitset&& bitset) : bitset_(std::move(bitset)), hash_(compute_state_hash(bitset_)) {}  // , const Problem& problem  // problem_(problem),

    uint32_t State::get_id() const { throw std::runtime_error("not implemented"); }

    // State::State(const std::vector<uint32_t>& ranks) :
    //     // , const Problem& problem
    //     bitset_(*std::max_element(ranks.cbegin(), ranks.cend()) + 1),
    //     // problem_(problem),
    //     hash_(0)
    // {
    //     for (auto rank : ranks)
    //     {
    //         bitset_.set(rank);
    //     }

    //     hash_ = compute_state_hash(bitset_);
    // }

    // State::State() : bitset_(0), problem_(nullptr), hash_(0) {}

    // State::State(const AtomList& atoms) : bitset_(problem->num_ranks()), hash_(0)  // , const Problem& problem  // problem_(problem),
    // {
    //     for (const auto& atom : atoms)
    //     {
    //         const auto rank = problem->get_rank(atom);
    //         bitset_.set(rank);
    //     }

    //     hash_ = compute_state_hash(bitset_);
    // }

    // State::State(const AtomSet& atoms) : bitset_(0), hash_(0)  // , const Problem& problem  // problem_(problem),
    // {
    //     for (const auto& atom : atoms)
    //     {
    //         const auto rank = problem->get_rank(atom);
    //         bitset_.set(rank);
    //     }

    //     hash_ = compute_state_hash(bitset_);
    // }

    AtomList State::get_atoms() const
    {
        throw std::runtime_error("not implemented");

        // AtomList atoms;
        // std::size_t position = 0;

        // while (true)
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);
        //     atoms.emplace_back(problem_.get_atom(rank));
        //     ++position;
        // }

        // return atoms;
    }

    AtomList State::get_static_atoms() const
    {
        throw std::runtime_error("not implemented");

        // AtomList atoms;
        // const auto& static_atoms = problem_.get_static_atoms();
        // std::size_t position = 0;

        // while (position < static_atoms.size())
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);
        //     assert(problem_.is_static(rank));
        //     atoms.emplace_back(problem_.get_atom(rank));
        //     ++position;
        // }

        // return atoms;
    }

    AtomList State::get_dynamic_atoms() const
    {
        throw std::runtime_error("not implemented");

        // AtomList atoms;
        // const auto& static_atoms = problem_.get_static_atoms();
        // std::size_t position = static_atoms.size();

        // while (true)
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);
        //     assert(problem_.is_dynamic(rank));
        //     atoms.emplace_back(problem_.get_atom(rank));
        //     ++position;
        // }

        // return atoms;
    }

    Repository State::get_repository() const { throw std::invalid_argument("not implemented"); }

    bool State::contains(uint32_t atom_id) const { throw std::invalid_argument("not implemented"); }

    bool State::contains(const Atom& atom) const { throw std::invalid_argument("not implemented"); }

    bool State::contains_all(const AtomList& atoms) const { throw std::invalid_argument("not implemented"); }

    bool State::holds(const Literal& literal) const { throw std::runtime_error("not implemented"); }

    bool State::holds(const LiteralList& literals) const { throw std::runtime_error("not implemented"); }

    bool State::holds(const LiteralList& literals, uint32_t min_arity) const { throw std::runtime_error("not implemented"); }

    std::vector<uint32_t> State::get_ranks() const
    {
        throw std::runtime_error("not implemented");

        // std::vector<uint32_t> ranks;
        // std::size_t position = 0;

        // while (true)
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);
        //     ranks.emplace_back(rank);
        //     ++position;
        // }

        // return ranks;
    }

    std::vector<uint32_t> State::get_static_ranks() const
    {
        throw std::runtime_error("not implemented");

        // std::vector<uint32_t> static_ranks;
        // std::size_t position = 0;

        // while (true)
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);

        //     if (problem_.is_static(rank))
        //     {
        //         static_ranks.emplace_back(rank);
        //     }

        //     ++position;
        // }

        // return static_ranks;
    }

    std::vector<uint32_t> State::get_dynamic_ranks() const
    {
        throw std::runtime_error("not implemented");

        // std::vector<uint32_t> static_ranks;
        // std::size_t position = 0;

        // while (true)
        // {
        //     position = bitset_.next_set_bit(position);

        //     if (position == bitset_.no_position)
        //     {
        //         break;
        //     }

        //     const auto rank = static_cast<uint32_t>(position);

        //     if (problem_.is_dynamic(rank))
        //     {
        //         static_ranks.emplace_back(rank);
        //     }

        //     ++position;
        // }

        // return static_ranks;
    }

    bool State::is_applicable(const Action& action) const { throw std::runtime_error("not implemented"); }

    State State::apply(const Action& action) const { throw std::runtime_error("not implemented"); }

    Problem State::get_problem() const { throw std::runtime_error("not implemented"); }

    std::map<Predicate, AtomList> State::get_atoms_grouped_by_predicate() const
    {
        throw std::runtime_error("not implemented");

        // std::map<Predicate, AtomList> grouped_atoms;

        // for (const auto& atom : get_atoms())
        // {
        //     const auto& predicate = atom.get_predicate();

        //     if (grouped_atoms.find(predicate) == grouped_atoms.end())
        //     {
        //         grouped_atoms.insert(std::make_pair(predicate, AtomList()));
        //     }

        //     auto& atom_list = grouped_atoms.at(predicate);
        //     atom_list.push_back(atom);
        // }

        // return grouped_atoms;
    }

    std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
    State::pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const
    {
        throw std::runtime_error("not implemented");

        // std::map<uint32_t, std::vector<uint32_t>> packed_ids;
        // std::map<uint32_t, std::pair<std::string, uint32_t>> id_to_name_arity;

        // for (const auto& atom : get_atoms())
        // {
        //     const auto& predicate = atom.get_predicate();

        //     if (packed_ids.find(predicate->id) == packed_ids.end())
        //     {
        //         packed_ids.insert(std::make_pair(predicate->id, std::vector<uint32_t>()));
        //         id_to_name_arity.insert(std::make_pair(predicate->id, std::make_pair(predicate->name, predicate.get_arity())));
        //     }

        //     auto& atom_list = packed_ids.at(predicate->id);

        //     for (const auto& object : atom.get_terms())
        //     {
        //         atom_list.push_back(object->id);
        //     }
        // }

        // auto num_predicates = problem_.domain->predicates.size();

        // if (include_types)
        // {
        //     std::map<Type, uint32_t> type_ids;
        //     const auto& types = problem_.domain->types;

        //     for (const auto& type : types)
        //     {
        //         type_ids.emplace(type, static_cast<uint32_t>(num_predicates + type_ids.size()));
        //     }

        //     for (const auto& object : problem_.objects)
        //     {
        //         const auto object_id = object->id;
        //         auto type = object->type;

        //         while (type != nullptr)
        //         {
        //             const auto predicate_id = type_ids[type];

        //             if (packed_ids.find(predicate_id) == packed_ids.end())
        //             {
        //                 packed_ids.insert(std::make_pair(predicate_id, std::vector<uint32_t>()));
        //                 id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(type->name + "_type", 1)));
        //             }

        //             auto& atom_list = packed_ids.at(predicate_id);
        //             atom_list.push_back(object_id);
        //             type = type->base;
        //         }
        //     }

        //     num_predicates += types.size();
        // }

        // if (include_goal)
        // {
        //     for (const auto& literal : problem_.goal)
        //     {
        //         if (literal->negated)
        //         {
        //             throw std::invalid_argument("negated literal in the goal");
        //         }

        //         const auto atom = literal->atom;
        //         const auto predicate_id = static_cast<uint32_t>(num_predicates) + atom.get_predicate()->id;

        //         if (packed_ids.find(predicate_id) == packed_ids.end())
        //         {
        //             packed_ids.emplace(predicate_id, std::vector<uint32_t>());
        //             id_to_name_arity.insert(std::make_pair(predicate_id, std::make_pair(atom.get_predicate()->name + "_goal", atom.get_predicate()->arity)));
        //         }

        //         auto& atom_list = packed_ids.at(predicate_id);

        //         for (const auto& object : atom.get_terms())
        //         {
        //             atom_list.push_back(object->id);
        //         }
        //     }
        // }

        // return std::make_pair(packed_ids, id_to_name_arity);
    }

    // State create_state() { return std::make_shared<State>(); }

    // State create_state(const AtomList& atoms, Problem problem) { return std::make_shared<State>(atoms, problem); }

    // State create_state(const AtomSet& atoms, Problem problem) { return std::make_shared<State>(atoms, problem); }

    std::size_t State::hash() const { return hash_; }

    bool State::operator<(const State& other) const
    {
        // if (problem_ != other.problem_)
        // {
        //     return problem_ < other.problem_;
        // }

        return bitset_ < other.bitset_;
    }

    bool State::operator==(const State& other) const
    {
        if (hash_ != other.hash_)
        {
            return false;
        }

        // if (problem_.get() != other.problem_.get())
        // {
        //     return false;
        // }

        return bitset_ == other.bitset_;
    }

    bool State::operator>(const State& other) const { return *this > other; }

    bool State::operator!=(const State& other) const { return !(*this == other); }

    bool State::operator<=(const State& other) const { return (*this < other) || (*this == other); }

    // bool contains(uint32_t rank, const State& state)
    // {
    //     throw std::runtime_error("not implemented");
    //     //  return state->bitset_.get(rank);
    // }

    // bool contains(const Atom& atom, const State& state)
    // {
    //     throw std::runtime_error("not implemented");
    //     // return contains(state->get_problem()->get_rank(atom), state);
    // }

    // bool subset_of_state(const std::vector<uint32_t>& ranks, const State& state)
    // {
    //     throw std::runtime_error("not implemented");

    //     // for (auto rank : ranks)
    //     // {
    //     //     if (!contains(rank, state))
    //     //     {
    //     //         return false;
    //     //     }
    //     // }

    //     // return true;
    // }

    // bool subset_of_state(const AtomList& atoms, const State& state)
    // {
    //     for (const auto& atom : atoms)
    //     {
    //         if (!contains(atom, state))
    //         {
    //             return false;
    //         }
    //     }

    //     return true;
    // }

    // bool is_applicable(const Action& action, const State& state)
    // {
    //     if (static_cast<int32_t>(action->get_arguments().size()) != action->schema->arity)
    //     {
    //         throw std::runtime_error("is_applicable: action is not ground");
    //     }

    //     auto bitset = state->bitset_;
    //     bitset |= action->applicability_positive_precondition_bitset_;
    //     bitset &= action->applicability_negative_precondition_bitset_;
    //     return state->bitset_ == bitset;
    // }

    // State apply(const Action& action, const State& state)
    // {
    //     // We first apply the delete lists, followed by the add lists to prevent actions from simultaneously negating and establishing the same condition.

    //     auto bitset = state->bitset_;
    //     const auto num_conditional_effects = action->conditional_positive_precondition_bitsets_.size();

    //     if (num_conditional_effects > 0)
    //     {
    //         std::vector<std::size_t> applicable_conditional_effects;

    //         for (std::size_t index = 0; index < num_conditional_effects; ++index)
    //         {
    //             if ((bitset == (bitset | action->conditional_positive_precondition_bitsets_[index])) &&  // positive preconditions
    //                 (bitset == (bitset & action->conditional_negative_precondition_bitsets_[index])))    // negative preconditions
    //             {
    //                 applicable_conditional_effects.emplace_back(index);
    //             }
    //         }

    //         // Apply the delete lists

    //         bitset &= action->unconditional_negative_effect_bitset_;

    //         for (const auto& index : applicable_conditional_effects)
    //         {
    //             bitset &= action->conditional_negative_effect_bitsets_[index];
    //         }

    //         // Apply the add lists

    //         bitset |= action->unconditional_positive_effect_bitset_;

    //         for (const auto& index : applicable_conditional_effects)
    //         {
    //             bitset |= action->conditional_positive_effect_bitsets_[index];
    //         }
    //     }
    //     else
    //     {
    //         // Apply the delete list, then the add list

    //         bitset &= action->unconditional_negative_effect_bitset_;
    //         bitset |= action->unconditional_positive_effect_bitset_;
    //     }

    //     return std::make_shared<State>(std::move(bitset), state->problem_);
    // }

    // bool atoms_hold(const AtomList& atoms, const State& state)
    // {
    //     for (const auto& atom : atoms)
    //     {
    //         if (!contains(atom, state))
    //         {
    //             return false;
    //         }
    //     }

    //     return true;
    // }

    // bool literal_holds(const Literal& literal, const State& state) { return contains(literal->atom, state) != literal->negated; }

    // bool literals_hold(const LiteralList& literal_list, const State& state, std::size_t min_arity)
    // {
    //     for (const auto& literal : literal_list)
    //     {
    //         const auto predicate_arity = literal->atom.get_predicate()->arity;

    //         if ((predicate_arity >= min_arity) && !literal_holds(literal, state))
    //         {
    //             return false;
    //         }
    //     }

    //     return true;
    // }

    // bool matches_any_in_state(const Atom& atom, const State& state)
    // {
    //     for (const auto& state_atom : state->get_atoms())
    //     {
    //         if (matches(atom, state_atom))
    //         {
    //             return true;
    //         }
    //     }

    //     return false;
    // }

    std::ostream& operator<<(std::ostream& os, const State& state)
    {
        throw std::runtime_error("not implemented");
        // os << state.get_atoms();
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const StateList& states)
    {
        print_vector(os, states);
        return os;
    }
}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructu1
    std::size_t hash<mimir::formalism::State>::operator()(const mimir::formalism::State& state) const { return state.hash(); }

    bool less<mimir::formalism::State>::operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const
    {
        return left_state < right_state;
    }

    bool equal_to<mimir::formalism::State>::operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const
    {
        return left_state == right_state;
    }
}  // namespace std
