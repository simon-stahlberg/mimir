#ifndef MIMIR_FORMALISM_TRANSLATIONS_HPP_
#define MIMIR_FORMALISM_TRANSLATIONS_HPP_

#include <loki/pddl/declarations.hpp>
#include <loki/pddl/factories.hpp>
#include <tuple>
#include <variant>
#include <vector>

namespace mimir
{

/**
 * Interface class.
 */
template<typename Derived>
class ITranslator
{
private:
    ITranslator() = default;
    friend Derived;

    /// @brief Helper to cast to Derived.
    constexpr const auto& self() const { return static_cast<const Derived&>(*this); }
    constexpr auto& self() { return static_cast<Derived&>(*this); }

public:
    /// @brief Collect information.
    ///        Default behavior does nothing.
    /// @param problem
    void prepare(const loki::pddl::Problem& problem) { self().prepare_base(problem); }

    /// @brief Collect information.
    ///        Default behavior does nothing.
    /// @param domain
    void prepare(const loki::pddl::Domain& domain) { self().prepare_base(domain); }

    /// @brief Collect information.
    ///        Default behavior does nothing.
    /// @param action
    void prepare(const loki::pddl::Action& action) { self().prepare_base(action); }

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param problem
    /// @return
    loki::pddl::Problem translate(const loki::pddl::Problem& problem) { self().translate_base(problem); }

    /// @brief Apply domain translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param domain
    /// @return
    loki::pddl::Domain translate(const loki::pddl::Domain& domain) { self().translate_base(domain); }

    /// @brief Apply action translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param action
    /// @return
    loki::pddl::Action translate(const loki::pddl::Action& action) { self().translate_base(action); }

    /// @brief Recursively apply preparation followed by translation.
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run(const loki::pddl::Problem& problem) { self().run_base(problem); }
};

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
    ///        Default behavior does nothing.
    /// @param problem
    void prepare_base(const loki::pddl::Problem& problem) { self().prepare_impl(problem); }
    void prepare_impl(const loki::pddl::Problem& problem) {}

    /// @brief Collect information.
    ///        Default behavior does nothing.
    /// @param domain
    void prepare_base(const loki::pddl::Domain& domain) { self().prepare_impl(domain); }
    void prepare_impl(const loki::pddl::Domain& domain) {}

    /// @brief Collect information.
    ///        Default behavior does nothing.
    /// @param action
    void prepare_base(const loki::pddl::Action& action) { self().prepare_impl(action); }
    void prepare_impl(const loki::pddl::Action& action) {}

    /// @brief Apply problem translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param problem
    /// @return
    loki::pddl::Problem translate_base(const loki::pddl::Problem& problem) { self().translate_impl(problem); }
    loki::pddl::Problem translate_impl(const loki::pddl::Problem& problem) {}

    /// @brief Apply domain translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param domain
    /// @return
    loki::pddl::Domain translate_base(const loki::pddl::Domain& domain) { self().translate_impl(domain); }
    loki::pddl::Domain translate_impl(const loki::pddl::Domain& domain) {}

    /// @brief Apply action translation.
    ///        Default behavior reparses it into the pddl_factories.
    /// @param action
    /// @return
    loki::pddl::Action translate_base(const loki::pddl::Action& action) { self().translate_impl(action); }
    loki::pddl::Action translate_impl(const loki::pddl::Action& action) {}

    /// @brief Recursively apply preparation followed by translation.
    ///        Default behavior runs prepare and translate and returns its results.
    /// @param problem
    /// @return
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_base(const loki::pddl::Problem& problem) { self().run_impl(problem); }
    std::tuple<loki::pddl::Problem, loki::PDDLFactories> run_impl(const loki::pddl::Problem& problem)
    {
        auto pddl_factories = loki::PDDLFactories();
        self().prepare(problem);
        auto translated_problem = self().translate(problem);
        return std::make_tuple(problem, std::move(pddl_factories));
    }
};

class TypeTranslator : public BaseTranslator<TypeTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<TypeTranslator>;
};

class ConjunctiveConditionsTranslator : public BaseTranslator<ConjunctiveConditionsTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<ConjunctiveConditionsTranslator>;
};

class PositiveNormalFormTranslator : public BaseTranslator<PositiveNormalFormTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<PositiveNormalFormTranslator>;
};

class EffectNormalFormTranslator : public BaseTranslator<EffectNormalFormTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<EffectNormalFormTranslator>;
};

class DeleteRelaxedTranslator : public BaseTranslator<DeleteRelaxedTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<DeleteRelaxedTranslator>;
};

}

#endif