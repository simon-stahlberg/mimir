#ifndef MIMIR_SEARCH_SEARCH_HPP_
#define MIMIR_SEARCH_SEARCH_HPP_


namespace mimir::search
{

/// @brief CRTP based interface for a search algorithm
/// @tparam Derived
template<typename Derived>
class SearchBase {
//private:
//    SearchBase() = default;
//    friend Derived;

public:
    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }

    void search() {
        self().search_impl();
    }
};

}  // namespace mimir::search

#endif  // MIMIR_SEARCH_SEARCH_HPP_
