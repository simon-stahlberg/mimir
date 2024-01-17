#ifndef MIMIR_FORMALISM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DECLARATIONS_HPP_

#include <vector>


namespace mimir {
    class ProblemImpl;
    using Problem = const ProblemImpl*;
    using ProblemList = std::vector<Problem>;
}

#endif