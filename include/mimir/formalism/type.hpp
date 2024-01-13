#ifndef MIMIR_FORMALISM_TYPE_HPP_
#define MIMIR_FORMALISM_TYPE_HPP_

#include "../common/mixins.hpp"

#include <loki/domain/pddl/type.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace mimir::formalism
{
    class Type;
    using TypeList = std::vector<Type>;

    class Type : public BaseMixin<Type>
    {
      private:
        loki::pddl::Type external_;

        explicit Type(loki::pddl::Type external_type);

      public:
        uint32_t get_id() const;
        const std::string& get_name() const;
        TypeList get_bases() const;
        bool is_subtype_of(const Type& type) const;

        bool operator<(const Type& other) const;
        bool operator>(const Type& other) const;
        bool operator==(const Type& other) const;
        bool operator!=(const Type& other) const;
        bool operator<=(const Type& other) const;

        std::size_t hash() const;

        friend std::ostream& operator<<(std::ostream& os, const mimir::formalism::Type& type);
        friend class Domain;
        friend class Term;
    };

    // std::ostream& operator<<(std::ostream& os, const mimir::formalism::TypeList& types);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<mimir::formalism::Type>
    {
        std::size_t operator()(const mimir::formalism::Type& type) const;
    };

    template<>
    struct hash<mimir::formalism::TypeList>
    {
        std::size_t operator()(const mimir::formalism::TypeList& types) const;
    };

    template<>
    struct less<mimir::formalism::Type>
    {
        bool operator()(const mimir::formalism::Type& left_type, const mimir::formalism::Type& right_type) const;
    };

    template<>
    struct equal_to<mimir::formalism::Type>
    {
        bool operator()(const mimir::formalism::Type& left_type, const mimir::formalism::Type& right_type) const;
    };

}  // namespace std

#endif  // MIMIR_FORMALISM_TYPE_HPP_
