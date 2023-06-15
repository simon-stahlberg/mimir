#if !defined(FORMALISM_PROBLEM_HPP_)
#define FORMALISM_PROBLEM_HPP_

#include "action_schema.hpp"
#include "atom.hpp"
#include "domain.hpp"
#include "object.hpp"
#include "predicate.hpp"
#include "state.hpp"
#include "type.hpp"

#include <memory>
#include <string>
#include <vector>

namespace formalism
{
    class ProblemImpl
    {
      private:
        formalism::AtomSet static_atoms_;
        std::vector<uint32_t> predicate_id_to_offset_;
        std::vector<formalism::Predicate> predicate_id_to_predicate_;
        std::vector<bool> predicate_id_to_static_;
        std::vector<formalism::Object> object_id_to_object_;
        std::vector<uint32_t> rank_to_arity_;
        std::vector<uint32_t> rank_to_predicate_id_;

      public:
        std::string name;
        formalism::DomainDescription domain;
        formalism::ObjectList objects;
        formalism::State initial;
        formalism::LiteralList goal;

        ProblemImpl(const std::string& name,
                    const formalism::DomainDescription& domain,
                    const formalism::ObjectList& objects,
                    const formalism::AtomSet& static_atoms,
                    const formalism::State& initial,
                    const formalism::LiteralList& goal);

        const formalism::AtomSet& get_static_atoms() const;

        uint32_t num_ranks() const;

        uint32_t get_rank(const formalism::Atom& atom) const;

        bool is_static(uint32_t rank) const;

        bool is_dynamic(uint32_t rank) const;

        uint32_t get_arity(uint32_t rank) const;

        uint32_t get_predicate_id(uint32_t rank) const;

        formalism::Predicate get_predicate(uint32_t rank) const;

        std::vector<uint32_t> get_argument_ids(uint32_t rank) const;

        formalism::Atom get_atom(uint32_t rank) const;

        uint32_t num_objects() const;

        friend ProblemDescription create_problem(const std::string& name,
                                                 const formalism::DomainDescription& domain,
                                                 const formalism::ObjectList& objects,
                                                 const formalism::AtomList& initial,
                                                 const formalism::LiteralList& goal);
    };

    ProblemDescription create_problem(const std::string& name,
                                      const formalism::DomainDescription& domain,
                                      const formalism::ObjectList& objects,
                                      const formalism::AtomList& initial,
                                      const formalism::LiteralList& goal);

    std::ostream& operator<<(std::ostream& os, const formalism::ProblemDescription& problem);

    std::ostream& operator<<(std::ostream& os, const formalism::ProblemDescriptionList& problems);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::ProblemDescription>
    {
        std::size_t operator()(const formalism::ProblemDescription& problem) const;
    };

    template<>
    struct less<formalism::ProblemDescription>
    {
        bool operator()(const formalism::ProblemDescription& left_problem, const formalism::ProblemDescription& right_problem) const;
    };

    template<>
    struct equal_to<formalism::ProblemDescription>
    {
        bool operator()(const formalism::ProblemDescription& left_problem, const formalism::ProblemDescription& right_problem) const;
    };

}  // namespace std

#endif  // FORMALISM_PROBLEM_HPP_
