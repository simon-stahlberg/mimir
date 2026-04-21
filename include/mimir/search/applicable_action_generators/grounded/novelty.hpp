#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_NOVELTY_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDED_NOVELTY_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/translator/novelty_compilation.hpp"
#include "mimir/search/applicable_action_generators/grounded/grounded.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/state.hpp"

namespace mimir::search
{

class GroundedNoveltyApplicableActionGeneratorImpl : public IApplicableActionGenerator
{
public:
    enum class Mode { Exhaustive, Minimal };

    using Statistics = GroundedApplicableActionGeneratorImpl::Statistics;

    using IEventHandler = GroundedApplicableActionGeneratorImpl::IEventHandler;
    using EventHandler = GroundedApplicableActionGeneratorImpl::EventHandler;

    using DebugEventHandlerImpl = GroundedApplicableActionGeneratorImpl::DebugEventHandlerImpl;
    using DebugEventHandler = GroundedApplicableActionGeneratorImpl::DebugEventHandler;

    using DefaultEventHandlerImpl = GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl;
    using DefaultEventHandler = GroundedApplicableActionGeneratorImpl::DefaultEventHandler;

    GroundedNoveltyApplicableActionGeneratorImpl(formalism::Problem problem,
                                                formalism::NoveltyProblemWrapper novelty_problem_wrapper,
                                                GroundedApplicableActionGenerator novelty_applicable_action_generator,
                                                std::unordered_map<formalism::GroundAction, formalism::GroundAction> novelty_to_original_action,
                                                std::unordered_map<formalism::GroundAction, formalism::GroundAction> original_to_novelty_action,
                                                std::unordered_map<formalism::GroundAction, formalism::GroundAtomList<formalism::FluentTag>> novelty_action_to_not_novel_atoms,
                                                std::vector<formalism::GroundAtom<formalism::FluentTag>> original_to_compiled_atom,
                                                std::vector<formalism::GroundAtom<formalism::FluentTag>> original_to_not_novel_atom,
                                                StateRepository novelty_state_repository,
                                                Mode mode);

    static GroundedNoveltyApplicableActionGenerator create(formalism::Problem problem,
                                                           std::size_t k = 1,
                                                           const match_tree::Options& options = match_tree::Options(),
                                                           EventHandler event_handler = nullptr,
                                                           Mode mode = Mode::Exhaustive);

    GroundedNoveltyApplicableActionGeneratorImpl(const GroundedNoveltyApplicableActionGeneratorImpl& other) = delete;
    GroundedNoveltyApplicableActionGeneratorImpl& operator=(const GroundedNoveltyApplicableActionGeneratorImpl& other) = delete;
    GroundedNoveltyApplicableActionGeneratorImpl(GroundedNoveltyApplicableActionGeneratorImpl&& other) = delete;
    GroundedNoveltyApplicableActionGeneratorImpl& operator=(GroundedNoveltyApplicableActionGeneratorImpl&& other) = delete;

    mimir::generator<formalism::GroundAction> create_applicable_action_generator(const State& state) override;

    void on_finish_search_layer() override;
    void on_end_search() override;

    const formalism::Problem& get_problem() const override;

    void reset_novelty_history();
    void update_novelty_history(formalism::GroundAction action);

private:
    formalism::Problem m_problem;
    formalism::NoveltyProblemWrapper m_novelty_problem_wrapper;
    GroundedApplicableActionGenerator m_novelty_applicable_action_generator;
    std::unordered_map<formalism::GroundAction, formalism::GroundAction> m_novelty_to_original_action;
    std::unordered_map<formalism::GroundAction, formalism::GroundAction> m_original_to_novelty_action;
    std::unordered_map<formalism::GroundAction, formalism::GroundAtomList<formalism::FluentTag>> m_novelty_action_to_not_novel_atoms;
    std::vector<formalism::GroundAtom<formalism::FluentTag>> m_original_to_compiled_atom;
    std::vector<formalism::GroundAtom<formalism::FluentTag>> m_original_to_not_novel_atom;
    StateRepository m_novelty_state_repository;

    formalism::GroundAtomList<formalism::FluentTag> m_seen_not_novel_atoms;
    Mode m_mode;
    formalism::GroundAtomList<formalism::FluentTag> collect_ground_atoms(const State& state) const;
    State make_novelty_state(const State& state) const;
    formalism::GroundAction translate_ground_action(formalism::GroundAction novelty_ground_action) const;
};

}

#endif
