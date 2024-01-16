#ifndef MIMIR_FORMALISM_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_HPP_

#include "atom.hpp"
#include "domain.hpp"
#include "literal.hpp"
#include "term.hpp"

#include "../common/mixins.hpp"

#include <loki/problem/pddl/problem.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir
{
    class Problem;
    using ProblemList = std::vector<Problem>;

    class Problem : public FormattingMixin<Problem>
    {
      private:
        loki::pddl::Problem external_;
        // AtomSet static_atoms_;
        // std::vector<bool> predicate_id_to_static_;

        Problem(loki::pddl::Problem problem);

      public:
        static Problem parse(const std::string& domain_path, const std::string& problem_path);
        static ProblemList parse(const std::string& domain_path, const std::vector<std::string>& problem_paths);
        // Problem replace_initial(const AtomList& initial) const;

        uint32_t get_id() const;
        Domain get_domain() const;
        const std::string& get_name() const;
        TermList get_objects() const;
        AtomList get_initial_atoms() const;
        LiteralList get_goal_literals() const;
        // const Requirements& get_requirements() const;
        const AtomSet& get_static_atoms() const;
        // uint32_t get_rank(const Atom& atom) const;

        std::size_t hash() const;

        // std::vector<uint32_t> to_ranks(const AtomList& atoms) const;
        // uint32_t num_ranks() const;
        // bool is_static(uint32_t rank) const;
        // bool is_dynamic(uint32_t rank) const;
        // uint32_t get_arity(uint32_t rank) const;
        // uint32_t get_predicate_id(uint32_t rank) const;
        // Predicate get_predicate(uint32_t rank) const;
        // const std::vector<uint32_t>& get_argument_ids(uint32_t rank) const;
        // Atom get_atom(uint32_t rank) const;
        // AtomList get_encountered_atoms() const;
        // uint32_t num_encountered_atoms() const;
        // Object get_object(uint32_t object_id) const;
        // uint32_t num_objects() const;

        bool operator<(const Problem& other) const;
        bool operator>(const Problem& other) const;
        bool operator==(const Problem& other) const;
        bool operator!=(const Problem& other) const;
        bool operator<=(const Problem& other) const;
    };

}  // namespace mimir

/*
namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::Problem>
    {
        std::size_t operator()(const mimir::Problem& problem) const;
    };

    template<>
    struct less<mimir::Problem>
    {
        bool operator()(const mimir::Problem& left_problem, const mimir::Problem& right_problem) const;
    };

    template<>
    struct equal_to<mimir::Problem>
    {
        bool operator()(const mimir::Problem& left_problem, const mimir::Problem& right_problem) const;
    };

}  // namespace std
*/

#endif  // MIMIR_FORMALISM_PROBLEM_HPP_
