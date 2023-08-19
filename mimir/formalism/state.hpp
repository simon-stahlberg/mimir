#if !defined(FORMALISM_STATE_HPP_)
#define FORMALISM_STATE_HPP_

#include "action.hpp"
#include "action_schema.hpp"
#include "atom.hpp"
#include "bitset.hpp"
#include "literal.hpp"
#include "transition.hpp"

#include <map>
#include <memory>
#include <unordered_set>
#include <vector>

namespace formalism
{
    class StateImpl
    {
      private:
        formalism::Bitset bitset_;
        formalism::ProblemDescription problem_;
        std::size_t hash_;

      public:
        StateImpl();

        StateImpl(formalism::Bitset&& bitset, const formalism::ProblemDescription& problem);

        StateImpl(const std::vector<uint32_t>& atoms, const formalism::ProblemDescription& problem);

        StateImpl(const formalism::AtomList& atoms, const formalism::ProblemDescription& problem);

        StateImpl(const formalism::AtomSet& atoms, const formalism::ProblemDescription& problem);

        inline bool operator<(const StateImpl& other) const;

        inline bool operator==(const StateImpl& other) const;

        inline bool operator!=(const StateImpl& other) const;

        formalism::AtomList get_atoms() const;

        formalism::AtomList get_static_atoms() const;

        formalism::AtomList get_dynamic_atoms() const;

        std::vector<uint32_t> get_ranks() const;

        std::vector<uint32_t> get_static_ranks() const;

        std::vector<uint32_t> get_dynamic_ranks() const;

        formalism::ProblemDescription get_problem() const;

        std::map<formalism::Predicate, formalism::AtomList> get_atoms_grouped_by_predicate() const;

        std::pair<std::map<uint32_t, std::vector<uint32_t>>, std::map<uint32_t, std::pair<std::string, uint32_t>>>
        pack_object_ids_by_predicate_id(bool include_types, bool include_goal) const;

        inline std::size_t hash() const { return hash_; }

        friend bool is_applicable(const formalism::Action& action, const formalism::State& state);

        friend formalism::State apply(const formalism::Action& action, const formalism::State& state);

        friend bool is_in_state(uint32_t rank, const formalism::State& state);

        friend bool is_in_state(const formalism::Atom& atom, const formalism::State& state);

        friend std::ostream& operator<<(std::ostream& os, const formalism::State& state);

        template<typename T>
        friend class std::hash;

        template<typename T>
        friend class std::equal_to;
    };

    // friend functions

    formalism::State apply(const formalism::Action& action, const formalism::State& state);

    bool is_in_state(uint32_t atom, const formalism::State& state);

    bool is_in_state(const formalism::Atom& atom, const formalism::State& state);

    // non-friend functions

    State create_state();

    State create_state(const formalism::AtomList& atoms, formalism::ProblemDescription problem);

    State create_state(const formalism::AtomSet& atoms, formalism::ProblemDescription problem);

    bool is_applicable(const formalism::Action& action, const formalism::State& state);

    bool atoms_hold(const AtomList& atoms, const formalism::State& state);

    bool literal_holds(const formalism::Literal& literal, const formalism::State& state);

    bool literals_hold(const LiteralList& literal_list, const formalism::State& state, std::size_t min_arity = 0);

    bool matches_any_in_state(const formalism::Atom& atom, const formalism::State& state);

    std::ostream& operator<<(std::ostream& os, const formalism::State& state);

    std::ostream& operator<<(std::ostream& os, const formalism::StateList& states);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::State>
    {
        std::size_t operator()(const formalism::State& state) const;
    };

    template<>
    struct less<formalism::State>
    {
        bool operator()(const formalism::State& left_state, const formalism::State& right_state) const;
    };

    template<>
    struct equal_to<formalism::State>
    {
        bool operator()(const formalism::State& left_state, const formalism::State& right_state) const;
    };

}  // namespace std

#endif  // FORMALISM_STATE_HPP_
