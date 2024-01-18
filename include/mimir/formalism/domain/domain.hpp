#ifndef MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/domain.hpp>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir
{
class DomainImpl : public loki::Base<DomainImpl> {
private:
    loki::pddl::Domain external_;

    std::string m_name;
    Requirements m_requirements;
    TypeList m_types;
    ObjectList m_constants;
    PredicateList m_predicates;
    FunctionSkeletonList m_functions;
    ActionList m_actions;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    DomainImpl(int identifier,
        loki::pddl::Domain external,
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
    const FunctionSkeletonList& get_functions() const;
    const ActionList& get_actions() const;
};


}  // namespace mimir

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::DomainImpl>
    {
        std::size_t operator()(const mimir::DomainImpl& domain) const;
    };

    template<>
    struct less<mimir::DomainImpl>
    {
        bool operator()(const mimir::DomainImpl& left_domain, const mimir::DomainImpl& right_domain) const;
    };

    template<>
    struct equal_to<mimir::DomainImpl>
    {
        bool operator()(const mimir::DomainImpl& left_domain, const mimir::DomainImpl& right_domain) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_
