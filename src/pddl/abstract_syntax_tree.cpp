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

#include "../../include/mimir/pddl/abstract_syntax_tree.hpp"

#include <boost/fusion/include/at_c.hpp>

namespace mimir::parsers
{
    /* Help functions */

    std::string to_lowercase(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    /* ASTNode */

    ASTNode::~ASTNode() {};

    /* CharacterNode */

    CharacterNode::CharacterNode(char character) : character(character) {}

    /* NameNode */

    NameNode::NameNode(char prefix, const std::vector<CharacterNode*>& character_nodes) : prefix(prefix), character_nodes(character_nodes) {}

    NameNode::~NameNode()
    {
        for (auto node : character_nodes)
        {
            if (node)
            {
                delete node;
            }
        }

        character_nodes.clear();
    }

    std::string NameNode::get_name() const
    {
        std::vector<char> characters;
        characters.push_back(prefix);

        for (auto node : character_nodes)
        {
            characters.push_back(node->character);
        }

        return std::string(characters.begin(), characters.end());
    }

    /* VariableNode */

    VariableNode::VariableNode(NameNode* name_node) : name_node(name_node) {}

    VariableNode::~VariableNode()
    {
        if (name_node)
        {
            delete name_node;
            name_node = nullptr;
        }
    }

    std::string VariableNode::get_variable() const { return "?" + name_node->get_name(); }

    /* TypeNode */

    TypeNode::TypeNode(std::string type_name) : name_string(type_name), name_node(nullptr) {}

    TypeNode::TypeNode(NameNode* type_name) : name_string(), name_node(type_name) {}

    TypeNode::~TypeNode()
    {
        if (name_node)
        {
            delete name_node;
            name_node = nullptr;
        }
    }

    std::string TypeNode::get_type() const
    {
        if (name_node)
        {
            return name_node->get_name();
        }
        else
        {
            return name_string;
        }
    }

    /* TermNode */

    TermNode::TermNode(NameNode* node) : name_node(node), variable_node(nullptr) {}

    TermNode::TermNode(VariableNode* node) : name_node(nullptr), variable_node(node) {}

    TermNode::~TermNode()
    {
        if (name_node)
        {
            delete name_node;
            name_node = nullptr;
        }

        if (variable_node)
        {
            delete variable_node;
            variable_node = nullptr;
        }
    }

    mimir::formalism::Object TermNode::get_term(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                const std::map<std::string, mimir::formalism::Object>& constants) const
    {
        std::string name;

        if (name_node)
        {
            name = name_node->get_name();
        }
        else if (variable_node)
        {
            name = variable_node->get_variable();
        }
        else
        {
            throw std::runtime_error("internal error: both name_node and variable_node are null");
        }

        // TODO: Check parameters and then constants

        const auto parameters_handler = parameters.find(name);
        if (parameters_handler != parameters.end())
        {
            return parameters_handler->second;
        }

        const auto constants_handler = constants.find(name);
        if (constants_handler != constants.end())
        {
            return constants_handler->second;
        }

        throw std::invalid_argument("the argument \"" + name + "\" is undefined");
    }

    /* AtomNode */

    AtomNode::AtomNode(NameNode* name, std::vector<TermNode*>& arguments) : name(name), arguments(arguments) {}

    AtomNode::~AtomNode()
    {
        if (name)
        {
            delete name;
            name = nullptr;
        }

        for (auto node : arguments)
        {
            delete node;
        }
        arguments.clear();
    }

    mimir::formalism::Atom AtomNode::get_atom(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                              const std::map<std::string, mimir::formalism::Object>& constants,
                                              const std::map<std::string, mimir::formalism::Predicate>& predicates) const
    {
        const auto atom_name = name->get_name();
        const auto predicate_handler = predicates.find(atom_name);

        if (predicate_handler != predicates.end())
        {
            const auto predicate = predicate_handler->second;
            mimir::formalism::ObjectList atom_arguments;

            for (const auto node : arguments)
            {
                atom_arguments.push_back(node->get_term(parameters, constants));
            }

            return mimir::formalism::create_atom(predicate, atom_arguments);
        }
        else
        {
            throw std::invalid_argument("the predicate of the atom \"" + atom_name + "\" is undefined");
        }
    }

    /* TypedNameListNode */

    TypedNameListNode::TypedNameListNode(std::vector<NameNode*>& untyped_names) : untyped_names(untyped_names), typed_names(), type(nullptr), recursion(nullptr)
    {
    }

    TypedNameListNode::TypedNameListNode(std::vector<NameNode*>& typed_names, TypeNode* type, TypedNameListNode* recursion) :
        untyped_names(),
        typed_names(typed_names),
        type(type),
        recursion(recursion)
    {
    }

    TypedNameListNode::~TypedNameListNode()
    {
        for (auto name : untyped_names)
        {
            delete name;
        }
        untyped_names.clear();

        for (auto name : typed_names)
        {
            delete name;
        }
        typed_names.clear();

        if (type)
        {
            delete type;
            type = nullptr;
        }

        if (recursion)
        {
            delete recursion;
            recursion = nullptr;
        }
    }

    std::vector<std::pair<std::string, std::string>> TypedNameListNode::get_typed_names() const
    {
        if ((typed_names.size() > 0) && (type == nullptr))
        {
            throw std::runtime_error("internal typed_names is not empty while type is null");
        }

        std::vector<std::pair<std::string, std::string>> result;

        for (auto name : untyped_names)
        {
            result.push_back(std::make_pair(name->get_name(), "object"));
        }

        if (type)
        {
            auto base = type->get_type();

            for (auto name : typed_names)
            {
                result.push_back(std::make_pair(name->get_name(), base));
            }
        }

        if (recursion)
        {
            auto recursive_result = recursion->get_typed_names();
            result.insert(result.end(), recursive_result.begin(), recursive_result.end());
        }

        return result;
    }

    /* TypedVariableListNode */

    TypedVariableListNode::TypedVariableListNode(std::vector<VariableNode*>& untyped_names) :
        untyped_names(untyped_names),
        typed_names(),
        type(nullptr),
        recursion(nullptr)
    {
    }

    TypedVariableListNode::TypedVariableListNode(std::vector<VariableNode*>& typed_names, TypeNode* type, TypedVariableListNode* recursion) :
        untyped_names(),
        typed_names(typed_names),
        type(type),
        recursion(recursion)
    {
    }

    TypedVariableListNode::~TypedVariableListNode()
    {
        for (auto name : untyped_names)
        {
            delete name;
        }
        untyped_names.clear();

        for (auto name : typed_names)
        {
            delete name;
        }
        typed_names.clear();

        if (type)
        {
            delete type;
            type = nullptr;
        }

        if (recursion)
        {
            delete recursion;
            recursion = nullptr;
        }
    }

    std::vector<std::pair<std::string, std::string>> TypedVariableListNode::get_typed_variables() const
    {
        if ((typed_names.size() > 0) && (type == nullptr))
        {
            throw std::runtime_error("internal typed_names is not empty while type is null");
        }

        std::vector<std::pair<std::string, std::string>> result;

        for (auto name : untyped_names)
        {
            result.push_back(std::make_pair(name->get_variable(), "object"));
        }

        if (type)
        {
            auto base = type->get_type();

            for (auto name : typed_names)
            {
                result.push_back(std::make_pair(name->get_variable(), base));
            }
        }

        if (recursion)
        {
            auto recursive_result = recursion->get_typed_variables();
            result.insert(result.end(), recursive_result.begin(), recursive_result.end());
        }

        return result;
    }

    /* RequirementNode */

    RequirementNode::RequirementNode(mimir::formalism::Requirement requirement) : requirement(requirement) {}

    mimir::formalism::Requirement RequirementNode::get_requirement() const { return requirement; }

    /* RequirementListNode */

    RequirementListNode::RequirementListNode(std::vector<RequirementNode*>& requirements) : requirements(requirements) {}

    RequirementListNode::~RequirementListNode()
    {
        for (auto node : requirements)
        {
            delete node;
        }

        requirements.clear();
    }

    mimir::formalism::RequirementList RequirementListNode::get_requirements() const
    {
        mimir::formalism::RequirementList result;

        for (auto node : requirements)
        {
            result.push_back(node->get_requirement());
        }

        return result;
    }

    /* PredicateNode */

    PredicateNode::PredicateNode(NameNode* name, TypedVariableListNode* parameters) : name(name), parameters(parameters) {}

    PredicateNode::~PredicateNode()
    {
        if (name)
        {
            delete name;
            name = nullptr;
        }

        if (parameters)
        {
            delete parameters;
            parameters = nullptr;
        }
    }

    mimir::formalism::Predicate PredicateNode::get_predicate(const uint32_t id, const std::map<std::string, mimir::formalism::Type>& types) const
    {
        const auto predicate_name = name->get_name();
        const auto typed_variables = parameters->get_typed_variables();
        mimir::formalism::ObjectList predicate_parameters;

        uint32_t obj_id = 0;

        for (const auto& parameter : typed_variables)
        {
            const auto parameter_name = parameter.first;
            const auto parameter_type_handler = types.find(parameter.second);

            if (parameter_type_handler != types.end())
            {
                const auto parameter_type = parameter_type_handler->second;
                predicate_parameters.push_back(mimir::formalism::create_object(obj_id++, parameter_name, parameter_type));
            }
            else
            {
                throw std::invalid_argument("the type of parameter \"" + parameter_name + "\" is undefined");
            }
        }

        return mimir::formalism::create_predicate(id, name->get_name(), predicate_parameters);
    }

    /* PredicateListNode */

    PredicateListNode::PredicateListNode(std::vector<PredicateNode*>& predicates) : predicates(predicates) {}

    PredicateListNode::~PredicateListNode()
    {
        for (auto node : predicates)
        {
            delete node;
        }
        predicates.clear();
    }

    mimir::formalism::PredicateList PredicateListNode::get_predicates(const mimir::formalism::RequirementList& requirements,
                                                                      const std::map<std::string, mimir::formalism::Type>& types) const
    {
        mimir::formalism::PredicateList result;
        uint32_t pred_id = 0;

        for (const auto predicate_node : predicates)
        {
            result.push_back(predicate_node->get_predicate(pred_id++, types));
        }

        if (std::count(requirements.cbegin(), requirements.cend(), ":equality"))
        {
            const auto lhs = mimir::formalism::create_object(0, "?lhs", types.at("object"));
            const auto rhs = mimir::formalism::create_object(1, "?rhs", types.at("object"));
            result.push_back(mimir::formalism::create_predicate(pred_id, "=", { lhs, rhs }));
        }

        return result;
    }

    /* FunctionDeclarationNode */

    FunctionDeclarationNode::FunctionDeclarationNode(PredicateNode* predicate, NameNode* type) : predicate(predicate), type(type) {}

    FunctionDeclarationNode::~FunctionDeclarationNode()
    {
        if (predicate)
        {
            delete predicate;
            predicate = nullptr;
        }

        if (type)
        {
            delete type;
            type = nullptr;
        }
    }

    mimir::formalism::Predicate FunctionDeclarationNode::get_function(const uint32_t id, const std::map<std::string, mimir::formalism::Type>& types) const
    {
        if (type && to_lowercase(type->get_name()) != "number")
        {
            throw std::invalid_argument("unsupported function output type");
        }

        return predicate->get_predicate(id, types);
    }

    /* FunctionDeclarationListNode */

    FunctionDeclarationListNode::FunctionDeclarationListNode(std::vector<FunctionDeclarationNode*>& functions) : functions(functions) {}

    FunctionDeclarationListNode::~FunctionDeclarationListNode()
    {
        for (auto node : functions)
        {
            delete node;
        }
        functions.clear();
    }

    mimir::formalism::PredicateList FunctionDeclarationListNode::get_functions(const std::map<std::string, mimir::formalism::Type>& types) const
    {
        mimir::formalism::PredicateList result;
        uint32_t func_id = 0;

        for (const auto predicate_node : functions)
        {
            result.push_back(predicate_node->get_function(func_id++, types));
        }

        return result;
    }

    /* FunctionNode */

    FunctionNode::FunctionNode(NameNode* op, AtomNode* first_operand, AtomNode* second_operand) :
        op(op),
        first_operand_atom(first_operand),
        second_operand_atom(second_operand),
        second_operand_value(0.0)
    {
    }

    FunctionNode::FunctionNode(NameNode* op, AtomNode* first_operand, double second_operand) :
        op(op),
        first_operand_atom(first_operand),
        second_operand_atom(nullptr),
        second_operand_value(second_operand)
    {
    }

    FunctionNode::~FunctionNode()
    {
        if (op)
        {
            delete op;
            op = nullptr;
        }

        if (first_operand_atom)
        {
            delete first_operand_atom;
            first_operand_atom = nullptr;
        }

        if (second_operand_atom)
        {
            delete second_operand_atom;
            second_operand_atom = nullptr;
        }
    }

    /* ConditionalNode */

    ConditionalNode::ConditionalNode(LiteralListNode* antecedent, LiteralListNode* consequence) : antecedent(antecedent), consequence(consequence) {}

    ConditionalNode::~ConditionalNode()
    {
        if (antecedent)
        {
            delete antecedent;
            antecedent = nullptr;
        }

        if (consequence)
        {
            delete consequence;
            consequence = nullptr;
        }
    }

    /* LiteralNode */

    LiteralNode::LiteralNode(bool negated, AtomNode* atom) : negated(negated), atom(atom) {}

    LiteralNode::~LiteralNode()
    {
        delete atom;
        atom = nullptr;
    }

    mimir::formalism::Literal LiteralNode::get_literal(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                       const std::map<std::string, mimir::formalism::Object>& constants,
                                                       const std::map<std::string, mimir::formalism::Predicate>& predicates) const
    {
        const auto literal_atom = atom->get_atom(parameters, constants, predicates);
        return mimir::formalism::create_literal(literal_atom, negated);
    }

    /* LiteralListNode */

    LiteralListNode::LiteralListNode(LiteralNode* literal) : literals() { literals.push_back(literal); }

    LiteralListNode::LiteralListNode(std::vector<LiteralNode*>& literals) : literals(literals) {}

    LiteralListNode::~LiteralListNode()
    {
        for (auto atom : literals)
        {
            delete atom;
        }
        literals.clear();
    }

    mimir::formalism::LiteralList LiteralListNode::get_literals(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                                const std::map<std::string, mimir::formalism::Object>& constants,
                                                                const std::map<std::string, mimir::formalism::Predicate>& predicates) const
    {
        mimir::formalism::LiteralList result;

        for (const auto node : literals)
        {
            result.push_back(node->get_literal(parameters, constants, predicates));
        }

        return result;
    }

    /* LiteralOrConditionalOrFunctionNode */

    LiteralOrConditionalOrFunctionNode::LiteralOrConditionalOrFunctionNode(LiteralNode* literal_node) :
        literal_node(literal_node),
        function_node(nullptr),
        conditional_node(nullptr)
    {
    }

    LiteralOrConditionalOrFunctionNode::LiteralOrConditionalOrFunctionNode(FunctionNode* function_node) :
        literal_node(nullptr),
        function_node(function_node),
        conditional_node(nullptr)
    {
    }

    LiteralOrConditionalOrFunctionNode::LiteralOrConditionalOrFunctionNode(ConditionalNode* conditional_node) :
        literal_node(nullptr),
        function_node(nullptr),
        conditional_node(conditional_node)
    {
    }

    LiteralOrConditionalOrFunctionNode::~LiteralOrConditionalOrFunctionNode()
    {
        if (literal_node)
        {
            delete literal_node;
            literal_node = nullptr;
        }

        if (function_node)
        {
            delete function_node;
            function_node = nullptr;
        }
    }

    /* LiteralOrConditionalOrFunctionListNode */

    LiteralOrConditionalOrFunctionListNode::LiteralOrConditionalOrFunctionListNode(LiteralOrConditionalOrFunctionNode* literal_or_function) :
        literal_or_functions()
    {
        literal_or_functions.push_back(literal_or_function);
    }

    LiteralOrConditionalOrFunctionListNode::LiteralOrConditionalOrFunctionListNode(std::vector<LiteralOrConditionalOrFunctionNode*>& literal_or_functions) :
        literal_or_functions(literal_or_functions)
    {
    }

    LiteralOrConditionalOrFunctionListNode::~LiteralOrConditionalOrFunctionListNode()
    {
        for (auto literal_or_function : literal_or_functions)
        {
            delete literal_or_function;
        }
        literal_or_functions.clear();
    }

    mimir::formalism::LiteralList
    LiteralOrConditionalOrFunctionListNode::get_literals(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                         const std::map<std::string, mimir::formalism::Object>& constants,
                                                         const std::map<std::string, mimir::formalism::Predicate>& predicates) const
    {
        mimir::formalism::LiteralList result;

        for (const auto node : literal_or_functions)
        {
            if (node->literal_node)
            {
                result.push_back(node->literal_node->get_literal(parameters, constants, predicates));
            }
        }

        return result;
    }

    mimir::formalism::ImplicationList
    LiteralOrConditionalOrFunctionListNode::get_conditionals(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                             const std::map<std::string, mimir::formalism::Object>& constants,
                                                             const std::map<std::string, mimir::formalism::Predicate>& predicates) const
    {
        mimir::formalism::ImplicationList result;

        for (const auto node : this->literal_or_functions)
        {
            if (node->conditional_node)
            {
                const auto antecedent = node->conditional_node->antecedent->get_literals(parameters, constants, predicates);
                const auto consequence = node->conditional_node->consequence->get_literals(parameters, constants, predicates);
                result.push_back(mimir::formalism::Implication(std::move(antecedent), std::move(consequence)));
            }
        }

        return result;
    }

    mimir::formalism::FunctionList
    LiteralOrConditionalOrFunctionListNode::get_functions(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                          const std::map<std::string, mimir::formalism::Object>& constants,
                                                          const std::map<std::string, mimir::formalism::Predicate>& functions) const
    {
        mimir::formalism::FunctionList result;

        for (const auto node : literal_or_functions)
        {
            const auto& function = node->function_node;

            if (function)
            {
                const auto operation_name = to_lowercase(function->op->get_name());
                mimir::formalism::FunctionOperation operation;

                if (operation_name == "increase")
                {
                    operation = mimir::formalism::FunctionOperation::INCREASE;
                }
                else if (operation_name == "decrease")
                {
                    operation = mimir::formalism::FunctionOperation::DECREASE;
                }
                else
                {
                    throw std::invalid_argument("unsupported function operation type");
                }

                const auto variable = function->first_operand_atom->get_atom(parameters, constants, functions)->predicate;

                if (variable->arity != 0)
                {
                    throw std::invalid_argument("variable is not nullary");
                }

                if (function->second_operand_atom)
                {
                    const auto atom = function->second_operand_atom->get_atom(parameters, constants, functions);
                    result.emplace_back(mimir::formalism::create_function(operation, variable, atom));
                }
                else
                {
                    const auto constant = function->second_operand_value;
                    result.emplace_back(mimir::formalism::create_function(operation, variable, constant));
                }
            }
        }

        return result;
    }

    /* ActionBodyNode */

    ActionBodyNode::ActionBodyNode(boost::optional<boost::fusion::vector<std::string, LiteralListNode*>>& precondition,
                                   boost::optional<boost::fusion::vector<std::string, LiteralOrConditionalOrFunctionListNode*>>& effect) :
        precondition(nullptr),
        effect(nullptr)
    {
        if (precondition)
        {
            this->precondition = boost::fusion::at_c<1>(precondition.value());
        }

        if (effect)
        {
            this->effect = boost::fusion::at_c<1>(effect.value());
        }
    }

    ActionBodyNode::ActionBodyNode(LiteralListNode* precondition, LiteralOrConditionalOrFunctionListNode* effect) : precondition(precondition), effect(effect)
    {
    }

    ActionBodyNode::~ActionBodyNode()
    {
        if (precondition)
        {
            delete precondition;
            precondition = nullptr;
        }

        if (effect)
        {
            delete effect;
            effect = nullptr;
        }
    }

    std::tuple<mimir::formalism::LiteralList, mimir::formalism::LiteralList, mimir::formalism::ImplicationList, mimir::formalism::Function>
    ActionBodyNode::get_precondition_effect_cost(const std::map<std::string, mimir::formalism::Parameter>& parameters,
                                                 const std::map<std::string, mimir::formalism::Object>& constants,
                                                 const std::map<std::string, mimir::formalism::Predicate>& predicates,
                                                 const std::map<std::string, mimir::formalism::Predicate>& functions) const
    {
        const auto precondition_literals = precondition->get_literals(parameters, constants, predicates);
        const auto effect_literals = effect->get_literals(parameters, constants, predicates);
        const auto effect_functions = effect->get_functions(parameters, constants, functions);
        const auto effect_conditional = effect->get_conditionals(parameters, constants, predicates);
        mimir::formalism::Function cost_function = nullptr;

        if (effect_functions.size() > 1)
        {
            throw std::invalid_argument("only one cost function is supported");
        }
        else if (effect_functions.size() == 1)
        {
            cost_function = effect_functions.at(0);
        }
        else if (functions.count("total-cost"))
        {
            cost_function = create_function(mimir::formalism::FunctionOperation::INCREASE, functions.at("total-cost"), 1.0);
        }

        if (!cost_function)
        {
            throw std::invalid_argument("missing cost function; can't implicitily define unit cost without \"total-cost\" function");
        }

        return std::make_tuple(precondition_literals, effect_literals, effect_conditional, cost_function);
    }

    /* ActionNode */

    ActionNode::ActionNode(NameNode* name, TypedVariableListNode* parameters, ActionBodyNode* body) : name(name), parameters(parameters), body(body) {}

    ActionNode::~ActionNode()
    {
        if (name)
        {
            delete name;
            name = nullptr;
        }

        if (parameters)
        {
            delete parameters;
            parameters = nullptr;
        }

        if (body)
        {
            delete body;
            body = nullptr;
        }
    }

    mimir::formalism::ActionSchema ActionNode::get_action(const std::map<std::string, mimir::formalism::Type>& types,
                                                          const std::map<std::string, mimir::formalism::Object>& constants,
                                                          const std::map<std::string, mimir::formalism::Predicate>& predicates,
                                                          const std::map<std::string, mimir::formalism::Predicate>& functions) const
    {
        const auto action_name = name->get_name();

        mimir::formalism::ParameterList action_parameter_list;
        std::map<std::string, mimir::formalism::Parameter> action_parameter_map;
        const auto typed_parameters = parameters->get_typed_variables();
        uint32_t obj_id = 0;

        for (const auto& name_type : typed_parameters)
        {
            const auto parameter_name = name_type.first;
            const auto type_name = name_type.second;
            const auto type_handler = types.find(type_name);

            if (type_handler != types.end())
            {
                const auto parameter_type = type_handler->second;
                const auto parameter = mimir::formalism::create_object(obj_id++, parameter_name, parameter_type);
                action_parameter_list.push_back(parameter);
                action_parameter_map.insert(std::make_pair(parameter_name, parameter));
            }
            else
            {
                throw std::invalid_argument("the type of object \"" + parameter_name + "\" is undefined");
            }
        }

        const auto& [precondition, effect, cond_effect, cost] = body->get_precondition_effect_cost(action_parameter_map, constants, predicates, functions);
        return mimir::formalism::create_action_schema(action_name, action_parameter_list, precondition, effect, cond_effect, cost);
    }

    /* DomainNode */

    DomainNode::DomainNode(NameNode* name,
                           boost::optional<RequirementListNode*> requirements,
                           boost::optional<TypedNameListNode*> types,
                           boost::optional<TypedNameListNode*> constants,
                           boost::optional<PredicateListNode*> predicates,
                           boost::optional<FunctionDeclarationListNode*> functions,
                           std::vector<ActionNode*>& actions) :
        name(name),
        requirements(nullptr),
        types(nullptr),
        constants(nullptr),
        predicates(nullptr),
        functions(nullptr),
        actions(actions)
    {
        if (requirements)
        {
            this->requirements = requirements.value();
        }

        if (types)
        {
            this->types = types.value();
        }

        if (constants)
        {
            this->constants = constants.value();
        }

        if (predicates)
        {
            this->predicates = predicates.value();
        }

        if (functions)
        {
            this->functions = functions.value();
        }
    }

    DomainNode::~DomainNode()
    {
        if (name)
        {
            delete name;
            name = nullptr;
        }

        if (requirements)
        {
            delete requirements;
            requirements = nullptr;
        }

        if (types)
        {
            delete types;
            types = nullptr;
        }

        if (constants)
        {
            delete constants;
            constants = nullptr;
        }

        if (predicates)
        {
            delete predicates;
            predicates = nullptr;
        }

        for (auto node : actions)
        {
            delete node;
        }
        actions.clear();
    }

    std::map<std::string, mimir::formalism::Type> DomainNode::get_types() const
    {
        std::map<std::string, mimir::formalism::Type> result;

        result.insert(std::make_pair("object", mimir::formalism::create_type("object")));

        if (types)
        {
            for (const auto& name_base : types->get_typed_names())
            {
                const auto type_name = name_base.first;
                const auto base_name = name_base.second;

                if (type_name == "object")
                {
                    throw std::invalid_argument("type name \"object\" is reserved");
                }

                // get pointer to base type, or create one if it does not exist

                const auto base_type_handler = result.find(base_name);
                mimir::formalism::Type base_type;

                if (base_type_handler != result.end())
                {
                    base_type = base_type_handler->second;
                }
                else
                {
                    base_type = mimir::formalism::create_type(base_name);
                    result.insert(std::make_pair(base_name, base_type));
                }

                // other types might inherit from this type, and have created one, update it

                const auto name_type_handler = result.find(type_name);

                if (name_type_handler != result.end())
                {
                    // update existing type
                    auto type = name_type_handler->second;
                    type->base = base_type;
                }
                else
                {
                    // create new type
                    auto type = mimir::formalism::create_type(type_name, base_type);
                    result.insert(std::make_pair(type_name, type));
                }
            }
        }

        return result;
    }

    std::map<std::string, mimir::formalism::Object> DomainNode::get_constants(const std::map<std::string, mimir::formalism::Type>& types) const
    {
        std::map<std::string, mimir::formalism::Object> result;

        if (constants)
        {
            uint32_t obj_id = 0;

            for (const auto& name_type : constants->get_typed_names())
            {
                const auto object_name = name_type.first;
                const auto type_name = name_type.second;
                const auto type_handler = types.find(type_name);

                if (type_handler != types.end())
                {
                    const auto type = type_handler->second;
                    const auto object = mimir::formalism::create_object(obj_id++, object_name, type);
                    result.insert(std::make_pair(object_name, object));
                }
                else
                {
                    throw std::invalid_argument("the type of object \"" + object_name + "\" is undefined");
                }
            }
        }

        return result;
    }

    std::map<std::string, mimir::formalism::Predicate> DomainNode::get_predicates(const mimir::formalism::RequirementList& requirements,
                                                                                  const std::map<std::string, mimir::formalism::Type>& types) const
    {
        std::map<std::string, mimir::formalism::Predicate> result;

        if (predicates)
        {
            const auto predicate_list = predicates->get_predicates(requirements, types);

            for (const auto& predicate : predicate_list)
            {
                result.insert(std::make_pair(predicate->name, predicate));
            }
        }

        return result;
    }

    std::map<std::string, mimir::formalism::Predicate> DomainNode::get_functions(const std::map<std::string, mimir::formalism::Type>& types) const
    {
        std::map<std::string, mimir::formalism::Predicate> result;

        if (functions)
        {
            const auto predicate_list = functions->get_functions(types);

            for (const auto& predicate : predicate_list)
            {
                result.emplace(predicate->name, predicate);
            }
        }
        else
        {
            result.emplace("total-cost", mimir::formalism::create_predicate(0, "total-cost", {}));
        }

        return result;
    }

    std::vector<mimir::formalism::ActionSchema> DomainNode::get_action_schemas(const std::map<std::string, mimir::formalism::Type>& types,
                                                                               const std::map<std::string, mimir::formalism::Object>& constants,
                                                                               const std::map<std::string, mimir::formalism::Predicate>& predicates,
                                                                               const std::map<std::string, mimir::formalism::Predicate>& functions) const
    {
        std::vector<mimir::formalism::ActionSchema> action_schemas;

        for (const auto node : this->actions)
        {
            action_schemas.push_back(node->get_action(types, constants, predicates, functions));
        }

        return action_schemas;
    }

    mimir::formalism::DomainDescription DomainNode::get_domain() const
    {
        const auto domain_name = name->get_name();
        const auto domain_requirements = (requirements != nullptr) ? (requirements->get_requirements()) : mimir::formalism::RequirementList();
        const auto domain_types = this->get_types();
        const auto domain_constants = this->get_constants(domain_types);
        const auto domain_predicates = this->get_predicates(domain_requirements, domain_types);
        const auto domain_functions = this->get_functions(domain_types);
        const auto domain_actions = this->get_action_schemas(domain_types, domain_constants, domain_predicates, domain_functions);

        return mimir::formalism::create_domain(domain_name,
                                               domain_requirements,
                                               get_values(domain_types),
                                               get_values(domain_constants),
                                               get_values(domain_predicates),
                                               get_values(domain_functions),
                                               domain_actions);
    }

    /* ProblemHeaderNode */

    ProblemHeaderNode::ProblemHeaderNode(NameNode* problem_name, NameNode* domain_name) : problem_name(problem_name), domain_name(domain_name) {}

    ProblemHeaderNode::~ProblemHeaderNode()
    {
        if (problem_name)
        {
            delete problem_name;
            problem_name = nullptr;
        }

        if (domain_name)
        {
            delete domain_name;
            domain_name = nullptr;
        }
    }

    std::string ProblemHeaderNode::get_problem_name() const { return problem_name->get_name(); }

    std::string ProblemHeaderNode::get_domain_name() const { return domain_name->get_name(); }

    /* ProblemNode */

    std::map<std::string, mimir::formalism::Object> ProblemNode::get_objects(uint32_t num_constants,
                                                                             const std::map<std::string, mimir::formalism::Type>& types) const
    {
        std::map<std::string, mimir::formalism::Object> result;

        if (objects)
        {
            uint32_t obj_id = num_constants;

            for (const auto& name_type : objects->get_typed_names())
            {
                const auto object_name = name_type.first;
                const auto type_name = name_type.second;
                const auto type_handler = types.find(type_name);

                if (type_handler != types.end())
                {
                    const auto type = type_handler->second;
                    const auto object = mimir::formalism::create_object(obj_id++, object_name, type);
                    result.insert(std::make_pair(object_name, object));
                }
                else
                {
                    throw std::invalid_argument("the type of object \"" + object_name + "\" is undefined");
                }
            }
        }

        return result;
    }

    template<typename K, typename V>
    std::vector<V> ProblemNode::get_values(std::map<K, V> map) const
    {
        std::vector<V> values;

        for (auto it = map.begin(); it != map.end(); ++it)
        {
            values.push_back(it->second);
        }

        return values;
    }

    mimir::formalism::AtomList ProblemNode::atoms_of(const mimir::formalism::LiteralList& literals) const
    {
        mimir::formalism::AtomList atoms;

        for (const auto& literal : literals)
        {
            atoms.push_back(literal->atom);
        }

        return atoms;
    }

    ProblemNode::ProblemNode(ProblemHeaderNode* problem_domain_name,
                             boost::optional<TypedNameListNode*> objects,
                             LiteralOrConditionalOrFunctionListNode* initial,
                             LiteralListNode* goal,
                             boost::optional<AtomNode*> metric) :
        problem_domain_name(problem_domain_name),
        objects(objects ? objects.value() : nullptr),
        initial(initial),
        goal(goal),
        metric(metric ? metric.value() : nullptr)
    {
    }

    ProblemNode::~ProblemNode()
    {
        if (problem_domain_name)
        {
            delete problem_domain_name;
            problem_domain_name = nullptr;
        }

        if (objects)
        {
            delete objects;
            objects = nullptr;
        }

        if (initial)
        {
            delete initial;
            initial = nullptr;
        }

        if (goal)
        {
            delete goal;
            goal = nullptr;
        }
    }

    mimir::formalism::ProblemDescription ProblemNode::get_problem(const std::string& filename, const mimir::formalism::DomainDescription& domain) const
    {
        const auto domain_name = problem_domain_name->get_domain_name();

        if (domain->name != domain_name)
        {
            throw std::invalid_argument("domain names do not match: \"" + domain_name + "\" and \"" + domain->name + "\"");
        }

        if (metric && (metric->name->get_name() != "total-cost"))
        {
            throw std::invalid_argument("expected metric to minimize: \"total-cost\"");
        }

        const auto type_map = domain->get_type_map();
        const auto predicate_map = domain->get_predicate_name_map();
        const auto function_map = domain->get_function_name_map();
        const auto constant_map = domain->get_constant_map();

        const auto problem_name = problem_domain_name->get_problem_name();
        const auto object_map = get_objects(constant_map.size(), type_map);

        // Merge constants and objects into a single map

        std::map<std::string, mimir::formalism::Parameter> constants_and_objects_map;

        for (const auto& [name, parameter] : constant_map)
        {
            constants_and_objects_map[name] = parameter;
        }

        for (const auto& [name, parameter] : object_map)
        {
            constants_and_objects_map[name] = parameter;
        }

        std::vector<mimir::formalism::Literal> initial_list;
        std::unordered_map<mimir::formalism::Atom, double> atom_costs;

        for (const auto& node : initial->literal_or_functions)
        {
            if (node->literal_node)
            {
                initial_list.push_back(node->literal_node->get_literal(object_map, constant_map, predicate_map));
            }
            else
            {
                const auto operation = node->function_node->op->get_name();

                if (operation == "=")
                {
                    const auto atom = node->function_node->first_operand_atom->get_atom({}, constants_and_objects_map, function_map);

                    if (node->function_node->second_operand_atom)
                    {
                        throw std::invalid_argument("expected a constant");
                    }
                    else
                    {
                        const auto constant = node->function_node->second_operand_value;
                        atom_costs.emplace(atom, constant);
                    }
                }
                else
                {
                    throw std::invalid_argument("expected operation to be \"=\"");
                }
            }
        }

        if (std::count(domain->requirements.cbegin(), domain->requirements.cend(), ":equality"))
        {
            for (const auto& [_, obj] : object_map)
            {
                const auto equality_predicate = predicate_map.at("=");
                const auto equality_atom = mimir::formalism::create_atom(equality_predicate, { obj, obj });
                const auto equality_literal = mimir::formalism::create_literal(equality_atom, false);
                initial_list.push_back(equality_literal);
            }
        }

        const auto goal_list = goal->get_literals(object_map, constant_map, predicate_map);

        auto objects = get_values(object_map);
        objects.insert(objects.end(), domain->constants.begin(), domain->constants.end());

        return mimir::formalism::create_problem(problem_name + " (" + filename + ")", domain, objects, atoms_of(initial_list), goal_list, atom_costs);
    }
}  // namespace parsers
