#ifndef MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_DOMAIN_HPP_

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/domain.hpp>

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir
{
    class DomainImpl : public FormattingMixin<DomainImpl>
    {
      private:
        loki::pddl::Domain external_;

        explicit DomainImpl(loki::pddl::Domain external_domain);

      public:
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
