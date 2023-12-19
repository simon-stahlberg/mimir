#ifndef MIMIR_FORMALISM_DECLARATIONS_HPP_
#define MIMIR_FORMALISM_DECLARATIONS_HPP_

#include <memory>
#include <string>
#include <vector>

namespace mimir::formalism
{
    using ActionSchemaFactory = loki::PersistentFactory<loki::pddl::ActionImpl>;

    class TransitionImpl;
    using Transition = std::shared_ptr<TransitionImpl>;
    using TransitionList = std::vector<Transition>;
}  // namespace mimir::formalism

#endif  // MIMIR_FORMALISM_DECLARATIONS_HPP_
