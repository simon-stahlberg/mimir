#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_

#include "mimir/formalism/declarations.hpp"
// Include to use our std::less for std::map
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/variable.hpp"

#include <map>

namespace mimir
{

// D: [x/o]
// D:
struct Assignment
{
    size_t parameter_index;
    size_t object_id;

    Assignment(size_t parameter_index, size_t object_id);
};

// D: ([x/o], [x'/o'])
struct AssignmentPair
{
    size_t first_position;
    size_t second_position;
    Assignment first_assignment;
    Assignment second_assignment;

    AssignmentPair(size_t first_position, const Assignment& first_assignment, size_t second_position, const Assignment& second_assignment);
};

}

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
