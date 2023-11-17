#include "../../include/mimir/formalism/implication.hpp"
#include "help_functions.hpp"

#include <algorithm>

namespace mimir::formalism
{
    Implication::Implication() = default;
    Implication::Implication(const LiteralList& ante, const LiteralList& cons) : antecedent(ante), consequence(cons) {}
    Implication::Implication(const Implication& other) : antecedent(other.antecedent), consequence(other.consequence) {}
    Implication::Implication(Implication&& other) noexcept : antecedent(std::move(other.antecedent)), consequence(std::move(other.consequence)) {}

    Implication::operator std::tuple<LiteralList&, LiteralList&>() { return std::tie(antecedent, consequence); }
    Implication::operator std::tuple<const LiteralList&, const LiteralList&>() const { return std::tie(antecedent, consequence); }

    bool Implication::operator<(const Implication& rhs) const { return std::less<Implication>()(*this, rhs); }
    bool Implication::operator>(const Implication& rhs) const { return std::less<Implication>()(rhs, *this); }
    bool Implication::operator==(const Implication& rhs) const { return std::equal_to<Implication>()(*this, rhs); }
    bool Implication::operator!=(const Implication& rhs) const { return !std::equal_to<Implication>()(*this, rhs); }
}  // namespace formalism

namespace std
{
    std::size_t hash<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& implication) const
    {
        return hash_combine(implication.antecedent, implication.consequence);
    }

    bool less<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const
    {
        if (lhs.antecedent != rhs.antecedent)
        {
            return lhs.antecedent < rhs.antecedent;
        }

        return lhs.consequence < rhs.consequence;
    }

    bool equal_to<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const
    {
        return lhs.antecedent == rhs.antecedent && lhs.consequence == rhs.consequence;
    }

    std::size_t hash<mimir::formalism::ImplicationList>::operator()(const mimir::formalism::ImplicationList& implication_list) const
    {
        return hash_vector(implication_list);
    }

    bool less<mimir::formalism::ImplicationList>::operator()(const mimir::formalism::ImplicationList& lhs, const mimir::formalism::ImplicationList& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            return lhs.size() < rhs.size();
        }

        return std::lexicographical_compare(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend());
    }

    bool equal_to<mimir::formalism::ImplicationList>::operator()(const mimir::formalism::ImplicationList& lhs,
                                                                 const mimir::formalism::ImplicationList& rhs) const
    {
        if (lhs.size() != rhs.size())
        {
            return false;
        }

        return std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
    }
}  // namespace std
