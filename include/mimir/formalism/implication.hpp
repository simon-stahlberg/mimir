#ifndef MIMIR_FORMALISM_CONDITIONAL_HPP_
#define MIMIR_FORMALISM_CONDITIONAL_HPP_

#include "literal.hpp"

#include "../common/mixins.hpp"

#include <stdexcept>
#include <tuple>
#include <vector>

namespace mimir::formalism
{
    class Implication : public FormattingMixin<Implication>
    {
      private:
        LiteralList antecedent;
        LiteralList consequence;

      public:
        explicit Implication();
        explicit Implication(const LiteralList& ante, const LiteralList& cons);
        explicit Implication(const Implication& other);
        explicit Implication(Implication&& other) noexcept;

        Implication& operator=(const Implication& other);

        const LiteralList& get_antecedent() const;
        const LiteralList& get_consequence() const;

        bool operator<(const Implication& other) const;
        bool operator>(const Implication& other) const;
        bool operator==(const Implication& other) const;
        bool operator!=(const Implication& other) const;
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
