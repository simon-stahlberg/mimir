#include "atom.hpp"
#include "factories.hpp"

namespace mimir::formalism
{
    Atom AtomFactory::get_or_add() { throw std::runtime_error("not implemented"); }

    // using TermFactory = loki::PersistentFactory<loki::pddl::ObjectImpl>;
    // using AtomFactory = loki::PersistentFactory<loki::pddl::AtomImpl>;
    // using ActionSchemaFactory = loki::PersistentFactory<loki::pddl::ActionImpl>;

}  // namespace mimir::formalism
