#ifndef MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_TYPES_HPP_

#include "mimir/formalism/translators/base.hpp"

class TypeTranslator : public BaseTranslator<TypeTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<TypeTranslator>;
};

#endif
