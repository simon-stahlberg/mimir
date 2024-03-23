#ifndef MIMIR_FORMALISM_TRANSLATORS_POSITIVE_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_POSITIVE_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

class PositiveNormalFormTranslator : public BaseTranslator<PositiveNormalFormTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<PositiveNormalFormTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;
};
}

#endif