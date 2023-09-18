#if !defined(PARSERS_ABSTRACT_SYNTAX_TREE_HPP_)
#define PARSERS_ABSTRACT_SYNTAX_TREE_HPP_

#include "../formalism/action_schema.hpp"
#include "../formalism/domain.hpp"
#include "../formalism/function.hpp"
#include "../formalism/literal.hpp"
#include "../formalism/object.hpp"
#include "../formalism/predicate.hpp"
#include "../formalism/problem.hpp"
#include "../formalism/type.hpp"

#include <boost/fusion/container/vector.hpp>
#include <boost/optional.hpp>
#include <map>
#include <string>
#include <vector>

namespace parsers
{
    class ASTNode
    {
      public:
        virtual ~ASTNode();
    };

    class CharacterNode : public ASTNode
    {
      public:
        char character;

        CharacterNode(char character);
    };

    class NameNode : public ASTNode
    {
      public:
        char prefix;
        std::vector<CharacterNode*> character_nodes;

        NameNode(char prefix, const std::vector<CharacterNode*>& character_nodes);
        ~NameNode() override;
        std::string get_name() const;
    };

    class VariableNode : public ASTNode
    {
      public:
        NameNode* name_node;

        VariableNode(NameNode* name_node);
        ~VariableNode() override;
        std::string get_variable() const;
    };

    class TypeNode : public ASTNode
    {
      public:
        std::string name_string;
        NameNode* name_node;

        TypeNode(std::string type_name);
        TypeNode(NameNode* type_name);
        ~TypeNode() override;
        std::string get_type() const;
    };

    class TermNode : public ASTNode
    {
      public:
        NameNode* name_node;
        VariableNode* variable_node;

        TermNode(NameNode* node);
        TermNode(VariableNode* node);
        ~TermNode() override;
        formalism::Object get_term(const std::map<std::string, formalism::Parameter>& parameters,
                                   const std::map<std::string, formalism::Object>& constants) const;
    };

    class AtomNode : public ASTNode
    {
      public:
        NameNode* name;
        std::vector<TermNode*> arguments;

        AtomNode(NameNode* name, std::vector<TermNode*>& arguments);
        ~AtomNode() override;
        formalism::Atom get_atom(const std::map<std::string, formalism::Parameter>& parameters,
                                 const std::map<std::string, formalism::Object>& constants,
                                 const std::map<std::string, formalism::Predicate>& predicates) const;
    };

    class TypedNameListNode : public ASTNode
    {
      public:
        std::vector<NameNode*> untyped_names;
        std::vector<NameNode*> typed_names;
        TypeNode* type;
        TypedNameListNode* recursion;

        TypedNameListNode(std::vector<NameNode*>& untyped_names);
        TypedNameListNode(std::vector<NameNode*>& typed_names, TypeNode* type, TypedNameListNode* recursion);
        ~TypedNameListNode() override;
        std::vector<std::pair<std::string, std::string>> get_typed_names() const;
    };

    class TypedVariableListNode : public ASTNode
    {
      public:
        std::vector<VariableNode*> untyped_names;
        std::vector<VariableNode*> typed_names;
        TypeNode* type;
        TypedVariableListNode* recursion;

        TypedVariableListNode(std::vector<VariableNode*>& untyped_names);
        TypedVariableListNode(std::vector<VariableNode*>& typed_names, TypeNode* type, TypedVariableListNode* recursion);
        ~TypedVariableListNode() override;
        std::vector<std::pair<std::string, std::string>> get_typed_variables() const;
    };

    class RequirementNode : public ASTNode
    {
      public:
        formalism::Requirement requirement;

        RequirementNode(formalism::Requirement requirement);
        formalism::Requirement get_requirement() const;
    };

    class RequirementListNode : public ASTNode
    {
      public:
        std::vector<RequirementNode*> requirements;

        RequirementListNode(std::vector<RequirementNode*>& requirements);
        ~RequirementListNode() override;
        formalism::RequirementList get_requirements() const;
    };

    class PredicateNode : public ASTNode
    {
      public:
        NameNode* name;
        TypedVariableListNode* parameters;

        PredicateNode(NameNode* name, TypedVariableListNode* parameters);
        ~PredicateNode() override;
        formalism::Predicate get_predicate(const uint32_t id, const std::map<std::string, formalism::Type>& types) const;
    };

    class PredicateListNode : public ASTNode
    {
      public:
        std::vector<PredicateNode*> predicates;

        PredicateListNode(std::vector<PredicateNode*>& predicates);
        ~PredicateListNode() override;
        formalism::PredicateList get_predicates(const formalism::RequirementList& requirements, const std::map<std::string, formalism::Type>& types) const;
    };

    class FunctionDeclarationNode : public ASTNode
    {
      public:
        PredicateNode* predicate;
        NameNode* type;

        FunctionDeclarationNode(PredicateNode* predicate, NameNode* type);
        ~FunctionDeclarationNode() override;
        formalism::Predicate get_function(const uint32_t id, const std::map<std::string, formalism::Type>& types) const;
    };

    class FunctionDeclarationListNode : public ASTNode
    {
      public:
        std::vector<FunctionDeclarationNode*> functions;

        FunctionDeclarationListNode(std::vector<FunctionDeclarationNode*>& functions);
        ~FunctionDeclarationListNode() override;
        formalism::PredicateList get_functions(const std::map<std::string, formalism::Type>& types) const;
    };

    class FunctionNode : public ASTNode
    {
      public:
        NameNode* op;
        AtomNode* first_operand_atom;
        AtomNode* second_operand_atom;
        double second_operand_value;

        FunctionNode(NameNode* op, AtomNode* first_operand, AtomNode* second_operand);
        FunctionNode(NameNode* op, AtomNode* first_operand, double second_operand);
        ~FunctionNode() override;
    };

    class LiteralNode : public ASTNode
    {
      public:
        bool negated;
        AtomNode* atom;

        LiteralNode(bool negated, AtomNode* atom);
        ~LiteralNode() override;
        formalism::Literal get_literal(const std::map<std::string, formalism::Parameter>& parameters,
                                       const std::map<std::string, formalism::Object>& constants,
                                       const std::map<std::string, formalism::Predicate>& predicates) const;
    };

    class LiteralListNode : public ASTNode
    {
      public:
        std::vector<LiteralNode*> literals;

        LiteralListNode(LiteralNode* literal);
        LiteralListNode(std::vector<LiteralNode*>& literals);
        ~LiteralListNode() override;
        formalism::LiteralList get_literals(const std::map<std::string, formalism::Parameter>& parameters,
                                            const std::map<std::string, formalism::Object>& constants,
                                            const std::map<std::string, formalism::Predicate>& predicates) const;
    };

    class ConditionalNode : public ASTNode
    {
      public:
        LiteralListNode* antecedent;
        LiteralListNode* consequence;

        ConditionalNode(LiteralListNode* antecedent, LiteralListNode* consequence);
        ~ConditionalNode() override;
    };

    class LiteralOrConditionalOrFunctionNode : public ASTNode
    {
      public:
        LiteralNode* literal_node;
        FunctionNode* function_node;
        ConditionalNode* conditional_node;

        LiteralOrConditionalOrFunctionNode(LiteralNode* literal_node);
        LiteralOrConditionalOrFunctionNode(FunctionNode* function_node);
        LiteralOrConditionalOrFunctionNode(ConditionalNode* conditional_node);
        ~LiteralOrConditionalOrFunctionNode() override;
    };

    class LiteralOrConditionalOrFunctionListNode : public ASTNode
    {
      public:
        std::vector<LiteralOrConditionalOrFunctionNode*> literal_or_functions;

        LiteralOrConditionalOrFunctionListNode(LiteralOrConditionalOrFunctionNode* literal_or_function);
        LiteralOrConditionalOrFunctionListNode(std::vector<LiteralOrConditionalOrFunctionNode*>& literal_or_functions);
        ~LiteralOrConditionalOrFunctionListNode() override;
        formalism::LiteralList get_literals(const std::map<std::string, formalism::Parameter>& parameters,
                                            const std::map<std::string, formalism::Object>& constants,
                                            const std::map<std::string, formalism::Predicate>& predicates) const;
        formalism::ImplicationList get_conditionals(const std::map<std::string, formalism::Parameter>& parameters,
                                                    const std::map<std::string, formalism::Object>& constants,
                                                    const std::map<std::string, formalism::Predicate>& predicates) const;
        formalism::FunctionList get_functions(const std::map<std::string, formalism::Parameter>& parameters,
                                              const std::map<std::string, formalism::Object>& constants,
                                              const std::map<std::string, formalism::Predicate>& predicates) const;
    };

    class ActionBodyNode : public ASTNode
    {
      public:
        LiteralListNode* precondition;
        LiteralOrConditionalOrFunctionListNode* effect;

        ActionBodyNode(boost::optional<boost::fusion::vector<std::string, LiteralListNode*>>& precondition,
                       boost::optional<boost::fusion::vector<std::string, LiteralOrConditionalOrFunctionListNode*>>& effect);
        ActionBodyNode(LiteralListNode* precondition, LiteralOrConditionalOrFunctionListNode* effect);
        ~ActionBodyNode() override;
        std::tuple<formalism::LiteralList, formalism::LiteralList, formalism::ImplicationList, formalism::Function>
        get_precondition_effect_cost(const std::map<std::string, formalism::Parameter>& parameters,
                                     const std::map<std::string, formalism::Object>& constants,
                                     const std::map<std::string, formalism::Predicate>& predicates,
                                     const std::map<std::string, formalism::Predicate>& functions) const;
    };

    class ActionNode : public ASTNode
    {
      public:
        NameNode* name;
        TypedVariableListNode* parameters;
        ActionBodyNode* body;

        ActionNode(NameNode* name, TypedVariableListNode* parameters, ActionBodyNode* body);
        ~ActionNode() override;
        formalism::ActionSchema get_action(const std::map<std::string, formalism::Type>& types,
                                           const std::map<std::string, formalism::Object>& constants,
                                           const std::map<std::string, formalism::Predicate>& predicates,
                                           const std::map<std::string, formalism::Predicate>& functions) const;
    };

    class DomainNode : public ASTNode
    {
      public:
        NameNode* name;
        RequirementListNode* requirements;
        TypedNameListNode* types;
        TypedNameListNode* constants;
        PredicateListNode* predicates;
        FunctionDeclarationListNode* functions;
        std::vector<ActionNode*> actions;

        DomainNode(NameNode* name,
                   boost::optional<RequirementListNode*> requirements,
                   boost::optional<TypedNameListNode*> types,
                   boost::optional<TypedNameListNode*> constants,
                   boost::optional<PredicateListNode*> predicates,
                   boost::optional<FunctionDeclarationListNode*> functions,
                   std::vector<ActionNode*>& actions);

        ~DomainNode() override;

      private:
        std::map<std::string, formalism::Type> get_types() const;
        std::map<std::string, formalism::Object> get_constants(const std::map<std::string, formalism::Type>& types) const;
        std::map<std::string, formalism::Predicate> get_predicates(const formalism::RequirementList& requirements,
                                                                   const std::map<std::string, formalism::Type>& types) const;
        std::map<std::string, formalism::Predicate> get_functions(const std::map<std::string, formalism::Type>& types) const;
        std::vector<formalism::ActionSchema> get_action_schemas(const std::map<std::string, formalism::Type>& types,
                                                                const std::map<std::string, formalism::Object>& constants,
                                                                const std::map<std::string, formalism::Predicate>& predicates,
                                                                const std::map<std::string, formalism::Predicate>& functions) const;
        template<typename K, typename V>
        inline std::vector<V> get_values(std::map<K, V> map) const
        {
            std::vector<V> values;

            for (auto it = map.begin(); it != map.end(); ++it)
            {
                values.push_back(it->second);
            }

            return values;
        }

      public:
        formalism::DomainDescription get_domain() const;
    };

    class ProblemHeaderNode : public ASTNode
    {
      public:
        NameNode* problem_name;
        NameNode* domain_name;

        ProblemHeaderNode(NameNode* problem_name, NameNode* domain_name);
        ~ProblemHeaderNode() override;
        std::string get_problem_name() const;
        std::string get_domain_name() const;
    };

    class ProblemNode : public ASTNode
    {
        ProblemHeaderNode* problem_domain_name;
        TypedNameListNode* objects;
        LiteralOrConditionalOrFunctionListNode* initial;
        LiteralListNode* goal;
        AtomNode* metric;

        std::map<std::string, formalism::Object> get_objects(uint32_t num_constants, const std::map<std::string, formalism::Type>& types) const;
        template<typename K, typename V>
        std::vector<V> get_values(std::map<K, V> map) const;
        formalism::AtomList atoms_of(const formalism::LiteralList& literals) const;

      public:
        ProblemNode(ProblemHeaderNode* problem_domain_name,
                    boost::optional<TypedNameListNode*> objects,
                    LiteralOrConditionalOrFunctionListNode* initial,
                    LiteralListNode* goal,
                    boost::optional<AtomNode*> metric);

        ~ProblemNode() override;
        formalism::ProblemDescription get_problem(const std::string& filename, const formalism::DomainDescription& domain) const;
    };
}  // namespace parsers

#endif  // PARSERS_ABSTRACT_SYNTAX_TREE_HPP_
