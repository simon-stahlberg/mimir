#if !defined(FORMALISM_CONDITIONAL_HPP_)
#define FORMALISM_CONDITIONAL_HPP_

#include "declarations.hpp"
#include "literal.hpp"

#include <vector>

namespace formalism
{
    struct Implication
    {
        LiteralList antecedent;
        LiteralList consequence;

        Implication();
        Implication(const LiteralList& ante, const LiteralList& cons);
        Implication(const Implication& other);
        Implication(Implication&& other) noexcept;

        operator std::tuple<LiteralList&, LiteralList&>();
        operator std::tuple<const LiteralList&, const LiteralList&>() const;

        bool operator<(const Implication& rhs) const;
        bool operator>(const Implication& rhs) const;
        bool operator==(const Implication& rhs) const;
        bool operator!=(const Implication& rhs) const;
    };

    using ImplicationList = std::vector<Implication>;
}  // namespace formalism

namespace std
{
    template<>
    struct hash<formalism::Implication>
    {
        std::size_t operator()(const formalism::Implication& implication) const;
    };

    template<>
    struct less<formalism::Implication>
    {
        bool operator()(const formalism::Implication& lhs, const formalism::Implication& rhs) const;
    };

    template<>
    struct equal_to<formalism::Implication>
    {
        bool operator()(const formalism::Implication& lhs, const formalism::Implication& rhs) const;
    };

    template<>
    struct hash<formalism::ImplicationList>
    {
        std::size_t operator()(const formalism::ImplicationList& implication_list) const;
    };

    template<>
    struct less<formalism::ImplicationList>
    {
        bool operator()(const formalism::ImplicationList& lhs, const formalism::ImplicationList& rhs) const;
    };

    template<>
    struct equal_to<formalism::ImplicationList>
    {
        bool operator()(const formalism::ImplicationList& lhs, const formalism::ImplicationList& rhs) const;
    };
}  // namespace std

#endif  // FORMALISM_CONDITIONAL_HPP_
