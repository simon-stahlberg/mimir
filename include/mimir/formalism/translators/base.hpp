#ifndef MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_BASE_HPP_

#include "mimir/formalism/translators/interface.hpp"

namespace mimir
{

/**
 * Base implementation.
 */
template<typename Derived>
class BaseTranslator : public ITranslator<BaseTranslator<Derived>>
{
private:
    BaseTranslator() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Collect information.
    ///        Default behavior recursively calls prepare.
    void prepare_base(const loki::pddl::Action& action) { self().prepare_impl(action); }
    void prepare_impl(const loki::pddl::Action& action) {}
    void prepare_base(const loki::pddl::Domain& domain) { self().prepare_impl(domain); }
    void prepare_impl(const loki::pddl::Domain& domain) {}
    void prepare_base(const loki::pddl::Problem& problem) { self().prepare_impl(problem); }
    void prepare_impl(const loki::pddl::Problem& problem)
    {
        this->prepare(problem->get_domain());
        // TODO
    }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    loki::pddl::Domain translate_base(const loki::pddl::Domain& domain) { self().translate_impl(domain); }
    loki::pddl::Domain translate_impl(const loki::pddl::Domain& domain) {}
    loki::pddl::Action translate_base(const loki::pddl::Action& action) { self().translate_impl(action); }
    loki::pddl::Action translate_impl(const loki::pddl::Action& action) {}
    loki::pddl::Problem translate_base(const loki::pddl::Problem& problem) { self().translate_impl(problem); }
    loki::pddl::Problem translate_impl(const loki::pddl::Problem& problem) {}

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_base(const loki::pddl::Problem& problem) { self().run_impl(problem); }
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_impl(const loki::pddl::Problem& problem)
    {
        auto pddl_factories = loki::PDDLFactories();
        self().prepare(problem);
        auto translated_problem = self().translate(problem);
        return std::make_tuple(problem, std::move(pddl_factories));
    }
};

}

#endif
