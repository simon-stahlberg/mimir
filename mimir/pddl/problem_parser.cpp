/*
 * Copyright (C) 2023 Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "problem_parser.hpp"

#include "../formalism/action_schema.hpp"
#include "../formalism/domain.hpp"
#include "../formalism/object.hpp"
#include "../formalism/predicate.hpp"
#include "abstract_syntax_tree.hpp"
#include "parser_includes.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace parsers
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    namespace spirit = boost::spirit;
    namespace fusion = boost::fusion;

    struct PDDLProblemGrammar : public qi::grammar<std::string::iterator, ProblemNode*(), ascii::space_type>
    {
        // list of rules, the middle argument to the template is the return value of the grammar
        qi::rule<std::string::iterator, CharacterNode*()> ANY_CHAR;
        qi::rule<std::string::iterator, NameNode*()> NAME;
        qi::rule<std::string::iterator, TypeNode*(), ascii::space_type> TYPE;
        qi::rule<std::string::iterator, TermNode*(), ascii::space_type> TERM;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> TYPED_NAME_LIST;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> OBJECT_LIST;
        qi::rule<std::string::iterator, LiteralOrFunctionListNode*(), ascii::space_type> INITIAL_LIST;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> GOAL_LIST;
        qi::rule<std::string::iterator, AtomNode*(), ascii::space_type> ATOM;
        qi::rule<std::string::iterator, LiteralNode*(), ascii::space_type> LITERAL;
        qi::rule<std::string::iterator, FunctionNode*(), ascii::space_type> FUNCTION;
        qi::rule<std::string::iterator, LiteralOrFunctionNode*(), ascii::space_type> LITERAL_OR_FUNCTION;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> CONJUNCTIVE_LITERALS;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> CONDITION;
        qi::rule<std::string::iterator, AtomNode*(), ascii::space_type> METRIC;
        qi::rule<std::string::iterator, ProblemHeaderNode*(), ascii::space_type> PROBLEM_HEADER;
        qi::rule<std::string::iterator, ProblemNode*(), ascii::space_type> PROBLEM_DESCRIPTION;

        PDDLProblemGrammar() : PDDLProblemGrammar::base_type(PROBLEM_DESCRIPTION)
        {
            // include the following to make the grammar below more readable
            using ascii::alnum;
            using ascii::alpha;
            using ascii::char_;
            using ascii::string;
            using phoenix::at_c;
            using phoenix::construct;
            using phoenix::new_;
            using phoenix::val;
            using qi::_1;
            using qi::_2;
            using qi::_3;
            using qi::_4;
            using qi::_5;
            using qi::_6;
            using qi::_7;
            using qi::_8;
            using qi::_9;
            using qi::_val;
            using spirit::double_;
            using spirit::int_;
            using spirit::lit;

            // See the file grammar_domain.bnf for the full PDDL grammar.
            // The following grammar implement PDDL with STRIPS, typing and negative preconditions.

            // Names
            ANY_CHAR = alpha[_val = new_<CharacterNode>(_1)]          // alphabetical character
                       | alnum[_val = new_<CharacterNode>(_1)]        // alphanumerical character
                       | char_('-')[_val = new_<CharacterNode>(_1)]   // dash character
                       | char_('_')[_val = new_<CharacterNode>(_1)];  // underscore character

            NAME = (alpha >> *ANY_CHAR)[_val = new_<NameNode>(_1, _2)];  // a name must start with an alphabetical character

            TYPE = NAME[_val = new_<TypeNode>(_1)]                 // user specified type
                   | string("object")[_val = new_<TypeNode>(_1)];  // default type

            TERM = NAME[_val = new_<TermNode>(_1)];  // atoms expects terms in the domain parser

            // Lists
            TYPED_NAME_LIST = ((+NAME >> string("-") >> TYPE) > TYPED_NAME_LIST)[_val = new_<TypedNameListNode>(at_c<0>(_1), at_c<2>(_1), _2)]
                              | (*NAME)[_val = new_<TypedNameListNode>(_1)];  // untyped names, and typed names with recursion below

            // Literals

            ATOM = ((string("(") >> NAME) > *TERM >> string(")"))[_val = new_<AtomNode>(at_c<1>(_1), at_c<0>(_2))];

            LITERAL = ((string("(") >> string("not")) >> ATOM >> string(")"))[_val = new_<LiteralNode>(true, _3)]  // negated
                      | ATOM[_val = new_<LiteralNode>(false, _1)];                                                 // not negated

            FUNCTION = (string("(") >> string("=") >> ATOM >> double_
                        >> string(")"))[_val = new_<FunctionNode>(new_<NameNode>('=', std::vector<CharacterNode*>()), _3, _4)];

            LITERAL_OR_FUNCTION = LITERAL[_val = new_<LiteralOrFunctionNode>(_1)] | FUNCTION[_val = new_<LiteralOrFunctionNode>(_1)];

            CONJUNCTIVE_LITERALS = (((string("(") >> string("and")) > *LITERAL) >> string(")"))[_val = new_<LiteralListNode>(at_c<1>(_1))];

            CONDITION = CONJUNCTIVE_LITERALS[_val = _1]               // forward the conjunctive literals node
                        | LITERAL[_val = new_<LiteralListNode>(_1)];  // create a new conjunction of size 1

            // Objects
            OBJECT_LIST = ((string("(") >> string(":objects")) > TYPED_NAME_LIST >> string(")"))[_val = at_c<0>(_2)];

            INITIAL_LIST = ((string("(") >> string(":init")) > *LITERAL_OR_FUNCTION > string(")"))[_val = new_<LiteralOrFunctionListNode>(_2)];

            GOAL_LIST = (string("(") > string(":goal") > CONDITION > string(")"))[_val = _3];

            // Metric

            METRIC = ((string("(") >> string(":metric") >> string("minimize")) > ATOM > string(")"))[_val = _2];

            // Problem
            PROBLEM_HEADER = (string("(") > string("define") > string("(") > string("problem") > NAME > string(")") > string("(") > string(":domain") > NAME
                              > string(")"))[_val = new_<ProblemHeaderNode>(_5, _9)];

            PROBLEM_DESCRIPTION = ((PROBLEM_HEADER   // start problem definition
                                    >> -OBJECT_LIST  // optional objects
                                    >> INITIAL_LIST  // initial atoms
                                    >> GOAL_LIST     // goal atoms
                                    >> -METRIC)      // optimization metric
                                   > string(")"))[_val = new_<ProblemNode>(at_c<0>(_1),
                                                                           at_c<1>(_1),
                                                                           at_c<2>(_1),
                                                                           at_c<3>(_1),
                                                                           at_c<4>(_1))];  // end domain definition

            qi::on_error<qi::fail>(PROBLEM_DESCRIPTION,
                                   std::cout << val("Error while parsing the problem. Expected: \"") << _4  // what failed?
                                             << val("\" but got \"") << construct<std::string>(_3, _2)      // iterators to error-pos, end
                                             << val("\"") << std::endl);
        }
    };

    ProblemParser::ProblemParser(const fs::path& problem_path) : problem_path(problem_path) {}

    formalism::ProblemDescription ProblemParser::parse(const formalism::DomainDescription& domain)
    {
        if (fs::exists(this->problem_path))
        {
            std::ifstream problem_stream(this->problem_path.c_str());

            if (problem_stream.is_open())
            {
                std::stringstream buffer;
                buffer << problem_stream.rdbuf();
                problem_stream.close();
                std::string problem_content = buffer.str();
                const auto comments_regex = std::regex(";[^\n]*\n");
                problem_content = std::regex_replace(problem_content, comments_regex, "\n");
                boost::algorithm::to_lower(problem_content);
                PDDLProblemGrammar problem_grammar;
                auto iterator_begin = problem_content.begin();
                auto iterator_end = problem_content.end();
                ProblemNode* problem_node = nullptr;

                if (qi::phrase_parse(iterator_begin, iterator_end, problem_grammar, ascii::space, problem_node))
                {
                    const auto problem = problem_node->get_problem(problem_path.filename().string(), domain);
                    delete problem_node;
                    return problem;
                }
                else
                {
                    if (problem_node)
                    {
                        delete problem_node;
                    }

                    throw std::runtime_error("problem could not be parsed");
                }
            }
            else
            {
                throw std::invalid_argument("could not open problem file");
            }
        }

        throw std::invalid_argument("problem file does not exist");
    }
}  // namespace parsers
