#ifndef MIMIR_FORMALISM_STATE_HPP_
#define MIMIR_FORMALISM_STATE_HPP_

// #include "atom.hpp"
#include "bitset.hpp"
#include "declarations.hpp"

#include <map>
#include <memory>
#include <vector>

namespace mimir::formalism
{
    class State
    {
      private:
        //  TODO: In the future, structure the state into two parts: a 'grounded' section and a 'lifted' section. The grounded segment should use an FDR
        //  representation instead of a bitset. Even in a lifted configuration, identifying certain FDR variables, such as the location of a truck, should be
        //  straightforward through the analysis of action schemas and the initial state. In a grounded configuration, a more in-depth mutex analysis can be
        //  performed to construct FDR variables (leaving the lifted part empty). This should improve performance significantly in some domains.

        Bitset bitset_;
        // Problem problem_;
        std::size_t hash_;

      public:
        State();

        State(Bitset&& bitset);  // , const Problem& problem

        // State(const std::vector<uint32_t>& atoms);  // , const Problem& problem
        // State(const AtomList& atoms);  // , const Problem& problem
        // State(const AtomSet& atoms);  // , const Problem& problem

        AtomList get_atoms() const;
        AtomList get_static_atoms() const;
        AtomList get_dynamic_atoms() const;

        bool contains(uint32_t atom_id) const;
        bool contains(const Atom& atom) const;
        bool contains_all(const AtomList& atoms) const;
        bool holds(const LiteralList& literals) const;

        std::vector<uint32_t> get_ranks() const;
        std::vector<uint32_t> get_static_ranks() const;
        std::vector<uint32_t> get_dynamic_ranks() const;

        Problem get_problem() const;

        std::map<Predicate, AtomList> get_atoms_grouped_by_predicate() const;

        std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
        pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const;

        std::size_t hash() const;

        // friend bool is_applicable(const Action& action, const State& state);

        // friend State apply(const Action& action, const State& state);

        // friend bool contains(uint32_t rank, const State& state);

        // friend bool contains(const Atom& atom, const State& state);

        bool operator<(const State& other) const;
        bool operator>(const State& other) const;
        bool operator==(const State& other) const;
        bool operator!=(const State& other) const;
        bool operator<=(const State& other) const;

        friend std::ostream& operator<<(std::ostream& os, const State& state);

        template<typename T>
        friend class std::hash;

        template<typename T>
        friend class std::equal_to;
    };

    // // friend functions

    // State apply(const Action& action, const State& state);

    // bool contains(uint32_t rank, const State& state);

    // bool contains(const Atom& atom, const State& state);

    // bool subset_of_state(const std::vector<uint32_t>& ranks, const State& state);

    // bool subset_of_state(const AtomList& atoms, const State& state);

    // // non-friend functions

    // State create_state();

    // State create_state(const AtomList& atoms, Problem problem);

    // State create_state(const AtomSet& atoms, Problem problem);

    // bool is_applicable(const Action& action, const State& state);

    // bool atoms_hold(const AtomList& atoms, const State& state);

    // bool literal_holds(const Literal& literal, const State& state);

    // bool literals_hold(const LiteralList& literal_list, const State& state, std::size_t min_arity = 0);

    // bool matches_any_in_state(const Atom& atom, const State& state);

    std::ostream& operator<<(std::ostream& os, const State& state);

    std::ostream& operator<<(std::ostream& os, const StateList& states);

}  // namespace mimir::formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::State>
    {
        std::size_t operator()(const mimir::formalism::State& state) const;
    };

    template<>
    struct less<mimir::formalism::State>
    {
        bool operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const;
    };

    template<>
    struct equal_to<mimir::formalism::State>
    {
        bool operator()(const mimir::formalism::State& left_state, const mimir::formalism::State& right_state) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_STATE_HPP_
