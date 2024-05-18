#include "mimir/search/states/dense.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/factories.hpp"

#include <flatmemory/flatmemory.hpp>
#include <ostream>
#include <tuple>

namespace mimir
{

std::ostream& operator<<(std::ostream& os, const std::tuple<DenseState, const PDDLFactories&>& data)
{
    const auto [state, pddl_factories] = data;
    auto out_ground_atoms = GroundAtomList {};
    pddl_factories.get_ground_atoms(state.get_atoms_bitset(), out_ground_atoms);

    os << "State("
       << "state_id=" << state.get_id() << ", "
       << "ground_atoms=" << out_ground_atoms << ")";

    return os;
}

}
