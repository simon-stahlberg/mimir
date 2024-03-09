#include <loki/domain/parser.hpp>
#include <loki/problem/parser.hpp>
#include <mimir/formalism/parsers/domain.hpp>
#include <mimir/formalism/parser.hpp>
#include <mimir/formalism/parsers/problem.hpp>

namespace mimir
{
PDDLParser::PDDLParser(const fs::path& domain_file_path, const fs::path& problem_file_path)
{
    /* Parse the domain */
    auto domain_parser = loki::DomainParser(domain_file_path);
    m_domain = parse(domain_parser.get_domain(), m_factories);

    /* Parse the problem */
    auto problem_parser = loki::ProblemParser(problem_file_path, domain_parser);
    m_problem = parse(problem_parser.get_problem(), m_factories);
}

PDDLFactories& PDDLParser::get_factories() { return m_factories; }

const Domain& PDDLParser::get_domain() const { return m_domain; }

const Problem& PDDLParser::get_problem() const { return m_problem; }

}
