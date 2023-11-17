#ifndef MIMIR_FORMALISM_STATE_HPP_
#define MIMIR_FORMALISM_STATE_HPP_

#include "action.hpp"
#include "action_schema.hpp"
#include "atom.hpp"
#include "bitset.hpp"
#include "literal.hpp"
#include "transition.hpp"

#include <map>
#include <memory>
#include <vector>

namespace mimir::formalism
{
    class StateImpl
    {
      private:
        //  TODO: In the future, structure the state into two parts: a 'grounded' section and a 'lifted' section. The grounded segment should use an FDR
        //  representation instead of a bitset. Even in a lifted configuration, identifying certain FDR variables, such as the location of a truck, should be
        //  straightforward through the analysis of action schemas and the initial state. In a grounded configuration, a more in-depth mutex analysis can be
        //  performed to construct FDR variables (leaving the lifted part empty). This should improve performance significantly in some domains.

        mimir::formalism::Bitset bitset_;
        mimir::formalism::ProblemDescription problem_;
        std::size_t hash_;

      public:
        StateImpl();

        StateImpl(mimir::formalism::Bitset&& bitset, const mimir::formalism::ProblemDescription& problem);

        StateImpl(const std::vector<uint32_t>& atoms, const mimir::formalism::ProblemDescription& problem);

        StateImpl(const mimir::formalism::AtomList& atoms, const mimir::formalism::ProblemDescription& problem);

        StateImpl(const mimir::formalism::AtomSet& atoms, const mimir::formalism::ProblemDescription& problem);

        bool operator<(const StateImpl& other) const;

        bool operator==(const StateImpl& other) const;

        bool operator!=(const StateImpl& other) const;

        mimir::formalism::AtomList get_atoms() const;

        mimir::formalism::AtomList get_static_atoms() const;

        mimir::formalism::AtomList get_dynamic_atoms() const;

        std::vector<uint32_t> get_ranks() const;

        std::vector<uint32_t> get_static_ranks() const;

        std::vector<uint32_t> get_dynamic_ranks() const;

        mimir::formalism::ProblemDescription get_problem() const;

        std::map<mimir::formalism::Predicate, mimir::formalism::AtomList> get_atoms_grouped_by_predicate() const;

        std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
        pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const;

        std::size_t hash() const;

        friend bool is_applicable(const mimir::formalism::Action& action, const mimir::formalism::State& state);

        friend mimir::formalism::State apply(const mimir::formalism::Action& action, const mimir::formalism::State& state);

        friend bool is_in_state(uint32_t rank, const mimir::formalism::State& state);

        friend bool is_in_state(const mimir::formalism::Atom& atom, const mimir::formalism::State& state);

        friend std::ostream& operator<<(std::ostream& os, const mimir::formalism::State& state);

        template<typename T>
        friend class std::hash;

        template<typename T>
        friend class std::equal_to;
    };

    // friend functions

    mimir::formalism::State apply(const mimir::formalism::Action& action, const mimir::formalism::State& state);

    bool is_in_state(uint32_t rank, const mimir::formalism::State& state);

    bool is_in_state(const mimir::formalism::Atom& atom, const mimir::formalism::State& state);

    bool subset_of_state(const std::vector<uint32_t>& ranks, const mimir::formalism::State& state);

    bool subset_of_state(const mimir::formalism::AtomList& atoms, const mimir::formalism::State& state);

    // non-friend functions

    State create_state();

    State create_state(const mimir::formalism::AtomList& atoms, mimir::formalism::ProblemDescription problem);

    State create_state(const mimir::formalism::AtomSet& atoms, mimir::formalism::ProblemDescription problem);

    bool is_applicable(const mimir::formalism::Action& action, const mimir::formalism::State& state);

    bool atoms_hold(const AtomList& atoms, const mimir::formalism::State& state);

    bool literal_holds(const mimir::formalism::Literal& literal, const mimir::formalism::State& state);

    bool literals_hold(const LiteralList& literal_list, const mimir::formalism::State& state, std::size_t min_arity = 0);

    bool matches_any_in_state(const mimir::formalism::Atom& atom, const mimir::formalism::State& state);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::State& state);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::StateList& states);

}  // namespace formalism

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
