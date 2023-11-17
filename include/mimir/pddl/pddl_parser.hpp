#ifndef MIMIR_PDDL_PARSER_DOMAIN_HPP_
#define MIMIR_PDDL_PARSER_DOMAIN_HPP_

#include "../formalism/domain.hpp"

#include <fstream>
#include <memory>

// Older versions of LibC++ does not have filesystem (e.g., ubuntu 18.04), use the experimental version
#if __has_include(<filesystem>)
#include <filesystem>
namespace fs = std::filesystem;
#elif __has_include(<experimental/filesystem>)
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#endif

namespace mimir::parsers
{
    class DomainParser
    {
      private:
        const fs::path domain_path;

      public:
        DomainParser(const fs::path& domain_path);

        mimir::formalism::DomainDescription parse();

        static mimir::formalism::DomainDescription parse(std::istream& stream);
    };

    class ProblemParser
    {
      private:
        const fs::path problem_path;

      public:
        ProblemParser(const fs::path& problem_path);

        mimir::formalism::ProblemDescription parse(const mimir::formalism::DomainDescription& domain);

        static mimir::formalism::ProblemDescription parse(const mimir::formalism::DomainDescription& domain, const std::string& name, std::istream& stream);
    };
}  // namespace parsers

#endif  // MIMIR_PDDL_PARSER_DOMAIN_HPP_
