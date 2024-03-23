#ifndef MIMIR_FORMALISM_TRANSLATORS_DELETE_RELAXATION_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_DELETE_RELAXATION_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

class DeleteRelaxedTranslator : public BaseTranslator<DeleteRelaxedTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<DeleteRelaxedTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;
};
}

#endif