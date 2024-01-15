#include <mimir/formalism/implication.hpp>

#include <algorithm>

namespace mimir::formalism
{
    Implication::Implication() = default;
    Implication::Implication(const LiteralList& ante, const LiteralList& cons) : antecedent(ante), consequence(cons) {}
    Implication::Implication(const Implication& other) : antecedent(other.antecedent), consequence(other.consequence) {}
    Implication::Implication(Implication&& other) noexcept : antecedent(std::move(other.antecedent)), consequence(std::move(other.consequence)) {}

    Implication& Implication::operator=(const Implication& other)
    {
        if (this != &other)
        {
            antecedent = other.antecedent;
            consequence = other.consequence;
        }

        return *this;
    }

    const LiteralList& Implication::get_antecedent() const { return antecedent; }
    const LiteralList& Implication::get_consequence() const { return consequence; }

    bool Implication::operator<(const Implication& rhs) const { return std::less<Implication>()(*this, rhs); }
    bool Implication::operator>(const Implication& rhs) const { return std::less<Implication>()(rhs, *this); }
    bool Implication::operator==(const Implication& rhs) const { return std::equal_to<Implication>()(*this, rhs); }
    bool Implication::operator!=(const Implication& rhs) const { return !std::equal_to<Implication>()(*this, rhs); }
}  // namespace formalism

/*
namespace std
{
    std::size_t hash<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& implication) const
    {
        return hash_combine(implication.get_antecedent(), implication.get_consequence());
    }

    bool less<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const
    {
        if (lhs.get_antecedent() != rhs.get_antecedent())
        {
            return lhs.get_antecedent() < rhs.get_antecedent();
        }

        return lhs.get_consequence() < rhs.get_consequence();
    }

    bool equal_to<mimir::formalism::Implication>::operator()(const mimir::formalism::Implication& lhs, const mimir::formalism::Implication& rhs) const
    {
        return lhs.get_antecedent() == rhs.get_antecedent() && lhs.get_consequence() == rhs.get_consequence();
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
*/

