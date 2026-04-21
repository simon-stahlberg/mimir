#include "mimir/search/applicable_action_generators/grounded/event_handlers/default.hpp"
#include "mimir/search/applicable_action_generators/grounded/novelty.hpp"
#include "mimir/search/axiom_evaluators/grounded/grounded.hpp"
#include "mimir/search/axiom_evaluators/interface.hpp"
#include "mimir/search/grounders/lifted.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

#include <stdexcept>
#include <unordered_map>

using namespace mimir::formalism;

namespace mimir::search
{

GroundedNoveltyApplicableActionGeneratorImpl::GroundedNoveltyApplicableActionGeneratorImpl(
    Problem problem,
    NoveltyProblemWrapper novelty_problem,
    GroundedApplicableActionGenerator novelty_applicable_action_generator,
    std::unordered_map<GroundAction, GroundAction> novelty_to_original_action,
    std::unordered_map<GroundAction, GroundAction> original_to_novelty_action,
    std::unordered_map<GroundAction, GroundAtomList<FluentTag>> novelty_action_to_not_novel_atoms,
    std::vector<GroundAtom<FluentTag>> original_to_compiled_atom,
    std::vector<GroundAtom<FluentTag>> original_to_not_novel_atom,
    StateRepository novelty_state_repository,
    Mode mode)
    : m_problem(std::move(problem)),
      m_novelty_problem_wrapper(std::move(novelty_problem)),
      m_novelty_applicable_action_generator(std::move(novelty_applicable_action_generator)),
      m_novelty_to_original_action(std::move(novelty_to_original_action)),
      m_original_to_novelty_action(std::move(original_to_novelty_action)),
      m_novelty_action_to_not_novel_atoms(std::move(novelty_action_to_not_novel_atoms)),
      m_original_to_compiled_atom(std::move(original_to_compiled_atom)),
      m_original_to_not_novel_atom(std::move(original_to_not_novel_atom)),
      m_novelty_state_repository(std::move(novelty_state_repository)),
      m_seen_not_novel_atoms(),
      m_mode(mode)
{
}

GroundedNoveltyApplicableActionGenerator
GroundedNoveltyApplicableActionGeneratorImpl::create(Problem problem, std::size_t k, const match_tree::Options& options, EventHandler event_handler, Mode mode)
{
    // Step 1: Translate to novelty problem
    auto novelty_translator = NoveltyTranslator(k);
    auto novelty_problem_wrapper = novelty_translator.translate(problem);
    auto novelty_problem = novelty_problem_wrapper.get_problem();

    // Step 2: Ground the original problem to get all possible relevant ground actions for the novelty problem
    auto original_grounder = LiftedGrounder(problem);

    // Precompute atom maps
    const auto num_fluent_atoms = std::ranges::distance(problem->get_repositories().get_ground_atoms<FluentTag>());
    std::vector<GroundAtom<FluentTag>> original_to_compiled_atom(num_fluent_atoms, nullptr);
    std::vector<GroundAtom<FluentTag>> original_to_not_novel_atom(num_fluent_atoms, nullptr);
    for (const auto& original_atom : problem->get_repositories().get_ground_atoms<FluentTag>())
    {
        original_to_compiled_atom[original_atom.get_index()] = novelty_problem_wrapper.get_compiled_atom(&original_atom);
        original_to_not_novel_atom[original_atom.get_index()] = novelty_problem_wrapper.get_not_novel_atom(&original_atom);
    }

    // Step 3: For each original ground action, create the corresponding novelty ground action and store the mapping
    std::vector<GroundAction> novelty_ground_actions;
    std::unordered_map<GroundAction, GroundAction> novelty_to_original_action;
    std::unordered_map<GroundAction, GroundAction> original_to_novelty_action;
    std::unordered_map<GroundAction, GroundAtomList<FluentTag>> novelty_action_to_not_novel_atoms;
    novelty_ground_actions.reserve(original_grounder.create_ground_actions().size());

    for (const auto& original_action : original_grounder.create_ground_actions())
    {
        // Map the action schema and objects to the novelty problem
        const auto& original_schema = original_action->get_action();
        const auto& original_objects = original_action->get_objects();

        Action novelty_schema = novelty_problem_wrapper.get_compiled_action(original_schema);

        // Map the objects to the novelty problem
        ObjectList novelty_terms;
        novelty_terms.reserve(original_objects.size());
        for (const auto& obj : original_objects)
        {
            novelty_terms.push_back(novelty_problem_wrapper.get_compiled_object(obj));
        }

        // Ground the novelty action
        auto novelty_ground_action = novelty_problem->ground(novelty_schema, novelty_terms);
        novelty_ground_actions.push_back(novelty_ground_action);
        novelty_to_original_action[novelty_ground_action] = original_action;
        original_to_novelty_action[original_action] = novelty_ground_action;

        auto not_novel_atoms = GroundAtomList<FluentTag>{};
        for (const auto& cond_eff : novelty_ground_action->get_conditional_effects())
        {
            if (cond_eff->get_conjunctive_condition()->get_num_preconditions<StaticTag, FluentTag, DerivedTag>() > 0 || !cond_eff->get_conjunctive_condition()->get_numeric_constraints().empty())
            {
                throw std::runtime_error("Novelty tracking with non-empty conditional effects is not supported.");
            }
            for (auto atom_index : cond_eff->get_conjunctive_effect()->get_propositional_effects<PositiveTag>())
            {
                auto atom = novelty_problem->get_repositories().get_ground_atom<FluentTag>(atom_index);
                if (novelty_problem_wrapper.is_not_novel_predicate(atom->get_predicate()))
                {
                    not_novel_atoms.push_back(atom);
                }
            }
        }
        novelty_action_to_not_novel_atoms[novelty_ground_action] = not_novel_atoms;
    }

    // Step 4: Create the match tree using the novelty ground actions
    auto& novelty_repositories = novelty_problem->get_repositories();
    auto novelty_match_tree = match_tree::MatchTreeImpl<GroundActionImpl>::create(novelty_repositories, novelty_ground_actions, options);

    // Step 5: Create the applicable action generator using the match tree
    auto novelty_event_handler = event_handler ? std::make_shared<GroundedApplicableActionGeneratorImpl::DefaultEventHandlerImpl>() : nullptr;
    auto novelty_applicable_action_generator = GroundedApplicableActionGeneratorImpl::create(novelty_problem, std::move(novelty_match_tree), novelty_event_handler);
    auto novelty_grounder = LiftedGrounder(novelty_problem);
    auto novelty_axiom_evaluator = novelty_grounder.create_grounded_axiom_evaluator();
    auto novelty_state_repository = StateRepositoryImpl::create(std::static_pointer_cast<IAxiomEvaluator>(novelty_axiom_evaluator));

    // Store novelty_to_original_action mapping in the class
    return std::shared_ptr<GroundedNoveltyApplicableActionGeneratorImpl>(
        new GroundedNoveltyApplicableActionGeneratorImpl(problem,
                                                         std::move(novelty_problem_wrapper),
                                                         std::move(novelty_applicable_action_generator),
                                                         std::move(novelty_to_original_action),
                                                         std::move(original_to_novelty_action),
                                                         std::move(novelty_action_to_not_novel_atoms),
                                                         std::move(original_to_compiled_atom),
                                                         std::move(original_to_not_novel_atom),
                                                         std::move(novelty_state_repository),
                                                         mode));
}

mimir::generator<GroundAction> GroundedNoveltyApplicableActionGeneratorImpl::create_applicable_action_generator(const State& state)
{
    if (&state.get_problem() != m_problem.get())
    {
        throw std::runtime_error("GroundedNoveltyApplicableActionGeneratorImpl requires states from its original problem.");
    }

    if (m_mode == Mode::Exhaustive)
    {
        const auto novelty_state = make_novelty_state(state);
        for (const auto& novelty_ground_action : m_novelty_applicable_action_generator->create_applicable_action_generator(novelty_state))
        {
            auto it = m_novelty_to_original_action.find(novelty_ground_action);
            if (it == m_novelty_to_original_action.end())
            {
                throw std::runtime_error("No mapping from novelty ground action to original ground action.");
            }
            co_yield it->second;
        }
    }
    else if (m_mode == Mode::Minimal)
    {
        auto current_seen_not_novel_atoms = m_seen_not_novel_atoms;

        while (true)
        {
            // Reconstruct the novelty state with the current set of "not-novel" atoms
            auto novelty_atoms = current_seen_not_novel_atoms;
            novelty_atoms.reserve(current_seen_not_novel_atoms.size() + state.get_atoms<FluentTag>().count() * 2);
            for (const auto atom_index : state.get_atoms<FluentTag>())
            {
                novelty_atoms.push_back(m_original_to_compiled_atom[atom_index]);
                novelty_atoms.push_back(m_original_to_not_novel_atom[atom_index]);
            }
            std::sort(novelty_atoms.begin(), novelty_atoms.end(), [](const auto& lhs, const auto& rhs) { return lhs->get_index() < rhs->get_index(); });
            novelty_atoms.erase(std::unique(novelty_atoms.begin(), novelty_atoms.end()), novelty_atoms.end());

            const auto [novelty_state, _] = m_novelty_state_repository->get_or_create_state(novelty_atoms, state.get_numeric_variables());

            // Get the generator
            auto generator = m_novelty_applicable_action_generator->create_applicable_action_generator(novelty_state);
            auto it = generator.begin();
            if (it == generator.end())
            {
                // No more applicable actions
                break;
            }

            // Yield the first one
            const auto& novelty_ground_action = *it;
            auto orig_it = m_novelty_to_original_action.find(novelty_ground_action);
            if (orig_it == m_novelty_to_original_action.end())
            {
                throw std::runtime_error("No mapping from novelty ground action to original ground action.");
            }
            co_yield orig_it->second;

            // Extract the novel atoms added by this action (which are encoded as not-novel atoms in its effects)
            // and add them to our temporary set so that we don't pick overlapping actions in the next iteration.
            const auto& not_novel_atoms = m_novelty_action_to_not_novel_atoms.at(novelty_ground_action);
            current_seen_not_novel_atoms.insert(current_seen_not_novel_atoms.end(), not_novel_atoms.begin(), not_novel_atoms.end());
        }
    }
}

void GroundedNoveltyApplicableActionGeneratorImpl::on_finish_search_layer() { m_novelty_applicable_action_generator->on_finish_search_layer(); }

void GroundedNoveltyApplicableActionGeneratorImpl::on_end_search() { m_novelty_applicable_action_generator->on_end_search(); }

const Problem& GroundedNoveltyApplicableActionGeneratorImpl::get_problem() const { return m_problem; }

void GroundedNoveltyApplicableActionGeneratorImpl::reset_novelty_history()
{
    m_seen_not_novel_atoms.clear();
}

void GroundedNoveltyApplicableActionGeneratorImpl::update_novelty_history(formalism::GroundAction action)
{
    auto it = m_original_to_novelty_action.find(action);
    if (it == m_original_to_novelty_action.end())
    {
        throw std::runtime_error("No mapping from original ground action to novelty ground action.");
    }

    const auto& novelty_ground_action = it->second;
    const auto& not_novel_atoms = m_novelty_action_to_not_novel_atoms.at(novelty_ground_action);
    m_seen_not_novel_atoms.insert(m_seen_not_novel_atoms.end(), not_novel_atoms.begin(), not_novel_atoms.end());
}

GroundAtomList<FluentTag> GroundedNoveltyApplicableActionGeneratorImpl::collect_ground_atoms(const State& state) const
{
    auto atoms = GroundAtomList<FluentTag> {};
    atoms.reserve(state.get_atoms<FluentTag>().count());

    const auto& repositories = m_problem->get_repositories();
    for (const auto atom_index : state.get_atoms<FluentTag>())
    {
        atoms.push_back(repositories.get_ground_atom<FluentTag>(atom_index));
    }

    return atoms;
}

State GroundedNoveltyApplicableActionGeneratorImpl::make_novelty_state(const State& state) const
{
    auto novelty_atoms = m_seen_not_novel_atoms;
    novelty_atoms.reserve(m_seen_not_novel_atoms.size() + state.get_atoms<FluentTag>().count() * 2);
    for (const auto atom_index : state.get_atoms<FluentTag>())
    {
        novelty_atoms.push_back(m_original_to_compiled_atom[atom_index]);
        novelty_atoms.push_back(m_original_to_not_novel_atom[atom_index]);
    }

    std::sort(novelty_atoms.begin(), novelty_atoms.end(), [](const auto& lhs, const auto& rhs) { return lhs->get_index() < rhs->get_index(); });
    novelty_atoms.erase(std::unique(novelty_atoms.begin(), novelty_atoms.end()), novelty_atoms.end());

    const auto [novelty_state, _] = m_novelty_state_repository->get_or_create_state(novelty_atoms, state.get_numeric_variables());
    return novelty_state;
}

}
