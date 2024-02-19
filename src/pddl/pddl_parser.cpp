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

#include "../../include/mimir/formalism/action_schema.hpp"
#include "../../include/mimir/formalism/domain.hpp"
#include "../../include/mimir/formalism/object.hpp"
#include "../../include/mimir/formalism/predicate.hpp"
#include "../../include/mimir/pddl/abstract_syntax_tree.hpp"
#include "../../include/mimir/pddl/parser_includes.hpp"
#include "../../include/mimir/pddl/pddl_parser.hpp"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace mimir::parsers
{
    namespace ascii = boost::spirit::ascii;
    namespace phoenix = boost::phoenix;
    namespace qi = boost::spirit::qi;
    namespace spirit = boost::spirit;
    namespace fusion = boost::fusion;

    struct PDDLDomainGrammar : public qi::grammar<std::string::iterator, DomainNode*(), ascii::space_type>
    {
        // list of rules, the middle argument to the template is the return value of the grammar
        qi::rule<std::string::iterator, CharacterNode*()> ANY_CHAR;
        qi::rule<std::string::iterator, NameNode*()> NAME;
        qi::rule<std::string::iterator, NameNode*()> NAME_OR_EQUALITY;
        qi::rule<std::string::iterator, VariableNode*(), ascii::space_type> VARIABLE;
        qi::rule<std::string::iterator, TypeNode*(), ascii::space_type> TYPE;
        qi::rule<std::string::iterator, TermNode*(), ascii::space_type> TERM;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> TYPED_NAME_LIST;
        qi::rule<std::string::iterator, TypedVariableListNode*(), ascii::space_type> TYPED_VARIABLE_LIST;
        qi::rule<std::string::iterator, RequirementNode*(), ascii::space_type> REQUIREMENT;
        qi::rule<std::string::iterator, RequirementListNode*(), ascii::space_type> REQUIREMENT_LIST;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> TYPE_LIST;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> CONSTANT_LIST;
        qi::rule<std::string::iterator, PredicateNode*(), ascii::space_type> PREDICATE;
        qi::rule<std::string::iterator, PredicateListNode*(), ascii::space_type> PREDICATE_LIST;
        qi::rule<std::string::iterator, FunctionDeclarationNode*(), ascii::space_type> FUNCTION_DECLARATION;
        qi::rule<std::string::iterator, FunctionDeclarationListNode*(), ascii::space_type> FUNCTION_DECLARATION_LIST;
        qi::rule<std::string::iterator, FunctionNode*(), ascii::space_type> FUNCTION;
        qi::rule<std::string::iterator, ConditionalNode*(), ascii::space_type> CONDITIONAL;
        qi::rule<std::string::iterator, AtomNode*(), ascii::space_type> ATOM;
        qi::rule<std::string::iterator, LiteralNode*(), ascii::space_type> LITERAL;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> CONJUNCTIVE_LITERALS;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> ONE_OR_MORE_LITERALS;
        qi::rule<std::string::iterator, LiteralOrConditionalOrFunctionNode*(), ascii::space_type> LITERAL_OR_CONDITIONAL_OR_FUNCTION;
        qi::rule<std::string::iterator, LiteralOrConditionalOrFunctionListNode*(), ascii::space_type> CONJUNCTIVE_LITERAL_OR_CONDITIONAL_OR_FUNCTIONS;
        qi::rule<std::string::iterator, LiteralOrConditionalOrFunctionListNode*(), ascii::space_type> ONE_OR_MORE_LITERALS_CONDITIONALS_FUNCTIONS;
        qi::rule<std::string::iterator, ActionBodyNode*(), ascii::space_type> ACTION_BODY;
        qi::rule<std::string::iterator, ActionNode*(), ascii::space_type> ACTION;
        qi::rule<std::string::iterator, NameNode*(), ascii::space_type> DOMAIN_HEADER;
        qi::rule<std::string::iterator, DomainNode*(), ascii::space_type> DOMAIN_DESCRIPTION;

        PDDLDomainGrammar() : PDDLDomainGrammar::base_type(DOMAIN_DESCRIPTION)
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

            NAME_OR_EQUALITY = NAME[_val = _1]                                                             // normal name
                               | string("=")[_val = new_<NameNode>('=', std::vector<CharacterNode*> {})];  // equality '='

            VARIABLE = (char_('?') >> NAME)[_val = new_<VariableNode>(_2)];  // the symbol '?' indicates a variable, and is implicit

            TYPE = NAME[_val = new_<TypeNode>(_1)]                 // user specified type
                   | string("object")[_val = new_<TypeNode>(_1)];  // default type

            TERM = NAME[_val = new_<TermNode>(_1)]         // term is a constant
                   | VARIABLE[_val = new_<TermNode>(_1)];  // term is a variable

            // Lists
            TYPED_NAME_LIST = ((+NAME >> string("-") >> TYPE) > TYPED_NAME_LIST)[_val = new_<TypedNameListNode>(at_c<0>(_1), at_c<2>(_1), _2)]
                              | (*NAME)[_val = new_<TypedNameListNode>(_1)];  // untyped names, and typed names with recursion below

            TYPED_VARIABLE_LIST = ((+VARIABLE >> string("-") >> TYPE) > TYPED_VARIABLE_LIST)[_val = new_<TypedVariableListNode>(at_c<0>(_1), at_c<2>(_1), _2)]
                                  | (*VARIABLE)[_val = new_<TypedVariableListNode>(_1)];  // untyped variables, and typed variables with recursion below

            // Requirements
            REQUIREMENT = string(":strips")[_val = new_<RequirementNode>(_1)]                    // strips requirement
                          | string(":typing")[_val = new_<RequirementNode>(_1)]                  // typing requirement
                          | string(":action-costs")[_val = new_<RequirementNode>(_1)]            // action-costs requirement
                          | string(":negative-preconditions")[_val = new_<RequirementNode>(_1)]  // negative precondition requirement
                          | string(":conditional-effects")[_val = new_<RequirementNode>(_1)]     // conditional effects requirement
                          | string(":equality")[_val = new_<RequirementNode>(_1)];               // equality requirement

            REQUIREMENT_LIST = ((string("(") >> string(":requirements")) > *REQUIREMENT > string(")"))[_val = new_<RequirementListNode>(_2)];

            // Types
            TYPE_LIST = ((string("(") >> string(":types")) > TYPED_NAME_LIST >> string(")"))[_val = at_c<0>(_2)];

            // Constants
            CONSTANT_LIST = ((string("(") >> string(":constants")) > TYPED_NAME_LIST >> string(")"))[_val = at_c<0>(_2)];

            // Predicates
            PREDICATE = (string("(") > NAME > TYPED_VARIABLE_LIST > string(")"))[_val = new_<PredicateNode>(_2, _3)];

            PREDICATE_LIST = (string("(") > string(":predicates") > *PREDICATE > string(")"))[_val = new_<PredicateListNode>(_3)];

            FUNCTION_DECLARATION =
                (PREDICATE >> string("-") >> NAME)[_val = new_<FunctionDeclarationNode>(_1, _3)] | PREDICATE[_val = new_<FunctionDeclarationNode>(_1, nullptr)];

            FUNCTION_DECLARATION_LIST =
                ((string("(") >> string(":functions")) > *FUNCTION_DECLARATION > string(")"))[_val = new_<FunctionDeclarationListNode>(_2)];

            // Actions
            ATOM = ((string("(") >> NAME_OR_EQUALITY) > *TERM >> string(")"))[_val = new_<AtomNode>(at_c<1>(_1), at_c<0>(_2))];

            LITERAL = ((string("(") >> string("not")) >> ATOM >> string(")"))[_val = new_<LiteralNode>(true, _3)]  // negated
                      | ATOM[_val = new_<LiteralNode>(false, _1)];                                                 // not negated

            CONJUNCTIVE_LITERALS = (((string("(") >> string("and")) > *LITERAL) >> string(")"))[_val = new_<LiteralListNode>(at_c<1>(_1))];

            ONE_OR_MORE_LITERALS = CONJUNCTIVE_LITERALS[_val = _1]               // forward the conjunctive literals node
                                   | LITERAL[_val = new_<LiteralListNode>(_1)];  // create a new list of size 1

            FUNCTION = (string("(") >> NAME >> ATOM >> double_ >> string(")"))[_val = new_<FunctionNode>(_2, _3, _4)]  // constant value
                       | (string("(") >> NAME >> ATOM >> ATOM >> string(")"))[_val = new_<FunctionNode>(_2, _3, _4)];  // variable value

            CONDITIONAL = (string("(") >> string("when") >> ONE_OR_MORE_LITERALS >> ONE_OR_MORE_LITERALS >> string(")"))[_val = new_<ConditionalNode>(_3, _4)];

            LITERAL_OR_CONDITIONAL_OR_FUNCTION = ((string("(") >> string("not")) >> ATOM
                                                  >> string(")"))[_val = new_<LiteralOrConditionalOrFunctionNode>(new_<LiteralNode>(true, _3))]  // negated
                                                 | CONDITIONAL[_val = new_<LiteralOrConditionalOrFunctionNode>(_1)]                              // conditional
                                                 | FUNCTION[_val = new_<LiteralOrConditionalOrFunctionNode>(_1)]                                 // action-costs
                                                 | ATOM[_val = new_<LiteralOrConditionalOrFunctionNode>(new_<LiteralNode>(false, _1))];          // not negated

            CONJUNCTIVE_LITERAL_OR_CONDITIONAL_OR_FUNCTIONS = (((string("(") >> string("and")) > *LITERAL_OR_CONDITIONAL_OR_FUNCTION)
                                                               >> string(")"))[_val = new_<LiteralOrConditionalOrFunctionListNode>(at_c<1>(_1))];

            ONE_OR_MORE_LITERALS_CONDITIONALS_FUNCTIONS =
                CONJUNCTIVE_LITERAL_OR_CONDITIONAL_OR_FUNCTIONS[_val = _1]                                      // forward the conjunctive literal or functions
                | LITERAL_OR_CONDITIONAL_OR_FUNCTION[_val = new_<LiteralOrConditionalOrFunctionListNode>(_1)];  // create a new list of size 1

            ACTION_BODY = (-(string(":precondition") > ONE_OR_MORE_LITERALS)
                           > -(string(":effect") > ONE_OR_MORE_LITERALS_CONDITIONALS_FUNCTIONS))[_val = new_<ActionBodyNode>(_1, _2)];

            ACTION = ((string("(") >> string(":action")) > NAME > string(":parameters") > string("(") > TYPED_VARIABLE_LIST > string(")") > ACTION_BODY
                      > string(")"))[_val = new_<ActionNode>(_2, _5, _7)];

            // Domain
            DOMAIN_HEADER = (string("(") > string("define") > string("(") > string("domain") > NAME > string(")"))[_val = _5];

            DOMAIN_DESCRIPTION = ((DOMAIN_HEADER                  // start domain definition
                                   >> -REQUIREMENT_LIST           // optional requirements
                                   >> -TYPE_LIST                  // optional types
                                   >> -CONSTANT_LIST              // optional constants
                                   >> -PREDICATE_LIST             // optional predicates
                                   >> -FUNCTION_DECLARATION_LIST  // optional functions
                                   >> *ACTION)                    // action list
                                  > string(")"))[_val = new_<DomainNode>(at_c<0>(_1),
                                                                         at_c<1>(_1),
                                                                         at_c<2>(_1),
                                                                         at_c<3>(_1),
                                                                         at_c<4>(_1),
                                                                         at_c<5>(_1),
                                                                         at_c<6>(_1))];  // end domain definition

            qi::on_error<qi::fail>(DOMAIN_DESCRIPTION,
                                   std::cout << val("Error while parsing the domain. Expected: \"") << _4  // what failed?
                                             << val("\" but got \"") << construct<std::string>(_3, _2)     // iterators to error-pos, end
                                             << val("\"") << std::endl);
        }
    };

    DomainParser::DomainParser(const fs::path& domain_path) : domain_path(domain_path) {}

    mimir::formalism::DomainDescription DomainParser::parse()
    {
        if (!fs::exists(this->domain_path))
        {
            throw std::invalid_argument("domain file does not exist (" + domain_path.string() + ")");
        }

        std::ifstream stream(this->domain_path.c_str());

        if (!stream.is_open())
        {
            throw std::invalid_argument("could not open domain file");
        }

        return parse(stream);
    }

    mimir::formalism::DomainDescription DomainParser::parse(std::istream& stream)
    {
        std::stringstream buffer;
        buffer << stream.rdbuf();
        std::string domain_content = buffer.str();
        const auto comments_regex = std::regex(";[^\n]*\n");
        domain_content = std::regex_replace(domain_content, comments_regex, "\n");
        boost::algorithm::to_lower(domain_content);
        PDDLDomainGrammar domain_grammar;
        auto iterator_begin = domain_content.begin();
        auto iterator_end = domain_content.end();
        DomainNode* domain_node = nullptr;

        if (qi::phrase_parse(iterator_begin, iterator_end, domain_grammar, ascii::space, domain_node))
        {
            const auto domain = domain_node->get_domain();
            delete domain_node;
            return domain;
        }
        else
        {
            if (domain_node)
            {
                delete domain_node;
            }

            throw std::runtime_error("domain could not be parsed");
        }
    }

    struct PDDLProblemGrammar : public qi::grammar<std::string::iterator, ProblemNode*(), ascii::space_type>
    {
        // list of rules, the middle argument to the template is the return value of the grammar
        qi::rule<std::string::iterator, CharacterNode*()> ANY_CHAR;
        qi::rule<std::string::iterator, NameNode*()> NAME;
        qi::rule<std::string::iterator, TypeNode*(), ascii::space_type> TYPE;
        qi::rule<std::string::iterator, TermNode*(), ascii::space_type> TERM;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> TYPED_NAME_LIST;
        qi::rule<std::string::iterator, TypedNameListNode*(), ascii::space_type> OBJECT_LIST;
        qi::rule<std::string::iterator, LiteralOrConditionalOrFunctionListNode*(), ascii::space_type> INITIAL_LIST;
        qi::rule<std::string::iterator, LiteralListNode*(), ascii::space_type> GOAL_LIST;
        qi::rule<std::string::iterator, AtomNode*(), ascii::space_type> ATOM;
        qi::rule<std::string::iterator, LiteralNode*(), ascii::space_type> LITERAL;
        qi::rule<std::string::iterator, FunctionNode*(), ascii::space_type> FUNCTION;
        qi::rule<std::string::iterator, LiteralOrConditionalOrFunctionNode*(), ascii::space_type> LITERAL_OR_FUNCTION;
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

            LITERAL_OR_FUNCTION = LITERAL[_val = new_<LiteralOrConditionalOrFunctionNode>(_1)] | FUNCTION[_val = new_<LiteralOrConditionalOrFunctionNode>(_1)];

            CONJUNCTIVE_LITERALS = (((string("(") >> string("and")) > *LITERAL) >> string(")"))[_val = new_<LiteralListNode>(at_c<1>(_1))];

            CONDITION = CONJUNCTIVE_LITERALS[_val = _1]               // forward the conjunctive literals node
                        | LITERAL[_val = new_<LiteralListNode>(_1)];  // create a new conjunction of size 1

            // Objects
            OBJECT_LIST = ((string("(") >> string(":objects")) > TYPED_NAME_LIST >> string(")"))[_val = at_c<0>(_2)];

            INITIAL_LIST = ((string("(") >> string(":init")) > *LITERAL_OR_FUNCTION > string(")"))[_val = new_<LiteralOrConditionalOrFunctionListNode>(_2)];

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

    mimir::formalism::ProblemDescription ProblemParser::parse(const mimir::formalism::DomainDescription& domain)
    {
        if (!fs::exists(this->problem_path))
        {
            throw std::invalid_argument("problem file does not exist (" + problem_path.string() + ")");
        }

        std::ifstream stream(this->problem_path.c_str());

        if (!stream.is_open())
        {
            throw std::invalid_argument("could not open problem file");
        }

        auto problem = parse(domain, problem_path.filename().string(), stream);
        problem->set_path(problem_path);
        return problem;
    }

    mimir::formalism::ProblemDescription ProblemParser::parse(const mimir::formalism::DomainDescription& domain, const std::string& name, std::istream& stream)
    {
        std::stringstream buffer;
        buffer << stream.rdbuf();
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
            const auto problem = problem_node->get_problem(name, domain);
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
}  // namespace parsers
