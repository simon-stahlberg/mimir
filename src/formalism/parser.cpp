#include <mimir/formalism/parser.hpp>

#include <mimir/formalism/domain/parsers/domain.hpp>
#include <mimir/formalism/problem/parsers/problem.hpp>

#include <loki/domain/parser.hpp>
#include <loki/problem/parser.hpp>


namespace mimir 
{
    PDDLParser::PDDLParser(const fs::path& domain_file_path, const fs::path& problem_file_path)
    { 
        
        /* Parse the domain */
        std::cout << "Loki parsing domain:" << std::endl;
        auto domain_parser = loki::DomainParser(domain_file_path);
        std::cout << "Domain:" << std::endl;
        std::cout << *domain_parser.get_domain() << std::endl;
        std::cout << "Mimir parsing domain:" << std::endl;
        m_domain = parse(domain_parser.get_domain(), m_factories);
        std::cout << "Domain:" << std::endl;
        std::cout << *m_domain << std::endl;

        /* Parse the problem */
        std::cout << "Loki parsing problem:" << std::endl;
        auto problem_parser = loki::ProblemParser(problem_file_path, domain_parser);
        std::cout << "Problem:" << std::endl;
        std::cout << *problem_parser.get_problem() << std::endl;
        std::cout << "Mimir parsing problem:" << std::endl;
        m_problem = parse(problem_parser.get_problem(), m_factories);
        std::cout << "Problem:" << std::endl;
        std::cout << *m_problem << std::endl;
    }

    PDDLFactories& PDDLParser::get_factories() { return m_factories; }

    const Domain& PDDLParser::get_domain() const { return m_domain; }

    const Problem& PDDLParser::get_problem() const { return m_problem; }

}