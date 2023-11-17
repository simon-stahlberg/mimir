#ifndef MIMIR_FORMALISM_CONDITIONAL_HPP_
#define MIMIR_FORMALISM_CONDITIONAL_HPP_

#include "declarations.hpp"
#include "literal.hpp"

#include <vector>

namespace mimir::formalism
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
    struct hash<mimir::formalism::Implication>
    {
        std::size_t operator()(const mimir::formalism::Implication& implication) const;
    };

    template<>
    struct less<mimir::formalism::Implication>
    {
        bool operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const;
    };

    template<>
    struct equal_to<mimir::formalism::Implication>
    {
        bool operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const;
    };

    template<>
    struct hash<mimir::formalism::ImplicationList>
    {
        std::size_t operator()(const mimir::formalism::ImplicationList& implication_list) const;
    };

    template<>
    struct less<mimir::formalism::ImplicationList>
    {
        bool operator()(const mimir::formalism::ImplicationList& lhs, const mimir::formalism::ImplicationList& rhs) const;
    };

    template<>
    struct equal_to<mimir::formalism::ImplicationList>
    {
        bool operator()(const mimir::formalism::ImplicationList& lhs, const mimir::formalism::ImplicationList& rhs) const;
    };
}  // namespace std

#endif  // MIMIR_FORMALISM_CONDITIONAL_HPP_
