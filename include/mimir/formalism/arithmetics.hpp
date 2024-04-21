#ifndef MIMIR_FORMALISM_ARITHMETICS_HPP_
#define MIMIR_FORMALISM_ARITHMETICS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/loki.hpp>

namespace mimir
{

inline double evaluate_binary(const loki::BinaryOperatorEnum& op, const double val_left, const double val_right)
{
    switch (op)
    {
        case loki::BinaryOperatorEnum::DIV:
        {
            return val_left / val_right;
        }
        case loki::BinaryOperatorEnum::MINUS:
        {
            return val_left - val_right;
        }
        case loki::BinaryOperatorEnum::MUL:
        {
            return val_left * val_right;
        }
        case loki::BinaryOperatorEnum::PLUS:
        {
            return val_left + val_right;
        }
        default:
        {
            throw std::logic_error("Evaluation of binary operator is undefined.");
        }
    }
}

inline double evaluate_multi(const loki::MultiOperatorEnum& op, const double val_left, const double val_right)
{
    switch (op)
    {
        case loki::MultiOperatorEnum::MUL:
        {
            return val_left * val_right;
        }
        case loki::MultiOperatorEnum::PLUS:
        {
            return val_left + val_right;
        }
        default:
        {
            throw std::logic_error("Evaluation of multi operator is undefined.");
        }
    }
}

}

#endif
