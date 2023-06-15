#if !defined(PARSERS_PARSER_PROBLEM_HPP_)
#define PARSERS_PARSER_PROBLEM_HPP_

#include "../formalism/domain.hpp"
#include "../formalism/problem.hpp"

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
    class ProblemParser
    {
      private:
        const fs::path problem_path;

      public:
        ProblemParser(const fs::path& problem_path);

        formalism::ProblemDescription parse(const formalism::DomainDescription& domain);
    };
}  // namespace parsers

#endif  // PARSERS_PARSER_PROBLEM_HPP_
