namespace Mimir.Pddl.Ast.Models;

public record Parameter(string Name, string TypeName = "object") : INode
{
    private string _name = PddlName.RequireVariable(Name, nameof(Name));
    private string _typeName = PddlName.RequireName(TypeName, nameof(TypeName));

    public string Name
    {
        get => _name;
        init => _name = PddlName.RequireVariable(value, nameof(Name));
    }

    public string TypeName
    {
        get => _typeName;
        init => _typeName = PddlName.RequireName(value, nameof(TypeName));
    }

    public string ToPddlString() => TypeName == "object" ? Name : $"{Name} - {TypeName}";

    public override string ToString() => ToPddlString();
}
