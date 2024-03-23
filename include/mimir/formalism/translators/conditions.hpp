#ifndef MIMIR_FORMALISM_TRANSLATORS_CONDITIONS_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_CONDITIONS_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{
class ConjunctiveConditionsTranslator : public BaseTranslator<ConjunctiveConditionsTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<ConjunctiveConditionsTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;
};

}

#endif