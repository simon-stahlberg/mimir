#ifndef MIMIR_FORMALISM_FACTORIES_HPP_
#define MIMIR_FORMALISM_FACTORIES_HPP_

#include <loki/common/pddl/persistent_factory.hpp>
#include <loki/domain/pddl/action.hpp>
#include <loki/domain/pddl/atom.hpp>
#include <loki/domain/pddl/object.hpp>

namespace mimir::formalism
{
    class AtomFactory
    {
      private:
        loki::PersistentFactory<loki::pddl::AtomImpl, 1000> external_;

      public:
        Atom get_or_add();
    };

    class TermFactory
    {
      private:
        loki::PersistentFactory<loki::pddl::ObjectImpl, 1000> external_;

      public:
        Term get_or_add();
    };

    class ActionSchemaFactory
    {
      private:
        loki::PersistentFactory<loki::pddl::ActionImpl, 100> external_;

      public:
        ActionSchema get_or_add();
    };

}  // namespace mimir::formalism

#endif  // MIMIR_FORMALISM_FACTORIES_HPP_
