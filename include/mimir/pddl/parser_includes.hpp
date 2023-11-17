#ifndef MIMIR_PARSERS_PARSER_INCLUDES_HPP_
#define MIMIR_PARSERS_PARSER_INCLUDES_HPP_

// MSVC produces a linker error when they are included from both domain_parser.cpp and problem_parser.cpp. To resolve this, include this file in both instead.

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/phoenix.hpp>
#include <boost/phoenix/object.hpp>
#include <boost/phoenix/operator.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_numeric.hpp>
#include <boost/spirit/include/qi_operator.hpp>

#endif  // MIMIR_PARSERS_PARSER_INCLUDES_HPP_
