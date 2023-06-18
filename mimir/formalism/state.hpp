#if !defined(FORMALISM_STATE_HPP_)
#define FORMALISM_STATE_HPP_

#include "action.hpp"
#include "action_schema.hpp"
#include "atom.hpp"
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
        std::vector<uint32_t> ranks_;
        formalism::ProblemDescription problem_;
        mutable std::size_t hash_;

      public:
        StateImpl();

        StateImpl(const std::vector<uint32_t>& atoms, const formalism::ProblemDescription& problem);

        StateImpl(const formalism::AtomList& atoms, const formalism::ProblemDescription& problem);

        StateImpl(const formalism::AtomSet& atoms, const formalism::ProblemDescription& problem);

        bool operator<(const StateImpl& other) const;

        bool operator==(const StateImpl& other) const;

        bool operator!=(const StateImpl& other) const;

        formalism::AtomList get_atoms() const;

        formalism::AtomList get_static_atoms() const;

        formalism::AtomList get_dynamic_atoms() const;

        std::vector<uint32_t> get_ranks() const;

        std::vector<uint32_t> get_static_ranks() const;

        std::vector<uint32_t> get_dynamic_ranks() const;

        formalism::ProblemDescription get_problem() const;

        std::map<formalism::Predicate, formalism::AtomList> get_atoms_grouped_by_predicate() const;

        std::map<uint32_t, std::vector<uint32_t>> get_atom_argument_ids_grouped_by_predicate_ids() const;

        std::size_t hash() const;

        friend bool is_applicable(const formalism::Action& action, const formalism::State& state);

        friend formalism::State apply(const formalism::Action& action, const formalism::State& state);

        friend bool is_in_state(uint32_t rank, const formalism::State& state);

        friend bool is_in_state(const formalism::Atom& atom, const formalism::State& state);

        friend std::ostream& operator<<(std::ostream& os, const formalism::State& state);

        template<typename T>
        friend class std::hash;
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
