#if !defined(PDDL_PARSER_DOMAIN_HPP_)
#define PDDL_PARSER_DOMAIN_HPP_

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

namespace parsers
{
    class DomainParser
    {
      private:
        const fs::path domain_path;

      public:
        DomainParser(const fs::path& domain_path);

        formalism::DomainDescription parse();

        static formalism::DomainDescription parse(std::istream& stream);
    };

    class ProblemParser
    {
      private:
        const fs::path problem_path;

      public:
        ProblemParser(const fs::path& problem_path);

        formalism::ProblemDescription parse(const formalism::DomainDescription& domain);

        static formalism::ProblemDescription parse(const formalism::DomainDescription& domain, const std::string& name, std::istream& stream);
    };
}  // namespace parsers

#endif  // PDDL_PARSER_DOMAIN_HPP_
