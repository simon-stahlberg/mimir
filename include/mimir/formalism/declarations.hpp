#ifndef MIMIR_FORMALISM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DECLARATIONS_HPP_

#include "../datastructures/robin_map.hpp"
#include "../datastructures/robin_set.hpp"

#include <loki/domain/pddl/action.hpp>
#include <vector>

namespace mimir::formalism
{
    class ActionSchema;
    using ActionSchemaList = std::vector<ActionSchema>;

    class Action;
    using ActionList = std::vector<Action>;

    class Atom;
    using AtomList = std::vector<Atom>;
    using AtomSet = std::unordered_set<Atom>;

    class Domain;
    using DomainList = std::vector<Domain>;

    class Function;
    using FunctionList = std::vector<Function>;

    class Implication;
    using ImplicationList = std::vector<Implication>;

    class Literal;
    using LiteralList = std::vector<Literal>;

    class Predicate;
    using PredicateList = std::vector<Predicate>;
    using PredicateSet = mimir::tsl::robin_set<Predicate>;

    class Problem;
    using ProblemList = std::vector<Problem>;

    class State;
    using StateList = std::vector<State>;

    class StateRepositoryImpl;
    using StateRepository = std::shared_ptr<StateRepositoryImpl>;
    using StateRepositoryPtr = const StateRepositoryImpl*;

    class Term;
    using TermList = std::vector<Term>;

    class Transition;
    using TransitionList = std::vector<Transition>;

    class Type;
    using TypeList = std::vector<Type>;

    // using Parameter = Object;
    // using ParameterList = ObjectList;
    using ParameterAssignment = mimir::tsl::robin_map<Term, Term>;

    // class TransitionImpl;
    // using Transition = std::shared_ptr<TransitionImpl>;
    // using TransitionList = std::vector<Transition>;
}  // namespace mimir::formalism

#endif  // MIMIR_FORMALISM_DECLARATIONS_HPP_
