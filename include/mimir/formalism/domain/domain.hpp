#ifndef MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_

#include "declarations.hpp"

#include <loki/domain/pddl/domain.hpp>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace loki
{
    template<typename HolderType, ElementsPerSegment N>
    class PersistentFactory;
}


namespace mimir
{
    class DomainImpl : public loki::Base<DomainImpl>
    {
    private:
        std::string m_name;
        Requirements m_requirements;
        TypeList m_types;
        ObjectList m_constants;
        PredicateList m_predicates;
        PredicateList m_static_predicates;
        PredicateList m_fluent_predicates;
        FunctionSkeletonList m_functions;
        ActionList m_actions;

        // Below: add additional members if needed and initialize them in the constructor

        DomainImpl(int identifier,
            std::string name,
            Requirements requirements,
            TypeList types,
            ObjectList constants,
            PredicateList predicates,
            FunctionSkeletonList functions,
            ActionList actions);

        // Give access to the constructor.
        template<typename HolderType, ElementsPerSegment N>
        friend class loki::PersistentFactory;

        /// @brief Test for structural equivalence
        bool is_structurally_equivalent_to_impl(const DomainImpl& other) const;
        size_t hash_impl() const;
        void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

        // Give access to the private interface implementations.
        friend class loki::Base<DomainImpl>;

    public:
        const std::string& get_name() const;
        const Requirements& get_requirements() const;
        const TypeList& get_types() const;
        const ObjectList& get_constants() const;
        const PredicateList& get_predicates() const;
        const PredicateList& get_static_predicates() const;
        const PredicateList& get_fluent_predicates() const;
        const FunctionSkeletonList& get_functions() const;
        const ActionList& get_actions() const;
    };
}

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<mimir::Domain>
    {
        bool operator()(const mimir::Domain& domain_left, const mimir::Domain& domain_right) const;
    };

    template<>
    struct hash<mimir::DomainImpl>
    {
        std::size_t operator()(const mimir::DomainImpl& domain) const;
    };
}

#endif
