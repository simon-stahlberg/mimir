#include "mimir/formalism/parser.hpp"

#include "mimir/formalism/transformers.hpp"
#include "mimir/formalism/translators.hpp"

#include <loki/loki.hpp>

namespace mimir
{
PDDLParser::PDDLParser(const fs::path& domain_file_path, const fs::path& problem_file_path) :
    m_loki_domain_parser(loki::DomainParser(domain_file_path)),
    m_loki_problem_parser(loki::ProblemParser(problem_file_path, m_loki_domain_parser))
{
    // Parse the loki domain and problem structures using a separate parser to free intermediate results after translation
    auto domain_parser = loki::DomainParser(domain_file_path);
    auto problem_parser = loki::ProblemParser(problem_file_path, domain_parser);
    auto problem = problem_parser.get_problem();

    // Remove types
    auto remove_types_translator = RemoveTypesTranslator(domain_parser.get_factories());
    problem = remove_types_translator.run(*problem);

    // Negation normal form translator
    auto to_nnf_translator = ToNNFTranslator(domain_parser.get_factories());

    // Rename quantified variables
    auto rename_quantifed_variables_translator = RenameQuantifiedVariablesTranslator(domain_parser.get_factories());
    problem = rename_quantifed_variables_translator.run(*problem);

    // Remove universal quantifiers
    auto remove_universal_quantifiers_translator = RemoveUniversalQuantifiersTranslator(domain_parser.get_factories(), to_nnf_translator);
    problem = remove_universal_quantifiers_translator.run(*problem);

    // Simplify goal
    auto simplify_goal_translator = SimplifyGoalTranslator(domain_parser.get_factories());
    problem = simplify_goal_translator.run(*problem);

    // To disjunctive normal form
    auto to_dnf_translator = ToDNFTranslator(domain_parser.get_factories(), to_nnf_translator);
    problem = to_dnf_translator.run(*problem);

    // Split disjunctive conditions
    auto split_disjunctive_conditions = SplitDisjunctiveConditionsTranslator(domain_parser.get_factories());
    problem = split_disjunctive_conditions.run(*problem);

    // To effect normal form
    auto to_enf_translator = ToENFTranslator(domain_parser.get_factories());
    problem = to_enf_translator.run(*problem);

    // Move existential quantifers
    auto move_existential_quantifiers_translator = MoveExistentialQuantifiersTranslator(domain_parser.get_factories());
    problem = move_existential_quantifiers_translator.run(*problem);

    std::cout << *problem->get_domain() << std::endl;
    std::cout << *problem << std::endl;

    // To mimir structures
    auto to_mimir_structures_translator = ToMimirStructures(m_factories);
    m_problem = to_mimir_structures_translator.run(*problem);
    m_domain = m_problem->get_domain();
}

PDDLFactories& PDDLParser::get_factories() { return m_factories; }

const loki::Domain PDDLParser::get_original_domain() const { return m_loki_domain_parser.get_domain(); }

const loki::Problem PDDLParser::get_original_problem() const { return m_loki_problem_parser.get_problem(); }

const Domain& PDDLParser::get_domain() const { return m_domain; }

const Problem& PDDLParser::get_problem() const { return m_problem; }

}
