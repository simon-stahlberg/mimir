#ifndef MIMIR_FORMALISM_TRANSLATORS_EFFECT_NORMAL_FORM_HPP_
#define MIMIR_FORMALISM_TRANSLATORS_EFFECT_NORMAL_FORM_HPP_

#include "mimir/formalism/translators/base.hpp"

namespace mimir
{

class EffectNormalFormTranslator : public BaseTranslator<EffectNormalFormTranslator>
{
private:
    /* Implement BaseTranslator interface. */
    friend class BaseTranslator<EffectNormalFormTranslator>;

    // Provide default implementations
    using BaseTranslator::prepare_impl;
    using BaseTranslator::translate_impl;
};
}

#endif
