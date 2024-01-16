#ifndef MIMIR_FORMALISM_CONDITIONAL_HPP_
#define MIMIR_FORMALISM_CONDITIONAL_HPP_

#include "literal.hpp"

#include "../common/mixins.hpp"

#include <stdexcept>
#include <tuple>
#include <vector>

namespace mimir
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

/*
namespace std
{
    template<>
    struct hash<mimir::Implication>
    {
        std::size_t operator()(const mimir::Implication& implication) const;
    };

    template<>
    struct less<mimir::Implication>
    {
        bool operator()(const mimir::Implication& lhs, const mimir::Implication& rhs) const;
    };

    template<>
    struct equal_to<mimir::Implication>
    {
        bool operator()(const mimir::Implication& lhs, const mimir::Implication& rhs) const;
    };

    template<>
    struct hash<mimir::ImplicationList>
    {
        std::size_t operator()(const mimir::ImplicationList& implication_list) const;
    };

    template<>
    struct less<mimir::ImplicationList>
    {
        bool operator()(const mimir::ImplicationList& lhs, const mimir::ImplicationList& rhs) const;
    };

    template<>
    struct equal_to<mimir::ImplicationList>
    {
        bool operator()(const mimir::ImplicationList& lhs, const mimir::ImplicationList& rhs) const;
    };
}  // namespace std
*/

#endif  // MIMIR_FORMALISM_CONDITIONAL_HPP_
