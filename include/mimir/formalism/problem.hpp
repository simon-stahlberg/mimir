#ifndef MIMIR_FORMALISM_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_HPP_

#include "../datastructures/robin_map.hpp"
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

// Older versions of LibC++ does not have filesystem (e.g., ubuntu 18.04), use the experimental version
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace mimir::formalism
{
    class ProblemImpl
    {
      private:
        mimir::formalism::AtomSet static_atoms_;
        std::vector<bool> predicate_id_to_static_;
        fs::path path_;
        mutable mimir::tsl::robin_map<mimir::formalism::Atom, uint32_t> atom_ranks_;
        mutable mimir::formalism::AtomList rank_to_atom_;
        mutable std::vector<uint32_t> rank_to_predicate_id_;
        mutable std::vector<uint32_t> rank_to_arity_;
        mutable std::vector<std::vector<uint32_t>> rank_to_argument_ids_;

        ProblemImpl(const std::string& name,
                    const mimir::formalism::DomainDescription& domain,
                    const mimir::formalism::ObjectList& objects,
                    const mimir::formalism::AtomList& initial,
                    const mimir::formalism::LiteralList& goal,
                    const std::unordered_map<mimir::formalism::Atom, double>& atom_costs);

      public:
        std::string name;
        mimir::formalism::DomainDescription domain;
        mimir::formalism::ObjectList objects;
        mimir::formalism::AtomList initial;
        mimir::formalism::LiteralList goal;
        std::unordered_map<mimir::formalism::Atom, double> atom_costs;

        mimir::formalism::ProblemDescription replace_initial(const mimir::formalism::AtomList& initial) const;

        const mimir::formalism::AtomSet& get_static_atoms() const;

        void set_path(const fs::path& path);

        fs::path get_path() const;

        uint32_t get_rank(const mimir::formalism::Atom& atom) const;

        std::vector<uint32_t> to_ranks(const mimir::formalism::AtomList& atoms) const;

        uint32_t num_ranks() const;

        bool is_static(uint32_t rank) const;

        bool is_dynamic(uint32_t rank) const;

        uint32_t get_arity(uint32_t rank) const;

        uint32_t get_predicate_id(uint32_t rank) const;

        mimir::formalism::Predicate get_predicate(uint32_t rank) const;

        const std::vector<uint32_t>& get_argument_ids(uint32_t rank) const;

        mimir::formalism::Atom get_atom(uint32_t rank) const;

        mimir::formalism::AtomList get_encountered_atoms() const;

        uint32_t num_encountered_atoms() const;

        mimir::formalism::Object get_object(uint32_t object_id) const;

        uint32_t num_objects() const;

        friend ProblemDescription create_problem(const std::string& name,
                                                 const mimir::formalism::DomainDescription& domain,
                                                 const mimir::formalism::ObjectList& objects,
                                                 const mimir::formalism::AtomList& initial,
                                                 const mimir::formalism::LiteralList& goal,
                                                 const std::unordered_map<mimir::formalism::Atom, double>& atom_costs);
    };

    ProblemDescription create_problem(const std::string& name,
                                      const mimir::formalism::DomainDescription& domain,
                                      const mimir::formalism::ObjectList& objects,
                                      const mimir::formalism::AtomList& initial,
                                      const mimir::formalism::LiteralList& goal,
                                      const std::unordered_map<mimir::formalism::Atom, double>& atom_costs);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ProblemDescription& problem);

    std::ostream& operator<<(std::ostream& os, const mimir::formalism::ProblemDescriptionList& problems);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::ProblemDescription>
    {
        std::size_t operator()(const mimir::formalism::ProblemDescription& problem) const;
    };

    template<>
    struct less<mimir::formalism::ProblemDescription>
    {
        bool operator()(const mimir::formalism::ProblemDescription& left_problem, const mimir::formalism::ProblemDescription& right_problem) const;
    };

    template<>
    struct equal_to<mimir::formalism::ProblemDescription>
    {
        bool operator()(const mimir::formalism::ProblemDescription& left_problem, const mimir::formalism::ProblemDescription& right_problem) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_PROBLEM_HPP_
