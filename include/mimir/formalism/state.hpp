#ifndef MIMIR_FORMALISM_STATE_HPP_
#define MIMIR_FORMALISM_STATE_HPP_

#include "action.hpp"
#include "atom.hpp"
#include "bitset.hpp"
#include "domain.hpp"
#include "literal.hpp"
#include "problem.hpp"
#include "term.hpp"

#include "../common/mixins.hpp"

#include <map>
#include <memory>
#include <vector>

namespace mimir::formalism
{
    class State;
    using StateList = std::vector<State>;

    class RepositoryImpl;
    using Repository = std::shared_ptr<RepositoryImpl>;
    using StateRepositoryPtr = const RepositoryImpl*;

    // TODO (Dominik): Should the name of repository be "instance"?
    class RepositoryImpl
    {
      private:
        Domain domain_;
        Problem problem_;
        // TODO (Dominik): Use factory from parser, not need to create another one for loki types
        // TermFactory term_factory_;
        // AtomFactory atom_factory_;

        // Declare the copy constructor and copy assignment operator as deleted
        RepositoryImpl(const RepositoryImpl&) = delete;
        RepositoryImpl& operator=(const RepositoryImpl&) = delete;

        // Declare the move constructor and move assignment operator as deleted
        RepositoryImpl(RepositoryImpl&&) = delete;
        RepositoryImpl& operator=(RepositoryImpl&&) = delete;

        RepositoryImpl(const Problem& problem);

      public:
        Atom create_atom(const Predicate& predicate, TermList&& terms);
        Literal create_literal(const Atom& atom, bool is_negated);
        State create_state(const AtomList& atoms);

        Action create_action(const ActionSchema& schema, TermList&& terms, double cost);
        Action create_action(const ActionSchema& schema,
                             TermList&& terms,
                             LiteralList&& precondition,
                             LiteralList&& unconditional_effect,
                             ImplicationList&& conditional_effect,
                             double cost);

        Repository delete_relax();

        Domain get_domain() const;
        Problem get_problem() const;
        // TODO (Dominik): What about switching from integer identifiers to Handle<T> for more type safety? The signature would look cleaner "get_arity(Handle<Atom> id) const" with less opportunities for missuse.
        uint32_t get_arity(uint32_t atom_id) const;
        // TODO (Dominik): Same idea of using Handle<T>
        uint32_t get_predicate_id(uint32_t atom_id) const;
        // TODO (Dominik): Same idea of using Handle<T>
        Term get_object(uint32_t object_id) const;

        std::vector<Atom> get_encountered_atoms() const;  // TODO: Ensure that the initial and goal atoms are added.
        // TODO (Dominik): Same idea of using Handle<T>
        std::vector<uint32_t> get_term_ids(uint32_t atom_id) const;

        friend Repository create_repository(const Problem&);
    };

    Repository create_repository(const Problem& problem);

    // TODO (Dominik): A state should not be copyable, we should use UncopyableMixin<State> to express that
    class State : public FormattingMixin<State>
    {
      private:
        //  TODO: In the future, structure the state into two parts: a 'grounded' section and a 'lifted' section. The grounded segment should use an FDR
        //  representation instead of a bitset. Even in a lifted configuration, identifying certain FDR variables, such as the location of a truck, should be
        //  straightforward through the analysis of action schemas and the initial state. In a grounded configuration, a more in-depth mutex analysis can be
        //  performed to construct FDR variables (leaving the lifted part empty). This should improve performance significantly in some domains.
        // TODO (Dominik): What about providing a GroundState and a LiftedState?
        //                 Then templatize our concepts by a State and provide spezializations for different state representations?
        //                 This provides additional flexibility in optimizing grounded and lifted planning
        //                 separately and adding alternative representations.

        // TODO (Dominik): I recommend using a bitset view, to some persistent preallocated buffer
        Bitset bitset_;
        // Problem problem_;
        std::size_t hash_;

      public:
        State();

        State(Bitset&& bitset);  // , const Problem& problem

        // State(const std::vector<uint32_t>& atoms);  // , const Problem& problem
        // State(const AtomList& atoms);  // , const Problem& problem
        // State(const AtomSet& atoms);  // , const Problem& problem

        uint32_t get_id() const;
        AtomList get_atoms() const;
        AtomList get_static_atoms() const;
        AtomList get_dynamic_atoms() const;
        Repository get_repository() const;

        bool contains(uint32_t atom_id) const;
        bool contains(const Atom& atom) const;
        bool contains_all(const AtomList& atoms) const;
        bool holds(const Literal& literal) const;
        bool holds(const LiteralList& literals) const;
        bool holds(const LiteralList& literals, uint32_t min_arity) const;

        std::vector<uint32_t> get_ranks() const;
        std::vector<uint32_t> get_static_ranks() const;
        std::vector<uint32_t> get_dynamic_ranks() const;

        bool is_applicable(const Action& action) const;
        State apply(const Action& state) const;

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
