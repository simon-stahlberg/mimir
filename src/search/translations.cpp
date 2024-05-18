#include "mimir/search/translations.hpp"

#include "mimir/formalism/ground_literal.hpp"

namespace mimir
{

void to_ground_atoms(const GroundLiteralList& literals, GroundAtomList& out_ground_atoms)
{
    out_ground_atoms.clear();

    for (const auto& literal : literals)
    {
        out_ground_atoms.push_back(literal->get_atom());
    }
}

}
