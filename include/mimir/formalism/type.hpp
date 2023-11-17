#if !defined(FORMALISM_TYPE_HPP_)
#define FORMALISM_TYPE_HPP_

#include <memory>
#include <string>
#include <vector>

namespace formalism
{
    struct TypeImpl;

    using Type = std::shared_ptr<TypeImpl>;
    using TypeList = std::vector<formalism::Type>;

    struct TypeImpl
    {
        std::string name;
        formalism::Type base;

        TypeImpl(const std::string& name, formalism::Type base = nullptr);
    };

    bool is_subtype_of(const formalism::Type& type, const formalism::Type& base_type);

    Type create_type(const std::string& name, formalism::Type base = nullptr);

    std::ostream& operator<<(std::ostream& os, const formalism::Type& type);

    std::ostream& operator<<(std::ostream& os, const formalism::TypeList& types);

}  // namespace formalism

namespace std
{
    // Inject comparison and hash functions to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct hash<formalism::Type>
    {
        std::size_t operator()(const formalism::Type& type) const;
    };

    template<>
    struct hash<formalism::TypeList>
    {
        std::size_t operator()(const formalism::TypeList& types) const;
    };

    template<>
    struct less<formalism::Type>
    {
        bool operator()(const formalism::Type& left_type, const formalism::Type& right_type) const;
    };

    template<>
    struct equal_to<formalism::Type>
    {
        bool operator()(const formalism::Type& left_type, const formalism::Type& right_type) const;
    };

}  // namespace std

#endif  // FORMALISM_TYPE_HPP_
