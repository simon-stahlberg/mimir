#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_

#include "mimir/formalism/declarations.hpp"
// Include to use our std::less for std::map
#include "mimir/formalism/parameter.hpp"
#include "mimir/formalism/variable.hpp"

#include <map>

namespace mimir
{

struct Assignment
{
    size_t parameter_index;
    size_t object_id;

    Assignment(size_t parameter_index, size_t object_id);
};

struct AssignmentPair
{
    size_t first_position;
    size_t second_position;
    Assignment first_assignment;
    Assignment second_assignment;

    AssignmentPair(size_t first_position, const Assignment& first_assignment, size_t second_position, const Assignment& second_assignment);
};

class ParameterIndexOrConstantId
{
private:
    size_t value;

public:
    ParameterIndexOrConstantId(size_t value, bool is_constant);

    bool is_constant() const;
    bool is_variable() const;
    size_t get_value() const;
};

size_t
get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects);

size_t num_assignments(int32_t arity, int32_t num_objects);

std::vector<std::vector<bool>> build_assignment_sets(Problem problem, const std::vector<size_t>& atom_identifiers, const PDDLFactories& factories);

bool literal_all_consistent(const std::vector<std::vector<bool>>& assignment_sets,
                            const std::vector<Literal>& literals,
                            const Assignment& first_assignment,
                            const Assignment& second_assignment,
                            Problem problem);

}

#endif  // MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_INTERNAL_REPRESENTATION_HPP_
